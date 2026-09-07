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
#include "dom/engravingitem.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/rest.h"
#include "dom/segment.h"

#include "translation.h"
#include "types/types.h"

#include "utils/scorerw.h"

using namespace mu;
using namespace mu::engraving;

static const String REST_DATA_DIR("rest_data/");

class Engraving_RestTests : public ::testing::Test
{
};

static Rest* restAt(MasterScore* score, int chordRestIndex, track_idx_t track)
{
    Measure* measure = score->firstMeasure();
    Segment* segment = measure ? measure->first(SegmentType::ChordRest) : nullptr;
    for (int i = 0; i < chordRestIndex && segment; ++i) {
        segment = segment->next(SegmentType::ChordRest);
    }
    EngravingItem* item = segment ? segment->element(track) : nullptr;
    return item && item->isRest() ? toRest(item) : nullptr;
}

TEST_F(Engraving_RestTests, cmdMoveRestStepsOneSpatiumWithoutMovingOthers)
{
    MasterScore* score = ScoreRW::readScore(REST_DATA_DIR + u"multiVoiceRests.mscx");
    ASSERT_TRUE(score);

    Rest* moved = restAt(score, 0, 0);
    Rest* sameVoiceNext = restAt(score, 1, 0);
    Rest* otherVoice = restAt(score, 1, 1);
    ASSERT_TRUE(moved);
    ASSERT_TRUE(sameVoiceNext);
    ASSERT_TRUE(otherVoice);

    const double spatium = score->style().spatium();
    const double movedY0 = moved->pos().y();
    const double sameVoiceLdataY0 = sameVoiceNext->ldata()->pos().y();
    const double otherVoiceLdataY0 = otherVoice->ldata()->pos().y();

    score->startCmd(TranslatableString::untranslatable("test"));
    score->cmdMoveRest(moved, DirectionV::UP);
    score->endCmd();

    EXPECT_NEAR(moved->pos().y(), movedY0 - spatium, 0.01);
    EXPECT_NEAR(sameVoiceNext->ldata()->pos().y(), sameVoiceLdataY0, 0.01);
    EXPECT_NEAR(otherVoice->ldata()->pos().y(), otherVoiceLdataY0, 0.01);

    score->startCmd(TranslatableString::untranslatable("test"));
    score->cmdMoveRest(moved, DirectionV::UP);
    score->endCmd();

    EXPECT_NEAR(moved->pos().y(), movedY0 - 2.0 * spatium, 0.01);
    EXPECT_NEAR(sameVoiceNext->ldata()->pos().y(), sameVoiceLdataY0, 0.01);
    EXPECT_NEAR(otherVoice->ldata()->pos().y(), otherVoiceLdataY0, 0.01);

    delete score;
}

TEST_F(Engraving_RestTests, dragPreviewMatchesLayout)
{
    MasterScore* score = ScoreRW::readScore(REST_DATA_DIR + u"multiVoiceRests.mscx");
    ASSERT_TRUE(score);

    Rest* rest = restAt(score, 0, 0);
    ASSERT_TRUE(rest);

    const double spatium = score->style().spatium();
    const double ldataY0 = rest->ldata()->pos().y();
    const double posY0 = rest->pos().y();

    EditData ed;
    ed.evtDelta = PointF(0.0, -spatium);
    static_cast<EngravingItem*>(rest)->drag(ed);

    const double previewY = rest->pos().y();
    EXPECT_NEAR(previewY, posY0 - spatium, 0.01);
    EXPECT_NEAR(rest->ldata()->pos().y(), ldataY0, 0.01);

    score->setLayoutAll();
    score->doLayout();

    EXPECT_NEAR(rest->pos().y(), previewY, 0.01);
    EXPECT_NEAR(rest->ldata()->pos().y(), ldataY0, 0.01);

    delete score;
}

TEST_F(Engraving_RestTests, offsetSurvivesSaveAndReload)
{
    MasterScore* score = ScoreRW::readScore(REST_DATA_DIR + u"multiVoiceRests.mscx");
    ASSERT_TRUE(score);

    Rest* rest = restAt(score, 0, 0);
    ASSERT_TRUE(rest);

    score->startCmd(TranslatableString::untranslatable("test"));
    score->cmdMoveRest(rest, DirectionV::UP);
    score->cmdMoveRest(rest, DirectionV::UP);
    score->endCmd();

    const double y = rest->pos().y();
    const double offsetY = rest->offset().y();

    const String saveName = u"/tmp/rest_offset_roundtrip.mscx";
    ASSERT_TRUE(ScoreRW::saveScore(score, saveName));

    MasterScore* reloaded = ScoreRW::readScore(saveName, true);
    ASSERT_TRUE(reloaded);

    Rest* reloadedRest = restAt(reloaded, 0, 0);
    ASSERT_TRUE(reloadedRest);
    EXPECT_NEAR(reloadedRest->offset().y(), offsetY, 0.01);
    EXPECT_NEAR(reloadedRest->pos().y(), y, 0.01);

    delete reloaded;
    delete score;
}
