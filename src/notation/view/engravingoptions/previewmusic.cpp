/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "previewmusic.h"

#include <algorithm>
#include <deque>
#include <iterator>
#include <map>
#include <optional>
#include <set>
#include <utility>
#include <vector>

#include <QRegularExpression>
#include <QStringList>
#include <QXmlStreamWriter>

#include "engraving/types/fraction.h"

using namespace mu::notation;
using mu::engraving::Fraction;

namespace {
//---------------------------------------------------------
//   The text, in tokens
//---------------------------------------------------------

struct SyntaxError {
    QString message;
    int position = 0;
};

//! NOTE Exceptions stay inside this translation unit: `toMscx` is the only way out and it catches `SyntaxError`
//! (capella.cpp is the precedent). Every `.at()` below is on an index which is checked first, so nothing else can
//! throw
[[noreturn]] void fail(const QString& message, int position)
{
    throw SyntaxError { message, position };
}

struct Token {
    QString text;
    int position = 0;
};

//! NOTE The end of a word of notes: white space, a brace or a bar line end it, but not inside a chord or a text
int endOfNoteWord(const QString& text, int start)
{
    int i = start;
    while (i < text.size()) {
        const QChar c = text.at(i);
        if (c.isSpace() || c == '{' || c == '}' || c == '|') {
            break;
        }

        if (c == '"' || c == '<') {
            const int close = text.indexOf(c == '"' ? '"' : '>', i + 1);
            if (close < 0) {
                fail(c == '"' ? "this text has no end" : "this chord has no '>'", i);
            }
            i = close + 1;
        } else {
            ++i;
        }
    }

    return i;
}

//! NOTE A command is a backslash and letters, which a `!` may end (`\turn!`), or `\<`, `\>`, `\!` and `\\`
int endOfCommand(const QString& text, int start)
{
    int i = start + 1;
    if (i < text.size() && QStringLiteral("<>!\\").contains(text.at(i))) {
        return i + 1;
    }

    while (i < text.size() && text.at(i).isLetter()) {
        ++i;
    }

    if (i > start + 1 && i < text.size() && text.at(i) == '!') {
        ++i;
    }

    return i;
}

std::vector<Token> tokenize(const QString& text)
{
    std::vector<Token> tokens;

    int i = 0;
    while (i < text.size()) {
        if (text.at(i).isSpace()) {
            ++i;
            continue;
        }

        int end = i + 1;
        if (text.mid(i, 2) == "<<" || text.mid(i, 2) == ">>") {
            end = i + 2;
        } else if (text.at(i) == '\\') {
            end = endOfCommand(text, i);
        } else if (!QStringLiteral("{}|").contains(text.at(i))) {
            end = endOfNoteWord(text, i);
        }

        tokens.push_back({ text.mid(i, end - i), i });
        i = end;
    }

    return tokens;
}

//---------------------------------------------------------
//   The music, as events
//---------------------------------------------------------

struct Duration {
    int denominator = 4;    // 1, 2, 4, …, 1024
    int dots = 0;

    Fraction length() const
    {
        const int dotted = (1 << (dots + 1)) - 1;
        return Fraction(dotted, denominator * (1 << dots));
    }

    QString typeName() const
    {
        static const std::map<int, QString> NAMES {
            { 1, "whole" }, { 2, "half" }, { 4, "quarter" }, { 8, "eighth" }, { 16, "16th" }, { 32, "32nd" },
            { 64, "64th" }, { 128, "128th" }, { 256, "256th" }, { 512, "512th" }, { 1024, "1024th" }
        };
        return NAMES.at(denominator);
    }
};

struct Pitch {
    //! How the accidental of the note is shown, if the notation says so: `!` shows it although it is not needed,
    //! `?` shows it in parentheses
    enum class Accidental { Auto, Shown, Parenthesised, Braced };

    int midi = 60;
    int tpc = 14;
    int alteration = 0;     // in semitones
    Accidental accidental = Accidental::Auto;
    //! `<c'~ e'>`: this note of the chord is tied to the same note of the next chord, the others are not
    bool tie = false;
};

//! A spanner which begins or ends at an event, in the terms of the file: how far the other end is
struct Link {
    enum class Kind { Slur, Tie, Hairpin };

    Kind kind = Kind::Slur;
    int midi = 0;           // of the note, for a tie
    int subtype = 0;        // of a hairpin: 0 crescendo, 1 decrescendo
    int slurDirection = 0;
    bool slurFlat = false;
    int measures = 0;
    Fraction fractions;
};

struct Place {
    int measure = 0;
    Fraction position;
};

constexpr int BUZZ_ROLL = -1;

//! `\\trill` `\\turn` `\\mordent` `\\prall`, written as an Ornament
struct OrnamentMark {
    QString symbol;
    //! `!` after the word: the accidentals of the notes it plays are shown, also those which the key has
    bool showsAllAccidentals = false;
};

//! `\\fall` `\\doit` `\\plop` `\\scoop`, curved, or with `Straight` or `Wavy` after the word
struct ChordLineMark {
    int type = 0;           // ChordLineType
    bool straight = false;
    bool wavy = false;
};

struct Event {
    enum class Grace { None, Grace, Acciaccatura, Appoggiatura };
    enum class Beam { Auto, Begin, Mid, No };
    enum class Stem { Auto, Up, Down };
    enum class Hairpin { None, Crescendo, Decrescendo };

    int textPosition = 0;

    bool isRest = false;
    bool isMeasureRest = false;     // `R`: a rest as long as the measure, of which the duration says the length
    std::vector<Pitch> pitches;
    Duration duration;
    Grace grace = Grace::None;
    Beam beam = Beam::Auto;
    Stem stem = Stem::Auto;

    bool tie = false;
    int tieStyle = 0;               // SlurStyleType: solid, dotted, dashed, wide dashed
    bool slashHeads = false;        // `\\slash`: the noteheads are slashes (rhythm slash notation)
    bool small = false;             // `\\small`: a small (cue) chord or rest
    int slurStyle = 0;
    int slurDirection = 0;          // DirectionV: 0 auto, 1 up, 2 down
    bool slurFlat = false;          // CurveMode::FlatMiddle
    bool laissezVibrer = false;
    int slurStarts = 0;
    int slurStops = 0;
    Hairpin hairpin = Hairpin::None;
    bool hairpinEnd = false;

    QStringList articulations;      // the subtypes in the file
    int fingering = 0;
    int tremolo = 0;                // 8, 16, 32, or BUZZ_ROLL
    int tremoloBetweenChords = 0;   // 8, 16, 32: this chord and the next one are joined by a tremolo
    bool ambitus = false;           // an ambitus stands before this event
    int staff = 0;                  // where the event is written
    int staffMove = 0;              // -1 and 1: shown on the staff above and below
    QString lyric;
    bool lyricContinues = false;
    //! `^"text"`: a staff text above the chord; `<sup>` `<sub>` `<b>` `<i>` in it format the letters between them
    QString staffText;
    bool fermata = false;
    int arpeggio = -1;              // ArpeggioType, or -1
    std::vector<OrnamentMark> ornaments;
    std::optional<ChordLineMark> chordLine;
    QString breath;                 // the symbol of the breath mark or caesura after the chord, or none
    QString handBracket;            // the symbol of the keyboard hand bracket at the lowest note, or none
    int tapping = 0;                // 0 none, 1 left hand, 2 right hand
    QString dynamic;

    int tupletActual = 0;
    int tupletNormal = 0;
    bool tupletStart = false;
    bool tupletEnd = false;

    std::optional<QString> clef;
    std::optional<int> key;
    std::optional<std::pair<int, int> > time;

    //! What the <Beam> element written before this chord carries: the y its ends have, in staff
    //! spaces — a Fragment, which reads back as a beam the user moved — and/or the move which says
    //! which staff a cross-staff beam counts as belonging to
    std::optional<std::pair<double, double> > beamAnchor;
    std::optional<int> beamMove;

    Place place;
    std::vector<Link> startingLinks;
    std::vector<Link> endingLinks;

    Fraction length() const
    {
        if (grace != Grace::None) {
            return Fraction();
        }

        const Fraction written = duration.length();
        return tupletActual > 0 ? written * Fraction(tupletNormal, tupletActual) : written;
    }
};

using Voice = std::deque<Event>;

struct Measure {
    std::vector<Voice> voices = std::vector<Voice>(1);
    bool lineBreak = false;     // the system ends after the measure
    bool pageBreak = false;     // the page ends after the measure
    bool startRepeat = false;   // a repeat begins with the measure
    int endRepeat = 0;          // a repeat ends with the measure, played this many times; 0: none
    QString barline;            // the type of its barline (as BarLineType is written), if `\\bar` gives one
    bool barlineSaid = false;   // `\\bar` stands at its end, so a `|` after it adds nothing
    QString voltaText;          // a volta begins with the measure, with this text,
    int voltaMeasures = 0;      // and spans this many measures; 0: no volta begins here
    int voltaPosition = 0;      // where `\\volta` stands in the text
    int repeatGroup = 0;        // the measure is one of a measure repeat of this many measures (1, 2 or 4),
    int repeatIndex = 0;        // the how-manieth of them, from 1,
    Fraction repeatedLength;    // and as long as the measure it repeats
    QStringList markers;        // `\\segno` `\\coda` `\\fine` `\\toCoda` of the measure, by their command names
    QString jump;               // the text of a jump at its end

    static Fraction lengthOf(const Voice& voice)
    {
        Fraction length;
        for (const Event& event : voice) {
            length += event.length();
        }

        return length;
    }

    bool isEmpty() const
    {
        if (repeatGroup > 0) {
            return false;
        }

        for (const Voice& voice : voices) {
            if (!voice.empty()) {
                return false;
            }
        }

        return true;
    }

    Fraction length() const { return repeatGroup > 0 ? repeatedLength : lengthOf(voices.front()); }
};

using Music = std::deque<Measure>;

//! The measures, and how many staves they are written on
struct Piece {
    Music measures;
    int staves = 1;
    //! the bracket over the staves, as the BracketType enum: brace by default (none for two instruments), -1 for none
    int bracket = 1;
    std::set<int> tabStaves;        // 0-based indices of the staves which are tablature
    std::map<int, int> staffLines;  // the number of lines of a staff which has not five, by its 0-based index
    std::set<int> smallStaves;      // 0-based indices of the small staves
    bool spanBarlines = false;      // the barlines of the first staff go down through the second
    bool hideStavesIndividually = false;    // an empty staff is hidden although the other staff has notes
    QString longName;               // of the instrument, on the first system
    QString shortName;              // on the other systems
    bool instruments = false;       // the two staves are two instruments of one staff each, not a grand staff
    QString title;                  // of the score, in a frame above the music; empty: no frame
};

//---------------------------------------------------------
//   Reading the tokens
//---------------------------------------------------------

//! The pitch of a note word: a letter, accidentals, octave marks
class PitchReader
{
public:
    PitchReader(const QString& word, int start, int wordPosition)
        : m_word(word), m_index(start), m_wordPosition(wordPosition) {}

    Pitch read()
    {
        static const std::map<QChar, std::pair<int, int> > LETTERS {   // semitones from C, tpc of the natural
            { 'c', { 0, 14 } }, { 'd', { 2, 16 } }, { 'e', { 4, 18 } }, { 'f', { 5, 13 } },
            { 'g', { 7, 15 } }, { 'a', { 9, 17 } }, { 'b', { 11, 19 } }
        };

        const QChar letter = m_word.at(m_index);
        auto found = LETTERS.find(letter);
        if (found == LETTERS.end()) {
            fail(QString("%1 is not a pitch").arg(letter), m_wordPosition + m_index);
        }
        ++m_index;

        const int alteration = readAlteration(letter);
        const int octave = readOctave();

        Pitch pitch;
        pitch.accidental = readAccidentalMark();
        pitch.alteration = alteration;
        pitch.midi = 48 + 12 * octave + found->second.first + alteration;
        pitch.tpc = found->second.second + 7 * alteration;
        return pitch;
    }

    int index() const { return m_index; }

private:
    //! `is` raises, `es` lowers; `as` and `es` alone are a-flat and e-flat
    int readAlteration(QChar letter)
    {
        int alteration = 0;
        while (true) {
            if (m_word.mid(m_index, 2) == "is") {
                ++alteration;
                m_index += 2;
            } else if (m_word.mid(m_index, 2) == "es") {
                --alteration;
                m_index += 2;
            } else if (alteration == 0 && (letter == 'a' || letter == 'e') && m_word.mid(m_index, 1) == "s") {
                --alteration;
                ++m_index;
            } else {
                return alteration;
            }
        }
    }

    //! `!` after the octave shows the accidental although it is not needed, `?` shows it in parentheses
    Pitch::Accidental readAccidentalMark()
    {
        const QChar mark = m_index < m_word.size() ? m_word.at(m_index) : QChar();
        if (mark != '!' && mark != '?' && mark != '*') {
            return Pitch::Accidental::Auto;
        }

        ++m_index;
        if (mark == '!') {
            return Pitch::Accidental::Shown;
        }

        return mark == '?' ? Pitch::Accidental::Parenthesised : Pitch::Accidental::Braced;
    }

    int readOctave()
    {
        int octave = 0;
        while (m_index < m_word.size() && (m_word.at(m_index) == '\'' || m_word.at(m_index) == ',')) {
            octave += m_word.at(m_index) == '\'' ? 1 : -1;
            ++m_index;
        }

        return octave;
    }

    const QString& m_word;
    int m_index = 0;
    int m_wordPosition = 0;
};

class MusicParser
{
public:
    explicit MusicParser(const QString& text)
        : m_tokens(tokenize(text))
    {
        m_music.emplace_back();
    }

    Piece parse()
    {
        while (m_next < m_tokens.size()) {
            handle(takeToken());
        }

        if (m_beamBegin) {
            fail("this beam is not closed with ']'", *m_beamBegin);
        }

        if (m_music.back().isEmpty()) {
            m_music.pop_back();
        }

        if (m_music.empty()) {
            fail("there is no music", 0);
        }

        for (size_t i = 0; i < m_music.size(); ++i) {
            if (i + m_music[i].voltaMeasures > m_music.size()) {
                fail("the volta is longer than the music after it", m_music[i].voltaPosition);
            }
        }

        return { std::move(m_music), m_staves, m_bracket.value_or(m_instruments ? -1 : 1), std::move(m_tabStaves),
                 std::move(m_staffLines), std::move(m_smallStaves), m_spanBarlines, m_hideStavesIndividually,
                 m_longName, m_shortName, m_instruments, m_title };
    }

private:
    Token takeToken()
    {
        return m_tokens.at(m_next++);
    }

    Token takeToken(const char* expected, const Token& after)
    {
        if (m_next >= m_tokens.size()) {
            fail(QString("%1 is missing after %2").arg(expected, after.text), after.position + after.text.size());
        }

        return takeToken();
    }

    Measure& measure() { return m_music.back(); }
    Voice& voice() { return measure().voices.at(m_voice); }

    void handle(const Token& token)
    {
        const QString& text = token.text;

        if (text == "|") {
            endMeasure(token);
        } else if (text == "<<") {
            readVoices(token);
        } else if (text.startsWith('\\') && text != "\\\\") {
            handleCommand(token);
        } else if (text == "{" || text == "}" || text == ">>" || text == "\\\\") {
            fail(QString("%1 is not expected here").arg(text), token.position);
        } else {
            readNote(token);
        }
    }

    void endMeasure(const Token& token)
    {
        if (m_noBarlines) {
            fail(m_noBarlines, token.position);
        }

        if (measure().isEmpty()) {
            //! NOTE `\break` ends its measure too: a bar line after it is said already (`c'4 \break |`)
            const Measure* before = m_music.size() >= 2 ? &m_music[m_music.size() - 2] : nullptr;
            if (before && (before->lineBreak || before->pageBreak || before->barlineSaid)) {
                return;
            }
            fail("this measure is empty", token.position);
        }

        m_music.emplace_back();
        m_isMeasureSealed = false;
    }

    //! `<< { … } \\ { … } >>`: the voices of one whole measure
    void readVoices(const Token& open)
    {
        if (!measure().isEmpty()) {
            fail("voices are the whole of a measure", open.position);
        }

        m_noBarlines = "a measure cannot end inside of << >>";

        for (m_voice = 0;; ++m_voice) {
            const Token brace = takeToken("{", open);
            if (brace.text != "{") {
                fail("a voice is written in { }", brace.position);
            }

            if (m_voice >= static_cast<int>(measure().voices.size())) {
                measure().voices.emplace_back();
            }

            m_staff = 0;
            m_crossStaff = 0;
            readUntil("}", brace);

            const Token next = takeToken("\\\\ or >>", brace);
            if (next.text == ">>") {
                break;
            }

            if (next.text != "\\\\") {
                fail("voices are separated by \\\\ and end with >>", next.position);
            }
        }

        for (const Voice& other : measure().voices) {
            if (Measure::lengthOf(other) != measure().length()) {
                fail("the voices of a measure must be equally long", open.position);
            }
        }

        m_voice = 0;
        m_noBarlines = nullptr;
        m_isMeasureSealed = true;
    }

    void readUntil(const QString& closing, const Token& opening)
    {
        while (true) {
            if (m_next >= m_tokens.size()) {
                fail(QString("%1 is not closed with %2").arg(opening.text, closing), opening.position);
            }

            const Token token = takeToken();
            if (token.text == closing) {
                return;
            }

            handle(token);
        }
    }

    //-----------------------------------------------------
    //   Commands
    //-----------------------------------------------------

    void handleCommand(const Token& token)
    {
        static const std::map<QString, Event::Stem> STEMS {
            { "\\stemUp", Event::Stem::Up }, { "\\stemDown", Event::Stem::Down }, { "\\stemNeutral", Event::Stem::Auto }
        };
        static const std::map<QString, Event::Grace> GRACES {
            { "\\grace", Event::Grace::Grace }, { "\\acciaccatura", Event::Grace::Acciaccatura },
            { "\\appoggiatura", Event::Grace::Appoggiatura }
        };
        static const std::map<QString, int> ARPEGGIOS {    // ArpeggioType
            { "\\arpeggio", 0 }, { "\\arpeggioUp", 1 }, { "\\arpeggioDown", 2 }, { "\\arpeggioBracket", 3 },
            { "\\arpeggioUpStraight", 4 }, { "\\arpeggioDownStraight", 5 }
        };
        static const QStringList DYNAMICS { "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff", "sf", "sfz", "fp" };
        //! NOTE Signs which are articulations in the file, but not in the narrow sense: their position has a style
        //! value of its own
        static const std::map<QString, QString> PLAYING_MARKS {
            { "\\upbow", "stringsUpBow" }, { "\\downbow", "stringsDownBow" }
        };
        static const std::map<QString, QString> BREATHS {
            { "\\breathe", "breathMarkComma" }, { "\\caesura", "caesura" }
        };

        const QString& text = token.text;

        if (STEMS.count(text)) {
            m_stem = STEMS.at(text);
        } else if (GRACES.count(text)) {
            readGrace(GRACES.at(text), token);
        } else if (text == "\\slurSolid" || text == "\\slurDotted" || text == "\\slurDashed"
                   || text == "\\slurWideDashed") {
            m_slurStyle = lineStyleOf(text.mid(5));
        } else if (text == "\\slurUp" || text == "\\slurDown" || text == "\\slurAuto") {
            m_slurDirection = text == "\\slurAuto" ? 0 : (text == "\\slurUp" ? 1 : 2);
        } else if (text == "\\slurFlat") {
            m_slurFlat = true;
        } else if (text == "\\tapping") {
            readTapping(token);
        } else if (text == "\\tab") {
            if (m_music.size() > 1 || !measure().isEmpty()) {
                fail("\\tab is written at the beginning", token.position);
            }
            m_tabStaves.insert(m_staff);
        } else if (text == "\\lines") {
            readLines(token);
        } else if (text == "\\name") {
            readName(token);
        } else if (text == "\\tieSolid" || text == "\\tieDotted" || text == "\\tieDashed"
                   || text == "\\tieWideDashed") {
            m_tieStyle = lineStyleOf(text.mid(4));
        } else if (text == "\\laissezVibrer") {
            lastEvent(token).laissezVibrer = true;
        } else if (text == "\\tremolo") {
            readTremoloBetweenChords(token);
        } else if (text == "\\ambitus") {
            m_ambitus = true;
        } else if (text == "\\staves") {
            readStaves(token);
        } else if (text == "\\instruments") {
            readInstruments(token);
        } else if (text == "\\title") {
            readTitle(token);
        } else if (text == "\\group") {
            readGroup(token);
        } else if (text == "\\staff") {
            readStaff(token);
        } else if (text == "\\crossStaff") {
            readCrossStaff(token);
        } else if (text == "\\beamAt") {
            readBeamAt(token);
        } else if (text == "\\beamMove") {
            readBeamMove(token);
        } else if (text == "\\noBeam") {
            m_noBeam = true;
        } else if (text == "\\slash") {
            m_slashHeads = true;
        } else if (text == "\\small") {
            m_small = true;
        } else if (text == "\\hideStavesIndividually") {
            if (m_music.size() > 1 || !measure().isEmpty()) {
                fail("\\hideStavesIndividually is written at the beginning", token.position);
            }
            m_hideStavesIndividually = true;
        } else if (text == "\\spanBarlines") {
            if (m_music.size() > 1 || !measure().isEmpty()) {
                fail("\\spanBarlines is written at the beginning", token.position);
            }
            m_spanBarlines = true;
        } else if (text == "\\smallStaff") {
            if (m_music.size() > 1 || !measure().isEmpty()) {
                fail("\\smallStaff is written at the beginning", token.position);
            }
            m_smallStaves.insert(m_staff);
        } else if (text == "\\break" || text == "\\pageBreak") {
            //! NOTE A break ends the measure it stands in, like a bar line, and the measure ends the system (the
            //! page). After `|` it takes back the bar line instead: `e''4 \break f''4` and `e''4 | \break f''4` are
            //! the same
            const bool page = text == "\\pageBreak";
            if (measure().isEmpty()) {
                if (m_music.size() < 2) {
                    fail("there is no measure before this break", token.position);
                }
                Measure& before = m_music[m_music.size() - 2];
                (page ? before.pageBreak : before.lineBreak) = true;
            } else {
                (page ? measure().pageBreak : measure().lineBreak) = true;
                endMeasure(token);
            }
        } else if (text == "\\bar") {
            readBar(token);
        } else if (text == "\\playCount") {
            readPlayCount(token);
        } else if (text == "\\volta") {
            readVolta(token);
        } else if (text == "\\measureRepeat") {
            readMeasureRepeat(token);
        } else if (text == "\\segno" || text == "\\coda" || text == "\\fine" || text == "\\toCoda") {
            measure().markers << text.mid(1);
        } else if (text == "\\jump") {
            measure().jump = quoted(takeToken("the text of the jump in quotes", token));
        } else if (text == "\\tuplet") {
            readTuplet(token);
        } else if (text == "\\clef") {
            readClef(token);
        } else if (text == "\\key") {
            readKey(token);
        } else if (text == "\\time") {
            readTime(token);
        } else if (text == "\\fermata") {
            lastEvent(token).fermata = true;
        } else if (const std::optional<OrnamentMark> ornament = ornamentOf(text)) {
            lastEvent(token).ornaments.push_back(*ornament);
        } else if (const std::optional<ChordLineMark> chordLine = chordLineOf(text)) {
            lastEvent(token).chordLine = chordLine;
        } else if (PLAYING_MARKS.count(text)) {
            lastEvent(token).articulations << PLAYING_MARKS.at(text);
        } else if (text == "\\luteFinger") {
            readLuteFinger(token);
        } else if (BREATHS.count(text)) {
            lastEvent(token).breath = BREATHS.at(text);
        } else if (text == "\\handBracket") {
            readHandBracket(token);
        } else if (ARPEGGIOS.count(text)) {
            lastEvent(token).arpeggio = ARPEGGIOS.at(text);
        } else if (text == "\\<" || text == "\\>") {
            lastEvent(token).hairpin = text == "\\<" ? Event::Hairpin::Crescendo : Event::Hairpin::Decrescendo;
        } else if (text == "\\!") {
            lastEvent(token).hairpinEnd = true;
        } else if (DYNAMICS.contains(text.mid(1))) {
            lastEvent(token).dynamic = text.mid(1);
        } else {
            fail(QString("%1 is not known").arg(text), token.position);
        }
    }

    //! The note which what follows a note (a dynamic, a fermata, …) belongs to
    Event& lastEvent(const Token& token)
    {
        if (!m_last) {
            fail(QString("%1 needs a note before it").arg(token.text), token.position);
        }

        return *m_last;
    }

    int readNumber(const QString& text, int position)
    {
        bool ok = false;
        const int number = text.toInt(&ok);
        if (!ok) {
            fail(QString("%1 is not a number").arg(text), position);
        }

        return number;
    }

    //! The line of a slur or tie: what follows `\\slur` or `\\tie` in the command
    static int lineStyleOf(const QString& name)
    {
        static const std::map<QString, int> STYLES { { "Solid", 0 }, { "Dotted", 1 }, { "Dashed", 2 },
                                                     { "WideDashed", 3 } };
        return STYLES.at(name);
    }

    //! `\\tapping` and `\\tapping left`: the note before it is tapped (a `T` mark, with a half slur)
    void readTapping(const Token& command)
    {
        int hand = 2;
        if (m_next < m_tokens.size() && (m_tokens.at(m_next).text == "left" || m_tokens.at(m_next).text == "right")) {
            hand = m_tokens.at(m_next).text == "left" ? 1 : 2;
            ++m_next;
        }
        lastEvent(command).tapping = hand;
    }

    //! `\\trill` `\\turn` `\\mordent` `\\prall`, also with `!`: the ornament the word asks for, if it is one
    static std::optional<OrnamentMark> ornamentOf(const QString& word)
    {
        static const std::map<QString, QString> ORNAMENTS {
            { "\\trill", "ornamentTrill" }, { "\\turn", "ornamentTurn" }, { "\\mordent", "ornamentMordent" },
            { "\\prall", "ornamentShortTrill" }
        };

        const bool showsAllAccidentals = word.endsWith('!');
        const auto ornament = ORNAMENTS.find(showsAllAccidentals ? word.chopped(1) : word);
        if (ornament == ORNAMENTS.end()) {
            return std::nullopt;
        }

        return OrnamentMark { ornament->second, showsAllAccidentals };
    }

    //! `\\fall` `\\doit` `\\plop` `\\scoop`, also with `Straight` or `Wavy`: the line the word asks for, if it
    //! is one
    static std::optional<ChordLineMark> chordLineOf(const QString& word)
    {
        static const std::map<QString, int> TYPES {    // ChordLineType
            { "fall", 1 }, { "doit", 2 }, { "plop", 3 }, { "scoop", 4 }
        };

        for (const auto& [name, type] : TYPES) {
            const QString command = "\\" + name;
            if (word == command) {
                return ChordLineMark { type, false, false };
            }
            if (word == command + "Straight") {
                return ChordLineMark { type, true, false };
            }
            if (word == command + "Wavy") {
                return ChordLineMark { type, false, true };
            }
        }

        return std::nullopt;
    }

    //! `\\luteFinger thumb|1|2|3`: the right-hand finger of lute music, a stroke (the thumb) or dots
    void readLuteFinger(const Token& command)
    {
        static const std::map<QString, QString> FINGERS {
            { "thumb", "luteFingeringRHThumb" }, { "1", "luteFingeringRHFirst" }, { "2", "luteFingeringRHSecond" },
            { "3", "luteFingeringRHThird" }
        };

        Event& event = lastEvent(command);
        const Token finger = takeToken("the finger (thumb, 1, 2 or 3)", command);
        if (!FINGERS.count(finger.text)) {
            fail("the finger is thumb, 1, 2 or 3", finger.position);
        }
        event.articulations << FINGERS.at(finger.text);
    }

    //! `\\handBracket left|right`: the bracket which says with which hand the chord is played, at its lowest note
    void readHandBracket(const Token& command)
    {
        Event& event = lastEvent(command);
        const Token hand = takeToken("the hand (left or right)", command);
        if (hand.text != "left" && hand.text != "right") {
            fail("the hand is left or right", hand.position);
        }
        event.handBracket = hand.text == "left" ? "keyboardPlayWithLH" : "keyboardPlayWithRH";
    }

    //! `\\grace`, `\\acciaccatura` and `\\appoggiatura` make the next chord a grace note of the one after it, or, with
    //! `{ … }`, all the chords inside; those are beamed together
    void readGrace(Event::Grace grace, const Token& command)
    {
        if (m_next >= m_tokens.size() || m_tokens.at(m_next).text != "{") {
            m_grace = grace;
            return;
        }

        const Token brace = takeToken();
        m_grace = grace;
        m_graceGroupFirst = true;
        m_inGraceGroup = true;
        readUntil("}", brace);
        m_inGraceGroup = false;
        m_grace = Event::Grace::None;

        if (m_graceGroupFirst) {
            fail("there is no note in this group of grace notes", command.position);
        }
    }

    //! `\\tremolo 16 { c'2 e'2 }`: two chords joined by a tremolo of that many strokes' worth of beams
    void readTremoloBetweenChords(const Token& command)
    {
        const Token number = takeToken("a number of the tremolo (8, 16, 32)", command);
        const int strokes = readNumber(number.text, number.position);
        if (strokes != 8 && strokes != 16 && strokes != 32) {
            fail("a tremolo is 8, 16 or 32", number.position);
        }

        const Token brace = takeToken("{", number);
        if (brace.text != "{") {
            fail("the chords of a tremolo are written in { }", brace.position);
        }

        const size_t before = voice().size();
        readUntil("}", brace);

        if (voice().size() != before + 2 || voice().back().isRest || voice().at(before).isRest) {
            fail("a tremolo joins two chords", brace.position);
        }

        voice().at(before).tremoloBetweenChords = strokes;
    }

    void readStaves(const Token& command)
    {
        const Token number = takeToken("a number of staves (1 or 2)", command);
        if (m_instruments) {
            fail("\\staves and \\instruments are not written together", command.position);
        }

        m_staves = readNumber(number.text, number.position);
        if (m_staves < 1 || m_staves > 2) {
            fail("one or two staves", number.position);
        }

        if (m_music.size() > 1 || !measure().isEmpty()) {
            fail("\\staves is written at the beginning", command.position);
        }
    }

    //! `\\bar "type"`: the barline where it stands — at the end of the measure being written, which it ends as a `|`
    //! does, or after a `|` at the end of the measure before. The types are LilyPond's: `||` `|.` `.|` `!` (dashed)
    //! `;` (dotted) `.` (heavy) `..` (double heavy), and the repeats `.|:` `:|.` `:|.|:`
    void readBar(const Token& command)
    {
        const Token type = takeToken("a barline type in quotes", command);
        const QString text = quoted(type);
        static const std::map<QString, QString> SUBTYPES {
            { "||", "double" }, { "|.", "end" }, { ".|", "reverse-end" }, { "!", "dashed" }, { ";", "dotted" },
            { ".", "heavy" }, { "..", "double-heavy" },
        };
        const bool startsRepeat = text == ".|:" || text == ":|.|:";
        const bool endsRepeat = text == ":|." || text == ":|.|:";
        if (!startsRepeat && !endsRepeat && !SUBTYPES.count(text)) {
            fail("a barline type: || |. .| ! ; . .. .|: :|. :|.|:", type.position);
        }

        const bool endsThisMeasure = !measure().isEmpty();
        if (!endsThisMeasure && m_music.size() < 2 && !(startsRepeat && !endsRepeat)) {
            fail("there is no measure before this barline", command.position);
        }

        //! NOTE The measure which ends at the barline, if there is one before it
        m_lastEndRepeat.reset();
        if (endsThisMeasure || m_music.size() >= 2) {
            const size_t index = m_music.size() - (endsThisMeasure ? 1 : 2);
            Measure& before = m_music[index];
            before.barlineSaid = true;
            if (endsRepeat) {
                before.endRepeat = 2;
                m_lastEndRepeat = index;
            }
            if (SUBTYPES.count(text)) {
                before.barline = SUBTYPES.at(text);
            }
        }

        if (endsThisMeasure) {
            endMeasure(command);
        }
        if (startsRepeat) {
            measure().startRepeat = true;
        }
    }

    //! `\\volta "text" N`: a volta with the text over the N measures which begin where it stands (at the beginning
    //! of a measure)
    void readVolta(const Token& command)
    {
        const QString text = quoted(takeToken("the text of the volta in quotes", command));
        const Token number = takeToken("how many measures the volta spans", command);
        const int measures = readNumber(number.text, number.position);
        if (measures < 1 || measures > 16) {
            fail("1 to 16 measures", number.position);
        }

        if (!measure().isEmpty()) {
            fail("a volta begins with a measure: \\volta is written after a `|`", command.position);
        }

        measure().voltaText = text;
        measure().voltaMeasures = measures;
        measure().voltaPosition = command.position;
    }

    //! `\\measureRepeat N`: N measures (1, 2 or 4) which repeat the N measures before them, written where a measure
    //! begins; the music goes on after them
    void readMeasureRepeat(const Token& command)
    {
        const Token number = takeToken("how many measures are repeated (1, 2 or 4)", command);
        const int count = readNumber(number.text, number.position);
        if (count != 1 && count != 2 && count != 4) {
            fail("1, 2 or 4 measures", number.position);
        }

        if (!measure().isEmpty()) {
            fail("a measure repeat begins with a measure: \\measureRepeat is written after a `|`", command.position);
        }

        if (m_music.size() - 1 < static_cast<size_t>(count)) {
            fail("there are not as many measures before it to repeat", command.position);
        }

        for (int i = 0; i < count; ++i) {
            Measure& repeat = measure();
            repeat.repeatGroup = count;
            repeat.repeatIndex = i + 1;
            repeat.repeatedLength = m_music[m_music.size() - 1 - count].length();
            m_music.emplace_back();
        }

        m_music[m_music.size() - 2].barlineSaid = true;
        m_isMeasureSealed = false;
    }

    //! `\\playCount N`: the repeat which ends at the `\\bar` before it is played N times
    void readPlayCount(const Token& command)
    {
        const Token number = takeToken("how many times the repeat is played", command);
        if (!m_lastEndRepeat) {
            fail("\\playCount follows the \\bar which ends a repeat", command.position);
        }

        const int count = readNumber(number.text, number.position);
        if (count < 2 || count > 99) {
            fail("2 to 99 times", number.position);
        }

        m_music[*m_lastEndRepeat].endRepeat = count;
        m_lastEndRepeat.reset();
    }

    //! `\\instruments 2`: two instruments of one staff each, where `\\staves 2` is one instrument on two staves
    void readInstruments(const Token& command)
    {
        const Token number = takeToken("a number of instruments (2)", command);
        if (m_staves > 1) {
            fail("\\staves and \\instruments are not written together", command.position);
        }

        if (readNumber(number.text, number.position) != 2) {
            fail("two instruments", number.position);
        }

        if (m_music.size() > 1 || !measure().isEmpty()) {
            fail("\\instruments is written at the beginning", command.position);
        }

        m_staves = 2;
        m_instruments = true;
    }

    //! `\\title "text"`: the title of the score, in a frame above the music (written at the beginning)
    void readTitle(const Token& command)
    {
        if (m_music.size() > 1 || !measure().isEmpty()) {
            fail("\\title is written at the beginning", command.position);
        }

        m_title = quoted(takeToken("the title in quotes", command));
    }

    //! `\\lines N`: the current staff has N lines instead of five (written at the beginning, like `\\tab`)
    void readLines(const Token& command)
    {
        const Token number = takeToken("a number of staff lines (1 to 5)", command);
        const int lines = readNumber(number.text, number.position);
        if (lines < 1 || lines > 5) {
            fail("1 to 5 staff lines", number.position);
        }

        if (m_music.size() > 1 || !measure().isEmpty()) {
            fail("\\lines is written at the beginning", command.position);
        }

        m_staffLines[m_staff] = lines;
    }

    //! `\\name "long" "short"`: the names of the instrument, on the first system and on the others (written at the
    //! beginning)
    void readName(const Token& command)
    {
        if (m_music.size() > 1 || !measure().isEmpty()) {
            fail("\\name is written at the beginning", command.position);
        }

        m_longName = quoted(takeToken("the long name in quotes", command));
        m_shortName = quoted(takeToken("the short name in quotes", command));
    }

    //! The text of a name or title, which is written in quotes
    static QString quoted(const Token& token)
    {
        const QString& word = token.text;
        if (word.size() < 2 || !word.startsWith('"') || !word.endsWith('"')) {
            fail("this is written in quotes", token.position);
        }

        return word.mid(1, word.size() - 2);
    }

    //! `\\group bracket|square|line|brace|none`: the kind of the bracket which the two staves
    //! of `\\staves 2` get (a grand staff has a brace; written at the beginning too)
    void readGroup(const Token& command)
    {
        const Token kind = takeToken("bracket, square, line, brace or none", command);
        static const std::map<QString, int> KINDS {
            { "bracket", 0 },
            { "brace", 1 },
            { "square", 2 },
            { "line", 3 },
            { "none", -1 },
        };
        if (!KINDS.count(kind.text)) {
            fail("bracket, square, line, brace or none", kind.position);
        }
        m_bracket = KINDS.at(kind.text);

        if (m_music.size() > 1 || !measure().isEmpty()) {
            fail("\\group is written at the beginning", command.position);
        }
    }

    //! `\\staff 2`: what follows is written on the second staff, until `\\staff 1`
    void readStaff(const Token& command)
    {
        const Token number = takeToken("1 or 2", command);
        const int staff = readNumber(number.text, number.position);
        if (staff < 1 || staff > m_staves) {
            fail(QString("there is no staff %1").arg(staff), number.position);
        }

        m_staff = staff - 1;
    }

    //! `\\crossStaff down|up|off`: the chords which follow are shown on the staff below or above the one
    //! they are written on
    void readCrossStaff(const Token& command)
    {
        const Token where = takeToken("down, up or off", command);
        if (where.text == "down") {
            m_crossStaff = 1;
        } else if (where.text == "up") {
            m_crossStaff = -1;
        } else if (where.text == "off") {
            m_crossStaff = 0;
        } else {
            fail(QString("%1 is not down, up or off").arg(where.text), where.position);
        }

        if (m_crossStaff != 0 && m_staves < 2) {
            fail("\\crossStaff needs \\staves 2", command.position);
        }
    }

    //! `\\beamAt y1 y2`: where the ends of the beam which the next chord begins lie, in staff
    //! spaces — written as the position a user would have dragged it to, which reads back as
    //! a user-modified beam
    void readBeamAt(const Token& command)
    {
        const Token y1 = takeToken("the y of the beam's ends, in staff spaces", command);
        const Token y2 = takeToken("the y of the beam's ends, in staff spaces", command);
        bool ok1 = false, ok2 = false;
        m_beamAt = { y1.text.toDouble(&ok1), y2.text.toDouble(&ok2) };
        if (!ok1 || !ok2) {
            fail("two numbers, in staff spaces", command.position);
        }
    }

    //! `\\beamMove n`: which staff the beam which the next chord begins counts as belonging to,
    //! as the move of a cross-staff beam (a cross-staff beam whose chords are all on one side
    //! of it sits above or below the staves only when it counts as the staff on that side)
    void readBeamMove(const Token& command)
    {
        const Token move = takeToken("a number (-1, 0 or 1)", command);
        m_beamMove = readNumber(move.text, move.position);
        if (*m_beamMove < -1 || *m_beamMove > 1) {
            fail("-1, 0 or 1", move.position);
        }
    }

    void readTuplet(const Token& command)
    {
        if (m_tuplet) {
            fail("a tuplet cannot be in a tuplet", command.position);
        }

        const Token ratio = takeToken("a ratio like 3/2", command);
        const QStringList parts = ratio.text.split('/');
        if (parts.size() != 2) {
            fail(QString("%1 is not a ratio like 3/2").arg(ratio.text), ratio.position);
        }

        const std::pair<int, int> numbers { readNumber(parts.at(0), ratio.position),
                                            readNumber(parts.at(1), ratio.position) };
        if (numbers.first < 2 || numbers.second < 1) {
            fail("a tuplet of less than two notes", ratio.position);
        }

        const Token brace = takeToken("{", ratio);
        if (brace.text != "{") {
            fail("a tuplet is written in { }", brace.position);
        }

        m_tuplet = numbers;
        m_tupletIsOpen = true;
        m_noBarlines = "a tuplet cannot be split over measures";
        readUntil("}", brace);

        if (m_tupletIsOpen) {
            fail("there is nothing in this tuplet", brace.position);
        }

        m_noBarlines = nullptr;
        m_tuplet.reset();
        lastEvent(brace).tupletEnd = true;
    }

    void readClef(const Token& command)
    {
        static const QStringList CLEFS { "treble", "bass", "alto", "tenor" };

        const Token name = takeToken("a clef", command);
        if (!CLEFS.contains(name.text)) {
            fail(QString("%1 is not a clef (treble, bass, alto, tenor)").arg(name.text), name.position);
        }

        m_clef = name.text;
    }

    void readKey(const Token& command)
    {
        const Token number = takeToken("a number of sharps", command);
        m_key = readNumber(number.text, number.position);
        if (std::abs(*m_key) > 7) {
            fail("a key has seven sharps or flats at most", number.position);
        }
    }

    void readTime(const Token& command)
    {
        const Token signature = takeToken("a time signature like 3/4", command);
        const QStringList parts = signature.text.split('/');
        if (parts.size() != 2) {
            fail(QString("%1 is not a time signature like 3/4").arg(signature.text), signature.position);
        }

        m_time = { readNumber(parts.at(0), signature.position), readNumber(parts.at(1), signature.position) };
    }

    //-----------------------------------------------------
    //   Notes
    //-----------------------------------------------------

    void readNote(const Token& token)
    {
        Event event;
        event.textPosition = token.position;

        int index = readPitches(token, event);
        index = readDuration(token, index, event);
        readMarks(token, index, event);

        add(std::move(event));
    }

    //! Returns where the word goes on after the pitches
    int readPitches(const Token& token, Event& event)
    {
        const QString& word = token.text;

        if (word.startsWith('r') || word.startsWith('R')) {
            event.isRest = true;
            event.isMeasureRest = word.startsWith('R');
            return 1;
        }

        if (word.startsWith('<')) {
            const int close = word.indexOf('>');
            const QStringList pitches = word.mid(1, close - 1).split(' ', Qt::SkipEmptyParts);
            if (pitches.isEmpty()) {
                fail("there is no note in this chord", token.position);
            }

            int offset = 1;
            for (const QString& written : pitches) {
                offset = word.indexOf(written, offset);
                const bool tied = written.endsWith('~');
                const QString pitch = tied ? written.chopped(1) : written;
                PitchReader reader(word, offset, token.position);
                event.pitches.push_back(reader.read());
                event.pitches.back().tie = tied;
                if (reader.index() != offset + pitch.size()) {
                    fail(QString("%1 is not a pitch").arg(pitch), token.position + offset);
                }
                offset += written.size();
            }

            return close + 1;
        }

        PitchReader reader(word, 0, token.position);
        event.pitches.push_back(reader.read());
        return reader.index();
    }

    //! Sets the duration of the event, which is the one before when the word has none.
    //! Returns where the word goes on after it
    int readDuration(const Token& token, int index, Event& event)
    {
        const QString& word = token.text;

        int digits = 0;
        while (index + digits < word.size() && word.at(index + digits).isDigit()) {
            ++digits;
        }

        if (digits > 0) {
            const int denominator = word.mid(index, digits).toInt();
            if (denominator < 1 || denominator > 1024 || (denominator & (denominator - 1)) != 0) {
                fail(QString("%1 is not a duration (1, 2, 4, …, 1024)").arg(denominator), token.position + index);
            }

            m_duration = { denominator, 0 };
            index += digits;

            while (index < word.size() && word.at(index) == '.') {
                ++m_duration.dots;
                ++index;
            }
        }

        event.duration = m_duration;
        return index;
    }

    //! Slurs, ties, beams, articulations, fingering, tremolo and lyrics, which follow the duration in any order
    void readMarks(const Token& token, int index, Event& event)
    {
        static const std::map<QChar, QString> ARTICULATIONS {
            { '.', "articStaccatoAbove" }, { '>', "articAccentAbove" }, { '-', "articTenutoAbove" },
            { '^', "articMarcatoAbove" }, { '!', "articStaccatissimoAbove" }
        };

        const QString& word = token.text;

        while (index < word.size()) {
            const QChar c = word.at(index);
            const int here = token.position + index;

            if (c == '~') {
                event.tie = true;
            } else if (c == '(') {
                ++event.slurStarts;
            } else if (c == ')') {
                ++event.slurStops;
            } else if (c == '[') {
                beginBeam(here);
                event.beam = Event::Beam::Begin;
                m_beamHasBegun = true;
            } else if (c == ']') {
                if (!m_beamBegin) {
                    fail("this ']' has no '['", here);
                }
                m_beamEnds = true;
            } else if (c == '-' && index + 1 < word.size()) {
                ++index;
                const QChar mark = word.at(index);
                if (ARTICULATIONS.count(mark)) {
                    event.articulations << ARTICULATIONS.at(mark);
                } else if (mark >= '1' && mark <= '5') {
                    event.fingering = mark.digitValue();
                } else {
                    fail("what follows '-' is not known (. > - ^ ! or a finger 1 to 5)", here + 1);
                }
            } else if (c == ':') {
                index = readTremolo(token, index, event);
                continue;
            } else if (c == '_') {
                index = readLyric(token, index, event);
                continue;
            } else if (c == '^') {
                index = readStaffText(token, index, event);
                continue;
            } else {
                fail(QString("'%1' is not known after a duration").arg(c), here);
            }

            ++index;
        }
    }

    int readTremolo(const Token& token, int index, Event& event)
    {
        const QString& word = token.text;
        if (index + 1 < word.size() && word.at(index + 1) == 'z') {
            event.tremolo = BUZZ_ROLL;
            return index + 2;
        }

        int end = index + 1;
        while (end < word.size() && word.at(end).isDigit()) {
            ++end;
        }

        const int strokes = word.mid(index + 1, end - index - 1).toInt();
        if (strokes != 8 && strokes != 16 && strokes != 32) {
            fail("a tremolo is :8, :16, :32 or :z", token.position + index);
        }

        event.tremolo = strokes;
        return end;
    }

    int readLyric(const Token& token, int index, Event& event)
    {
        const QString& word = token.text;
        if (word.mid(index + 1, 1) != "\"") {
            fail("a lyric is _\"text\"", token.position + index);
        }

        const int close = word.indexOf('"', index + 2);
        QString syllable = word.mid(index + 2, close - index - 2);
        event.lyricContinues = syllable.endsWith('-');
        if (event.lyricContinues) {
            syllable.chop(1);
        }

        event.lyric = syllable;
        return close + 1;
    }

    int readStaffText(const Token& token, int index, Event& event)
    {
        const QString& word = token.text;
        if (word.mid(index + 1, 1) != "\"") {
            fail("a staff text is ^\"text\"", token.position + index);
        }

        const int close = word.indexOf('"', index + 2);
        event.staffText = word.mid(index + 2, close - index - 2);
        if (event.staffText.isEmpty()) {
            fail("this staff text is empty", token.position + index);
        }

        static const QRegularExpression TAG("<(/?)([a-z]+)>");
        QRegularExpressionMatchIterator tags = TAG.globalMatch(event.staffText);
        while (tags.hasNext()) {
            const QRegularExpressionMatch tag = tags.next();
            if (!QStringList { "sup", "sub", "b", "i" }.contains(tag.captured(2))) {
                fail(QString("<%1> is not a format of a staff text (sup, sub, b, i)").arg(tag.captured(2)),
                     token.position + index + 2 + tag.capturedStart());
            }
        }

        return close + 1;
    }

    void beginBeam(int position)
    {
        if (m_beamBegin) {
            fail("a beam is open already", position);
        }

        m_beamBegin = position;
    }

    //-----------------------------------------------------
    //   An event takes what was said before it
    //-----------------------------------------------------

    void add(Event event)
    {
        if (m_isMeasureSealed) {
            fail("a measure with voices has nothing after them", event.textPosition);
        }

        if (event.isRest && !event.lyric.isEmpty()) {
            fail("a rest has no lyrics", event.textPosition);
        }

        takePending(event);
        event.slashHeads = std::exchange(m_slashHeads, false);
        event.small = std::exchange(m_small, false);
        takeBeam(event);
        takeTuplet(event);

        if ((event.beamAnchor || event.beamMove) && event.beam != Event::Beam::Begin) {
            fail("\\beamAt and \\beamMove stand before the first note of a beam group", event.textPosition);
        }

        voice().push_back(std::move(event));
        m_last = &voice().back();
    }

    void takePending(Event& event)
    {
        event.stem = m_stem;
        event.grace = m_grace;
        event.slurStyle = m_slurStyle;
        event.slurDirection = m_slurDirection;
        event.slurFlat = m_slurFlat;
        event.tieStyle = m_tieStyle;
        event.staff = m_staff;
        event.staffMove = m_crossStaff;
        event.ambitus = std::exchange(m_ambitus, false);

        if (!m_inGraceGroup) {
            m_grace = Event::Grace::None;
        }

        event.clef = std::exchange(m_clef, std::nullopt);
        event.key = std::exchange(m_key, std::nullopt);
        event.time = std::exchange(m_time, std::nullopt);
        event.beamAnchor = std::exchange(m_beamAt, std::nullopt);
        event.beamMove = std::exchange(m_beamMove, std::nullopt);
    }

    //! A beam begins at the chord with `[`, goes on through the ones after it, and ends with the chord with `]`
    void takeBeam(Event& event)
    {
        if (m_inGraceGroup) {
            event.beam = std::exchange(m_graceGroupFirst, false) ? Event::Beam::Begin : Event::Beam::Mid;
            return;
        }

        if (std::exchange(m_noBeam, false)) {
            event.beam = Event::Beam::No;
        } else if (m_beamBegin && !std::exchange(m_beamHasBegun, false)) {
            event.beam = Event::Beam::Mid;
        }

        if (std::exchange(m_beamEnds, false)) {
            m_beamBegin.reset();
        }
    }

    void takeTuplet(Event& event)
    {
        if (!m_tuplet) {
            return;
        }

        event.tupletActual = m_tuplet->first;
        event.tupletNormal = m_tuplet->second;
        event.tupletStart = std::exchange(m_tupletIsOpen, false);
    }

    std::vector<Token> m_tokens;
    size_t m_next = 0;

    Music m_music;
    int m_voice = 0;
    Event* m_last = nullptr;
    bool m_isMeasureSealed = false;
    const char* m_noBarlines = nullptr;     // why a bar line is not allowed here, if it is not

    int m_staves = 1;
    bool m_instruments = false;
    std::optional<int> m_bracket;   // as given by `\\group`
    std::optional<size_t> m_lastEndRepeat;  // the measure whose repeat the last `\\bar` ended, for `\\playCount`
    int m_staff = 0;
    int m_crossStaff = 0;
    int m_slurStyle = 0;
    int m_slurDirection = 0;
    bool m_slurFlat = false;
    int m_tieStyle = 0;
    std::set<int> m_tabStaves;
    std::map<int, int> m_staffLines;
    QString m_longName;
    QString m_shortName;
    QString m_title;
    bool m_ambitus = false;
    bool m_inGraceGroup = false;
    bool m_graceGroupFirst = false;

    Duration m_duration;
    Event::Stem m_stem = Event::Stem::Auto;
    Event::Grace m_grace = Event::Grace::None;
    bool m_noBeam = false;
    bool m_slashHeads = false;
    bool m_small = false;
    std::set<int> m_smallStaves;
    bool m_spanBarlines = false;
    bool m_hideStavesIndividually = false;
    std::optional<int> m_beamBegin;         // the place of the `[` of the beam which is open
    bool m_beamHasBegun = false;            // the `[` is on the chord which is being read
    bool m_beamEnds = false;
    std::optional<std::pair<double, double> > m_beamAt;
    std::optional<int> m_beamMove;
    std::optional<std::pair<int, int> > m_tuplet;
    bool m_tupletIsOpen = false;            // no event of the tuplet yet

    std::optional<QString> m_clef;
    std::optional<int> m_key;
    std::optional<std::pair<int, int> > m_time;
};

//---------------------------------------------------------
//   Where spanners go
//---------------------------------------------------------

//! Gives each event its place and finds the other end of each slur, tie and hairpin
class SpannerPlanner
{
public:
    explicit SpannerPlanner(Piece& piece)
        : m_music(piece.measures) {}

    void run()
    {
        for (size_t v = 0; v < maxVoiceCount(); ++v) {
            std::vector<Event*> events = locate(v);
            planSlurs(events);
            planTies(events);
            planHairpins(events);
        }
    }

private:
    size_t maxVoiceCount() const
    {
        size_t count = 1;
        for (const Measure& measure : m_music) {
            count = std::max(count, measure.voices.size());
        }

        return count;
    }

    //! The events of a voice through the score
    std::vector<Event*> locate(size_t voiceIndex)
    {
        std::vector<Event*> events;
        for (size_t m = 0; m < m_music.size(); ++m) {
            if (voiceIndex >= m_music[m].voices.size()) {
                continue;
            }

            Fraction position;
            for (Event& event : m_music[m].voices[voiceIndex]) {
                event.place = { static_cast<int>(m), position };
                position += event.length();
                events.push_back(&event);
            }
        }

        return events;
    }

    static void connect(Event& from, Event& to, Link link)
    {
        if (from.staff != to.staff) {
            fail("a slur, a tie and a hairpin stay on one staff", from.textPosition);
        }

        link.measures = to.place.measure - from.place.measure;
        link.fractions = to.place.position - from.place.position;
        from.startingLinks.push_back(link);

        link.measures = -link.measures;
        link.fractions = -link.fractions;
        to.endingLinks.push_back(link);
    }

    void planSlurs(const std::vector<Event*>& events)
    {
        std::vector<Event*> open;
        for (Event* event : events) {
            for (int i = 0; i < event->slurStops; ++i) {
                if (open.empty()) {
                    fail("this ')' has no '('", event->textPosition);
                }

                Link link { Link::Kind::Slur };
                link.subtype = open.back()->slurStyle;
                link.slurDirection = open.back()->slurDirection;
                link.slurFlat = open.back()->slurFlat;
                connect(*open.back(), *event, link);
                open.pop_back();
            }

            open.insert(open.end(), event->slurStarts, event);
        }

        if (!open.empty()) {
            fail("this slur is not closed with ')'", open.back()->textPosition);
        }
    }

    void planTies(const std::vector<Event*>& events)
    {
        for (size_t i = 0; i < events.size(); ++i) {
            Event& from = *events[i];
            const bool someNoteTied = std::any_of(from.pitches.begin(), from.pitches.end(), [](const Pitch& pitch) {
                return pitch.tie;
            });
            if (!from.tie && !someNoteTied) {
                continue;
            }

            Event* to = nextChord(events, i);
            if (!to) {
                fail("this tie has no chord to go to", from.textPosition);
            }

            for (const Pitch& pitch : from.pitches) {
                if (!from.tie && !pitch.tie) {
                    continue;
                }

                const bool isThere = std::any_of(to->pitches.begin(), to->pitches.end(), [&pitch](const Pitch& other) {
                    return other.midi == pitch.midi;
                });
                if (!isThere) {
                    fail("a note of this tie is not in the chord it goes to", from.textPosition);
                }

                Link link { Link::Kind::Tie };
                link.midi = pitch.midi;
                link.subtype = from.tieStyle;
                connect(from, *to, link);
            }
        }
    }

    static Event* nextChord(const std::vector<Event*>& events, size_t from)
    {
        for (size_t i = from + 1; i < events.size(); ++i) {
            if (events[i]->grace == Event::Grace::None) {
                return events[i]->isRest ? nullptr : events[i];
            }
        }

        return nullptr;
    }

    void planHairpins(const std::vector<Event*>& events)
    {
        for (size_t i = 0; i < events.size(); ++i) {
            Event& from = *events[i];
            if (from.hairpin == Event::Hairpin::None) {
                continue;
            }

            const auto end = std::find_if(events.begin() + i + 1, events.end(), [](const Event* event) {
                return event->hairpinEnd;
            });
            if (end == events.end()) {
                fail("this hairpin has no '\\!'", from.textPosition);
            }

            Link link { Link::Kind::Hairpin };
            link.subtype = from.hairpin == Event::Hairpin::Decrescendo ? 1 : 0;
            connect(from, **end, link);
        }
    }

    Music& m_music;
};

//---------------------------------------------------------
//   The score file
//---------------------------------------------------------

class MscxWriter
{
public:
    explicit MscxWriter(const Piece& piece)
        : m_piece(piece), m_xml(&m_bytes)
    {
        m_xml.setAutoFormatting(true);
        m_xml.setAutoFormattingIndent(1);
    }

    QByteArray write()
    {
        m_xml.writeStartDocument();
        m_xml.writeStartElement("museScore");
        m_xml.writeAttribute("version", "4.60");
        m_xml.writeStartElement("Score");

        writeScoreProperties();
        writeParts();
        for (int staff = 0; staff < m_piece.staves; ++staff) {
            writeStaff(staff);
        }

        m_xml.writeEndDocument();
        return m_bytes;
    }

private:
    void text(const QString& tag, const QString& value) { m_xml.writeTextElement(tag, value); }
    void number(const QString& tag, int value) { text(tag, QString::number(value)); }
    void number(const QString& tag, double value) { text(tag, QString::number(value)); }
    void fraction(const QString& tag, const Fraction& value)
    {
        const Fraction reduced = value.reduced();
        text(tag, QString("%1/%2").arg(reduced.numerator()).arg(reduced.denominator()));
    }

    void writeScoreProperties()
    {
        number("Division", 480);
        for (const char* tag : { "showInvisible", "showUnprintable", "showFrames", "showMargins" }) {
            number(tag, 0);
        }
        if (!m_piece.title.isEmpty()) {
            //! NOTE: for the variables of headers and footers
            m_xml.writeStartElement("metaTag");
            m_xml.writeAttribute("name", "workTitle");
            m_xml.writeCharacters(m_piece.title);
            m_xml.writeEndElement();
        }
    }

    //! The instrument on all the staves, or (`\\instruments 2`) an instrument on each staff
    void writeParts()
    {
        if (!m_piece.instruments) {
            writePart(0, m_piece.staves);
            return;
        }

        for (int staff = 0; staff < m_piece.staves; ++staff) {
            writePart(staff, 1);
        }
    }

    void writePart(int firstStaff, int staves)
    {
        m_xml.writeStartElement("Part");
        m_xml.writeAttribute("id", QString::number(firstStaff + 1));

        for (int staff = firstStaff; staff < firstStaff + staves; ++staff) {
            m_xml.writeStartElement("Staff");
            m_xml.writeAttribute("id", QString::number(staff + 1));
            m_xml.writeStartElement("StaffType");
            if (m_piece.tabStaves.count(staff)) {
                m_xml.writeAttribute("group", "tablature");
                text("name", "tab6StrSimple");
            } else {
                m_xml.writeAttribute("group", "pitched");
                text("name", "stdNormal");
            }
            if (m_piece.staffLines.count(staff)) {
                number("lines", m_piece.staffLines.at(staff));
            }
            if (m_piece.smallStaves.count(staff)) {
                number("small", 1);
            }
            m_xml.writeEndElement();
            if (staff == 0 && m_piece.staves > 1 && m_piece.spanBarlines) {
                number("barLineSpan", 1);
            }
            if (staff == 1 && !m_piece.instruments && !m_piece.tabStaves.count(staff)) {
                text("defaultClef", "F");
            }
            //! NOTE: the staves' bracket, as TWrite writes it (twrite.cpp): type, span, col, visible
            if (staff == 0 && m_piece.staves > 1 && m_piece.bracket >= 0) {
                m_xml.writeEmptyElement("bracket");
                m_xml.writeAttribute("type", QString::number(m_piece.bracket));
                m_xml.writeAttribute("span", QString::number(m_piece.staves));
                m_xml.writeAttribute("col", "0");
                m_xml.writeAttribute("visible", "1");
            }
            m_xml.writeEndElement();
        }

        text("trackName", "Piano");
        if (m_piece.hideStavesIndividually) {
            number("hideStavesWhenIndividuallyEmpty", 1);
        }
        m_xml.writeStartElement("Instrument");
        m_xml.writeAttribute("id", "piano");
        if (firstStaff == 0 && !m_piece.longName.isEmpty()) {
            text("longName", m_piece.longName);
            text("shortName", m_piece.shortName);
        }
        text("trackName", "Piano");
        m_xml.writeStartElement("Channel");
        m_xml.writeEmptyElement("program");
        m_xml.writeAttribute("value", "0");
        m_xml.writeEndElement();
        m_xml.writeEndElement();

        m_xml.writeEndElement();
    }

    void writeStaff(int staff)
    {
        m_xml.writeStartElement("Staff");
        m_xml.writeAttribute("id", QString::number(staff + 1));

        //! NOTE: a frame of the size of its text (VBox::m_isAutoSizeEnabled)
        if (staff == 0 && !m_piece.title.isEmpty()) {
            m_xml.writeStartElement("VBox");
            m_xml.writeStartElement("Text");
            text("style", "title");
            text("text", m_piece.title);
            m_xml.writeEndElement();
            m_xml.writeEndElement();
        }

        for (size_t i = 0; i < m_piece.measures.size(); ++i) {
            writeMeasure(i, staff);
        }

        m_xml.writeEndElement();
    }

    void writeMeasure(size_t index, int staff)
    {
        const Measure& measure = m_piece.measures[index];
        m_xml.writeStartElement("Measure");
        const Fraction length = writtenLength(measure);
        m_xml.writeAttribute("len", QString("%1/%2").arg(length.numerator()).arg(length.denominator()));

        //! NOTE What belongs to the whole system is written with the first staff (as MeasureWrite does)
        if (staff == 0) {
            if (measure.startRepeat) {
                m_xml.writeEmptyElement("startRepeat");
            }
            if (measure.endRepeat > 0) {
                number("endRepeat", measure.endRepeat);
            }
        }

        if (measure.repeatGroup > 0) {
            number("measureRepeatCount", measure.repeatIndex);
        }

        if ((measure.lineBreak || measure.pageBreak) && staff == 0) {
            m_xml.writeStartElement("LayoutBreak");
            text("subtype", measure.pageBreak ? "page" : "line");
            m_xml.writeEndElement();
        } else if (measure.repeatIndex < measure.repeatGroup && staff == 0) {
            //! NOTE The measures of a measure repeat stay on one system (as the command which makes them does)
            m_xml.writeStartElement("LayoutBreak");
            text("subtype", "nobreak");
            m_xml.writeEndElement();
        }

        if (staff == 0) {
            writeMarkersAndJump(measure);
        }

        //! NOTE The voices which have events on the staff are its voices, one after the other; the barline ends the
        //! first of them
        bool hasVoice = false;
        for (size_t v = 0; v < measure.voices.size(); ++v) {
            if (isOnStaff(measure.voices[v], staff)) {
                writeVoice(index, v, staff, !hasVoice);
                hasVoice = true;
            }
        }

        if (!hasVoice) {
            m_xml.writeStartElement("voice");
            writeVoltas(index, staff, true);
            writeSignaturesOfOtherStaves(measure, staff);
            //! NOTE The sign of a measure repeat is in its only measure, or the last of its first half
            if (measure.repeatGroup > 0 && measure.repeatIndex == std::max(1, measure.repeatGroup / 2)) {
                m_xml.writeStartElement("MeasureRepeat");
                number("subtype", measure.repeatGroup);
                text("durationType", "measure");
                fraction("duration", writtenLength(measure));
                m_xml.writeEndElement();
            } else {
                writeMeasureRest(writtenLength(measure));
            }
            writeVoltas(index, staff, false);
            writeBarline(measure);
            m_xml.writeEndElement();
        }

        m_xml.writeEndElement();
    }

    //! NOTE The reader takes no measure shorter than a 128th: a shorter one is as long as that, with rests
    //! after its notes
    static Fraction writtenLength(const Measure& measure)
    {
        const Fraction length = measure.length().reduced();
        if (length.denominator() <= 128) {
            return length;
        }

        const int units = (length.numerator() * 128 + length.denominator() - 1) / length.denominator();
        return Fraction(units, 128).reduced();
    }

    static bool isOnStaff(const Voice& voice, int staff)
    {
        return std::any_of(voice.begin(), voice.end(), [staff](const Event& event) {
            return event.staff == staff;
        });
    }

    //! A key or a time signature at the beginning of the measure is one of the score, on all its staves: on a staff
    //! where no event of the beginning gives it, it is written as an event of another staff gives it
    void writeSignaturesOfOtherStaves(const Measure& measure, int staff)
    {
        std::optional<int> key;
        std::optional<std::pair<int, int> > time;
        bool hasOwnKey = false;
        bool hasOwnTime = false;
        for (const Voice& voice : measure.voices) {
            for (const Event& event : voice) {
                if (event.place.position != Fraction()) {
                    continue;
                }
                if (event.key) {
                    key = event.key;
                    hasOwnKey |= event.staff == staff;
                }
                if (event.time) {
                    time = event.time;
                    hasOwnTime |= event.staff == staff;
                }
            }
        }

        Event signatures;
        signatures.key = hasOwnKey ? std::nullopt : key;
        signatures.time = hasOwnTime ? std::nullopt : time;
        writePrefix(signatures);
    }

    //! The voice of a measure on a staff: its events written there, with rests where the voice is on the other staff
    //! The voltas of the system which begin or end at the beginning of the measure (`atStart`), or end at its end
    //! when it is the last one; written in its first voice on the first staff, as TWrite writes spanners
    void writeVoltas(size_t index, int staff, bool atStart)
    {
        if (staff != 0 || (!atStart && index + 1 != m_piece.measures.size())) {
            return;
        }

        const size_t end = atStart ? index : index + 1;
        for (size_t start = 0; start < end; ++start) {
            const int measures = m_piece.measures[start].voltaMeasures;
            if (measures > 0 && start + measures == end) {
                m_xml.writeStartElement("Spanner");
                m_xml.writeAttribute("type", "Volta");
                m_xml.writeStartElement("prev");
                m_xml.writeStartElement("location");
                number("measures", -measures);
                m_xml.writeEndElement();
                m_xml.writeEndElement();
                m_xml.writeEndElement();
            }
        }

        const Measure& measure = m_piece.measures[index];
        if (!atStart || measure.voltaMeasures == 0) {
            return;
        }

        //! NOTE The endings it is played on are the numbers in its text ("1." or "1, 2.")
        QStringList endings;
        static const QRegularExpression NUMBER("\\d+");
        QRegularExpressionMatchIterator it = NUMBER.globalMatch(measure.voltaText);
        while (it.hasNext()) {
            endings << it.next().captured(0);
        }

        m_xml.writeStartElement("Spanner");
        m_xml.writeAttribute("type", "Volta");
        m_xml.writeStartElement("Volta");
        number("endHookType", 1);
        text("beginText", measure.voltaText);
        text("endings", endings.isEmpty() ? QString("1") : endings.join(", "));
        m_xml.writeEndElement();
        m_xml.writeStartElement("next");
        m_xml.writeStartElement("location");
        number("measures", measure.voltaMeasures);
        m_xml.writeEndElement();
        m_xml.writeEndElement();
        m_xml.writeEndElement();
    }

    void writeVoice(size_t index, size_t voiceIndex, int staff, bool withBarline)
    {
        const Measure& measure = m_piece.measures[index];
        std::vector<const Event*> events;
        for (const Event& event : measure.voices[voiceIndex]) {
            if (event.staff == staff) {
                events.push_back(&event);
            }
        }

        m_xml.writeStartElement("voice");
        if (withBarline) {
            writeVoltas(index, staff, true);
            writeSignaturesOfOtherStaves(measure, staff);
        }

        Fraction cursor;
        for (size_t i = 0; i < events.size(); ++i) {
            writeGapRests(events[i]->place.position - cursor);

            writeEvent(*events[i], i > 0 && events[i - 1]->lyricContinues);
            cursor = events[i]->place.position + events[i]->length();
        }

        writeGapRests(writtenLength(measure) - cursor);
        if (withBarline) {
            writeVoltas(index, staff, false);
            writeBarline(measure);
        }

        m_xml.writeEndElement();
    }

    //! The markers of the measure (as TWrite writes a Marker: the text is a symbol or a word) and its jump, which
    //! plays from the start to the end
    void writeMarkersAndJump(const Measure& measure)
    {
        struct MarkerKind {
            const char* style;      // repeat_left: at the beginning of the measure; repeat_right: at its end
            const char* symbol;     // the text is this symbol, or else
            const char* word;       // this word
            const char* type;       // the MarkerType, as TConv writes it
        };
        static const std::map<QString, MarkerKind> KINDS {
            { "segno", { "repeat_left", "segno", nullptr, "segno" } },
            { "coda", { "repeat_left", "coda", nullptr, "codab" } },
            { "fine", { "repeat_right", nullptr, "Fine", "fine" } },
            { "toCoda", { "repeat_right", nullptr, "To Coda", "coda" } },
        };

        for (const QString& name : measure.markers) {
            const MarkerKind& kind = KINDS.at(name);
            m_xml.writeStartElement("Marker");
            text("style", kind.style);
            m_xml.writeStartElement("text");
            if (kind.symbol) {
                text("sym", kind.symbol);
            } else {
                m_xml.writeCharacters(kind.word);
            }
            m_xml.writeEndElement();
            text("label", kind.type);
            text("markerType", kind.type);
            m_xml.writeEndElement();
        }

        if (!measure.jump.isEmpty()) {
            m_xml.writeStartElement("Jump");
            text("style", "repeat_right");
            text("text", measure.jump);
            text("jumpTo", "start");
            text("playUntil", "end");
            text("continueAt", QString());
            m_xml.writeEndElement();
        }
    }

    //! The barline at the end of the measure, if `\\bar` gives one other than a repeat (which the measure's repeats
    //! make)
    void writeBarline(const Measure& measure)
    {
        if (measure.barline.isEmpty()) {
            return;
        }

        m_xml.writeStartElement("BarLine");
        text("subtype", measure.barline);
        m_xml.writeEndElement();
    }

    void writeMeasureRest(const Fraction& length)
    {
        m_xml.writeStartElement("Rest");
        text("durationType", "measure");
        fraction("duration", length);
        m_xml.writeEndElement();
    }

    //! Rests which are not shown, as long as `gap`, in the notes of the biggest values which fit
    void writeGapRests(Fraction gap)
    {
        for (int denominator = 1; gap.isNotZero() && denominator <= 1024; denominator *= 2) {
            while (gap >= Fraction(1, denominator)) {
                m_xml.writeStartElement("Rest");
                text("durationType", Duration { denominator, 0 }.typeName());
                number("visible", 0);
                m_xml.writeEndElement();

                gap -= Fraction(1, denominator);
            }
        }
    }

    void writeEvent(const Event& event, bool continuesLyric)
    {
        writePrefix(event);

        if (!event.dynamic.isEmpty()) {
            m_xml.writeStartElement("Dynamic");
            text("subtype", event.dynamic);
            m_xml.writeEndElement();
        }

        writeLinks(event, Link::Kind::Hairpin);

        if (!event.staffText.isEmpty()) {
            writeStaffText(event.staffText);
        }

        if (event.fermata) {
            m_xml.writeStartElement("Fermata");
            text("subtype", "fermataAbove");
            m_xml.writeEndElement();
        }

        if (event.tupletStart) {
            writeTupletStart(event);
        }

        if (event.beamAnchor || event.beamMove) {
            writeBeamElement(event);
        }

        writeChordRest(event, continuesLyric);

        if (event.tupletEnd) {
            m_xml.writeEmptyElement("endTuplet");
        }

        //! NOTE The reader puts a breath mark at the time where it is read, which is the end of the chord before it
        if (!event.breath.isEmpty()) {
            m_xml.writeStartElement("Breath");
            text("symbol", event.breath);
            m_xml.writeEndElement();
        }
    }

    //! The format tags of the text are written as elements in the text, as rich text is in a score file
    void writeStaffText(const QString& staffText)
    {
        m_xml.writeStartElement("StaffText");
        m_xml.writeStartElement("text");

        static const QRegularExpression TAG("<(/?)([a-z]+)>");
        int at = 0;
        QRegularExpressionMatchIterator tags = TAG.globalMatch(staffText);
        while (tags.hasNext()) {
            const QRegularExpressionMatch tag = tags.next();
            m_xml.writeCharacters(staffText.mid(at, tag.capturedStart() - at));
            if (tag.captured(1).isEmpty()) {
                m_xml.writeStartElement(tag.captured(2));
            } else {
                m_xml.writeEndElement();
            }
            at = tag.capturedEnd();
        }
        m_xml.writeCharacters(staffText.mid(at));

        m_xml.writeEndElement();
        m_xml.writeEndElement();
    }

    void writePrefix(const Event& event)
    {
        static const std::map<QString, QString> CLEF_TYPES {
            { "treble", "G" }, { "bass", "F" }, { "alto", "C3" }, { "tenor", "C4" }
        };

        if (event.clef) {
            m_xml.writeStartElement("Clef");
            text("concertClefType", CLEF_TYPES.at(*event.clef));
            text("transposingClefType", CLEF_TYPES.at(*event.clef));
            m_xml.writeEndElement();
        }

        if (event.ambitus) {
            writeAmbitus();
        }

        if (event.key) {
            m_xml.writeStartElement("KeySig");
            number("concertKey", *event.key);
            number("actualKey", *event.key);
            m_xml.writeEndElement();
        }

        if (event.time) {
            m_xml.writeStartElement("TimeSig");
            number("sigN", event.time->first);
            number("sigD", event.time->second);
            m_xml.writeEndElement();
        }
    }

    //! An ambitus of the highest and the lowest note of the music
    void writeAmbitus()
    {
        std::vector<Pitch> pitches;
        for (const Measure& measure : m_piece.measures) {
            for (const Voice& voice : measure.voices) {
                for (const Event& event : voice) {
                    pitches.insert(pitches.end(), event.pitches.begin(), event.pitches.end());
                }
            }
        }

        const auto [lowest, highest] = std::minmax_element(pitches.begin(), pitches.end(),
                                                           [](const Pitch& a, const Pitch& b) {
            return a.midi < b.midi;
        });

        m_xml.writeStartElement("Ambitus");
        number("topPitch", highest->midi);
        number("topTpc", highest->tpc);
        number("bottomPitch", lowest->midi);
        number("bottomTpc", lowest->tpc);
        m_xml.writeEndElement();
    }

    void writeTupletStart(const Event& event)
    {
        m_xml.writeStartElement("Tuplet");
        number("normalNotes", event.tupletNormal);
        number("actualNotes", event.tupletActual);
        text("baseNote", event.duration.typeName());
        m_xml.writeStartElement("Number");
        text("style", "tuplet");
        number("text", event.tupletActual);
        m_xml.writeEndElement();
        m_xml.writeEndElement();
    }

    //! The beam which begins at this chord, written so that a file carries it: the anchor
    //! position a user gave it (a Fragment, which reads back as a user-modified beam) and/or
    //! the staff it counts as belonging to when it is cross-staff — as TWrite writes them
    void writeBeamElement(const Event& event)
    {
        m_xml.writeStartElement("Beam");
        if (event.beamMove) {
            number("crossStaffMove", *event.beamMove);
        }
        if (event.beamAnchor) {
            m_xml.writeStartElement("Fragment");
            number("y1", event.beamAnchor->first);
            number("y2", event.beamAnchor->second);
            m_xml.writeEndElement();
        }
        m_xml.writeEndElement();
    }

    void writeChordRest(const Event& event, bool continuesLyric)
    {
        m_xml.writeStartElement(event.isRest ? "Rest" : "Chord");

        writeGrace(event);
        if (event.isMeasureRest) {
            text("durationType", "measure");
            fraction("duration", event.duration.length());
            m_xml.writeEndElement();
            return;
        }

        if (event.duration.dots > 0) {
            number("dots", event.duration.dots);
        }
        text("durationType", event.duration.typeName());
        if (event.staffMove != 0) {
            number("staffMove", event.staffMove);
        }
        if (event.small) {
            number("small", 1);
        }
        writeBeam(event);

        if (!event.isRest) {
            writeChordContent(event, continuesLyric);
        }

        m_xml.writeEndElement();
    }

    void writeGrace(const Event& event)
    {
        static const std::map<Event::Grace, const char*> TAGS {
            { Event::Grace::Grace, "grace16" }, { Event::Grace::Acciaccatura, "acciaccatura" },
            { Event::Grace::Appoggiatura, "appoggiatura" }
        };

        if (event.grace != Event::Grace::None) {
            m_xml.writeEmptyElement(TAGS.at(event.grace));
        }
    }

    void writeBeam(const Event& event)
    {
        static const std::map<Event::Beam, const char*> MODES {
            { Event::Beam::Begin, "begin" }, { Event::Beam::Mid, "mid" }, { Event::Beam::No, "no" } };

        if (event.beam != Event::Beam::Auto) {
            text("BeamMode", MODES.at(event.beam));
        }
    }

    void writeChordContent(const Event& event, bool continuesLyric)
    {
        if (!event.lyric.isEmpty()) {
            writeLyric(event, continuesLyric);
        }

        writeLinks(event, Link::Kind::Slur);

        if (event.stem != Event::Stem::Auto) {
            text("StemDirection", event.stem == Event::Stem::Up ? "up" : "down");
        }

        for (const QString& articulation : event.articulations) {
            m_xml.writeStartElement("Articulation");
            text("subtype", articulation);
            m_xml.writeEndElement();
        }

        for (const OrnamentMark& ornament : event.ornaments) {
            m_xml.writeStartElement("Ornament");
            if (ornament.showsAllAccidentals) {
                number("ornamentShowAccidental", 2);    // OrnamentShowAccidental::ALWAYS
            }
            text("subtype", ornament.symbol);
            m_xml.writeEndElement();
        }

        if (event.arpeggio >= 0) {
            m_xml.writeStartElement("Arpeggio");
            number("subtype", event.arpeggio);
            m_xml.writeEndElement();
        }

        if (event.chordLine) {
            m_xml.writeStartElement("ChordLine");
            number("subtype", event.chordLine->type);
            if (event.chordLine->straight) {
                number("straight", 1);
            }
            if (event.chordLine->wavy) {
                number("wavy", 1);
            }
            m_xml.writeEndElement();
        }

        if (event.tapping) {
            //! NOTE No TappingHalfSlur is written; layout makes it, like it does for a tapping added at run time
            m_xml.writeStartElement("Tapping");
            text("hand", event.tapping == 1 ? "left" : "right");
            m_xml.writeEndElement();
        }

        const auto lowest = std::min_element(event.pitches.cbegin(), event.pitches.cend(),
                                             [](const Pitch& a, const Pitch& b) { return a.midi < b.midi; });
        for (auto pitch = event.pitches.cbegin(); pitch != event.pitches.cend(); ++pitch) {
            writeNote(event, *pitch, pitch == lowest, std::next(pitch) == event.pitches.cend());
        }

        //! NOTE The reader takes a tremolo only after the notes of its chord
        if (event.tremolo != 0) {
            m_xml.writeStartElement("TremoloSingleChord");
            text("subtype", event.tremolo == BUZZ_ROLL ? QString("buzzroll") : QString("r%1").arg(event.tremolo));
            m_xml.writeEndElement();
        }

        if (event.tremoloBetweenChords > 0) {
            m_xml.writeStartElement("TremoloTwoChord");
            text("subtype", QString("c%1").arg(event.tremoloBetweenChords));
            m_xml.writeEndElement();
        }
    }

    void writeLyric(const Event& event, bool continuesLyric)
    {
        m_xml.writeStartElement("Lyrics");

        if (event.lyricContinues) {
            text("syllabic", continuesLyric ? "middle" : "begin");
        } else if (continuesLyric) {
            text("syllabic", "end");
        }

        text("text", event.lyric);
        m_xml.writeEndElement();
    }

    void writeNote(const Event& event, const Pitch& pitch, bool isBottomNote, bool isTopNote)
    {
        m_xml.writeStartElement("Note");

        if (pitch.accidental != Pitch::Accidental::Auto) {
            writeAccidental(pitch);
        }

        if (!event.handBracket.isEmpty() && isBottomNote) {
            writeHandBracket(event.handBracket);
        }

        if (event.fingering > 0 && isTopNote) {
            m_xml.writeStartElement("Fingering");
            number("text", event.fingering);
            m_xml.writeEndElement();
        }

        writeLinks(event, Link::Kind::Tie, pitch.midi);

        if (event.slashHeads) {
            text("head", "slash");
        }

        if (event.laissezVibrer) {
            m_xml.writeEmptyElement("LaissezVib");
        }

        number("pitch", pitch.midi);
        number("tpc", pitch.tpc);
        m_xml.writeEndElement();
    }

    //! NOTE A hand bracket is a symbol of the note, which the user places: it stands left of the notehead and
    //! reaches up from just below it
    void writeHandBracket(const QString& symbol)
    {
        m_xml.writeStartElement("Symbol");
        text("name", symbol);
        m_xml.writeEmptyElement("offset");
        m_xml.writeAttribute("x", "-2");
        m_xml.writeAttribute("y", "0.5");
        m_xml.writeEndElement();
    }

    void writeAccidental(const Pitch& pitch)
    {
        static const std::map<int, QString> NAMES {
            { -2, "accidentalDoubleFlat" }, { -1, "accidentalFlat" }, { 0, "accidentalNatural" },
            { 1, "accidentalSharp" }, { 2, "accidentalDoubleSharp" }
        };

        const auto name = NAMES.find(pitch.alteration);
        if (name == NAMES.end()) {
            fail("an accidental of more than two semitones is not shown", 0);
        }

        m_xml.writeStartElement("Accidental");
        text("role", "user");
        if (pitch.accidental == Pitch::Accidental::Parenthesised) {
            number("bracket", 1);
        } else if (pitch.accidental == Pitch::Accidental::Braced) {
            number("bracket", 2);
        }
        text("subtype", name->second);
        m_xml.writeEndElement();
    }

    //-----------------------------------------------------
    //   Spanners
    //-----------------------------------------------------

    static const char* spannerType(Link::Kind kind)
    {
        switch (kind) {
        case Link::Kind::Slur: return "Slur";
        case Link::Kind::Tie: return "Tie";
        case Link::Kind::Hairpin: return "HairPin";
        }

        return "";
    }

    //! The ends of the spanners of a kind which are at the event. A tie belongs to a note, which is given
    void writeLinks(const Event& event, Link::Kind kind, int midi = 0)
    {
        for (const Link& link : event.startingLinks) {
            if (link.kind == kind && link.midi == midi) {
                writeSpanner(link, true);
            }
        }

        for (const Link& link : event.endingLinks) {
            if (link.kind == kind && link.midi == midi) {
                writeSpanner(link, false);
            }
        }
    }

    void writeSpanner(const Link& link, bool isStart)
    {
        m_xml.writeStartElement("Spanner");
        m_xml.writeAttribute("type", spannerType(link.kind));

        if (isStart) {
            m_xml.writeStartElement(spannerType(link.kind));
            if (link.kind == Link::Kind::Hairpin) {
                number("subtype", link.subtype);
            } else if (link.subtype != 0) {
                number("lineType", link.subtype);
            }
            if (link.slurDirection != 0) {
                text("up", link.slurDirection == 1 ? "up" : "down");
            }
            if (link.slurFlat) {
                number("slurCurveMode", 1);
            }
            m_xml.writeEndElement();
        }

        writeLocation(isStart ? "next" : "prev", link);
        m_xml.writeEndElement();
    }

    void writeLocation(const char* tag, const Link& link)
    {
        m_xml.writeStartElement(tag);
        m_xml.writeStartElement("location");

        if (link.measures != 0) {
            number("measures", link.measures);
        }

        if (link.fractions.isNotZero() || link.measures == 0) {
            fraction("fractions", link.fractions);
        }

        m_xml.writeEndElement();
        m_xml.writeEndElement();
    }

    const Piece& m_piece;
    QByteArray m_bytes;
    QXmlStreamWriter m_xml;
};
}

muse::RetVal<muse::ByteArray> PreviewMusic::toMscx(const QString& notation)
{
    try {
        Piece piece = MusicParser(notation).parse();
        SpannerPlanner(piece).run();

        const QByteArray mscx = MscxWriter(piece).write();
        return muse::RetVal<muse::ByteArray>::make_ok(muse::ByteArray::fromQByteArray(mscx));
    } catch (const SyntaxError& error) {
        const QString message = QString("%1 (at character %2)").arg(error.message).arg(error.position + 1);
        return muse::RetVal<muse::ByteArray>::make_ret(static_cast<int>(muse::Ret::Code::InternalError),
                                                       message.toStdString());
    }
}
