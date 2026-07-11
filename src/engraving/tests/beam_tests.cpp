/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include <cmath>
#include <set>

#include "dom/beam.h"
#include "dom/chord.h"
#include "dom/chordrest.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/property.h"
#include "dom/segment.h"
#include "dom/tremolotwochord.h"
#include "dom/undo.h"

#include "style/styledef.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String BEAM_DATA_DIR("beam_data/");
static const String MEASURE_DATA_DIR("measure_data/");

static std::vector<Beam*> collectBeams(MasterScore* score)
{
    std::vector<Beam*> beams;
    std::set<Beam*> seen;

    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(SegmentType::ChordRest); segment;
             segment = segment->next(SegmentType::ChordRest)) {
            for (track_idx_t track = 0; track < score->ntracks(); ++track) {
                ChordRest* cr = segment->cr(track);
                if (!cr) {
                    continue;
                }

                Beam* beam = cr->beam();
                if (!beam || beam->elements().empty() || beam->elements().front() != cr) {
                    continue;
                }

                if (seen.insert(beam).second) {
                    beams.push_back(beam);
                }
            }
        }
    }

    return beams;
}

static std::vector<Beam*> collectCrossStaffBeams(MasterScore* score)
{
    std::vector<Beam*> beams;
    std::set<Beam*> seen;

    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
            for (track_idx_t track = 0; track < score->ntracks(); ++track) {
                ChordRest* cr = segment->cr(track);
                if (!cr) {
                    continue;
                }

                Beam* beam = cr->beam();
                if (!beam || !beam->cross() || beam->elements().empty() || beam->elements().front() != cr) {
                    continue;
                }

                if (seen.insert(beam).second) {
                    beams.push_back(beam);
                }
            }
        }
    }

    return beams;
}

//---------------------------------------------------------
//   TestBeam
//---------------------------------------------------------

class Engraving_BeamTests : public ::testing::Test
{
public:
    void beam(const char* path);
};

//---------------------------------------------------------
//   beam
//---------------------------------------------------------
void Engraving_BeamTests::beam(const char* path)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + String::fromUtf8(path));
    EXPECT_TRUE(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String::fromUtf8(path), BEAM_DATA_DIR + String::fromUtf8(path)));
    delete score;
}

TEST_F(Engraving_BeamTests, beamA)
{
    beam("Beam-A.mscx");
}

TEST_F(Engraving_BeamTests, beamB)
{
    beam("Beam-B.mscx");
}

TEST_F(Engraving_BeamTests, beamC)
{
    beam("Beam-C.mscx");
}

TEST_F(Engraving_BeamTests, beamD)
{
    beam("Beam-D.mscx");
}

TEST_F(Engraving_BeamTests, beamE)
{
    beam("Beam-E.mscx");
}

TEST_F(Engraving_BeamTests, beamF)
{
    beam("Beam-F.mscx");
}

TEST_F(Engraving_BeamTests, beamG)
{
    beam("Beam-G.mscx");
}

// make sure the beam end positions are correct for 2+ beams
TEST_F(Engraving_BeamTests, beamPositions)
{
    beam("beamPositions.mscx");
}

// if the beamNoSlope style parameter is true, all beams are flat
TEST_F(Engraving_BeamTests, beamNoSlope)
{
    beam("beamNoSlope.mscx");
}

TEST_F(Engraving_BeamTests, crossStaffBeamsRespectCustomSlantRules)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-2.mscx");
    EXPECT_TRUE(score);

    score->doLayout();

    std::vector<Beam*> defaultCrossBeams = collectCrossStaffBeams(score);
    EXPECT_FALSE(defaultCrossBeams.empty());

    int nonZeroDefaultSlopes = 0;
    for (Beam* beam : defaultCrossBeams) {
        if (std::abs(beam->slope()) > 0.001) {
            ++nonZeroDefaultSlopes;
        }
    }
    EXPECT_GT(nonZeroDefaultSlopes, 0);

    score->style().set(Sid::useDefaultBeamSlantRules, false);
    score->style().set(Sid::beamCustomMaxSlantForTwoNotes, 0);
    score->style().set(Sid::beamCustomTwoNoteMaxSlantSecondInterval, 0);
    score->style().set(Sid::beamCustomTwoNoteMaxSlantThirdInterval, 0);
    score->style().set(Sid::beamCustomTwoNoteMaxSlantFourthToNinthInterval, 0);
    score->style().set(Sid::beamCustomTwoNoteMaxSlantTenthInterval, 0);
    score->style().set(Sid::beamCustomTwoNoteMaxSlantGreaterThanTenthInterval, 0);
    score->style().set(Sid::beamCustomMaxSlantSecondInterval, 0);
    score->style().set(Sid::beamCustomMaxSlantThirdInterval, 0);
    score->style().set(Sid::beamCustomMaxSlantFourthInterval, 0);
    score->style().set(Sid::beamCustomMaxSlantFifthInterval, 0);
    score->style().set(Sid::beamCustomMaxSlantSixthInterval, 0);
    score->style().set(Sid::beamCustomMaxSlantSeventhInterval, 0);
    score->style().set(Sid::beamCustomMaxSlantOctave, 0);
    score->style().set(Sid::beamCustomMaxSlantGreaterThanOctave, 0);

    score->setLayoutAll();
    score->doLayout();

    std::vector<Beam*> customCrossBeams = collectCrossStaffBeams(score);
    EXPECT_EQ(customCrossBeams.size(), defaultCrossBeams.size());

    for (Beam* beam : customCrossBeams) {
        EXPECT_NEAR(beam->slope(), 0.0, 0.001);
    }

    delete score;
}

// A cross-staff beam gets a special horizontal-spacing offset when its chords straddle the two
// staves with opposite stem directions. That straddle only holds while crossStaffMove == 0. If a
// beam is made cross-staff, then a flip/drag seeds a non-zero crossStaffMove, then it is taken out
// of and put back into cross-staff, the same (reused) Beam object must not retain the stale
// crossStaffMove - otherwise crossStaffIdx is shifted, the chords no longer straddle, and the
// spacing offset silently disappears on re-entry.
TEST_F(Engraving_BeamTests, crossStaffBeamOffsetSurvivesReentry)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + u"crossStaffBeamReentry.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    // The 2nd eighth note of the beamed group on staff 2 (track 4 = staff idx 1, voice 0).
    auto eighthAt = [score](int index) -> Chord* {
        Measure* m = score->firstMeasure();
        Segment* s = m ? m->first(SegmentType::ChordRest) : nullptr;
        for (int i = 0; i < index && s; ++i) {
            s = s->next(SegmentType::ChordRest);
        }
        EngravingItem* e = s ? s->element(4) : nullptr;   // staff 2 (idx 1), voice 0 -> track 4
        return e && e->isChord() ? toChord(e) : nullptr;
    };
    auto firstEighth = [&eighthAt]() { return eighthAt(0); };
    auto secondEighth = [&eighthAt]() { return eighthAt(1); };

    Chord* target = secondEighth();
    ASSERT_TRUE(target);
    ASSERT_TRUE(target->beam());
    EXPECT_FALSE(target->beam()->cross());

    // First entry: move the note to the staff above -> cross-staff straddle, offset applies.
    score->startCmd(TranslatableString::untranslatable("test"));
    score->moveUp(target);
    score->endCmd();

    target = secondEighth();
    ASSERT_TRUE(target);
    Beam* beam = target->beam();
    ASSERT_TRUE(beam);
    EXPECT_TRUE(beam->cross());
    EXPECT_EQ(beam->crossStaffMove(), 0);
    // Straddle: the moved note has the opposite stem direction to its beam neighbour - exactly the
    // condition the cross-staff spacing offset keys off (see horizontalspacing.cpp:1081).
    Chord* neighbour = firstEighth();
    ASSERT_TRUE(neighbour);
    EXPECT_EQ(neighbour->beam(), beam);
    EXPECT_NE(target->up(), neighbour->up());

    // Seed a non-zero crossStaffMove, exactly as flipping (edit.cpp) or a beam drag would.
    score->startCmd(TranslatableString::untranslatable("test"));
    beam->undoChangeProperty(Pid::BEAM_CROSS_STAFF_MOVE, 1);
    score->endCmd();

    target = secondEighth();
    ASSERT_TRUE(target);
    beam = target->beam();
    ASSERT_TRUE(beam);
    EXPECT_EQ(beam->crossStaffMove(), 1);

    // Exit cross-staff: move the note back. The beam is no longer cross.
    score->startCmd(TranslatableString::untranslatable("test"));
    score->moveDown(target);
    score->endCmd();

    target = secondEighth();
    ASSERT_TRUE(target);
    beam = target->beam();
    ASSERT_TRUE(beam);
    EXPECT_FALSE(beam->cross());
    EXPECT_EQ(beam->crossStaffMove(), 0);   // <-- the fix: stale offset must be cleared on de-cross

    // Re-enter cross-staff: the straddle (and its spacing offset) must be restored.
    score->startCmd(TranslatableString::untranslatable("test"));
    score->moveUp(target);
    score->endCmd();

    target = secondEighth();
    ASSERT_TRUE(target);
    beam = target->beam();
    ASSERT_TRUE(beam);
    EXPECT_TRUE(beam->cross());
    EXPECT_EQ(beam->crossStaffMove(), 0);
    neighbour = firstEighth();
    ASSERT_TRUE(neighbour);
    EXPECT_EQ(neighbour->beam(), beam);
    EXPECT_NE(target->up(), neighbour->up());   // straddle (and its spacing offset) restored

    delete score;
}

// The ±notehead cross-staff spacing offset can be disabled via Sid::crossStaffBeamSpacingOffset.
// Verifies the whole chain: the style key is readable, defaults to on, and the gate in
// applyCrossBeamSpacingCorrection actually drops the extra width when turned off.
TEST_F(Engraving_BeamTests, crossStaffBeamSpacingOffsetToggle)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + u"crossStaffBeamReentry.mscx");
    ASSERT_TRUE(score);

    auto segAt = [score](int index) -> Segment* {
        Measure* m = score->firstMeasure();
        Segment* s = m ? m->first(SegmentType::ChordRest) : nullptr;
        for (int i = 0; i < index && s; ++i) {
            s = s->next(SegmentType::ChordRest);
        }
        return s;
    };
    auto chordAt = [&segAt](int index) -> Chord* {
        Segment* s = segAt(index);
        EngravingItem* e = s ? s->element(4) : nullptr;
        return e && e->isChord() ? toChord(e) : nullptr;
    };

    // Make the 2nd eighth cross-staff: notes 1->2 straddle up->down, which adds the spacing offset.
    score->doLayout();
    ASSERT_TRUE(chordAt(1));
    score->startCmd(TranslatableString::untranslatable("test"));
    score->moveUp(chordAt(1));
    score->endCmd();
    ASSERT_TRUE(chordAt(1) && chordAt(1)->beam() && chordAt(1)->beam()->cross());

    // Offset on (default): the gap between segments 1 and 2 includes the extra width.
    EXPECT_TRUE(score->style().styleB(Sid::crossStaffBeamSpacingOffset));
    double gapOn = segAt(1)->x() - segAt(0)->x();

    // Offset off: the gate is skipped, so the gap shrinks.
    score->style().set(Sid::crossStaffBeamSpacingOffset, false);
    score->setLayoutAll();
    score->doLayout();
    double gapOff = segAt(1)->x() - segAt(0)->x();

    EXPECT_GT(gapOn, gapOff);

    delete score;
}

TEST_F(Engraving_BeamTests, flippedDirection)
{
    beam("flippedDirection.mscx");
}

TEST_F(Engraving_BeamTests, wideBeams)
{
    beam("wideBeams.mscx");
}

TEST_F(Engraving_BeamTests, flatBeams)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + u"flatBeams.mscx");
    EXPECT_TRUE(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"flatBeams.mscx", BEAM_DATA_DIR + u"flatBeams-ref.mscx"));
    delete score;
}

TEST_F(Engraving_BeamTests, twoPitchBeamSlantRules)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + u"twoPitchBeamSlantRules.mscx");
    ASSERT_TRUE(score);

    score->setLayoutAll();
    score->doLayout();

    std::vector<Beam*> beams = collectBeams(score);
    ASSERT_EQ(beams.size(), 6);

    auto expectFlat = [](const Beam* beam) {
        ASSERT_TRUE(beam);
        EXPECT_NEAR(beam->slope(), 0.0, 0.001);
    };
    auto expectSloped = [](const Beam* beam) {
        ASSERT_TRUE(beam);
        EXPECT_GT(std::abs(beam->slope()), 0.001);
    };

    expectSloped(beams[0]); // isolated closest-to-beam pitch at the start, stems up
    expectFlat(beams[1]);  // isolated farthest-from-beam pitch at the start, stems up
    expectSloped(beams[2]); // equal pitch counts with different endpoints, stems up
    expectSloped(beams[3]); // isolated closest-to-beam pitch at the end, stems up
    expectSloped(beams[4]); // isolated closest-to-beam pitch at the start, stems down
    expectFlat(beams[5]);  // isolated farthest-from-beam pitch at the start, stems down

    score->style().set(Sid::useDefaultBeamSlantRules, false);
    score->setLayoutAll();
    score->doLayout();

    beams = collectBeams(score);
    ASSERT_EQ(beams.size(), 6);
    expectSloped(beams[0]);
    expectFlat(beams[1]);
    expectSloped(beams[2]);
    expectSloped(beams[3]);
    expectSloped(beams[4]);
    expectFlat(beams[5]);

    delete score;
}

TEST_F(Engraving_BeamTests, customBeamPositioningRules)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + u"customBeamPositioning.mscx");
    ASSERT_TRUE(score);

    score->style().set(Sid::useDefaultBeamSlantRules, false);
    score->setLayoutAll();
    score->doLayout();

    std::vector<Beam*> beams = collectBeams(score);
    ASSERT_EQ(beams.size(), 9);

    const double quarterSpace = score->style().spatium() / 4;
    auto slantQuarters = [quarterSpace](const Beam* beam) {
        return (beam->endAnchor().y() - beam->startAnchor().y()) / quarterSpace;
    };
    // Staff-line grid origin (page y of the top staff line) recovered from a note of the beam:
    // a note on line n sits n half spaces below the top line
    auto gridOrigin = [quarterSpace](const Beam* beam) {
        for (const ChordRest* cr : beam->elements()) {
            if (cr->isChord()) {
                const Note* note = toChord(cr)->upNote();
                return note->pagePos().y() - note->line() * 2 * quarterSpace;
            }
        }
        return 0.0;
    };
    // Final page-frame y of the primary (level 0) beam segment ends, in quarter spaces from
    // the staff-line grid origin. The stored layout anchors live in a pre-positioning frame,
    // so absolute positions must be read from the beam segments instead.
    auto segmentEndPos = [quarterSpace, gridOrigin](const Beam* beam, bool left) {
        for (const BeamSegment* seg : beam->beamSegments()) {
            if (seg->level == 0 && !seg->isBeamlet) {
                const double y = left ? seg->line.y1() : seg->line.y2();
                return (y + beam->pagePos().y() - gridOrigin(beam)) / quarterSpace;
            }
        }
        ADD_FAILURE() << "beam has no primary segment";
        return 0.0;
    };
    auto residual = [](double pos) {
        return std::abs(std::remainder(pos, 4.0));
    };

    // m1: in-staff second whose half-space slant straddles the top line: the end farther from
    // the notes gives up a quarter space and centres on the line (slant reduced to 1/4 sp)
    EXPECT_NEAR(std::abs(slantQuarters(beams[0])), 1.0, 0.01);
    EXPECT_NEAR(std::min(segmentEndPos(beams[0], true), segmentEndPos(beams[0], false)), 0.0, 0.05);
    EXPECT_NEAR(std::max(segmentEndPos(beams[0], true), segmentEndPos(beams[0], false)), 1.0, 0.05);

    // m2: in-staff third (table slant 3/4 sp) keeps its slant, dictator end on a line
    EXPECT_NEAR(std::abs(slantQuarters(beams[1])), 3.0, 0.01);
    EXPECT_NEAR(std::min(residual(segmentEndPos(beams[1], true)), residual(segmentEndPos(beams[1], false))), 0.0, 0.05);

    // m3: all notes on ledger lines above the staff: stems extend to the middle line and the
    // beam edge of the base end rests on it, slanted end a quarter space towards the staff
    EXPECT_NEAR(std::abs(slantQuarters(beams[2])), 1.0, 0.01);
    EXPECT_NEAR(std::min(segmentEndPos(beams[2], true), segmentEndPos(beams[2], false)), 7.0, 0.05);
    EXPECT_NEAR(std::max(segmentEndPos(beams[2], true), segmentEndPos(beams[2], false)), 8.0, 0.05);

    // m4: concave flat group (E4 C5 E4, stems up): the beam stays flat and gets pushed away
    // from the notes until the middle C5 reaches its full (shortened, 2.5 sp) default stem
    // length - inner chords get no fitting allowance
    EXPECT_NEAR(slantQuarters(beams[3]), 0.0, 0.01);
    EXPECT_NEAR(segmentEndPos(beams[3], true), -5.0, 0.05);

    // m5: descending 16ths high above the staff: stems extend to the middle line, the beam keeps
    // a quarter-space slant with the base end's beam edge resting on the middle line
    EXPECT_NEAR(std::abs(slantQuarters(beams[4])), 1.0, 0.01);
    EXPECT_NEAR(std::min(segmentEndPos(beams[4], true), segmentEndPos(beams[4], false)), 7.0, 0.05);
    EXPECT_NEAR(std::max(segmentEndPos(beams[4], true), segmentEndPos(beams[4], false)), 8.0, 0.05);

    // m6: two-note ledger group with a large drop (E6 to A5): stays slanted by a quarter space
    // instead of being flattened, and the deep-end stem deficit moves the seat towards the
    // staff by whole spaces (edge of the beam resting on the middle line)
    EXPECT_NEAR(std::abs(slantQuarters(beams[5])), 1.0, 0.01);
    EXPECT_NEAR(std::min(segmentEndPos(beams[5], true), segmentEndPos(beams[5], false)), 7.0, 0.05);
    EXPECT_NEAR(std::max(segmentEndPos(beams[5], true), segmentEndPos(beams[5], false)), 8.0, 0.05);

    // m7: in-staff second whose half-space slant spans the inside of the top space (lower end
    // sitting on the second line, upper end hanging from the top line): left alone
    EXPECT_NEAR(std::abs(slantQuarters(beams[6])), 2.0, 0.01);
    EXPECT_NEAR(std::min(segmentEndPos(beams[6], true), segmentEndPos(beams[6], false)), 1.0, 0.05);
    EXPECT_NEAR(std::max(segmentEndPos(beams[6], true), segmentEndPos(beams[6], false)), 3.0, 0.05);

    // m8: descending 16ths with an inner chord (E5) that falls short of its stem length under
    // the table slant: the slant is flattened until the inner stem fits and the beam still
    // touches the line grid afterwards
    EXPECT_NEAR(std::abs(slantQuarters(beams[7])), 1.0, 0.01);
    EXPECT_NEAR(std::min(segmentEndPos(beams[7], true), segmentEndPos(beams[7], false)), 15.0, 0.05);
    EXPECT_NEAR(std::max(segmentEndPos(beams[7], true), segmentEndPos(beams[7], false)), 16.0, 0.05);

    // m9: stepwise descending 16ths (G5 F5 E5 D5): the un-shortened E5 in the first space
    // misses 3.5 sp by a quarter under the full table slant, so the shallow end is pressed a
    // quarter space towards the notes (slant 1 sp to 3/4 sp)
    EXPECT_NEAR(std::abs(slantQuarters(beams[8])), 3.0, 0.01);
    EXPECT_NEAR(std::min(segmentEndPos(beams[8], true), segmentEndPos(beams[8], false)), 13.0, 0.05);
    EXPECT_NEAR(std::max(segmentEndPos(beams[8], true), segmentEndPos(beams[8], false)), 16.0, 0.05);

    // Disabling the positioning rules restores the plain custom slant behaviour (the ledger
    // group keeps the full table slant instead of the quarter-space one)
    score->style().set(Sid::beamCustomPositioningRules, false);
    score->setLayoutAll();
    score->doLayout();

    beams = collectBeams(score);
    ASSERT_EQ(beams.size(), 9);
    EXPECT_NEAR(std::abs(slantQuarters(beams[4])), 4.0, 0.01);

    delete score;
}

// cross staff beaming is not yet supported
// in the new beams code
TEST_F(Engraving_BeamTests, DISABLED_beamCrossMeasure2)
{
    beam("Beam-CrossM2.mscx");
}

TEST_F(Engraving_BeamTests, DISABLED_beamCrossMeasure3)
{
    beam("Beam-CrossM3.mscx");
}

TEST_F(Engraving_BeamTests, DISABLED_beamCrossMeasure4)
{
    beam("Beam-CrossM4.mscx");
}

//---------------------------------------------------------
//   beamCrossMeasure1
//   This method simulates following operations:
//   - Update the score
//   - Check if the beam has been recreated. If yes, this is wrong behaviour
//---------------------------------------------------------

// cross measure beams are not yet supported
// in the refactored beams code
TEST_F(Engraving_BeamTests, DISABLED_beamCrossMeasure1)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + u"Beam-CrossM1.mscx");
    EXPECT_TRUE(score);

    Measure* first_measure = score->firstMeasure();
    EXPECT_TRUE(first_measure);

    // find the first segment that has a chord
    Segment* s = first_measure->first(SegmentType::ChordRest);
    while (s && !s->element(0)->isChord()) {
        s = s->next(SegmentType::ChordRest);
    }
    EXPECT_TRUE(s);

    // locate the first beam
    ChordRest* first_note = toChordRest(s->element(0));
    EXPECT_TRUE(first_note);

    Beam* b = first_note->beam();
    score->update();
    // locate the beam again, and check if it is still b
    Beam* new_b = first_note->beam();

    EXPECT_EQ(new_b, b);

    delete score;
}

//---------------------------------------------------------
//   beamStemDir
//   This method tests if a beam's stem direction will be set to
//   all its chords and will not affect other beams in score
//---------------------------------------------------------

TEST_F(Engraving_BeamTests, beamStemDir)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + u"beamStemDir.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ChordRest* cr = toChordRest(m1->findSegment(SegmentType::ChordRest, m1->tick())->element(0));

    cr->beam()->setDirection(DirectionV::UP);

    score->update();
    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"beamStemDir-01.mscx", BEAM_DATA_DIR + u"beamStemDir-01-ref.mscx"));

    delete score;
}

//---------------------------------------------------------
//   flipBeamStemDir
//   This method tests if a beam's stem direction will be set to
//   all its chords and will not affect other beams in score
//   after using the flip command
//---------------------------------------------------------

TEST_F(Engraving_BeamTests, flipBeamStemDir)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + u"flipBeamStemDir.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ChordRest* cr = toChordRest(m1->findSegment(SegmentType::ChordRest, m1->tick())->element(0));
    Chord* c2 = toChord(cr->beam()->elements()[1]);

    score->select(c2);
    score->startCmd(TranslatableString::untranslatable("Engraving beam tests"));
    score->cmdFlip();
    score->endCmd();
    cr->beam()->setDirection(DirectionV::DOWN);

    score->update();
    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"flipBeamStemDir-01.mscx", BEAM_DATA_DIR + u"flipBeamStemDir-01-ref.mscx"));

    delete score;
}

//---------------------------------------------------------
//   flipTremoloStemDir
//   This method tests if a tremolo's stem direction will be set to
//   all its chords and will not affect other tremolos in score
//   after using the flip command
//---------------------------------------------------------

TEST_F(Engraving_BeamTests, flipTremoloStemDir)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + "flipTremoloStemDir.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ChordRest* cr = toChordRest(m1->findSegment(SegmentType::ChordRest, m1->tick())->element(0));
    TremoloTwoChord* t = toChord(cr)->tremoloTwoChord();
    Chord* c1 = t->chord1();
    Chord* c2 = t->chord2();
    EXPECT_TRUE(t->up() && c1->up() && c2->up());

    score->select(c1->upNote());
    score->startCmd(TranslatableString::untranslatable("Engraving beam tests"));
    score->cmdFlip();
    score->endCmd();

    score->update();
    score->doLayout();
    EXPECT_FALSE(t->up() || c1->up() || c2->up());

    delete score;
}

TEST_F(Engraving_BeamTests, deleteBeamStemDirection)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + "deleteBeamStemDirection.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ChordRest* cr1 = toChordRest(m1->findSegment(SegmentType::ChordRest, Fraction(0, 8))->element(0));
    EXPECT_TRUE(cr1);
    ChordRest* cr2 = toChordRest(m1->findSegment(SegmentType::ChordRest, Fraction(1, 8))->element(0));
    EXPECT_TRUE(cr2);
    ChordRest* cr3 = toChordRest(m1->findSegment(SegmentType::ChordRest, Fraction(2, 8))->element(0));
    EXPECT_TRUE(cr3);
    ChordRest* cr4 = toChordRest(m1->findSegment(SegmentType::ChordRest, Fraction(3, 8))->element(0));
    EXPECT_TRUE(cr4);

    for (ChordRest* cr : { cr1, cr2, cr3, cr4 }) {
        EXPECT_TRUE(cr->ldata()->up);
    }

    score->startCmd(TranslatableString::untranslatable("Engraving beam tests"));
    score->select({ cr2, cr3, cr4 }, SelectType::RANGE);
    score->cmdDeleteSelection();
    score->endCmd();
    score->setLayoutAll();
    score->doLayout();

    EXPECT_FALSE(cr1->ldata()->up);

    score->undoRedo(true, nullptr);

    toChord(cr1)->setStemDirection(DirectionV::UP);

    score->startCmd(TranslatableString::untranslatable("Engraving beam tests"));
    score->select({ cr2, cr3, cr4 }, SelectType::RANGE);
    score->cmdDeleteSelection();
    score->endCmd();
    score->setLayoutAll();
    score->doLayout();

    EXPECT_TRUE(cr1->ldata()->up);
}

TEST_F(Engraving_BeamTests, drumKitBeam)
{
    bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + "drumKitBeam.mscx");
    EXPECT_TRUE(score);
    score->setLayoutAll();
    score->doLayout();
    Measure* m = score->firstMeasure();
    Chord* cr1 = toChord(m->findSegment(SegmentType::ChordRest, Fraction(0, 1))->element(0));
    EXPECT_TRUE(cr1);
    EXPECT_TRUE(cr1->up() && cr1->stemDirection() == DirectionV::UP);
    Chord* cr2 = toChord(m->findSegment(SegmentType::ChordRest, Fraction(2, 8))->element(0));
    EXPECT_TRUE(cr2);
    EXPECT_TRUE(cr2->up() && cr2->stemDirection() == DirectionV::UP);
    Chord* cr3 = toChord(m->findSegment(SegmentType::ChordRest, Fraction(3, 8))->element(0));
    EXPECT_TRUE(cr3);
    EXPECT_TRUE(cr3->up() && cr3->stemDirection() == DirectionV::UP);

    score->startCmd(TranslatableString::untranslatable("Engraving beam tests"));
    score->select({ cr1, cr2, cr3 }, SelectType::RANGE);
    score->cmdFlip();
    score->setLayoutAll();
    score->doLayout();
    score->endCmd();

    EXPECT_TRUE(!cr1->up() && cr1->stemDirection() == DirectionV::DOWN);
    EXPECT_TRUE(!cr2->up() && cr2->stemDirection() == DirectionV::DOWN);
    EXPECT_TRUE(!cr3->up() && cr3->stemDirection() == DirectionV::DOWN);

    score->undoRedo(true, nullptr);

    EXPECT_TRUE(cr1->up() && cr1->stemDirection() == DirectionV::UP);
    // These chords inherit their direction from the beam
    EXPECT_TRUE(cr2->up() && cr2->stemDirection() == DirectionV::AUTO);
    EXPECT_TRUE(cr3->up() && cr3->stemDirection() == DirectionV::AUTO);

    MScore::useRead302InTestMode = useRead302;
}
