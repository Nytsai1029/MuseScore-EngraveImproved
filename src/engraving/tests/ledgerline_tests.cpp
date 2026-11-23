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
#include <limits>
#include <map>
#include <set>
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

static void toggleVisible(MasterScore* score, LedgerLine* ledgerLine)
{
    score->startCmd(TranslatableString::untranslatable("Engraving ledger line tests"));
    score->select(ledgerLine, SelectType::SINGLE, 0);
    score->cmdToggleVisible();
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

// The ledger lines of the first voice that follow each other at the same height of the same system,
// from left to right. Layout makes the lines anew: ask again after every layout.
static std::vector<std::vector<const LedgerLine*> > ledgerLineRows(MasterScore* score)
{
    std::map<std::tuple<const System*, staff_idx_t, int>, std::vector<const LedgerLine*> > rows;
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(SegmentType::ChordRest); segment;
             segment = segment->next(SegmentType::ChordRest)) {
            EngravingItem* item = segment->element(0);
            if (!item || !item->isChord()) {
                continue;
            }
            for (const LedgerLine* ledgerLine : toChord(item)->ledgerLines()) {
                const auto row = std::make_tuple(measure->system(), ledgerLine->staffIdx(), ledgerLine->line());
                rows[row].push_back(ledgerLine);
            }
        }
    }

    std::vector<std::vector<const LedgerLine*> > result;
    for (auto& row : rows) {
        std::vector<const LedgerLine*>& lines = row.second;
        std::sort(lines.begin(), lines.end(), [](const LedgerLine* a, const LedgerLine* b) {
            return a->pageX() < b->pageX();
        });
        result.push_back(lines);
    }
    return result;
}

// The ends of a ledger line at its automatic length: before the automatic cut, without the user's offsets
static double automaticLeft(const LedgerLine* ledgerLine)
{
    const LedgerLine::LayoutData* ldata = ledgerLine->ldata();
    return ledgerLine->pageX() - ldata->autoCutLeft + ldata->userExtensionLeft;
}

static double automaticRight(const LedgerLine* ledgerLine)
{
    const LedgerLine::LayoutData* ldata = ledgerLine->ldata();
    return ledgerLine->pageX() + ledgerLine->len() + ldata->autoCutRight - ldata->userExtensionRight;
}

// The distance between the noteheads of two ledger lines that follow each other
static double headsDistance(const LedgerLine* left, const LedgerLine* right)
{
    return (automaticLeft(right) + right->ldata()->defaultExtension)
           - (automaticRight(left) - left->ldata()->defaultExtension);
}

static double meanMag(const LedgerLine* a, const LedgerLine* b)
{
    return 0.5 * (a->chord()->mag() + b->chord()->mag());
}

// A side of a ledger line that the automatic shortening has cut
struct CutSide {
    double extension = 0.0; // how far the line reaches past the noteheads when it is not cut
    double kept = 0.0;      // what the cut has left of that
    double mag = 1.0;       // the size of the chord
};

static std::vector<CutSide> cutSides(MasterScore* score)
{
    std::vector<CutSide> sides;
    for (const std::vector<const LedgerLine*>& row : ledgerLineRows(score)) {
        for (const LedgerLine* ledgerLine : row) {
            const LedgerLine::LayoutData* ldata = ledgerLine->ldata();
            for (double cut : { ldata->autoCutLeft, ldata->autoCutRight }) {
                if (cut > 0.0) {
                    CutSide side;
                    side.extension = ldata->defaultExtension;
                    side.kept = ldata->defaultExtension - cut;
                    side.mag = ledgerLine->chord()->mag();
                    sides.push_back(side);
                }
            }
        }
    }
    return sides;
}

// Sid::ledgerLineAutoCutMinGap and Sid::ledgerLineAutoCutMinExtension are 0 by default: the share of the padding
// alone decides how far the lines are cut, as it did before these two values existed
TEST_F(Engraving_LedgerLineTests, autoCutMinimumsDoNothingByDefault)
{
    MasterScore* score = ScoreRW::readScore(LEDGERLINE_DATA_DIR + u"tightSixteenths.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    EXPECT_DOUBLE_EQ(score->style().styleS(Sid::ledgerLineAutoCutMinGap).val(), 0.0);
    EXPECT_DOUBLE_EQ(score->style().styleS(Sid::ledgerLineAutoCutMinExtension).val(), 0.0);
    EXPECT_DOUBLE_EQ(score->style().styleD(Sid::ledgerLineAutoCutGapRatio), 0.5);

    score->style().set(Sid::ledgerLineAutoCut, true);
    score->setLayoutAll();
    score->doLayout();

    // what autoCutKeepsNeighboursApartWithoutMovingNotes expects still holds...
    const double padding = score->paddingTable().at(ElementType::LEDGER_LINE).at(ElementType::LEDGER_LINE);
    const std::vector<double> gaps = neighbourGaps(score);
    ASSERT_FALSE(gaps.empty());
    EXPECT_NEAR(gaps.front(), 0.5 * padding, 1e-6);

    // ...and so does the cut of every line: the room between the noteheads, less the gap, is shared between
    // the two lines in proportion to their extensions, and the rest of the extensions is cut
    size_t shortenedPairs = 0;
    for (const std::vector<const LedgerLine*>& row : ledgerLineRows(score)) {
        ASSERT_FALSE(row.empty());
        EXPECT_DOUBLE_EQ(row.front()->ldata()->autoCutLeft, 0.0);
        EXPECT_DOUBLE_EQ(row.back()->ldata()->autoCutRight, 0.0);

        for (size_t i = 0; i + 1 < row.size(); ++i) {
            const LedgerLine* a = row[i];
            const LedgerLine* b = row[i + 1];
            const double aExtension = a->ldata()->defaultExtension;
            const double bExtension = b->ldata()->defaultExtension;
            ASSERT_GT(aExtension + bExtension, 0.0);

            const double gap = 0.5 * padding * meanMag(a, b);
            double kept = 1.0;
            if (automaticLeft(b) - automaticRight(a) < gap) {
                const double room = std::max(headsDistance(a, b) - gap, 0.0);
                kept = std::min(room / (aExtension + bExtension), 1.0);
                ++shortenedPairs;
            }
            const double aCut = a->ldata()->autoCutRight;
            const double bCut = b->ldata()->autoCutLeft;
            EXPECT_NEAR(aCut, aExtension * (1.0 - kept), 1e-6) << "line " << a->line() << " #" << i;
            EXPECT_NEAR(bCut, bExtension * (1.0 - kept), 1e-6) << "line " << a->line() << " #" << i;
        }
    }
    EXPECT_GT(shortenedPairs, 0u);

    delete score;
}

// Sid::ledgerLineAutoCutMinGap: where it is more than the share of the padding, it is the gap between shortened lines
TEST_F(Engraving_LedgerLineTests, autoCutMinGapIsTheGapBetweenShortenedNeighbours)
{
    MasterScore* score = ScoreRW::readScore(LEDGERLINE_DATA_DIR + u"tightSixteenths.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    const double sp = score->style().spatium();
    const double padding = score->paddingTable().at(ElementType::LEDGER_LINE).at(ElementType::LEDGER_LINE);
    const double paddingGap = score->style().styleD(Sid::ledgerLineAutoCutGapRatio) * padding;
    const std::vector<double> positionsOff = chordRestSegmentPositions(score);

    // The gap has to fit between the noteheads: lines cannot be cut any further than down to their noteheads.
    // So take a minimum gap half-way between the share of the padding and the closest noteheads of the score.
    double closestHeads = std::numeric_limits<double>::max();
    for (const std::vector<const LedgerLine*>& row : ledgerLineRows(score)) {
        for (size_t i = 0; i + 1 < row.size(); ++i) {
            ASSERT_DOUBLE_EQ(meanMag(row[i], row[i + 1]), 1.0);
            closestHeads = std::min(closestHeads, headsDistance(row[i], row[i + 1]));
        }
    }
    ASSERT_GT(closestHeads, paddingGap + 0.02 * sp);
    const double minGap = 0.5 * (paddingGap + closestHeads);

    score->style().set(Sid::ledgerLineAutoCut, true);
    score->style().set(Sid::ledgerLineAutoCutMinGap, Spatium(minGap / sp));
    score->setLayoutAll();
    score->doLayout();

    size_t shortenedPairs = 0;
    for (const std::vector<const LedgerLine*>& row : ledgerLineRows(score)) {
        for (size_t i = 0; i + 1 < row.size(); ++i) {
            const LedgerLine* a = row[i];
            const LedgerLine* b = row[i + 1];
            const double gap = b->pageX() - (a->pageX() + a->len());
            if (a->ldata()->autoCutRight > 0.0 || b->ldata()->autoCutLeft > 0.0) {
                ++shortenedPairs;
                EXPECT_NEAR(gap, minGap, 1e-6 * sp) << "line " << a->line() << " #" << i;
            } else {
                // far enough apart as they are
                EXPECT_GE(gap, minGap - 1e-6 * sp) << "line " << a->line() << " #" << i;
            }
        }
    }
    EXPECT_GT(shortenedPairs, 0u);

    const std::vector<double> gaps = neighbourGaps(score);
    ASSERT_FALSE(gaps.empty());
    EXPECT_NEAR(gaps.front(), minGap, 1e-6 * sp);
    EXPECT_GT(gaps.front(), paddingGap + 0.01 * sp);

    // still without moving a single note
    const std::vector<double> positionsOn = chordRestSegmentPositions(score);
    ASSERT_EQ(positionsOn.size(), positionsOff.size());
    for (size_t i = 0; i < positionsOn.size(); ++i) {
        EXPECT_NEAR(positionsOn[i], positionsOff[i], 1e-6);
    }

    // a minimum gap below the share of the padding changes nothing
    score->style().set(Sid::ledgerLineAutoCutMinGap, Spatium(0.5 * paddingGap / sp));
    score->setLayoutAll();
    score->doLayout();
    const std::vector<double> gapsSmallMinimum = neighbourGaps(score);
    ASSERT_FALSE(gapsSmallMinimum.empty());
    EXPECT_NEAR(gapsSmallMinimum.front(), paddingGap, 1e-6 * sp);

    delete score;
}

// Sid::ledgerLineAutoCutMinExtension: a shortened line keeps that much of its extension on a cut side - or all of
// its extension, if it has less - even where the lines then come closer than the gap
TEST_F(Engraving_LedgerLineTests, autoCutMinExtensionIsKeptOnCutSides)
{
    MasterScore* score = ScoreRW::readScore(LEDGERLINE_DATA_DIR + u"tightSixteenths.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    const double sp = score->style().spatium();
    const double padding = score->paddingTable().at(ElementType::LEDGER_LINE).at(ElementType::LEDGER_LINE);
    const double paddingGap = score->style().styleD(Sid::ledgerLineAutoCutGapRatio) * padding;
    const std::vector<double> gapsOff = neighbourGaps(score);

    score->style().set(Sid::ledgerLineAutoCut, true);
    score->setLayoutAll();
    score->doLayout();

    // without a minimum: the side of which the gap leaves least
    const std::vector<CutSide> sidesWithoutMinimum = cutSides(score);
    ASSERT_FALSE(sidesWithoutMinimum.empty());
    const CutSide shortest = *std::min_element(sidesWithoutMinimum.begin(), sidesWithoutMinimum.end(),
                                               [](const CutSide& a, const CutSide& b) {
        return a.kept < b.kept;
    });
    ASSERT_LT(shortest.kept + 0.02 * sp, shortest.extension);

    // a minimum which that side does not reach, and which is less than its whole extension
    const double minExtension = 0.5 * (shortest.kept + shortest.extension) / shortest.mag;
    score->style().set(Sid::ledgerLineAutoCutMinExtension, Spatium(minExtension / sp));
    score->setLayoutAll();
    score->doLayout();

    const std::vector<CutSide> sides = cutSides(score);
    ASSERT_FALSE(sides.empty());
    double leastKept = std::numeric_limits<double>::max();
    for (const CutSide& side : sides) {
        EXPECT_GE(side.kept, std::min(side.extension, minExtension * side.mag) - 1e-6 * sp);
        leastKept = std::min(leastKept, side.kept);
    }
    // the sides that kept less are cut down to the minimum, and not less
    EXPECT_NEAR(leastKept, minExtension * shortest.mag, 1e-6 * sp);

    // those lines are closer than the gap now
    const std::vector<double> gaps = neighbourGaps(score);
    ASSERT_FALSE(gaps.empty());
    EXPECT_LT(gaps.front(), paddingGap - 0.01 * sp);

    // a minimum that is more than the extension: a line keeps all it has, so no line is shortened at all
    score->style().set(Sid::ledgerLineAutoCutMinExtension, Spatium(10.0));
    score->setLayoutAll();
    score->doLayout();

    EXPECT_TRUE(cutSides(score).empty());
    const std::vector<double> gapsUncut = neighbourGaps(score);
    ASSERT_EQ(gapsUncut.size(), gapsOff.size());
    for (size_t i = 0; i < gapsUncut.size(); ++i) {
        EXPECT_NEAR(gapsUncut[i], gapsOff[i], 1e-6);
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

// Only the length offsets and the visibility of a ledger line are recorded for undo, and on the chord:
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

    toggleVisible(score, ledgerLineAt(chord, 12));

    Chord* copy = toChord(chord->clone());
    EXPECT_EQ(copy->ledgerLineOffsets(-4).left, Spatium(1.5));
    EXPECT_EQ(copy->ledgerLineOffsets().size(), 1u);
    EXPECT_EQ(copy->hiddenLedgerLines(), std::set<int> { 12 });
    delete copy;

    delete score;
}

// One ledger line can be made invisible on its own (V, or Visible in the inspector); the setting is kept on the chord
TEST_F(Engraving_LedgerLineTests, hiddenLineIsInvisibleAndUndoable)
{
    MasterScore* score = ScoreRW::readScore(LEDGERLINE_DATA_DIR + u"chordBothSides.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Chord* chord = firstChord(score);
    ASSERT_TRUE(chord);
    ASSERT_TRUE(ledgerLineAt(chord, -4));
    const size_t lineCount = chord->ledgerLines().size();

    toggleVisible(score, ledgerLineAt(chord, -4));

    EXPECT_EQ(chord->hiddenLedgerLines(), std::set<int> { -4 });
    EXPECT_EQ(chord->ledgerLines().size(), lineCount);
    ASSERT_TRUE(ledgerLineAt(chord, -4));
    EXPECT_FALSE(ledgerLineAt(chord, -4)->visible());
    EXPECT_TRUE(ledgerLineAt(chord, -4)->generated());
    for (int line : { -2, 10, 12 }) {
        ASSERT_TRUE(ledgerLineAt(chord, line));
        EXPECT_TRUE(ledgerLineAt(chord, line)->visible());
    }

    score->undoRedo(true, nullptr);
    EXPECT_TRUE(chord->hiddenLedgerLines().empty());
    EXPECT_TRUE(ledgerLineAt(chord, -4)->visible());
    EXPECT_FALSE(score->undoStack()->canUndo());

    score->undoRedo(false, nullptr);
    EXPECT_FALSE(ledgerLineAt(chord, -4)->visible());

    // toggling the hidden line shows it again
    toggleVisible(score, ledgerLineAt(chord, -4));
    EXPECT_TRUE(chord->hiddenLedgerLines().empty());
    EXPECT_TRUE(ledgerLineAt(chord, -4)->visible());

    delete score;
}

// Like the offsets, the setting stays with its staff line while the line comes and goes with the notes
TEST_F(Engraving_LedgerLineTests, hiddenLineStaysOnItsStaffLine)
{
    MasterScore* score = ScoreRW::readScore(LEDGERLINE_DATA_DIR + u"chordBothSides.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Chord* chord = firstChord(score);
    ASSERT_TRUE(chord);
    ASSERT_TRUE(ledgerLineAt(chord, -4));

    toggleVisible(score, ledgerLineAt(chord, -4));

    // C6 -> B5: the upper ledger line is not needed any more, the one under it stays visible
    moveNote(score, chord->upNote(), false);
    ASSERT_FALSE(ledgerLineAt(chord, -4));
    ASSERT_TRUE(ledgerLineAt(chord, -2));
    EXPECT_TRUE(ledgerLineAt(chord, -2)->visible());

    // undo the pitch change: the line is back, still hidden
    score->undoRedo(true, nullptr);
    ASSERT_TRUE(ledgerLineAt(chord, -4));
    EXPECT_FALSE(ledgerLineAt(chord, -4)->visible());
    EXPECT_TRUE(ledgerLineAt(chord, -2)->visible());

    delete score;
}

TEST_F(Engraving_LedgerLineTests, hiddenLinesSurviveSaveAndReload)
{
    MasterScore* score = ScoreRW::readScore(LEDGERLINE_DATA_DIR + u"chordBothSides.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Chord* chord = firstChord(score);
    ASSERT_TRUE(chord);
    ASSERT_TRUE(ledgerLineAt(chord, -4));
    ASSERT_TRUE(ledgerLineAt(chord, 12));

    toggleVisible(score, ledgerLineAt(chord, -4));
    toggleVisible(score, ledgerLineAt(chord, 12));

    const std::filesystem::path saveDir = std::filesystem::temp_directory_path();
    const String saveName = String::fromStdString((saveDir / "musescore-hiddenLedgerLines-test.mscx").string());
    ASSERT_TRUE(ScoreRW::saveScore(score, saveName));
    delete score;

    MasterScore* restored = ScoreRW::readScore(saveName, true);
    ASSERT_TRUE(restored);
    restored->doLayout();

    Chord* restoredChord = firstChord(restored);
    ASSERT_TRUE(restoredChord);
    EXPECT_EQ(restoredChord->hiddenLedgerLines(), (std::set<int> { -4, 12 }));
    for (int line : { -4, -2, 10, 12 }) {
        ASSERT_TRUE(ledgerLineAt(restoredChord, line));
        EXPECT_EQ(ledgerLineAt(restoredChord, line)->visible(), line != -4 && line != 12);
    }

    delete restored;
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
