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

#include <set>
#include <vector>

#include "utils/scorerw.h"

#include "dom/fitmusicoptions.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/system.h"
#include "dom/systemlock.h"

using namespace mu;
using namespace mu::engraving;

// Reuse the multi-system fixture score from the system locks tests.
static const String FIT_MUSIC_DATA_DIR("system_locks_data/");

class Engraving_FitMusicReflowTests : public ::testing::Test
{
protected:
    // Returns the set of locks as (startIndex, endIndex) measure-index pairs,
    // so two layouts can be compared independently of SystemLock identity.
    static std::set<std::pair<int, int> > lockRanges(Score* score)
    {
        std::set<std::pair<int, int> > ranges;
        for (const SystemLock* lock : score->systemLocks()->allLocks()) {
            ranges.insert({ lock->startMB()->index(), lock->endMB()->index() });
        }
        return ranges;
    }

    static void selectRange(MasterScore* score, MeasureBase* first, MeasureBase* last)
    {
        score->deselectAll();
        score->select(first, SelectType::RANGE);
        score->select(last, SelectType::RANGE);
    }
};

TEST_F(Engraving_FitMusicReflowTests, distributesRegionIntoTargetSystems)
{
    bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    MasterScore* score = ScoreRW::readScore(FIT_MUSIC_DATA_DIR + u"system_locks-1.mscx");
    ASSERT_TRUE(score);

    Measure* first = score->firstMeasure();
    ASSERT_TRUE(first);

    // Build a region of the first 6 measures.
    Measure* last = first;
    for (int i = 0; i < 5 && last->nextMeasure(); ++i) {
        last = last->nextMeasure();
    }

    selectRange(score, first, last);

    FitMusicOptions options;
    options.relativeMode = false;
    options.targetSystemCount = 2;
    options.smoothing = true;

    score->startCmd(TranslatableString::untranslatable("Fit music reflow tests"));
    bool changed = score->fitMusicReflow(options);
    score->endCmd();

    EXPECT_TRUE(changed);

    // The region's measures should occupy exactly 2 distinct, locked systems.
    std::set<const System*> systems;
    for (MeasureBase* mb = first; mb && mb->isBeforeOrEqual(last); mb = mb->next()) {
        if (mb->isMeasure() && mb->system()) {
            systems.insert(mb->system());
            EXPECT_TRUE(mb->system()->isLocked());
        }
    }
    EXPECT_EQ(systems.size(), size_t(2));

    // The region boundaries must be a system start and a system end.
    EXPECT_TRUE(first->isStartOfSystemLock());
    EXPECT_TRUE(last->isEndOfSystemLock());

    delete score;
    MScore::useRead302InTestMode = useRead302;
}

TEST_F(Engraving_FitMusicReflowTests, relativeModeZeroKeepsSystemCount)
{
    bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    MasterScore* score = ScoreRW::readScore(FIT_MUSIC_DATA_DIR + u"system_locks-1.mscx");
    ASSERT_TRUE(score);

    Measure* first = score->firstMeasure();
    ASSERT_TRUE(first);
    Measure* last = first;
    for (int i = 0; i < 7 && last->nextMeasure(); ++i) {
        last = last->nextMeasure();
    }

    selectRange(score, first, last);

    // Count distinct systems currently spanned by the region.
    std::set<const System*> systemsBefore;
    for (MeasureBase* mb = first; mb && mb->isBeforeOrEqual(last); mb = mb->next()) {
        if (mb->isMeasure() && mb->system()) {
            systemsBefore.insert(mb->system());
        }
    }

    FitMusicOptions options;
    options.relativeMode = true;
    options.relativeDelta = 0;
    options.smoothing = true;

    score->startCmd(TranslatableString::untranslatable("Fit music reflow tests"));
    score->fitMusicReflow(options);
    score->endCmd();

    std::set<const System*> systemsAfter;
    for (MeasureBase* mb = first; mb && mb->isBeforeOrEqual(last); mb = mb->next()) {
        if (mb->isMeasure() && mb->system()) {
            systemsAfter.insert(mb->system());
        }
    }

    EXPECT_EQ(systemsAfter.size(), systemsBefore.size());

    delete score;
    MScore::useRead302InTestMode = useRead302;
}

TEST_F(Engraving_FitMusicReflowTests, undoRestoresOriginalLocks)
{
    bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    MasterScore* score = ScoreRW::readScore(FIT_MUSIC_DATA_DIR + u"system_locks-1.mscx");
    ASSERT_TRUE(score);

    std::set<std::pair<int, int> > locksBefore = lockRanges(score);

    Measure* first = score->firstMeasure();
    ASSERT_TRUE(first);
    Measure* last = first;
    for (int i = 0; i < 5 && last->nextMeasure(); ++i) {
        last = last->nextMeasure();
    }

    selectRange(score, first, last);

    FitMusicOptions options;
    options.relativeMode = false;
    options.targetSystemCount = 3;
    options.smoothing = false;

    score->startCmd(TranslatableString::untranslatable("Fit music reflow tests"));
    score->fitMusicReflow(options);
    score->endCmd();

    EXPECT_NE(lockRanges(score), locksBefore);

    score->undoRedo(true, nullptr);
    score->doLayout();

    EXPECT_EQ(lockRanges(score), locksBefore);

    delete score;
    MScore::useRead302InTestMode = useRead302;
}

TEST_F(Engraving_FitMusicReflowTests, invalidTargetMakesNoChange)
{
    bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    MasterScore* score = ScoreRW::readScore(FIT_MUSIC_DATA_DIR + u"system_locks-1.mscx");
    ASSERT_TRUE(score);

    Measure* first = score->firstMeasure();
    ASSERT_TRUE(first);
    Measure* last = first->nextMeasure();
    ASSERT_TRUE(last);

    selectRange(score, first, last);

    // Region has 2 cells; a target of 5 systems is out of range.
    FitMusicOptions options;
    options.relativeMode = false;
    options.targetSystemCount = 5;
    options.smoothing = false;

    score->startCmd(TranslatableString::untranslatable("Fit music reflow tests"));
    bool changed = score->fitMusicReflow(options);
    score->endCmd();

    EXPECT_FALSE(changed);

    delete score;
    MScore::useRead302InTestMode = useRead302;
}
