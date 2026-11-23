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

#include <vector>

#include "engraving/dom/accidental.h"
#include "engraving/dom/ambitus.h"
#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/beam.h"
#include "engraving/dom/box.h"
#include "engraving/dom/bracketitem.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/tapping.h"
#include "engraving/dom/text.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/tremolotwochord.h"
#include "engraving/dom/laissezvib.h"
#include "engraving/dom/tuplet.h"

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
