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
};

TEST_F(Engraving_LaissezVibTests, gripsAreHorizontal)
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

    EXPECT_EQ(segment->gripsCount(), 2);
    EXPECT_EQ(segment->initialEditModeGrip(), Grip::END);
    EXPECT_EQ(segment->defaultGrip(), Grip::END);

    EditData editData;
    editData.curGrip = Grip::START;
    editData.delta = PointF(1.5, 7.0);

    const PointF startOffsetBefore = segment->ups(Grip::START).off;
    segment->editDrag(editData);
    EXPECT_DOUBLE_EQ(segment->ups(Grip::START).off.x(), startOffsetBefore.x() + 1.5);
    EXPECT_DOUBLE_EQ(segment->ups(Grip::START).off.y(), startOffsetBefore.y());

    editData.curGrip = Grip::END;
    editData.delta = PointF(2.5, -5.0);

    const PointF endOffsetBefore = segment->ups(Grip::END).off;
    segment->editDrag(editData);
    EXPECT_DOUBLE_EQ(segment->ups(Grip::END).off.x(), endOffsetBefore.x() + 2.5);
    EXPECT_DOUBLE_EQ(segment->ups(Grip::END).off.y(), endOffsetBefore.y());

    delete score;
}
