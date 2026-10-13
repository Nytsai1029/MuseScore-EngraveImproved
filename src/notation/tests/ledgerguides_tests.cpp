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

#include "engraving/dom/chord.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/page.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/stafflines.h"
#include "engraving/tests/utils/scorerw.h"

#include "notation/internal/ledgerguides.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;

static const String LEDGER_GUIDES_SCORE_PATH(u"data/ledger_guides.mscx");

static constexpr int HIGH_C_PITCH = 84;  // on the second ledger line above the treble staff
static constexpr int HIGH_E_PITCH = 88;  // on the third one
static constexpr int E_TPC = 18;

class Notation_LedgerGuidesTests : public ::testing::Test
{
public:
    struct StaffGuides {
        std::vector<LedgerGuide> above;     // from the staff outwards
        std::vector<LedgerGuide> below;
        double top = 0.0;
        double bottom = 0.0;
        double lineDistance = 0.0;
    };

    //! The guides of a staff of the first system, which is the only one of the score
    static StaffGuides guidesOfStaff(MasterScore* score, staff_idx_t staffIdx)
    {
        StaffGuides result;

        const StaffLines* staffLines = score->firstMeasure()->staffLines(staffIdx);
        const std::vector<LineF>& lines = staffLines->lines();

        result.top = staffLines->pagePos().y() + lines.front().y1();
        result.bottom = staffLines->pagePos().y() + lines.back().y1();
        result.lineDistance = lines.at(1).y1() - lines.at(0).y1();

        for (const LedgerGuide& guide : ledgerGuides(score->pages().front())) {
            if (guide.colorIndex != staffIdx % 2) {
                continue;
            }

            if (guide.line.y1() < result.top) {
                result.above.push_back(guide);
            } else if (guide.line.y1() > result.bottom) {
                result.below.push_back(guide);
            }
        }

        std::sort(result.above.begin(), result.above.end(), [](const LedgerGuide& a, const LedgerGuide& b) {
            return a.line.y1() > b.line.y1();
        });

        std::sort(result.below.begin(), result.below.end(), [](const LedgerGuide& a, const LedgerGuide& b) {
            return a.line.y1() < b.line.y1();
        });

        return result;
    }

    static Note* findNote(MasterScore* score, int pitch)
    {
        for (Segment* segment = score->firstMeasure()->first(SegmentType::ChordRest); segment;
             segment = segment->next(SegmentType::ChordRest)) {
            EngravingItem* element = segment->element(0);
            if (element && element->isChord() && toChord(element)->upNote()->pitch() == pitch) {
                return toChord(element)->upNote();
            }
        }

        return nullptr;
    }
};

/**
 * @TEST_CASE Count_Is_The_First_Ledger_Line_Beyond
 * @details The guides of a side of a staff end at the first ledger line which is further out
 *          than the outermost element, and there is always one
 */
TEST_F(Notation_LedgerGuidesTests, Count_Is_The_First_Ledger_Line_Beyond)
{
    // [THEN] Nothing outside of the staff: one ledger line
    EXPECT_EQ(ledgerGuideCount(-3.0, 1.0), 1);
    EXPECT_EQ(ledgerGuideCount(0.0, 1.0), 1);
    EXPECT_EQ(ledgerGuideCount(0.5, 1.0), 1);

    // [THEN] Something between two ledger lines: the outer one of them
    EXPECT_EQ(ledgerGuideCount(1.9, 1.0), 2);
    EXPECT_EQ(ledgerGuideCount(2.5, 1.0), 3);

    // [THEN] Something which ends at a ledger line: the next one
    EXPECT_EQ(ledgerGuideCount(1.0, 1.0), 2);
    EXPECT_EQ(ledgerGuideCount(2.0, 1.0), 3);
    EXPECT_EQ(ledgerGuideCount(49.6, 24.8), 3);

    // [THEN] The count has a limit, and a staff without a line distance has no guides
    EXPECT_EQ(ledgerGuideCount(1000.0, 1.0), MAX_LEDGER_GUIDES);
    EXPECT_EQ(ledgerGuideCount(2.0, 0.0), 0);
}

/**
 * @TEST_CASE Guides_Reach_One_Ledger_Line_Beyond_The_Notes
 * @details Every staff has its guides above and below it. The upper staff has a note on the second ledger line
 *          above it and nothing below it; the lower staff has only rests
 */
TEST_F(Notation_LedgerGuidesTests, Guides_Reach_One_Ledger_Line_Beyond_The_Notes)
{
    // [GIVEN] A piano score of one system
    MasterScore* score = ScoreRW::readScore(LEDGER_GUIDES_SCORE_PATH);
    ASSERT_TRUE(score);
    ASSERT_TRUE(findNote(score, HIGH_C_PITCH));

    // [WHEN] Get the guides of the upper staff
    const StaffGuides upper = guidesOfStaff(score, 0);

    // [THEN] Three ledger lines above (the note is on the second one), one below; with the spaces between them
    ASSERT_EQ(upper.above.size(), 6);
    ASSERT_EQ(upper.below.size(), 2);

    // [THEN] From the staff outwards: a space, a ledger line, a space...
    for (size_t i = 0; i < upper.above.size(); ++i) {
        EXPECT_EQ(upper.above.at(i).isSpace, i % 2 == 0);
        EXPECT_NEAR(upper.above.at(i).line.y1(), upper.top - (i + 1) * upper.lineDistance / 2.0, 0.001);
    }

    for (size_t i = 0; i < upper.below.size(); ++i) {
        EXPECT_EQ(upper.below.at(i).isSpace, i % 2 == 0);
        EXPECT_NEAR(upper.below.at(i).line.y1(), upper.bottom + (i + 1) * upper.lineDistance / 2.0, 0.001);
    }

    // [THEN] A guide is as long as the staff of the system, over both measures
    Measure* first = score->firstMeasure();
    Measure* last = score->lastMeasure();
    EXPECT_NEAR(upper.above.front().line.x1(), first->staffLines(0)->pagePos().x(), 0.001);
    EXPECT_NEAR(upper.above.front().line.x2(), last->staffLines(0)->pagePos().x() + last->width(), 0.001);

    // [WHEN] Get the guides of the lower staff, which have the other colour
    const StaffGuides lower = guidesOfStaff(score, 1);

    // [THEN] One ledger line on each side
    EXPECT_EQ(lower.above.size(), 2);
    EXPECT_EQ(lower.below.size(), 2);

    delete score;
}

/**
 * @TEST_CASE Guides_Follow_The_Notes
 * @details When the outermost note is moved further out, the guides go one ledger line beyond it again
 */
TEST_F(Notation_LedgerGuidesTests, Guides_Follow_The_Notes)
{
    // [GIVEN] The highest note is on the second ledger line, so the guides reach the third one
    MasterScore* score = ScoreRW::readScore(LEDGER_GUIDES_SCORE_PATH);
    ASSERT_TRUE(score);

    Note* note = findNote(score, HIGH_C_PITCH);
    ASSERT_TRUE(note);
    ASSERT_EQ(guidesOfStaff(score, 0).above.size(), 6);

    // [WHEN] Move that note to the third ledger line
    note->setPitch(HIGH_E_PITCH, E_TPC, E_TPC);
    score->setLayoutAll();
    score->doLayout();

    // [THEN] The guides reach the fourth one
    EXPECT_EQ(guidesOfStaff(score, 0).above.size(), 8);

    // [THEN] The lower staff is not concerned
    EXPECT_EQ(guidesOfStaff(score, 1).above.size(), 2);

    delete score;
}
