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

#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/page.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/system.h"
#include "engraving/tests/utils/scorerw.h"

#include "notation/view/linepreview.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;

//! NOTE Six measures in three systems of two (system breaks after the second and the fourth measure)
static const String LINE_PREVIEW_SCORE_PATH(u"data/line_preview.mscx");

class Notation_LinePreviewTests : public ::testing::Test
{
public:
    //! The measure by its number (from 1)
    static Measure* measure(MasterScore* score, int number)
    {
        Measure* result = score->firstMeasure();
        for (int i = 1; i < number && result; ++i) {
            result = result->nextMeasure();
        }

        return result;
    }

    //! Selects the measures from the first one to the last one, as clicks on them do
    static void selectMeasures(MasterScore* score, int first, int last)
    {
        score->deselectAll();
        score->select(measure(score, first), SelectType::RANGE, 0);
        score->select(measure(score, last), SelectType::RANGE, 0);
    }
};

/**
 * @TEST_CASE Whole_Systems_Are_Found
 * @details A range from the first measure of a system to the last measure of the same system,
 *          or of a later one, gives these systems
 */
TEST_F(Notation_LinePreviewTests, Whole_Systems_Are_Found)
{
    // [GIVEN] Three systems of two measures
    MasterScore* score = ScoreRW::readScore(LINE_PREVIEW_SCORE_PATH);
    ASSERT_TRUE(score);
    ASSERT_EQ(score->systems().size(), 3);

    // [WHEN] Select both measures of the second system
    selectMeasures(score, 3, 4);

    // [THEN] That system
    std::vector<const System*> lines = wholeSelectedLines(score);
    ASSERT_EQ(lines.size(), 1);
    EXPECT_EQ(lines.front(), score->systems().at(1));

    // [WHEN] Select the second and the third system, to the end of the score
    selectMeasures(score, 3, 6);

    // [THEN] Both, in their order
    lines = wholeSelectedLines(score);
    ASSERT_EQ(lines.size(), 2);
    EXPECT_EQ(lines.at(0), score->systems().at(1));
    EXPECT_EQ(lines.at(1), score->systems().at(2));

    delete score;
}

/**
 * @TEST_CASE Part_Of_A_System_Is_Not_Enough
 * @details No system is given when a system which has a part of the selection does not have all of it
 *          in the selection, or when the selection is not a range
 */
TEST_F(Notation_LinePreviewTests, Part_Of_A_System_Is_Not_Enough)
{
    // [GIVEN] Three systems of two measures
    MasterScore* score = ScoreRW::readScore(LINE_PREVIEW_SCORE_PATH);
    ASSERT_TRUE(score);
    ASSERT_EQ(score->systems().size(), 3);

    // [THEN] One measure of a system: none
    selectMeasures(score, 3, 3);
    EXPECT_TRUE(wholeSelectedLines(score).empty());

    // [THEN] From the last measure of a system to the end of the next one: none
    selectMeasures(score, 2, 4);
    EXPECT_TRUE(wholeSelectedLines(score).empty());

    // [THEN] A single rest: none
    score->deselectAll();
    Segment* segment = measure(score, 3)->first(SegmentType::ChordRest);
    ASSERT_TRUE(segment && segment->element(0));
    score->select(segment->element(0), SelectType::SINGLE, 0);
    EXPECT_TRUE(wholeSelectedLines(score).empty());

    // [THEN] Nothing selected: none
    score->deselectAll();
    EXPECT_TRUE(wholeSelectedLines(score).empty());

    delete score;
}

/**
 * @TEST_CASE Card_Covers_The_System_On_Its_Page
 * @details The card of a system has the whole system, with a margin, and does not go off its page
 */
TEST_F(Notation_LinePreviewTests, Card_Covers_The_System_On_Its_Page)
{
    // [GIVEN] Three systems of two measures
    MasterScore* score = ScoreRW::readScore(LINE_PREVIEW_SCORE_PATH);
    ASSERT_TRUE(score);
    ASSERT_EQ(score->systems().size(), 3);

    for (const System* system : score->systems()) {
        // [WHEN] Get the card of the system
        const RectF card = linePreviewCardRect(system);
        const RectF systemRect = system->canvasBoundingRect();

        // [THEN] It has the system, and more around it
        EXPECT_TRUE(card.contains(systemRect));
        EXPECT_LT(card.top(), systemRect.top());
        EXPECT_GT(card.bottom(), systemRect.bottom());

        // [THEN] It is on the page of the system
        EXPECT_TRUE(system->page()->canvasBoundingRect().contains(card));
    }

    delete score;
}
