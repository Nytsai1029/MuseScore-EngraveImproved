/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "dom/chord.h"
#include "dom/editdata.h"
#include "dom/factory.h"
#include "dom/laissezvib.h"
#include "dom/layoutbreak.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/mscore.h"
#include "dom/note.h"
#include "dom/segment.h"
#include "dom/staff.h"
#include "dom/tie.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

static const String LAISSEZVIB_DATA_DIR(u"exchangevoices_data/");

namespace {
void findLaissezVibNote(void* data, EngravingItem* item)
{
    Note** note = static_cast<Note**>(data);
    if (*note || !item->isNote()) {
        return;
    }

    Note* candidate = toNote(item);
    if (candidate->laissezVib()) {
        *note = candidate;
    }
}

void findRegularTieNote(void* data, EngravingItem* item)
{
    Note** note = static_cast<Note**>(data);
    if (*note || !item->isNote()) {
        return;
    }

    Note* candidate = toNote(item);
    if (candidate->tieFor() && candidate->tieFor()->type() == ElementType::TIE) {
        *note = candidate;
    }
}

void expectNear(const PointF& actual, const PointF& expected)
{
    EXPECT_NEAR(actual.x(), expected.x(), 1e-9);
    EXPECT_NEAR(actual.y(), expected.y(), 1e-9);
}

// With autoplace off, each shoulder handle moves on its own and the middle handle moves both by the full drag
void expectShoulderHandlesMoveFreely(TieSegment* segment)
{
    segment->setAutoplace(false);
    segment->consolidateAdjustmentOffsetIntoUserOffset();

    EditData editData;

    editData.curGrip = Grip::BEZIER1;
    editData.delta = PointF(2.0, -1.0);
    PointF bezier1Before = segment->ups(Grip::BEZIER1).off;
    PointF bezier2Before = segment->ups(Grip::BEZIER2).off;
    segment->editDrag(editData);
    expectNear(segment->ups(Grip::BEZIER1).off, bezier1Before + PointF(2.0, -1.0));
    expectNear(segment->ups(Grip::BEZIER2).off, bezier2Before);

    editData.curGrip = Grip::BEZIER2;
    editData.delta = PointF(-1.5, 2.5);
    bezier1Before = segment->ups(Grip::BEZIER1).off;
    bezier2Before = segment->ups(Grip::BEZIER2).off;
    segment->editDrag(editData);
    expectNear(segment->ups(Grip::BEZIER1).off, bezier1Before);
    expectNear(segment->ups(Grip::BEZIER2).off, bezier2Before + PointF(-1.5, 2.5));

    editData.curGrip = Grip::SHOULDER;
    editData.delta = PointF(3.0, -4.0);
    bezier1Before = segment->ups(Grip::BEZIER1).off;
    bezier2Before = segment->ups(Grip::BEZIER2).off;
    segment->editDrag(editData);
    expectNear(segment->ups(Grip::BEZIER1).off, bezier1Before + PointF(3.0, -4.0));
    expectNear(segment->ups(Grip::BEZIER2).off, bezier2Before + PointF(3.0, -4.0));
    EXPECT_TRUE(segment->ups(Grip::SHOULDER).off.isNull());
}

void addLineBreak(Score* score, Measure* measure)
{
    LayoutBreak* lb = Factory::createLayoutBreak(measure);
    lb->setLayoutBreakType(LayoutBreakType::LINE);
    lb->setTrack(0);
    lb->setParent(measure);
    score->undoAddElement(lb);
}
}

class Engraving_LaissezVibTests : public ::testing::Test
{
protected:
    // The fixture is a 4.60 score containing LaissezVib/PartialTie, which the 3.02 reader cannot read.
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

TEST_F(Engraving_LaissezVibTests, gripsEditLikeTies)
{
    MasterScore* score = ScoreRW::readScore(LAISSEZVIB_DATA_DIR + u"exchangevoices-range.mscx");
    ASSERT_TRUE(score);

    Note* note = nullptr;
    score->scanElements(&note, findLaissezVibNote, true);
    ASSERT_TRUE(note);
    ASSERT_TRUE(note->laissezVib());

    LaissezVibSegment* segment = note->laissezVib()->frontSegment();
    ASSERT_TRUE(segment);
    segment->consolidateAdjustmentOffsetIntoUserOffset();

    // Same handles as a tie; the default one moves the whole line
    EXPECT_EQ(segment->gripsCount(), int(Grip::GRIPS));
    EXPECT_EQ(segment->initialEditModeGrip(), Grip::END);
    EXPECT_EQ(segment->defaultGrip(), Grip::DRAG);

    // No view: a laissez vibrer has no end note, so dragging an end must not look for a note to re-anchor to
    EditData editData;

    // The whole line moves in both directions
    editData.curGrip = Grip::DRAG;
    editData.delta = PointF(1.0, -6.0);
    const PointF offsetBefore = segment->offset();
    segment->editDrag(editData);
    EXPECT_DOUBLE_EQ(segment->offset().x(), offsetBefore.x() + 1.0);
    EXPECT_DOUBLE_EQ(segment->offset().y(), offsetBefore.y() - 6.0);

    // The shoulder handle only raises or lowers both shoulders; the line is still horizontal here
    editData.curGrip = Grip::SHOULDER;
    editData.delta = PointF(3.0, -4.0);
    const PointF bezier1Before = segment->ups(Grip::BEZIER1).off;
    const PointF bezier2Before = segment->ups(Grip::BEZIER2).off;
    segment->editDrag(editData);
    EXPECT_NEAR(segment->ups(Grip::BEZIER1).off.x(), bezier1Before.x(), 1e-9);
    EXPECT_NEAR(segment->ups(Grip::BEZIER1).off.y(), bezier1Before.y() - 4.0, 1e-9);
    EXPECT_NEAR(segment->ups(Grip::BEZIER2).off.x(), bezier2Before.x(), 1e-9);
    EXPECT_NEAR(segment->ups(Grip::BEZIER2).off.y(), bezier2Before.y() - 4.0, 1e-9);
    EXPECT_TRUE(segment->ups(Grip::SHOULDER).off.isNull());

    // One shoulder handle is mirrored onto the other
    editData.curGrip = Grip::BEZIER1;
    editData.delta = PointF(2.0, -1.0);
    const PointF mirroredBefore = segment->ups(Grip::BEZIER2).off;
    segment->editDrag(editData);
    EXPECT_NEAR(segment->ups(Grip::BEZIER1).off.x(), bezier1Before.x() + 2.0, 1e-9);
    EXPECT_NEAR(segment->ups(Grip::BEZIER1).off.y(), bezier1Before.y() - 5.0, 1e-9);
    EXPECT_NEAR(segment->ups(Grip::BEZIER2).off.x(), mirroredBefore.x() - 2.0, 1e-9);
    EXPECT_NEAR(segment->ups(Grip::BEZIER2).off.y(), mirroredBefore.y() - 1.0, 1e-9);

    // The ends move freely
    editData.curGrip = Grip::START;
    editData.delta = PointF(1.5, 7.0);
    const PointF startOffsetBefore = segment->ups(Grip::START).off;
    segment->editDrag(editData);
    EXPECT_DOUBLE_EQ(segment->ups(Grip::START).off.x(), startOffsetBefore.x() + 1.5);
    EXPECT_DOUBLE_EQ(segment->ups(Grip::START).off.y(), startOffsetBefore.y() + 7.0);

    editData.curGrip = Grip::END;
    editData.delta = PointF(2.5, -5.0);
    const PointF endOffsetBefore = segment->ups(Grip::END).off;
    segment->editDrag(editData);
    EXPECT_DOUBLE_EQ(segment->ups(Grip::END).off.x(), endOffsetBefore.x() + 2.5);
    EXPECT_DOUBLE_EQ(segment->ups(Grip::END).off.y(), endOffsetBefore.y() - 5.0);

    delete score;
}

TEST_F(Engraving_LaissezVibTests, tieShoulderHandlesMoveFreelyWithoutAutoplace)
{
    MasterScore* score = ScoreRW::readScore(LAISSEZVIB_DATA_DIR + u"exchangevoices-range.mscx");
    ASSERT_TRUE(score);

    Note* note = nullptr;
    score->scanElements(&note, findRegularTieNote, true);
    ASSERT_TRUE(note);

    TieSegment* segment = note->tieFor()->frontSegment();
    ASSERT_TRUE(segment);
    segment->consolidateAdjustmentOffsetIntoUserOffset();

    // With autoplace on, a shoulder handle is still mirrored onto the other one
    ASSERT_TRUE(segment->autoplace());
    EditData editData;
    editData.curGrip = Grip::BEZIER1;
    editData.delta = PointF(0.0, -2.0);
    const PointF bezier2Before = segment->ups(Grip::BEZIER2).off;
    segment->editDrag(editData);
    EXPECT_FALSE(segment->ups(Grip::BEZIER2).off == bezier2Before);

    expectShoulderHandlesMoveFreely(segment);

    delete score;
}

TEST_F(Engraving_LaissezVibTests, shoulderHandlesMoveFreelyWithoutAutoplace)
{
    MasterScore* score = ScoreRW::readScore(LAISSEZVIB_DATA_DIR + u"exchangevoices-range.mscx");
    ASSERT_TRUE(score);

    Note* note = nullptr;
    score->scanElements(&note, findLaissezVibNote, true);
    ASSERT_TRUE(note);

    LaissezVibSegment* segment = note->laissezVib()->frontSegment();
    ASSERT_TRUE(segment);
    expectShoulderHandlesMoveFreely(segment);

    delete score;
}

TEST_F(Engraving_LaissezVibTests, crossSystemTieEndKeepsUserOffsetsWithoutAutoplace)
{
    MasterScore* score = ScoreRW::readScore(u"join_data/join04.mscx");
    ASSERT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ASSERT_TRUE(m1);
    Measure* m2 = m1->nextMeasure();
    ASSERT_TRUE(m2);
    addLineBreak(score, m1);
    score->doLayout();
    ASSERT_NE(m1->system(), m2->system());

    Segment* firstSeg = m2->first(SegmentType::ChordRest);
    ASSERT_TRUE(firstSeg);
    EngravingItem* firstItem = firstSeg->element(0);
    ASSERT_TRUE(firstItem && firstItem->isChord());
    Tie* tie = toChord(firstItem)->upNote()->tieBack();
    ASSERT_TRUE(tie);
    ASSERT_EQ(tie->spannerSegments().size(), 2u);

    TieSegment* endSegment = tie->backSegment();
    endSegment->setAutoplace(false);
    score->doLayout();

    // Move the whole arc so that its middle sits on the middle staff line, where the automatic arc
    // correction would kick in. The shoulder handles follow the end points, so only those are moved.
    const double spatium = endSegment->spatium();
    const double lineDist = score->staff(endSegment->vStaffIdx())->lineDistance(endSegment->tick()) * spatium;
    const RectF bbox = endSegment->ldata()->bbox();
    const int upSign = tie->up() ? -1 : 1;
    const double yOuter = tie->up() ? bbox.top() : bbox.bottom();
    const double yMid = yOuter - upSign * endSegment->ldata()->midThickness();
    const PointF shift(0.0, 2.0 * lineDist - yMid);
    for (Grip grip : { Grip::START, Grip::END }) {
        endSegment->ups(grip).off += shift;
    }
    score->doLayout();

    // No automatic adjustment is left behind for the next drag to turn into user offsets
    for (int i = 0; i < int(Grip::GRIPS); ++i) {
        EXPECT_TRUE(endSegment->adjustmentOffset(static_cast<Grip>(i)).isNull()) << "grip " << i;
    }

    // Drags add up across layouts, and the other shoulder handle stays put
    const PointF bezier1Before = endSegment->ups(Grip::BEZIER1).off;
    const PointF bezier2Before = endSegment->ups(Grip::BEZIER2).off;
    EditData editData;
    editData.curGrip = Grip::BEZIER1;
    editData.delta = PointF(0.0, -1.0);
    endSegment->editDrag(editData);
    score->doLayout();
    endSegment->editDrag(editData);
    score->doLayout();
    expectNear(endSegment->ups(Grip::BEZIER1).off, bezier1Before + PointF(0.0, -2.0));
    expectNear(endSegment->ups(Grip::BEZIER2).off, bezier2Before);

    delete score;
}
