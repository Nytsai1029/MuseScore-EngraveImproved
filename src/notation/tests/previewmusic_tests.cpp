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
#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include "engraving/dom/accidental.h"
#include "engraving/dom/ambitus.h"
#include "engraving/dom/arpeggio.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/beam.h"
#include "engraving/dom/bend.h"
#include "engraving/dom/box.h"
#include "engraving/dom/bracketitem.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordline.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/expression.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/glissando.h"
#include "engraving/dom/gradualtempochange.h"
#include "engraving/dom/guitarbend.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/letring.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurerepeat.h"
#include "engraving/dom/note.h"
#include "engraving/dom/noteline.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/palmmute.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pedal.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/stringdata.h"
#include "engraving/dom/sticking.h"
#include "engraving/dom/symbol.h"
#include "engraving/dom/tapping.h"
#include "engraving/dom/text.h"
#include "engraving/dom/textline.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tremolobar.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/tremolotwochord.h"
#include "engraving/dom/trill.h"
#include "engraving/dom/laissezvib.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/vibrato.h"
#include "engraving/dom/volta.h"

#include "notation/view/engravingoptions/engravingpreviewrenderer.h"
#include "notation/view/engravingoptions/previewmusic.h"

using namespace mu::notation;
using namespace mu::engraving;

class Notation_PreviewMusicTests : public ::testing::Test
{
protected:
    //! NOTE The tests read as the application does, not with the reader of old files which test mode takes
    void SetUp() override
    {
        m_useRead302 = MScore::useRead302InTestMode;
        MScore::useRead302InTestMode = false;
    }

    void TearDown() override
    {
        MScore::useRead302InTestMode = m_useRead302;
    }

    //! The music, loaded as a preview loads it
    bool load(const QString& notation)
    {
        m_renderer = std::make_unique<EngravingPreviewRenderer>(nullptr);
        const muse::Ret ret = m_renderer->setMusic(notation);
        EXPECT_TRUE(ret) << notation.toStdString() << ": " << ret.text();
        return ret && m_renderer->score();
    }

    const MasterScore* score() const { return m_renderer->score(); }

    std::vector<ChordRest*> chordRests(track_idx_t track = 0) const
    {
        std::vector<ChordRest*> result;
        for (Measure* measure = score()->firstMeasure(); measure; measure = measure->nextMeasure()) {
            for (Segment* segment = measure->first(SegmentType::ChordRest); segment;
                 segment = segment->next(SegmentType::ChordRest)) {
                EngravingItem* element = segment->element(track);
                if (element && element->isChordRest()) {
                    result.push_back(toChordRest(element));
                }
            }
        }

        return result;
    }

    std::vector<Chord*> chords(track_idx_t track = 0) const
    {
        std::vector<Chord*> result;
        for (ChordRest* chordRest : chordRests(track)) {
            if (chordRest->isChord()) {
                result.push_back(toChord(chordRest));
            }
        }

        return result;
    }

    int spannerCount(ElementType type) const
    {
        int count = 0;
        for (const auto& [tick, spanner] : score()->spannerMap().map()) {
            count += spanner->type() == type ? 1 : 0;
        }

        return count;
    }

    //! The spanners of a type, by the time they begin
    std::vector<const Spanner*> spannersOf(ElementType type) const
    {
        std::vector<const Spanner*> found;
        for (const auto& [tick, spanner] : score()->spannerMap().map()) {
            if (spanner->type() == type) {
                found.push_back(spanner);
            }
        }
        std::stable_sort(found.begin(), found.end(), [](const Spanner* a, const Spanner* b) {
            return a->tick() < b->tick();
        });

        return found;
    }

    static std::string errorOf(const QString& notation)
    {
        return PreviewMusic::toMscx(notation).ret.text();
    }

    std::unique_ptr<EngravingPreviewRenderer> m_renderer;
    bool m_useRead302 = false;
};

/**
 * @TEST_CASE notesHavePitchesAndDurations
 * @details Pitches are absolute, with sharps and flats, and the duration stays until it is changed
 */
TEST_F(Notation_PreviewMusicTests, notesHavePitchesAndDurations)
{
    ASSERT_TRUE(load("c'4 d'' e,8 fis'8 bes'4. aes' r2"));

    const std::vector<ChordRest*> events = chordRests();
    ASSERT_EQ(events.size(), 7u);

    const int pitches[] = { 60, 74, 40, 66, 70, 68 };
    for (size_t i = 0; i < 6; ++i) {
        ASSERT_TRUE(events[i]->isChord()) << i;
        EXPECT_EQ(toChord(events[i])->upNote()->pitch(), pitches[i]) << i;
    }

    EXPECT_EQ(events[0]->durationType().type(), DurationType::V_QUARTER);
    EXPECT_EQ(events[1]->durationType().type(), DurationType::V_QUARTER);
    EXPECT_EQ(events[2]->durationType().type(), DurationType::V_EIGHTH);
    EXPECT_EQ(events[3]->durationType().type(), DurationType::V_EIGHTH);
    EXPECT_EQ(events[4]->durationType().type(), DurationType::V_QUARTER);
    EXPECT_EQ(events[4]->dots(), 1);
    EXPECT_EQ(events[5]->durationType().type(), DurationType::V_QUARTER);
    EXPECT_EQ(events[5]->dots(), 1);

    ASSERT_TRUE(events[6]->isRest());
    EXPECT_EQ(events[6]->durationType().type(), DurationType::V_HALF);

    EXPECT_EQ(toChord(events[3])->upNote()->tpc(), 20);     // f sharp
    EXPECT_EQ(toChord(events[4])->upNote()->tpc(), 12);     // b flat
}

/**
 * @TEST_CASE chordHasAllItsNotes
 * @details A chord is the notes between < and >
 */
TEST_F(Notation_PreviewMusicTests, chordHasAllItsNotes)
{
    ASSERT_TRUE(load("<c' e' g'>2 <d' fis'>4"));

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 2u);
    ASSERT_EQ(found[0]->notes().size(), 3u);
    EXPECT_EQ(found[0]->notes()[0]->pitch(), 60);
    EXPECT_EQ(found[0]->notes()[2]->pitch(), 67);
    EXPECT_EQ(found[1]->notes().size(), 2u);
}

/**
 * @TEST_CASE tieJoinsChords
 * @details A tie goes from each note of a chord to the same note of the next one
 */
TEST_F(Notation_PreviewMusicTests, tieJoinsChords)
{
    ASSERT_TRUE(load("<c'' e''>2~ <c'' e''>2 | g'4~ g'4"));

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 4u);
    for (Note* note : found[0]->notes()) {
        ASSERT_TRUE(note->tieFor());
        EXPECT_EQ(note->tieFor()->endNote()->chord(), found[1]);
    }

    EXPECT_FALSE(found[1]->notes().front()->tieFor());
    EXPECT_TRUE(found[2]->notes().front()->tieFor());
}

/**
 * @TEST_CASE slurGoesAcrossBarline
 * @details A slur ends where its ) is, in the measure after
 */
TEST_F(Notation_PreviewMusicTests, slurGoesAcrossBarline)
{
    ASSERT_TRUE(load("c'4( d' e' f' | g'4) a'"));

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 6u);
    ASSERT_EQ(spannerCount(ElementType::SLUR), 1);

    for (const auto& [tick, spanner] : score()->spannerMap().map()) {
        EXPECT_EQ(toSlur(spanner)->startCR(), found[0]);
        EXPECT_EQ(toSlur(spanner)->endCR(), found[4]);
    }

    EXPECT_NE(found[0]->measure(), found[4]->measure());
}

/**
 * @TEST_CASE tupletHoldsItsNotes
 * @details \tuplet 3/2 is three notes in the time of two, and the measure is as long as they make it
 */
TEST_F(Notation_PreviewMusicTests, tupletHoldsItsNotes)
{
    ASSERT_TRUE(load("\\tuplet 3/2 { c'8 d' e' } f'4 g'2"));

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 5u);
    for (size_t i = 0; i < 3; ++i) {
        ASSERT_TRUE(found[i]->tuplet()) << i;
        EXPECT_EQ(found[i]->tuplet()->ratio().numerator(), 3);
        EXPECT_EQ(found[i]->tuplet()->ratio().denominator(), 2);
    }

    EXPECT_FALSE(found[3]->tuplet());
    EXPECT_EQ(score()->firstMeasure()->ticks(), Fraction(1, 1));
}

/**
 * @TEST_CASE voicesShareAMeasure
 * @details << { } \\ { } >> puts two voices into one measure
 */
TEST_F(Notation_PreviewMusicTests, voicesShareAMeasure)
{
    ASSERT_TRUE(load("<< { c''2 d''2 } \\\\ { e'2 f'2 } >>"));

    EXPECT_EQ(chords(0).size(), 2u);
    EXPECT_EQ(chords(1).size(), 2u);
    EXPECT_EQ(chords(1).front()->upNote()->pitch(), 64);
    EXPECT_EQ(score()->firstMeasure()->ticks(), Fraction(1, 1));
}

/**
 * @TEST_CASE graceNoteBelongsToTheNextChord
 * @details \acciaccatura makes the next note a grace note of the one after it
 */
TEST_F(Notation_PreviewMusicTests, graceNoteBelongsToTheNextChord)
{
    ASSERT_TRUE(load("\\acciaccatura d''8 c''4 d''2"));

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 2u);
    ASSERT_EQ(found[0]->graceNotes().size(), 1u);
    EXPECT_EQ(found[0]->graceNotes().front()->upNote()->pitch(), 74);
    EXPECT_EQ(found[0]->graceNotes().front()->noteType(), NoteType::ACCIACCATURA);
    EXPECT_TRUE(found[1]->graceNotes().empty());
    EXPECT_EQ(score()->firstMeasure()->ticks(), Fraction(3, 4));
}

/**
 * @TEST_CASE keyAndTimeSignatureAreOnlyThereWhenAsked
 * @details No time signature is shown unless \time asks for one
 */
TEST_F(Notation_PreviewMusicTests, keyAndTimeSignatureAreOnlyThereWhenAsked)
{
    ASSERT_TRUE(load("c'4 d' e'"));
    EXPECT_FALSE(score()->firstMeasure()->first(SegmentType::KeySig));
    EXPECT_FALSE(score()->firstMeasure()->first(SegmentType::TimeSig));

    ASSERT_TRUE(load("\\key 2 \\time 3/4 c'4 d' e'"));
    const Segment* key = score()->firstMeasure()->first(SegmentType::KeySig);
    ASSERT_TRUE(key);
    EXPECT_EQ(toKeySig(key->element(0))->key(), Key::D);
    EXPECT_TRUE(score()->firstMeasure()->first(SegmentType::TimeSig));
}

/**
 * @TEST_CASE keyAndTimeSignatureAreOnEveryStaff
 * @details A key or a time signature at the beginning of a measure is one of the score: it is on every staff,
 *          whichever staff the notes which ask for it are on
 */
TEST_F(Notation_PreviewMusicTests, keyAndTimeSignatureAreOnEveryStaff)
{
    ASSERT_TRUE(load("\\instruments 2 \\key 2 \\time 3/4 c''2. | \\staff 2 \\key -1 \\time 2/4 c''2"));
    ASSERT_EQ(score()->nstaves(), 2u);

    for (const Measure* measure : { score()->firstMeasure(), score()->firstMeasure()->nextMeasure() }) {
        ASSERT_TRUE(measure);
        const Segment* key = measure->first(SegmentType::KeySig);
        const Segment* time = measure->first(SegmentType::TimeSig);
        ASSERT_TRUE(key);
        ASSERT_TRUE(time);

        const Key wanted = measure == score()->firstMeasure() ? Key::D : Key::F;
        const int numerator = measure == score()->firstMeasure() ? 3 : 2;
        for (staff_idx_t staff = 0; staff < 2; ++staff) {
            const KeySig* keySig = toKeySig(key->element(staff * VOICES));
            const TimeSig* timeSig = toTimeSig(time->element(staff * VOICES));
            ASSERT_TRUE(keySig) << "staff " << staff;
            ASSERT_TRUE(timeSig) << "staff " << staff;
            EXPECT_EQ(keySig->key(), wanted) << "staff " << staff;
            EXPECT_EQ(timeSig->sig().numerator(), numerator) << "staff " << staff;
        }
    }
}

/**
 * @TEST_CASE measuresAreAsLongAsTheirContent
 * @details And \break ends the system after its measure
 */
TEST_F(Notation_PreviewMusicTests, measuresAreAsLongAsTheirContent)
{
    ASSERT_TRUE(load("c'4 d' | e'2 f'2 g'2 \\break | a'8"));

    const Measure* first = score()->firstMeasure();
    ASSERT_TRUE(first);
    const Measure* second = first->nextMeasure();
    ASSERT_TRUE(second);
    const Measure* third = second->nextMeasure();
    ASSERT_TRUE(third);

    EXPECT_EQ(first->ticks(), Fraction(1, 2));
    EXPECT_EQ(second->ticks(), Fraction(3, 2));
    EXPECT_EQ(third->ticks(), Fraction(1, 8));

    EXPECT_FALSE(first->lineBreak());
    EXPECT_TRUE(second->lineBreak());
}

/**
 * @TEST_CASE marksAreOnTheirNotes
 * @details Articulations, fingering, tremolo, lyrics, arpeggio, fermata, dynamics and hairpins
 */
TEST_F(Notation_PreviewMusicTests, marksAreOnTheirNotes)
{
    ASSERT_TRUE(load("c'4-.->-1 d':16 e'_\"Glo-\" f'_\"ri\" \\fermata | <c' e' g'>4 \\arpeggio \\p \\< g'4 \\!"));

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 6u);

    ASSERT_EQ(found[0]->articulations().size(), 2u);
    EXPECT_EQ(found[0]->articulations()[0]->symId(), SymId::articStaccatoAbove);
    EXPECT_EQ(found[0]->articulations()[1]->symId(), SymId::articAccentAbove);
    ASSERT_FALSE(found[0]->notes().front()->el().empty());
    EXPECT_TRUE(found[0]->notes().front()->el().front()->isFingering());

    EXPECT_TRUE(found[1]->tremoloSingleChord());

    ASSERT_EQ(found[2]->lyrics().size(), 1u);
    EXPECT_EQ(found[2]->lyrics().front()->plainText(), muse::String(u"Glo"));
    EXPECT_EQ(found[2]->lyrics().front()->syllabic(), LyricsSyllabic::BEGIN);
    ASSERT_EQ(found[3]->lyrics().size(), 1u);
    EXPECT_EQ(found[3]->lyrics().front()->syllabic(), LyricsSyllabic::END);

    EXPECT_NE(found[3]->segment()->findAnnotation(ElementType::FERMATA, 0, 0), nullptr);

    EXPECT_TRUE(found[4]->arpeggio());
    EXPECT_NE(found[4]->segment()->findAnnotation(ElementType::DYNAMIC, 0, 0), nullptr);
    EXPECT_EQ(spannerCount(ElementType::HAIRPIN), 1);
}

/**
 * @TEST_CASE beamsAndStemsFollowTheirCommands
 * @details [ ] joins chords, \noBeam leaves one out, \stemUp and \stemDown stay until changed
 */
TEST_F(Notation_PreviewMusicTests, beamsAndStemsFollowTheirCommands)
{
    ASSERT_TRUE(load("\\stemDown c'8[ e' g']  \\stemNeutral \\noBeam a'8 \\stemUp b'4"));

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 5u);
    EXPECT_EQ(found[0]->stemDirection(), DirectionV::DOWN);
    EXPECT_EQ(found[2]->stemDirection(), DirectionV::DOWN);
    EXPECT_EQ(found[0]->beamMode(), BeamMode::BEGIN);
    EXPECT_EQ(found[1]->beamMode(), BeamMode::MID);
    EXPECT_EQ(found[2]->beamMode(), BeamMode::MID);
    EXPECT_EQ(found[3]->beamMode(), BeamMode::NONE);
    EXPECT_EQ(found[3]->stemDirection(), DirectionV::AUTO);
    EXPECT_EQ(found[4]->stemDirection(), DirectionV::UP);
}

/**
 * @TEST_CASE mistakesAreReportedWhereTheyAre
 * @details The reason and the character of the text, counted from 1
 */
TEST_F(Notation_PreviewMusicTests, mistakesAreReportedWhereTheyAre)
{
    struct Mistake {
        const char* notation;
        const char* where;
    };

    const Mistake mistakes[] = {
        { "", "(at character 1)" },
        { "c'4 x'4", "(at character 5)" },
        { "c'4 \\foo", "(at character 5)" },
        { "c'4( d'4", "(at character 1)" },
        { "c'4 d'4)", "(at character 5)" },
        { "c'4~", "(at character 1)" },
        { "c'4~ d'4", "(at character 1)" },
        { "<c' e'", "(at character 1)" },
        { "\\tuplet 3/2 { c'8 d'8", "(at character 13)" },
        { "c'4 [", "(at character 5)" },
        { "c'4[ d'", "(at character 4)" },
        { "| c'4", "(at character 1)" },
        { "c'4 \\p", "" },
        { "\\p c'4", "(at character 1)" },
        { "c'3", "(at character 3)" },
        { "r4_\"la\"", "(at character 1)" },
        { "c'4 \\time 3", "(at character 11)" },
        { "<< { c'2 } \\\\ { d'4 } >>", "(at character 1)" },
        { "\\staves 3 c'4", "(at character 9)" },
        { "\\crossStaff down c'4", "(at character 1)" },
        { "c'4 \\staff 2", "(at character 12)" },
        { "\\tremolo 16 { c'2 }", "(at character 13)" },
        { "\\grace { }", "(at character 1)" },
    };

    for (const Mistake& mistake : mistakes) {
        const std::string error = errorOf(mistake.notation);
        if (std::string(mistake.where).empty()) {
            EXPECT_TRUE(error.empty()) << mistake.notation << ": " << error;
            continue;
        }

        EXPECT_NE(error.find(mistake.where), std::string::npos) << mistake.notation << ": " << error;
    }
}

/**
 * @TEST_CASE measureRestAndVeryShortNotes
 * @details `R` is a rest of the whole measure, and the durations go down to 1024
 */
TEST_F(Notation_PreviewMusicTests, measureRestAndVeryShortNotes)
{
    ASSERT_TRUE(load("R1 | R2. | c''128 c''256 c''512 c''1024"));

    const std::vector<ChordRest*> found = chordRests();
    //! NOTE the written measure is padded to the 128th grid with a rest after the notes
    ASSERT_EQ(found.size(), 7u);
    EXPECT_TRUE(found[0]->isRest());
    EXPECT_EQ(found[0]->durationType().type(), DurationType::V_MEASURE);
    EXPECT_EQ(found[0]->ticks(), Fraction(1, 1));
    EXPECT_EQ(found[1]->ticks(), Fraction(3, 4));

    EXPECT_EQ(found[2]->durationType().type(), DurationType::V_128TH);
    EXPECT_EQ(found[3]->durationType().type(), DurationType::V_256TH);
    EXPECT_EQ(found[4]->durationType().type(), DurationType::V_512TH);
    EXPECT_EQ(found[5]->durationType().type(), DurationType::V_1024TH);
    EXPECT_TRUE(found[6]->isRest());
    EXPECT_EQ(found[6]->ticks(), Fraction(1, 1024));
}

/**
 * @TEST_CASE slursAndTiesHaveTheLineOfTheCommand
 * @details \slurDotted and \tieDashed stay until changed
 */
TEST_F(Notation_PreviewMusicTests, slursAndTiesHaveTheLineOfTheCommand)
{
    ASSERT_TRUE(load("\\slurDotted \\tieDashed c''4( d'') e''2~ e''2 \\slurSolid \\tieWideDashed f''4( g'') a'2~ a'2"));

    std::vector<SlurStyleType> slurs;
    for (const auto& [tick, spanner] : score()->spannerMap().map()) {
        slurs.push_back(toSlur(spanner)->styleType());
    }
    ASSERT_EQ(slurs.size(), 2u);
    EXPECT_EQ(slurs[0], SlurStyleType::Dotted);
    EXPECT_EQ(slurs[1], SlurStyleType::Solid);

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 8u);
    EXPECT_EQ(found[2]->notes().front()->tieFor()->styleType(), SlurStyleType::Dashed);
    EXPECT_EQ(found[6]->notes().front()->tieFor()->styleType(), SlurStyleType::WideDashed);
}

/**
 * @TEST_CASE laissezVibrerAndAccidentalMarks
 * @details \laissezVibrer follows a note; ! shows an accidental, ? shows it in parentheses
 */
/**
 * @TEST_CASE groupChoosesTheBracket
 * @details `\\staves 2` gives the staves a brace; `\\group` with `bracket`, `square`, `line`,
 *          `brace` or `none` chooses the kind of their bracket
 */
TEST_F(Notation_PreviewMusicTests, groupChoosesTheBracket)
{
    ASSERT_TRUE(load("\\staves 2 c''1 | c''1"));
    ASSERT_EQ(score()->staff(0)->brackets().size(), 1u);
    EXPECT_EQ(score()->staff(0)->brackets().front()->bracketType(), BracketType::BRACE);
    EXPECT_EQ(score()->staff(0)->brackets().front()->bracketSpan(), 2u);

    ASSERT_TRUE(load("\\staves 2 \\group square c''1 | c''1"));
    EXPECT_EQ(score()->staff(0)->brackets().front()->bracketType(), BracketType::SQUARE);

    ASSERT_TRUE(load("\\staves 2 \\group bracket c''1 | c''1"));
    EXPECT_EQ(score()->staff(0)->brackets().front()->bracketType(), BracketType::NORMAL);

    ASSERT_TRUE(load("\\staves 2 \\group line c''1 | c''1"));
    EXPECT_EQ(score()->staff(0)->brackets().front()->bracketType(), BracketType::LINE);

    ASSERT_TRUE(load("\\staves 2 \\group none c''1 | c''1"));
    EXPECT_TRUE(score()->staff(0)->brackets().empty());
}

TEST_F(Notation_PreviewMusicTests, laissezVibrerAndAccidentalMarks)
{
    ASSERT_TRUE(load("c'2 \\laissezVibrer cis'?4 c''!4 <d' f'!>4"));

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 4u);
    EXPECT_TRUE(found[0]->notes().front()->laissezVib());

    ASSERT_TRUE(found[1]->notes().front()->accidental());
    EXPECT_EQ(found[1]->notes().front()->accidental()->bracket(), AccidentalBracket::PARENTHESIS);
    EXPECT_EQ(found[1]->notes().front()->accidental()->accidentalType(), AccidentalType::SHARP);

    ASSERT_TRUE(found[2]->notes().front()->accidental());
    EXPECT_EQ(found[2]->notes().front()->accidental()->accidentalType(), AccidentalType::NATURAL);
    EXPECT_EQ(found[2]->notes().front()->accidental()->bracket(), AccidentalBracket::NONE);

    EXPECT_TRUE(found[3]->notes().back()->accidental());
}

/**
 * @TEST_CASE graceGroupsAndTremoloBetweenChords
 * @details A group of grace notes is beamed; \tremolo joins two chords
 */
TEST_F(Notation_PreviewMusicTests, graceGroupsAndTremoloBetweenChords)
{
    ASSERT_TRUE(load("\\acciaccatura { c''16 d''16 } e''4 \\tremolo 16 { c''2 e''2 }"));

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 3u);
    ASSERT_EQ(found[0]->graceNotes().size(), 2u);
    EXPECT_EQ(found[0]->graceNotes().front()->noteType(), NoteType::ACCIACCATURA);
    EXPECT_EQ(found[0]->graceNotes().front()->beamMode(), BeamMode::BEGIN);
    EXPECT_EQ(found[0]->graceNotes().back()->beamMode(), BeamMode::MID);
    EXPECT_TRUE(found[1]->tremoloTwoChord());
    EXPECT_EQ(found[1]->tremoloTwoChord()->chord2(), found[2]);
}

/**
 * @TEST_CASE ambitusAndStaves
 * @details \ambitus stands at the beginning; \staves 2 makes a grand staff, \staff says where notes are
 *          written and \crossStaff moves the chords which follow to the other staff
 */
TEST_F(Notation_PreviewMusicTests, ambitusAndStaves)
{
    ASSERT_TRUE(load("\\ambitus c'4 g''4"));
    EXPECT_TRUE(score()->firstMeasure()->first(SegmentType::Ambitus));

    ASSERT_TRUE(load("\\staves 2 << { c''4 d''4 \\crossStaff down e'8 f'8 g'4 } \\\\ { \\staff 2 c2 d2 } >>"));
    EXPECT_EQ(score()->nstaves(), 2u);
    EXPECT_EQ(chords(0).size(), 5u);
    EXPECT_EQ(chords(4).size(), 2u);

    const std::vector<Chord*> upper = chords(0);
    EXPECT_EQ(upper[0]->staffMove(), 0);
    EXPECT_EQ(upper[2]->staffMove(), 1);
    EXPECT_EQ(upper[4]->staffMove(), 1);
    EXPECT_EQ(score()->firstMeasure()->ticks(), Fraction(1, 1));
}

/**
 * @TEST_CASE slurCommandsGiveDirectionAndFlatness
 * @details `\slurUp` and `\slurDown` force the direction of the slurs which begin after them,
 *          `\slurAuto` gives it back to the layout, `\slurFlat` makes them flat in the middle
 */
TEST_F(Notation_PreviewMusicTests, slurCommandsGiveDirectionAndFlatness)
{
    ASSERT_TRUE(load("\\slurUp c'4( d') \\slurDown e'4( f') \\slurAuto g'4( a') \\slurFlat b'4( c'')"));

    std::vector<Slur*> slurs;
    for (const auto& [tick, spanner] : score()->spannerMap().map()) {
        if (spanner->isSlur()) {
            slurs.push_back(toSlur(spanner));
        }
    }

    ASSERT_EQ(slurs.size(), 4u);
    EXPECT_EQ(slurs[0]->slurDirection(), DirectionV::UP);
    EXPECT_EQ(slurs[1]->slurDirection(), DirectionV::DOWN);
    EXPECT_EQ(slurs[2]->slurDirection(), DirectionV::AUTO);
    EXPECT_FALSE(slurs[2]->isFlatMiddleCurve());
    EXPECT_TRUE(slurs[3]->isFlatMiddleCurve());
}

/**
 * @TEST_CASE tieOfOneNoteOfAChord
 * @details `~` after a note inside a chord ties only that note; the next chord need not have the others
 */
TEST_F(Notation_PreviewMusicTests, tieOfOneNoteOfAChord)
{
    ASSERT_TRUE(load("<e' g'~ b'>2 <e' g' bes'>2"));

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 2u);

    std::vector<int> tied;
    for (const Note* note : found.front()->notes()) {
        if (note->tieFor()) {
            tied.push_back(note->pitch());
        }
    }
    EXPECT_EQ(tied, std::vector<int> { 67 });

    EXPECT_FALSE(errorOf("<e' g'~>2 <e' a'>2").empty());
}

/**
 * @TEST_CASE slashNoteheads
 * @details `\slash` gives the next chord slash noteheads, and only that one
 */
TEST_F(Notation_PreviewMusicTests, slashNoteheads)
{
    ASSERT_TRUE(load("\\slash b'4 b'4"));

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 2u);
    EXPECT_EQ(found[0]->notes().front()->headGroup(), NoteHeadGroup::HEAD_SLASH);
    EXPECT_EQ(found[1]->notes().front()->headGroup(), NoteHeadGroup::HEAD_NORMAL);
}

/**
 * @TEST_CASE staffTextWithFormats
 * @details `^"text"` writes a staff text at the chord; its format tags are the rich text of the score, others are an
 *          error
 */
TEST_F(Notation_PreviewMusicTests, staffTextWithFormats)
{
    ASSERT_TRUE(load("a'4^\"1<sup>st</sup> H<sub>2</sub>O\" b'4"));

    std::vector<const StaffText*> texts;
    for (const Segment* segment = score()->firstSegment(SegmentType::ChordRest); segment;
         segment = segment->next1(SegmentType::ChordRest)) {
        for (const EngravingItem* annotation : segment->annotations()) {
            if (annotation->isStaffText()) {
                texts.push_back(toStaffText(annotation));
            }
        }
    }

    ASSERT_EQ(texts.size(), 1u);
    EXPECT_EQ(texts.front()->xmlText(), String(u"1<sup>st</sup> H<sub>2</sub>O"));
    EXPECT_EQ(texts.front()->tick(), Fraction(0, 1));

    EXPECT_FALSE(errorOf("a'4^\"x<u>y</u>\"").empty());
    EXPECT_FALSE(errorOf("a'4^text").empty());
}

/**
 * @TEST_CASE namedInstrument
 * @details `\name "long" "short"` gives the instrument its names; without it the instrument has none
 */
TEST_F(Notation_PreviewMusicTests, namedInstrument)
{
    ASSERT_TRUE(load("\\name \"Violin I\" \"Vln. I\" c''1"));
    const Part* part = score()->parts().front();
    EXPECT_EQ(part->longName(), String(u"Violin I"));
    EXPECT_EQ(part->shortName(), String(u"Vln. I"));

    ASSERT_TRUE(load("c''1"));
    EXPECT_TRUE(score()->parts().front()->longName().isEmpty());

    EXPECT_FALSE(errorOf("c''1 \\name \"A\" \"B\"").empty());
    EXPECT_FALSE(errorOf("\\name Violin \"B\" c''1").empty());
}

/**
 * @TEST_CASE smallChordsAndStaves
 * @details `\small` makes the next chord or rest small; `\smallStaff` at the beginning makes the staff small
 */
TEST_F(Notation_PreviewMusicTests, smallChordsAndStaves)
{
    ASSERT_TRUE(load("\\small c''4 d''4 \\small r4"));

    const std::vector<ChordRest*> found = chordRests();
    ASSERT_EQ(found.size(), 3u);
    EXPECT_TRUE(found[0]->isSmall());
    EXPECT_FALSE(found[1]->isSmall());
    EXPECT_TRUE(found[2]->isSmall());
    EXPECT_FALSE(score()->staff(0)->constStaffType(Fraction(0, 1))->isSmall());

    ASSERT_TRUE(load("\\smallStaff c''1"));
    EXPECT_TRUE(score()->staff(0)->constStaffType(Fraction(0, 1))->isSmall());

    EXPECT_FALSE(errorOf("c''4 \\smallStaff c''4").empty());
}

/**
 * @TEST_CASE barlinesSpanTwoStaves
 * @details `\spanBarlines` makes the barlines of the first staff go down through the second, as in a grand staff of
 *          a keyboard part; without it each staff has its own
 */
TEST_F(Notation_PreviewMusicTests, barlinesSpanTwoStaves)
{
    ASSERT_TRUE(load("\\staves 2 \\spanBarlines c''1"));
    EXPECT_EQ(score()->staff(0)->barLineSpan(), 1);
    EXPECT_EQ(score()->staff(1)->barLineSpan(), 0);

    ASSERT_TRUE(load("\\staves 2 c''1"));
    EXPECT_EQ(score()->staff(0)->barLineSpan(), 0);

    EXPECT_FALSE(errorOf("\\staves 2 c''1 | \\spanBarlines c''1").empty());
}

/**
 * @TEST_CASE twoInstruments
 * @details `\instruments 2` writes the two staves as two instruments of one staff each, both in the treble clef and
 *          with no bracket unless `\group` gives one; it is not written together with `\staves`
 */
TEST_F(Notation_PreviewMusicTests, twoInstruments)
{
    ASSERT_TRUE(load("\\instruments 2 << { c''1 } \\\\ { \\staff 2 e'1 } >>"));
    ASSERT_EQ(score()->parts().size(), 2u);
    EXPECT_EQ(score()->staff(0)->part(), score()->parts().at(0));
    EXPECT_EQ(score()->staff(1)->part(), score()->parts().at(1));
    EXPECT_EQ(score()->staff(1)->defaultClefType().concertClef, ClefType::G);
    EXPECT_TRUE(score()->staff(0)->brackets().empty());
    EXPECT_EQ(score()->firstMeasure()->findChord(Fraction(0, 1), 4)->upNote()->pitch(), 64);

    ASSERT_TRUE(load("\\instruments 2 \\group bracket c''1"));
    ASSERT_EQ(score()->staff(0)->brackets().size(), 1u);
    EXPECT_EQ(score()->staff(0)->brackets().front()->bracketType(), BracketType::NORMAL);
    EXPECT_EQ(score()->staff(0)->brackets().front()->bracketSpan(), 2u);

    EXPECT_FALSE(errorOf("\\staves 2 \\instruments 2 c''1").empty());
    EXPECT_FALSE(errorOf("\\instruments 3 c''1").empty());
    EXPECT_FALSE(errorOf("c''1 | \\instruments 2 c''1").empty());
}

/**
 * @TEST_CASE pageBreaks
 * @details `\pageBreak` ends the page after its measure, as `\break` ends the system, also after a `|`
 */
TEST_F(Notation_PreviewMusicTests, pageBreaks)
{
    ASSERT_TRUE(load("c''1 \\pageBreak c''1 | \\pageBreak c''1 | c''1 \\break c''1"));

    std::vector<const Measure*> measures;
    for (const Measure* measure = score()->firstMeasure(); measure; measure = measure->nextMeasure()) {
        measures.push_back(measure);
    }
    ASSERT_EQ(measures.size(), 5u);
    EXPECT_TRUE(measures[0]->pageBreak());
    EXPECT_TRUE(measures[1]->pageBreak());
    EXPECT_FALSE(measures[2]->pageBreak());
    EXPECT_FALSE(measures[3]->pageBreak());
    EXPECT_TRUE(measures[3]->lineBreak());
    EXPECT_FALSE(measures[0]->lineBreak());

    EXPECT_FALSE(errorOf("\\pageBreak c''1").empty());
}

/**
 * @TEST_CASE barlinesAndRepeats
 * @details `\bar` gives the barline where it stands — ending the measure, or after a `|` the measure before —
 *          with LilyPond's types; the repeats are the measures' repeat flags, `\playCount` how often a repeat is
 *          played
 */
TEST_F(Notation_PreviewMusicTests, barlinesAndRepeats)
{
    ASSERT_TRUE(load("\\bar \".|:\" c''1 \\bar \":|.\" \\playCount 3 c''1 | \\bar \"||\" c''1 \\bar \":|.|:\" c''1 "
                     "\\bar \"!\" | c''1 \\bar \"|.\""));

    std::vector<const Measure*> measures;
    for (const Measure* measure = score()->firstMeasure(); measure; measure = measure->nextMeasure()) {
        measures.push_back(measure);
    }
    ASSERT_EQ(measures.size(), 5u);

    EXPECT_TRUE(measures[0]->repeatStart());
    EXPECT_TRUE(measures[0]->repeatEnd());
    EXPECT_EQ(measures[0]->repeatCount(), 3);
    EXPECT_FALSE(measures[1]->repeatStart());
    EXPECT_FALSE(measures[1]->repeatEnd());
    EXPECT_TRUE(measures[2]->repeatEnd());
    EXPECT_EQ(measures[2]->repeatCount(), 2);
    EXPECT_TRUE(measures[3]->repeatStart());

    const auto endBarline = [](const Measure* measure) {
        const Segment* segment = measure->findSegment(SegmentType::EndBarLine, measure->endTick());
        return segment && segment->element(0) ? toBarLine(segment->element(0))->barLineType() : BarLineType::NORMAL;
    };
    EXPECT_EQ(endBarline(measures[1]), BarLineType::DOUBLE);
    EXPECT_EQ(endBarline(measures[3]), BarLineType::BROKEN);
    EXPECT_EQ(endBarline(measures[4]), BarLineType::END);

    EXPECT_FALSE(errorOf("\\bar \"||\" c''1").empty());
    EXPECT_FALSE(errorOf("c''1 \\bar \"x\"").empty());
    EXPECT_FALSE(errorOf("c''1 \\playCount 3").empty());
    EXPECT_FALSE(errorOf("c''1 \\bar \":|.\" \\playCount 1").empty());
}

/**
 * @TEST_CASE voltas
 * @details `\volta "text" N` puts a volta with the text over the N measures which begin where it stands, also over
 *          the last measures of the music; it begins with a measure and fits in the music
 */
TEST_F(Notation_PreviewMusicTests, voltas)
{
    ASSERT_TRUE(load("c''1 | \\volta \"1.\" 2 c''1 | c''1 \\bar \":|.\" \\volta \"2.\" 1 c''1"));

    std::vector<const Volta*> voltas;
    for (const auto& [tick, spanner] : score()->spannerMap().map()) {
        if (spanner->isVolta()) {
            voltas.push_back(toVolta(spanner));
        }
    }
    ASSERT_EQ(voltas.size(), 2u);
    std::sort(voltas.begin(), voltas.end(), [](const Volta* a, const Volta* b) { return a->tick() < b->tick(); });

    EXPECT_EQ(voltas[0]->tick(), Fraction(1, 1));
    EXPECT_EQ(voltas[0]->tick2(), Fraction(3, 1));
    EXPECT_EQ(voltas[0]->beginText(), u"1.");
    EXPECT_EQ(voltas[0]->endings(), std::vector<int> { 1 });
    EXPECT_EQ(voltas[1]->tick(), Fraction(3, 1));
    EXPECT_EQ(voltas[1]->tick2(), Fraction(4, 1));
    EXPECT_EQ(voltas[1]->endings(), std::vector<int> { 2 });

    EXPECT_FALSE(errorOf("c''4 \\volta \"1.\" 1 c''4").empty());
    EXPECT_FALSE(errorOf("\\volta \"1.\" 3 c''1 | c''1").empty());
    EXPECT_FALSE(errorOf("\\volta 1. 1 c''1").empty());
}

/**
 * @TEST_CASE measureRepeats
 * @details `\measureRepeat N` adds N measures which repeat the N before them, as long as those, with the sign in
 *          the middle of the group; it begins with a measure and has enough measures before it
 */
TEST_F(Notation_PreviewMusicTests, measureRepeats)
{
    ASSERT_TRUE(load("c''4 c''4 c''4 c''4 | \\measureRepeat 1 \\measureRepeat 2 c''2 c''2"));

    std::vector<const Measure*> measures;
    for (const Measure* measure = score()->firstMeasure(); measure; measure = measure->nextMeasure()) {
        measures.push_back(measure);
    }
    ASSERT_EQ(measures.size(), 5u);

    EXPECT_EQ(measures[0]->measureRepeatCount(0), 0);
    EXPECT_EQ(measures[1]->measureRepeatCount(0), 1);
    ASSERT_TRUE(measures[1]->measureRepeatElement(0));
    EXPECT_EQ(measures[1]->measureRepeatElement(0)->numMeasures(), 1);
    EXPECT_EQ(measures[2]->measureRepeatCount(0), 1);
    EXPECT_EQ(measures[3]->measureRepeatCount(0), 2);
    ASSERT_TRUE(measures[3]->measureRepeatElement(0));
    EXPECT_EQ(measures[3]->measureRepeatElement(0)->numMeasures(), 2);
    EXPECT_EQ(measures[3]->ticks(), Fraction(1, 1));
    EXPECT_EQ(measures[4]->measureRepeatCount(0), 0);

    EXPECT_FALSE(errorOf("\\measureRepeat 1 c''1").empty());
    EXPECT_FALSE(errorOf("c''1 | \\measureRepeat 3").empty());
    EXPECT_FALSE(errorOf("c''1 | \\measureRepeat 2").empty());
    EXPECT_FALSE(errorOf("c''2 \\measureRepeat 1").empty());
}

/**
 * @TEST_CASE markersAndJumps
 * @details `\segno` `\coda` `\fine` `\toCoda` are the markers of the measure being written, `\jump` its jump
 */
TEST_F(Notation_PreviewMusicTests, markersAndJumps)
{
    ASSERT_TRUE(load("\\segno c''1 | c''1 \\fine | \\coda c''1 \\toCoda \\jump \"D.S. al Coda\""));

    std::vector<std::vector<const EngravingItem*> > items;
    for (const Measure* measure = score()->firstMeasure(); measure; measure = measure->nextMeasure()) {
        items.emplace_back(measure->el().begin(), measure->el().end());
    }
    ASSERT_EQ(items.size(), 3u);

    const auto markerTypes = [](const std::vector<const EngravingItem*>& list) {
        std::vector<MarkerType> types;
        for (const EngravingItem* item : list) {
            if (item->isMarker()) {
                types.push_back(toMarker(item)->markerType());
            }
        }
        return types;
    };
    EXPECT_EQ(markerTypes(items[0]), std::vector<MarkerType> { MarkerType::SEGNO });
    EXPECT_EQ(markerTypes(items[1]), std::vector<MarkerType> { MarkerType::FINE });
    EXPECT_EQ(markerTypes(items[2]), (std::vector<MarkerType> { MarkerType::CODA, MarkerType::TOCODA }));

    const auto jump = std::find_if(items[2].begin(), items[2].end(), [](const EngravingItem* item) {
        return item->isJump();
    });
    ASSERT_NE(jump, items[2].end());
    EXPECT_EQ(toJump(*jump)->plainText(), u"D.S. al Coda");

    EXPECT_FALSE(errorOf("c''1 \\jump D.C.").empty());
}

/**
 * @TEST_CASE titleFrame
 * @details `\title "text"` puts a frame with the title text before the first measure, of the size of its text, and
 *          makes it the work title of the score
 */
TEST_F(Notation_PreviewMusicTests, titleFrame)
{
    ASSERT_TRUE(load("\\title \"Sonata in C\" c''1"));
    const MeasureBase* first = score()->first();
    ASSERT_TRUE(first && first->isVBox());
    EXPECT_TRUE(toVBox(first)->isAutoSizeEnabled());

    const Text* title = nullptr;
    for (const EngravingItem* item : toVBox(first)->el()) {
        if (item->isText()) {
            title = toText(item);
        }
    }
    ASSERT_TRUE(title);
    EXPECT_EQ(title->textStyleType(), TextStyleType::TITLE);
    EXPECT_EQ(title->plainText(), u"Sonata in C");
    EXPECT_EQ(score()->metaTag(u"workTitle"), u"Sonata in C");

    EXPECT_FALSE(errorOf("\\title Sonata c''1").empty());
    EXPECT_FALSE(errorOf("c''1 | \\title \"Sonata\" c''1").empty());
}

/**
 * @TEST_CASE stavesHiddenIndividually
 * @details `\hideStavesIndividually` sets the part's ‘hide staves when individually empty’, without it the part
 *          hides its staves only together
 */
TEST_F(Notation_PreviewMusicTests, stavesHiddenIndividually)
{
    ASSERT_TRUE(load("\\staves 2 \\hideStavesIndividually c''1"));
    EXPECT_TRUE(score()->parts().front()->hideStavesWhenIndividuallyEmpty());

    ASSERT_TRUE(load("\\staves 2 c''1"));
    EXPECT_FALSE(score()->parts().front()->hideStavesWhenIndividuallyEmpty());

    EXPECT_FALSE(errorOf("c''1 | \\hideStavesIndividually c''1").empty());
}

/**
 * @TEST_CASE staffWithOneLine
 * @details `\lines 1` gives the staff one line instead of five; written later, or with a count which is not
 *          1 to 5, it is an error
 */
TEST_F(Notation_PreviewMusicTests, staffWithOneLine)
{
    ASSERT_TRUE(load("\\lines 1 R1 | c'1"));
    EXPECT_EQ(score()->staff(0)->lines(Fraction(0, 1)), 1);

    ASSERT_TRUE(load("c'1"));
    EXPECT_EQ(score()->staff(0)->lines(Fraction(0, 1)), 5);

    EXPECT_FALSE(errorOf("c'4 \\lines 1 c'4").empty());
    EXPECT_FALSE(errorOf("\\lines 6 c'1").empty());
}

/**
 * @TEST_CASE tabStaffAndTapping
 * @details `\tab` makes the staff a tablature; `\tapping` and `\tapping left` mark the note before them
 */
TEST_F(Notation_PreviewMusicTests, tabStaffAndTapping)
{
    ASSERT_TRUE(load("\\tab c'4 \\tapping e'4 \\tapping left g'4"));

    EXPECT_TRUE(score()->staff(0)->constStaffType(Fraction(0, 1))->isTabStaff());

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 3u);

    const auto handOf = [](const Chord* chord) -> TappingHand {
        for (const Articulation* articulation : chord->articulations()) {
            if (articulation->isTapping()) {
                return toTapping(articulation)->hand();
            }
        }
        return TappingHand::INVALID;
    };

    EXPECT_EQ(handOf(found[0]), TappingHand::RIGHT);
    EXPECT_EQ(handOf(found[1]), TappingHand::LEFT);
    EXPECT_EQ(handOf(found[2]), TappingHand::INVALID);
}

/**
 * @TEST_CASE tablatureNotesHaveStringAndFret
 * @details The part of a tablature staff is a six-string guitar in standard tuning, and each note is on the highest
 * string which has it, the lower notes of a chord on lower strings
 */
TEST_F(Notation_PreviewMusicTests, tablatureNotesHaveStringAndFret)
{
    ASSERT_TRUE(load("\\tab a'4 b'4 <e' c''>4 c'4"));

    const StringData* strings = score()->staff(0)->part()->instrument()->stringData();
    ASSERT_TRUE(strings);
    EXPECT_EQ(strings->strings(), 6u);

    const auto placeOf = [](const Note* note) {
        return std::pair(note->string(), note->fret());
    };
    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 4u);
    EXPECT_EQ(placeOf(found[0]->upNote()), std::pair(0, 5));
    EXPECT_EQ(placeOf(found[1]->upNote()), std::pair(0, 7));
    EXPECT_EQ(placeOf(found[2]->upNote()), std::pair(0, 8));
    EXPECT_EQ(placeOf(found[2]->downNote()), std::pair(1, 5));
    EXPECT_EQ(placeOf(found[3]->upNote()), std::pair(1, 1));
}

/**
 * @TEST_CASE ornamentsLinesAndBreathMarks
 * @details Ornaments (with `!` all their accidentals are shown), bowings, lute fingering, falls and the like,
 * breath marks and caesuras after their chords, and a keyboard hand bracket at the lowest note of its chord
 */
TEST_F(Notation_PreviewMusicTests, ornamentsLinesAndBreathMarks)
{
    ASSERT_TRUE(load("c''4 \\trill \\upbow d''4 \\turn! \\breathe e''4 \\fallWavy \\luteFinger 2 "
                     "<g' e' c''>4 \\doitStraight \\caesura \\handBracket left"));

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 4u);

    ASSERT_EQ(found[0]->articulations().size(), 2u);
    const Articulation* trill = found[0]->articulations()[0]->isOrnament() ? found[0]->articulations()[0]
                                : found[0]->articulations()[1];
    const Articulation* bowing = trill == found[0]->articulations()[0] ? found[0]->articulations()[1]
                                 : found[0]->articulations()[0];
    ASSERT_TRUE(trill->isOrnament());
    EXPECT_EQ(trill->symId(), SymId::ornamentTrill);
    EXPECT_EQ(toOrnament(trill)->showAccidental(), OrnamentShowAccidental::DEFAULT);
    EXPECT_EQ(bowing->symId(), SymId::stringsUpBow);

    ASSERT_EQ(found[1]->articulations().size(), 1u);
    ASSERT_TRUE(found[1]->articulations().front()->isOrnament());
    EXPECT_EQ(found[1]->articulations().front()->symId(), SymId::ornamentTurn);
    EXPECT_EQ(toOrnament(found[1]->articulations().front())->showAccidental(), OrnamentShowAccidental::ALWAYS);

    const Segment* breathAfterTurn = found[1]->segment()->next(SegmentType::Breath);
    ASSERT_TRUE(breathAfterTurn);
    EXPECT_EQ(breathAfterTurn->tick(), found[1]->tick() + found[1]->actualTicks());
    ASSERT_TRUE(breathAfterTurn->element(0) && breathAfterTurn->element(0)->isBreath());
    EXPECT_EQ(toBreath(breathAfterTurn->element(0))->symId(), SymId::breathMarkComma);

    ASSERT_TRUE(found[2]->chordLine());
    EXPECT_EQ(found[2]->chordLine()->chordLineType(), ChordLineType::FALL);
    EXPECT_TRUE(found[2]->chordLine()->isWavy());
    EXPECT_FALSE(found[2]->chordLine()->isStraight());
    ASSERT_EQ(found[2]->articulations().size(), 1u);
    EXPECT_EQ(found[2]->articulations().front()->symId(), SymId::luteFingeringRHSecond);

    ASSERT_TRUE(found[3]->chordLine());
    EXPECT_EQ(found[3]->chordLine()->chordLineType(), ChordLineType::DOIT);
    EXPECT_TRUE(found[3]->chordLine()->isStraight());
    const Segment* caesura = found[3]->segment()->next(SegmentType::Breath);
    ASSERT_TRUE(caesura && caesura->element(0));
    EXPECT_EQ(toBreath(caesura->element(0))->symId(), SymId::caesura);

    int brackets = 0;
    for (const Note* note : found[3]->notes()) {
        for (const EngravingItem* item : note->el()) {
            if (item->isSymbol() && toSymbol(item)->sym() == SymId::keyboardPlayWithLH) {
                EXPECT_EQ(note->pitch(), 64) << "the bracket is at the lowest note";
                ++brackets;
            }
        }
    }
    EXPECT_EQ(brackets, 1);

    EXPECT_FALSE(errorOf("c'4 \\luteFinger 4").empty());
    EXPECT_FALSE(errorOf("c'4 \\handBracket").empty());
    EXPECT_FALSE(errorOf("c'4 \\fermata!").empty());
    EXPECT_FALSE(errorOf("\\trill c'4").empty());
}

/**
 * @TEST_CASE hairpinLinesExpressionsAndInstruments
 * @details `\cresc` and `\dim` are hairpins drawn as lines with text, `\niente` gives the hairpin a circled tip,
 * `\diagonal` makes it diagonal, `\atEndOfPrevious` puts a dynamic before its beat, `\expression` writes an
 * expression text and `\instrument` chooses the instrument of the part
 */
TEST_F(Notation_PreviewMusicTests, hairpinLinesExpressionsAndInstruments)
{
    ASSERT_TRUE(load("\\instrument voice c''4 \\cresc \\niente d''4 \\! e''4 \\p \\expression \"dolce\" | "
                     "f''4 \\f \\atEndOfPrevious \\dim \\diagonal -2 e''4 \\!"));

    EXPECT_TRUE(score()->staff(0)->part()->instrument()->isVocalInstrument());

    std::vector<const Hairpin*> hairpins;
    for (const auto& [tick, spanner] : score()->spannerMap().map()) {
        if (spanner->isHairpin()) {
            hairpins.push_back(toHairpin(spanner));
        }
    }
    ASSERT_EQ(hairpins.size(), 2u);
    std::sort(hairpins.begin(), hairpins.end(), [](const Hairpin* a, const Hairpin* b) {
        return a->tick() < b->tick();
    });
    EXPECT_EQ(hairpins[0]->hairpinType(), HairpinType::CRESC_LINE);
    EXPECT_TRUE(hairpins[0]->hairpinCircledTip());
    EXPECT_EQ(hairpins[1]->hairpinType(), HairpinType::DIM_LINE);
    EXPECT_FALSE(hairpins[1]->hairpinCircledTip());
    EXPECT_FALSE(hairpins[0]->diagonal());
    EXPECT_TRUE(hairpins[1]->diagonal());
    ASSERT_EQ(hairpins[1]->spannerSegments().size(), 1u);
    EXPECT_DOUBLE_EQ(hairpins[1]->frontSegment()->userOff2().y(), -2 * hairpins[1]->spatium());

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 5u);
    const EngravingItem* piano = found[2]->segment()->findAnnotation(ElementType::DYNAMIC, 0, 0);
    ASSERT_TRUE(piano);
    EXPECT_FALSE(toDynamic(piano)->anchorToEndOfPrevious());
    const EngravingItem* dolce = found[2]->segment()->findAnnotation(ElementType::EXPRESSION, 0, 0);
    ASSERT_TRUE(dolce);
    EXPECT_EQ(toExpression(dolce)->plainText(), muse::String(u"dolce"));

    const EngravingItem* forte = found[3]->segment()->findAnnotation(ElementType::DYNAMIC, 0, 0);
    ASSERT_TRUE(forte);
    EXPECT_TRUE(toDynamic(forte)->anchorToEndOfPrevious());

    EXPECT_FALSE(errorOf("c'4 \\niente").empty());
    EXPECT_FALSE(errorOf("c'4 \\diagonal 2").empty());
    EXPECT_FALSE(errorOf("c'4 \\< \\diagonal x d'4 \\!").empty());
    EXPECT_FALSE(errorOf("c'4 \\atEndOfPrevious").empty());
    EXPECT_FALSE(errorOf("c'4 \\instrument voice").empty());
    EXPECT_FALSE(errorOf("\\instrument \"Voice\" c'4").empty());
}

/**
 * @TEST_CASE stickingNamesTheHand
 * @details `\\sticking "R"` writes the letters which name the hand above or below the note
 */
TEST_F(Notation_PreviewMusicTests, stickingNamesTheHand)
{
    ASSERT_TRUE(load("c''4 \\sticking \"R\" c''4 \\sticking \"L\" c''2"));

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 3u);
    const EngravingItem* right = found[0]->segment()->findAnnotation(ElementType::STICKING, 0, 0);
    ASSERT_TRUE(right);
    EXPECT_EQ(toSticking(right)->plainText(), muse::String(u"R"));
    EXPECT_TRUE(found[1]->segment()->findAnnotation(ElementType::STICKING, 0, 0));
    EXPECT_FALSE(found[2]->segment()->findAnnotation(ElementType::STICKING, 0, 0));

    EXPECT_FALSE(errorOf("c'4 \\sticking R").empty());
}

/**
 * @TEST_CASE parenthesesAndChordSymbols
 * @details `\\parenthesize` puts each note of the next chord in parentheses, `\\parenthesizeChord` the chord as a
 * whole; `\\chordSymbol` writes a chord symbol, in parentheses when its text is
 */
TEST_F(Notation_PreviewMusicTests, parenthesesAndChordSymbols)
{
    ASSERT_TRUE(load("\\parenthesize <a' c''>4 \\parenthesizeChord <d' fis'>4 \\chordSymbol \"D7(b9)/F#\" "
                     "a'2 \\chordSymbol \"(G)\""));

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 3u);

    for (const Note* note : found[0]->notes()) {
        EXPECT_TRUE(note->leftParen() && note->rightParen());
    }
    EXPECT_FALSE(found[0]->leftParen());
    EXPECT_TRUE(found[1]->leftParen() && found[1]->rightParen());
    EXPECT_FALSE(found[1]->notes().front()->leftParen());
    EXPECT_FALSE(found[2]->notes().front()->leftParen());

    const EngravingItem* seventh = found[1]->segment()->findAnnotation(ElementType::HARMONY, 0, 0);
    ASSERT_TRUE(seventh);
    ASSERT_FALSE(toHarmony(seventh)->chords().empty());
    const HarmonyInfo* info = toHarmony(seventh)->chords().front();
    EXPECT_EQ(info->rootTpc(), 16);
    EXPECT_EQ(info->bassTpc(), 20);
    EXPECT_EQ(info->textName(), muse::String(u"7(b9)"));
    EXPECT_FALSE(seventh->leftParen());

    const EngravingItem* g = found[2]->segment()->findAnnotation(ElementType::HARMONY, 0, 0);
    ASSERT_TRUE(g);
    EXPECT_TRUE(g->leftParen() && g->rightParen());

    EXPECT_FALSE(errorOf("\\parenthesize r4").empty());
    EXPECT_FALSE(errorOf("c'4 \\chordSymbol \"H7\"").empty());
}

/**
 * @TEST_CASE versesAndExtensionLines
 * @details Each further `_"text"` of a chord is the next verse; the syllables of a verse are joined across barlines
 * and across notes without a syllable, and `__` draws an extension line to the last note before the next syllable
 */
TEST_F(Notation_PreviewMusicTests, versesAndExtensionLines)
{
    ASSERT_TRUE(load("c'4_\"1. Glo-\"_\"2. Ah\" d'4_\"ri-\" | e'4_\"a__\" f'4 g'2 | c''1_\"Deo\""));

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 6u);

    ASSERT_EQ(found[0]->lyrics().size(), 2u);
    const auto verseOf = [](const Chord* chord, int verse) -> const Lyrics* {
        for (const Lyrics* lyrics : chord->lyrics()) {
            if (lyrics->no() == verse) {
                return lyrics;
            }
        }
        return nullptr;
    };
    ASSERT_TRUE(verseOf(found[0], 0) && verseOf(found[0], 1));
    EXPECT_EQ(verseOf(found[0], 0)->syllabic(), LyricsSyllabic::BEGIN);
    EXPECT_EQ(verseOf(found[0], 1)->plainText(), muse::String(u"2. Ah"));
    EXPECT_EQ(verseOf(found[0], 1)->syllabic(), LyricsSyllabic::SINGLE);

    ASSERT_TRUE(verseOf(found[1], 0));
    EXPECT_EQ(verseOf(found[1], 0)->syllabic(), LyricsSyllabic::MIDDLE);

    //! "a" ends the word begun before the barline, and its extension line reaches g' (half a whole note later)
    const Lyrics* melisma = verseOf(found[2], 0);
    ASSERT_TRUE(melisma);
    EXPECT_EQ(melisma->syllabic(), LyricsSyllabic::END);
    EXPECT_EQ(melisma->ticks(), Fraction(1, 2));
    EXPECT_TRUE(found[3]->lyrics().empty());

    ASSERT_TRUE(verseOf(found[5], 0));
    EXPECT_EQ(verseOf(found[5], 0)->syllabic(), LyricsSyllabic::SINGLE);
    EXPECT_TRUE(verseOf(found[5], 0)->ticks().isZero());
}

/**
 * @TEST_CASE linesOverChordsAndBetweenNotes
 * @details `\\start…` begins a line with a chord and `\\stop…` ends it with the end of a chord (also of the last
 * chord of a measure), of the kind and with the hooks, texts and accidental the commands say; `\\glissando` and
 * `\\noteLine` join the notes of a chord to those of the next one, from the lowest up
 */
TEST_F(Notation_PreviewMusicTests, linesOverChordsAndBetweenNotes)
{
    ASSERT_TRUE(load("c''4 \\startOttava 15mb \\startPedal \\hooks 90 45 d''4 \\stopPedal "
                     "e''4 \\startPedal \\noLine f''4 \\stopOttava | "
                     "g'4 \\stopPedal \\startTrill! upprall a'4 \\startTextLine \"poco\" \\startVibrato wide "
                     "b'4 \\stopTrill \\stopVibrato \\startSystemTextLine \"Solo\" "
                     "c''4 \\stopTextLine \\stopSystemTextLine \\startLetRing \\startPalmMute "
                     "\\startTempoChange rit \\stopLetRing \\stopPalmMute \\stopTempoChange | "
                     "<c' e'>4 \\glissando <g' c''>4 \\noteLine e'4 r4"));

    const auto lasts = [](const Spanner* spanner, Fraction from, Fraction to) {
        return spanner->tick() == from && spanner->tick2() == to;
    };

    const std::vector<const Spanner*> ottavas = spannersOf(ElementType::OTTAVA);
    ASSERT_EQ(ottavas.size(), 1u);
    EXPECT_EQ(toOttava(ottavas[0])->ottavaType(), OttavaType::OTTAVA_15MB);
    EXPECT_TRUE(lasts(ottavas[0], Fraction(0, 1), Fraction(1, 1)));

    const std::vector<const Spanner*> pedals = spannersOf(ElementType::PEDAL);
    ASSERT_EQ(pedals.size(), 2u);
    EXPECT_TRUE(lasts(pedals[0], Fraction(0, 1), Fraction(1, 2)));
    EXPECT_EQ(toPedal(pedals[0])->beginHookType(), HookType::HOOK_90);
    EXPECT_EQ(toPedal(pedals[0])->endHookType(), HookType::HOOK_45);
    EXPECT_TRUE(toPedal(pedals[0])->lineVisible());
    EXPECT_TRUE(lasts(pedals[1], Fraction(1, 2), Fraction(5, 4)));
    EXPECT_FALSE(toPedal(pedals[1])->lineVisible());

    const std::vector<const Spanner*> trills = spannersOf(ElementType::TRILL);
    ASSERT_EQ(trills.size(), 1u);
    EXPECT_EQ(toTrill(trills[0])->trillType(), TrillType::UPPRALL_LINE);
    ASSERT_TRUE(toTrill(trills[0])->ornament());
    EXPECT_EQ(toTrill(trills[0])->ornament()->showAccidental(), OrnamentShowAccidental::ALWAYS);
    EXPECT_TRUE(lasts(trills[0], Fraction(1, 1), Fraction(7, 4)));

    const std::vector<const Spanner*> vibratos = spannersOf(ElementType::VIBRATO);
    ASSERT_EQ(vibratos.size(), 1u);
    EXPECT_EQ(toVibrato(vibratos[0])->vibratoType(), VibratoType::GUITAR_VIBRATO_WIDE);
    EXPECT_TRUE(lasts(vibratos[0], Fraction(5, 4), Fraction(7, 4)));

    const std::vector<const Spanner*> textLines = spannersOf(ElementType::TEXTLINE);
    ASSERT_EQ(textLines.size(), 2u);
    EXPECT_EQ(toTextLine(textLines[0])->beginText(), muse::String(u"poco"));
    EXPECT_FALSE(textLines[0]->systemFlag());
    EXPECT_TRUE(lasts(textLines[0], Fraction(5, 4), Fraction(2, 1)));
    EXPECT_EQ(toTextLine(textLines[1])->beginText(), muse::String(u"Solo"));
    EXPECT_TRUE(textLines[1]->systemFlag());
    EXPECT_TRUE(lasts(textLines[1], Fraction(3, 2), Fraction(2, 1)));

    for (const ElementType type : { ElementType::LET_RING, ElementType::PALM_MUTE,
                                    ElementType::GRADUAL_TEMPO_CHANGE }) {
        const std::vector<const Spanner*> found = spannersOf(type);
        ASSERT_EQ(found.size(), 1u);
        EXPECT_TRUE(lasts(found[0], Fraction(7, 4), Fraction(2, 1)));
    }
    const GradualTempoChange* rit = toGradualTempoChange(spannersOf(ElementType::GRADUAL_TEMPO_CHANGE).front());
    EXPECT_EQ(rit->tempoChangeType(), GradualTempoChangeType::Ritardando);
    EXPECT_EQ(rit->beginText(), muse::String(u"rit."));

    //! NOTE A glissando and a note-anchored line belong to the note where they begin, not to the spanners of the score
    std::vector<std::pair<int, int> > glissandos;
    std::vector<std::pair<int, int> > noteLines;
    for (const Chord* chord : chords()) {
        for (const Note* note : chord->notes()) {
            for (const Spanner* spanner : note->spannerFor()) {
                ASSERT_TRUE(spanner->endElement() && spanner->endElement()->isNote());
                const std::pair<int, int> pitches(note->pitch(), toNote(spanner->endElement())->pitch());
                if (spanner->isGlissando()) {
                    glissandos.push_back(pitches);
                } else if (spanner->isNoteLine()) {
                    noteLines.push_back(pitches);
                }
            }
        }
    }
    std::sort(glissandos.begin(), glissandos.end());
    EXPECT_EQ(glissandos, (std::vector<std::pair<int, int> > { { 60, 67 }, { 64, 72 } }));
    EXPECT_EQ(noteLines, (std::vector<std::pair<int, int> > { { 67, 64 } }));

    EXPECT_FALSE(errorOf("c'4 \\stopPedal").empty());
    EXPECT_FALSE(errorOf("c'4 \\startPedal d'4").empty());
    EXPECT_FALSE(errorOf("c'4 \\startOttava 9va d'4 \\stopOttava").empty());
    EXPECT_FALSE(errorOf("c'4 \\startTrill \\hooks 90 90 \\stopTrill").empty());
    EXPECT_FALSE(errorOf("c'4 \\hooks 90 90").empty());
    EXPECT_FALSE(errorOf("c'4 \\startPedal \\hooks 90 x \\stopPedal").empty());
    EXPECT_FALSE(errorOf("c'4 \\startTextLine poco \\stopTextLine").empty());
    EXPECT_FALSE(errorOf("c'4 \\startTempoChange slower \\stopTempoChange").empty());
    EXPECT_FALSE(errorOf("c'4 \\glissando r4").empty());
}

/**
 * @TEST_CASE bendsAndTremoloBars
 * @details `\\bend` bends a note up to the note of the next chord (from a grace note: a grace note bend), `\\preBend`
 * bends a grace note before it is struck, `\\slightBend` stays on its note; `\\oldBend` and `\\tremoloBar` are the
 * bend of MuseScore 3 and a tremolo bar dip. A spanner which begins on a grace note ends where it should
 */
TEST_F(Notation_PreviewMusicTests, bendsAndTremoloBars)
{
    ASSERT_TRUE(load("a'4 \\bend b'4 \\grace g'8 \\bend a'4 \\grace g'8 \\preBend a'4 | "
                     "a'4 \\slightBend c''4 \\oldBend d''4 \\tremoloBar \\grace c''8( d''4)"));

    struct BendSeen {
        GuitarBendType type;
        int from;
        int to;
        bool fromGrace;
        bool operator==(const BendSeen& other) const
        {
            return type == other.type && from == other.from && to == other.to && fromGrace == other.fromGrace;
        }
    };
    std::vector<BendSeen> bends;
    std::vector<const Note*> notes;
    for (const Chord* chord : chords()) {
        for (const Chord* grace : chord->graceNotes()) {
            notes.insert(notes.end(), grace->notes().begin(), grace->notes().end());
        }
        notes.insert(notes.end(), chord->notes().begin(), chord->notes().end());
    }
    for (const Note* note : notes) {
        for (const Spanner* spanner : note->spannerFor()) {
            if (spanner->isGuitarBend()) {
                ASSERT_TRUE(spanner->endElement() && spanner->endElement()->isNote());
                bends.push_back({ toGuitarBend(spanner)->type(), note->pitch(), toNote(spanner->endElement())->pitch(),
                                  note->chord()->isGrace() });
            }
        }
    }
    const std::vector<BendSeen> expected {
        { GuitarBendType::BEND, 69, 71, false },
        { GuitarBendType::GRACE_NOTE_BEND, 67, 69, true },
        { GuitarBendType::PRE_BEND, 67, 69, true },
        { GuitarBendType::SLIGHT_BEND, 69, 69, false },
    };
    EXPECT_TRUE(bends == expected) << bends.size() << " bends";
    for (const Note* note : notes) {
        if (note->chord()->isGrace()) {
            // [THEN] A grace note which begins a bend has no stem, the others have one
            EXPECT_EQ(note->chord()->noStem(), note->bendFor() != nullptr);
        }
    }

    const std::vector<Chord*> found = chords();
    ASSERT_EQ(found.size(), 8u);
    const Note* bent = found[5]->upNote();
    EXPECT_TRUE(std::any_of(bent->el().begin(), bent->el().end(), [](const EngravingItem* item) {
        return item->isBend();
    }));
    EXPECT_TRUE(found[6]->segment()->findAnnotation(ElementType::TREMOLOBAR, 0, 0));

    const std::vector<const Spanner*> slurs = spannersOf(ElementType::SLUR);
    ASSERT_EQ(slurs.size(), 1u);
    ASSERT_TRUE(slurs[0]->startElement() && slurs[0]->endElement());
    EXPECT_TRUE(toChord(slurs[0]->startElement())->isGrace());
    EXPECT_EQ(slurs[0]->endElement(), found[7]);

    // [THEN] In a chord, a note which keeps its pitch is held, not bent
    ASSERT_TRUE(load("<a' e''>4 \\bend <b' e''>4 r2"));
    const std::vector<Chord*> chordBend = chords();
    ASSERT_EQ(chordBend.size(), 2u);
    EXPECT_TRUE(chordBend[0]->downNote()->bendFor());
    EXPECT_FALSE(chordBend[0]->upNote()->bendFor());

    EXPECT_FALSE(errorOf("c'4 \\preBend d'4").empty());
    EXPECT_FALSE(errorOf("r4 \\bend d'4").empty());
    EXPECT_FALSE(errorOf("c'4 \\bend").empty());
}

/**
 * @TEST_CASE hammerOnPullOffSlurs
 * @details The slurs which begin between `\\hopoOn` and `\\hopoOff` are hammer-on / pull-off slurs, the others
 * plain slurs
 */
TEST_F(Notation_PreviewMusicTests, hammerOnPullOffSlurs)
{
    ASSERT_TRUE(load("\\hopoOn e'4( g'4 a'4) \\hopoOff g'4( | e'1)"));

    const std::vector<const Spanner*> hopos = spannersOf(ElementType::HAMMER_ON_PULL_OFF);
    ASSERT_EQ(hopos.size(), 1u);
    EXPECT_EQ(hopos[0]->tick(), Fraction(0, 1));
    EXPECT_EQ(hopos[0]->tick2(), Fraction(1, 2));
    const std::vector<const Spanner*> slurs = spannersOf(ElementType::SLUR);
    ASSERT_EQ(slurs.size(), 1u);
    EXPECT_EQ(slurs[0]->tick(), Fraction(3, 4));
}
