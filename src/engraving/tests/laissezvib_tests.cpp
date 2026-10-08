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

#include "dom/editdata.h"
#include "dom/laissezvib.h"
#include "dom/masterscore.h"
#include "dom/mscore.h"
#include "dom/note.h"

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
