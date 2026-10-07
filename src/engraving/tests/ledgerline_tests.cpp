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

#include <algorithm>
#include <filesystem>
#include <map>
#include <tuple>
#include <vector>

#include "dom/chord.h"
#include "dom/editdata.h"
#include "dom/ledgerline.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/mscore.h"
#include "dom/note.h"
#include "dom/property.h"
#include "dom/segment.h"
#include "dom/system.h"
#include "dom/undo.h"

#include "style/styledef.h"

#include "utils/scorerw.h"

using namespace mu;
using namespace mu::engraving;

static const String LEDGERLINE_DATA_DIR("ledgerline_data/");

class Engraving_LedgerLineTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_useRead302 = MScore::useRead302InTestMode;
        MScore::useRead302InTestMode = false;
    }

    void TearDown() override
    {
        MScore::useRead302InTestMode = m_useRead302;
    }

private:
    bool m_useRead302 = false;
};

static Chord* firstChord(MasterScore* score)
{
    Measure* measure = score->firstMeasure();
    Segment* segment = measure ? measure->first(SegmentType::ChordRest) : nullptr;
    EngravingItem* item = segment ? segment->element(0) : nullptr;
    return item && item->isChord() ? toChord(item) : nullptr;
}

static LedgerLine* ledgerLineAt(const Chord* chord, int line)
{
    for (LedgerLine* ledgerLine : chord->ledgerLines()) {
        if (ledgerLine->line() == line) {
            return ledgerLine;
        }
    }
    return nullptr;
}

static void setOffsetLeft(MasterScore* score, LedgerLine* ledgerLine, double sp)
{
    score->startCmd(TranslatableString::untranslatable("Engraving ledger line tests"));
    ledgerLine->undoChangeProperty(Pid::LEDGER_LINE_LENGTH_OFFSET_LEFT, Spatium(sp));
    score->endCmd();
}

static void moveNote(MasterScore* score, Note* note, bool up)
{
    score->startCmd(TranslatableString::untranslatable("Engraving ledger line tests"));
    score->select(note, SelectType::SINGLE, 0);
    score->upDown(up, UpDownMode::DIATONIC);
    score->endCmd();
}

// Horizontal gaps between ledger lines that follow each other at the same height of the same system, smallest first
static std::vector<double> neighbourGaps(MasterScore* score)
{
    std::map<std::tuple<const System*, staff_idx_t, int>, std::vector<std::pair<double, double> > > rows;
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(SegmentType::ChordRest); segment;
             segment = segment->next(SegmentType::ChordRest)) {
            EngravingItem* item = segment->element(0);
            if (!item || !item->isChord()) {
                continue;
            }
            for (const LedgerLine* ledgerLine : toChord(item)->ledgerLines()) {
                const double x = ledgerLine->pageX();
                const auto row = std::make_tuple(measure->system(), ledgerLine->staffIdx(), ledgerLine->line());
                rows[row].emplace_back(x, x + ledgerLine->len());
            }
        }
    }

    std::vector<double> gaps;
    for (auto& row : rows) {
        std::vector<std::pair<double, double> >& extents = row.second;
        std::sort(extents.begin(), extents.end());
        for (size_t i = 0; i + 1 < extents.size(); ++i) {
            gaps.push_back(extents[i + 1].first - extents[i].second);
        }
    }
    // the rows come in the order of the system addresses, which is not the same from one layout to the next
    std::sort(gaps.begin(), gaps.end());
    return gaps;
}

static std::vector<double> chordRestSegmentPositions(MasterScore* score)
{
    std::vector<double> positions;
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(SegmentType::ChordRest); segment;
             segment = segment->next(SegmentType::ChordRest)) {
            positions.push_back(measure->x() + segment->x());
        }
    }
    return positions;
}

// Spacing does not count ledger lines, so those of neighbouring notes run into each other in tight spacing.
// Sid::ledgerLineAutoCut keeps them apart by shortening them, without moving a single note.
TEST_F(Engraving_LedgerLineTests, autoCutKeepsNeighboursApartWithoutMovingNotes)
{
    MasterScore* score = ScoreRW::readScore(LEDGERLINE_DATA_DIR + u"tightSixteenths.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    // off by default: scores keep the layout they had
    EXPECT_FALSE(score->style().styleB(Sid::ledgerLineAutoCut));
    const std::vector<double> gapsOff = neighbourGaps(score);
    const std::vector<double> positionsOff = chordRestSegmentPositions(score);
    ASSERT_FALSE(gapsOff.empty());
    EXPECT_LT(gapsOff.front(), 0.0);

    score->style().set(Sid::ledgerLineAutoCut, true);
    score->setLayoutAll();
    score->doLayout();

    // shortened lines end up half as far apart as the padding that spacing keeps between ledger lines
    const double padding = score->paddingTable().at(ElementType::LEDGER_LINE).at(ElementType::LEDGER_LINE);
    const std::vector<double> gapsOn = neighbourGaps(score);
    ASSERT_EQ(gapsOn.size(), gapsOff.size());
    EXPECT_NEAR(gapsOn.front(), 0.5 * padding, 1e-6);

    const std::vector<double> positionsOn = chordRestSegmentPositions(score);
    ASSERT_EQ(positionsOn.size(), positionsOff.size());
    for (size_t i = 0; i < positionsOn.size(); ++i) {
        EXPECT_NEAR(positionsOn[i], positionsOff[i], 1e-6);
    }

    // a second layout pass must not cut the lines once more
    score->setLayoutAll();
    score->doLayout();
    const std::vector<double> gapsAgain = neighbourGaps(score);
    ASSERT_EQ(gapsAgain.size(), gapsOn.size());
    for (size_t i = 0; i < gapsOn.size(); ++i) {
        EXPECT_NEAR(gapsAgain[i], gapsOn[i], 1e-6);
    }

    delete score;
}

// A length offset belongs to a staff line. It used to belong to the n-th ledger line of the chord,
// so a change to another note of the chord handed it to a different line.
TEST_F(Engraving_LedgerLineTests, offsetStaysOnItsStaffLine)
{
    MasterScore* score = ScoreRW::readScore(LEDGERLINE_DATA_DIR + u"chordBothSides.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Chord* chord = firstChord(score);
    ASSERT_TRUE(chord);
    ASSERT_EQ(chord->ledgerLines().size(), 4u);
    ASSERT_TRUE(ledgerLineAt(chord, -4));
    const double defaultLen = ledgerLineAt(chord, -4)->len();
    const double sp = chord->spatium();

    setOffsetLeft(score, ledgerLineAt(chord, -4), 3.0);
    EXPECT_NEAR(ledgerLineAt(chord, -4)->len(), defaultLen + 3.0 * sp, 1e-6);

    // A3 -> F3: a third ledger line appears below the staff
    moveNote(score, chord->downNote(), false);
    moveNote(score, chord->downNote(), false);
    ASSERT_EQ(chord->ledgerLines().size(), 5u);
    ASSERT_TRUE(ledgerLineAt(chord, -4));
    ASSERT_TRUE(ledgerLineAt(chord, 14));

    EXPECT_NEAR(ledgerLineAt(chord, -4)->len(), defaultLen + 3.0 * sp, 1e-6);
    for (int line : { -2, 10, 12, 14 }) {
        ASSERT_TRUE(ledgerLineAt(chord, line));
        EXPECT_NEAR(ledgerLineAt(chord, line)->len(), defaultLen, 1e-6) << "line " << line;
    }

    delete score;
}

// Layout deletes a ledger line as soon as no note needs it. Undo must neither touch the deleted line
// nor lose the offset: it comes back together with the line.
TEST_F(Engraving_LedgerLineTests, undoAfterLedgerLineWasRemoved)
{
    MasterScore* score = ScoreRW::readScore(LEDGERLINE_DATA_DIR + u"chordBothSides.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Chord* chord = firstChord(score);
    ASSERT_TRUE(chord);
    ASSERT_TRUE(ledgerLineAt(chord, -4));
    const double defaultLen = ledgerLineAt(chord, -4)->len();
    const double sp = chord->spatium();

    setOffsetLeft(score, ledgerLineAt(chord, -4), 2.0);

    // C6 -> B5: the upper ledger line is not needed any more
    moveNote(score, chord->upNote(), false);
    ASSERT_FALSE(ledgerLineAt(chord, -4));
    ASSERT_TRUE(ledgerLineAt(chord, -2));
    EXPECT_NEAR(ledgerLineAt(chord, -2)->len(), defaultLen, 1e-6);

    // undo the pitch change: the line is back, with its offset
    score->undoRedo(true, nullptr);
    ASSERT_TRUE(ledgerLineAt(chord, -4));
    EXPECT_NEAR(ledgerLineAt(chord, -4)->len(), defaultLen + 2.0 * sp, 1e-6);

    // undo the offset
    score->undoRedo(true, nullptr);
    ASSERT_TRUE(ledgerLineAt(chord, -4));
    EXPECT_NEAR(ledgerLineAt(chord, -4)->len(), defaultLen, 1e-6);
    EXPECT_TRUE(chord->ledgerLineOffsets().empty());

    // and redo both
    score->undoRedo(false, nullptr);
    EXPECT_NEAR(ledgerLineAt(chord, -4)->len(), defaultLen + 2.0 * sp, 1e-6);
    score->undoRedo(false, nullptr);
    EXPECT_FALSE(ledgerLineAt(chord, -4));
    EXPECT_EQ(chord->ledgerLineOffsets(-4).left, Spatium(2.0));

    delete score;
}

// Only the length offsets of a ledger line are recorded for undo, and on the chord:
// nothing on the undo stack may point at an object that layout deletes
TEST_F(Engraving_LedgerLineTests, otherPropertiesAreNotRecorded)
{
    MasterScore* score = ScoreRW::readScore(LEDGERLINE_DATA_DIR + u"chordBothSides.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Chord* chord = firstChord(score);
    ASSERT_TRUE(chord);
    LedgerLine* ledgerLine = ledgerLineAt(chord, -4);
    ASSERT_TRUE(ledgerLine);

    score->startCmd(TranslatableString::untranslatable("Engraving ledger line tests"));
    ledgerLine->undoChangeProperty(Pid::VISIBLE, false);
    ledgerLine->undoChangeProperty(Pid::Z, 1);
    score->endCmd();

    EXPECT_TRUE(ledgerLine->generated());
    EXPECT_FALSE(score->undoStack()->canUndo());

    delete score;
}

// Dragging a handle goes the same way: one undo step on the chord, and the line stays a generated element
TEST_F(Engraving_LedgerLineTests, gripDragIsUndoable)
{
    MasterScore* score = ScoreRW::readScore(LEDGERLINE_DATA_DIR + u"chordBothSides.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Chord* chord = firstChord(score);
    ASSERT_TRUE(chord);
    LedgerLine* ledgerLine = ledgerLineAt(chord, -4);
    ASSERT_TRUE(ledgerLine);
    const double defaultLen = ledgerLine->len();
    const double sp = chord->spatium();

    EditData ed;
    ed.curGrip = Grip::END;
    score->startCmd(TranslatableString::untranslatable("Engraving ledger line tests"));
    ledgerLine->startEditDrag(ed);
    ed.delta = PointF(1.0 * sp, 0.0);
    ledgerLine->editDrag(ed);
    ed.delta = PointF(0.5 * sp, 0.0);
    ledgerLine->editDrag(ed);
    ledgerLine->endEditDrag(ed);
    score->endCmd();

    ASSERT_TRUE(ledgerLineAt(chord, -4));
    EXPECT_EQ(chord->ledgerLineOffsets(-4).right, Spatium(1.5));
    EXPECT_NEAR(ledgerLineAt(chord, -4)->len(), defaultLen + 1.5 * sp, 1e-6);
    EXPECT_TRUE(ledgerLineAt(chord, -4)->generated());

    score->undoRedo(true, nullptr);
    EXPECT_TRUE(chord->ledgerLineOffsets().empty());
    EXPECT_NEAR(ledgerLineAt(chord, -4)->len(), defaultLen, 1e-6);
    EXPECT_FALSE(score->undoStack()->canUndo());

    score->undoRedo(false, nullptr);
    EXPECT_EQ(chord->ledgerLineOffsets(-4).right, Spatium(1.5));

    delete score;
}

TEST_F(Engraving_LedgerLineTests, clonedChordKeepsOffsets)
{
    MasterScore* score = ScoreRW::readScore(LEDGERLINE_DATA_DIR + u"chordBothSides.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Chord* chord = firstChord(score);
    ASSERT_TRUE(chord);
    ASSERT_TRUE(ledgerLineAt(chord, -4));
    setOffsetLeft(score, ledgerLineAt(chord, -4), 1.5);

    Chord* copy = toChord(chord->clone());
    EXPECT_EQ(copy->ledgerLineOffsets(-4).left, Spatium(1.5));
    EXPECT_EQ(copy->ledgerLineOffsets().size(), 1u);
    delete copy;

    delete score;
}

TEST_F(Engraving_LedgerLineTests, offsetsSurviveSaveAndReload)
{
    MasterScore* score = ScoreRW::readScore(LEDGERLINE_DATA_DIR + u"chordBothSides.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Chord* chord = firstChord(score);
    ASSERT_TRUE(chord);
    ASSERT_TRUE(ledgerLineAt(chord, -4));
    ASSERT_TRUE(ledgerLineAt(chord, 12));
    const double defaultLen = ledgerLineAt(chord, -4)->len();
    const double sp = chord->spatium();

    setOffsetLeft(score, ledgerLineAt(chord, -4), 3.0);
    score->startCmd(TranslatableString::untranslatable("Engraving ledger line tests"));
    ledgerLineAt(chord, 12)->undoChangeProperty(Pid::LEDGER_LINE_LENGTH_OFFSET_RIGHT, Spatium(-0.2));
    score->endCmd();

    const std::filesystem::path saveDir = std::filesystem::temp_directory_path();
    const String saveName = String::fromStdString((saveDir / "musescore-ledgerLineOffsets-test.mscx").string());
    ASSERT_TRUE(ScoreRW::saveScore(score, saveName));
    delete score;

    MasterScore* restored = ScoreRW::readScore(saveName, true);
    ASSERT_TRUE(restored);
    restored->doLayout();

    Chord* restoredChord = firstChord(restored);
    ASSERT_TRUE(restoredChord);
    EXPECT_EQ(restoredChord->ledgerLineOffsets().size(), 2u);
    EXPECT_EQ(restoredChord->ledgerLineOffsets(-4).left, Spatium(3.0));
    EXPECT_EQ(restoredChord->ledgerLineOffsets(12).right, Spatium(-0.2));
    ASSERT_TRUE(ledgerLineAt(restoredChord, -4));
    ASSERT_TRUE(ledgerLineAt(restoredChord, 12));
    EXPECT_NEAR(ledgerLineAt(restoredChord, -4)->len(), defaultLen + 3.0 * sp, 1e-6);
    EXPECT_NEAR(ledgerLineAt(restoredChord, 12)->len(), defaultLen - 0.2 * sp, 1e-6);

    delete restored;
}

// Earlier builds of the fork saved the offsets on the n-th <LedgerLine> of the chord
TEST_F(Engraving_LedgerLineTests, legacyOffsetsLandOnTheirStaffLine)
{
    MasterScore* score = ScoreRW::readScore(LEDGERLINE_DATA_DIR + u"legacyLedgerLines.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Chord* chord = firstChord(score);
    ASSERT_TRUE(chord);
    ASSERT_EQ(chord->ledgerLines().size(), 4u);

    // third in layout order: the outer one of the two lines above the staff
    EXPECT_EQ(chord->ledgerLineOffsets().size(), 1u);
    EXPECT_EQ(chord->ledgerLineOffsets(-4).left, Spatium(3.0));

    ASSERT_TRUE(ledgerLineAt(chord, -4));
    ASSERT_TRUE(ledgerLineAt(chord, -2));
    EXPECT_NEAR(ledgerLineAt(chord, -4)->len() - ledgerLineAt(chord, -2)->len(), 3.0 * chord->spatium(), 1e-6);

    delete score;
}
