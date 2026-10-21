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

#include <cmath>
#include <vector>

#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/mscore.h"
#include "dom/segment.h"

#include "style/styledef.h"

#include "utils/scorerw.h"

using namespace mu;
using namespace mu::engraving;

static const String SPACING_DATA_DIR(u"spacing_data/");

static constexpr track_idx_t RIGHT_HAND = 0;
static constexpr track_idx_t LEFT_HAND = 4;

class Engraving_SpacingTests : public ::testing::Test
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

// The stretch that equal-duration spacing gives a segment of this length
static double durationStretch(const MasterScore* score, const Fraction& ticks)
{
    return std::pow(score->style().styleD(Sid::measureSpacing), std::log2((ticks / Fraction(1, 4)).toDouble()));
}

// The stretch of each chord/rest of a voice: the sum over the segments it sounds through
static std::vector<double> noteStretches(const Measure* measure, track_idx_t track)
{
    std::vector<double> stretches;
    for (const Segment* segment = measure->first(SegmentType::ChordRest); segment;
         segment = segment->next(SegmentType::ChordRest)) {
        if (segment->element(track)) {
            stretches.push_back(0.0);
        }
        if (!stretches.empty()) {
            stretches.back() += segment->stretch();
        }
    }
    return stretches;
}

static void expectEven(const std::vector<double>& values, size_t count, const char* what)
{
    ASSERT_EQ(values.size(), count) << what;
    for (size_t i = 1; i < values.size(); ++i) {
        EXPECT_NEAR(values[i], values.front(), 1e-9) << what << ", note " << i;
    }
}

// Equal-duration spacing gave every segment the width of its own length. Where the notes of two voices cross, the
// segments cut notes into pieces that added up to more than a whole note, so equal notes came out uneven.
TEST_F(Engraving_SpacingTests, equalNotesStayEvenInCrossRhythms)
{
    MasterScore* score = ScoreRW::readScore(SPACING_DATA_DIR + u"crossRhythms.mscx");
    ASSERT_TRUE(score);
    score->style().set(Sid::keepEqualDurationSpacing, true);
    score->setLayoutAll();
    score->doLayout();

    const double sixteenth = durationStretch(score, Fraction(1, 16));

    // 9:6 tuplet against sixteenths: the tuplet is the denser voice and keeps the width of its notes
    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);
    std::vector<double> leftHand = noteStretches(measure, LEFT_HAND);
    expectEven(leftHand, 9, "9:6 tuplet");
    EXPECT_NEAR(leftHand.front(), durationStretch(score, Fraction(1, 24)), 1e-9);
    expectEven(noteStretches(measure, RIGHT_HAND), 6, "sixteenths against the 9:6 tuplet");

    // eighth triplets against sixteenths: the sixteenths are denser and keep their width, the triplets widen evenly
    measure = measure->nextMeasure();
    ASSERT_TRUE(measure);
    expectEven(noteStretches(measure, LEFT_HAND), 6, "eighth triplets against sixteenths");
    std::vector<double> rightHand = noteStretches(measure, RIGHT_HAND);
    expectEven(rightHand, 8, "sixteenths against eighth triplets");
    EXPECT_NEAR(rightHand.front(), sixteenth, 1e-9);

    // eighth triplets against dotted eighth and sixteenth
    measure = measure->nextMeasure();
    ASSERT_TRUE(measure);
    expectEven(noteStretches(measure, LEFT_HAND), 6, "eighth triplets against dotted rhythms");
    rightHand = noteStretches(measure, RIGHT_HAND);
    ASSERT_EQ(rightHand.size(), 4u);
    EXPECT_NEAR(rightHand[1], sixteenth, 1e-9);
    EXPECT_NEAR(rightHand[3], sixteenth, 1e-9);

    // even eighths against dotted eighth and sixteenth
    measure = measure->nextMeasure();
    ASSERT_TRUE(measure);
    expectEven(noteStretches(measure, LEFT_HAND), 4, "eighths against dotted rhythms");
    rightHand = noteStretches(measure, RIGHT_HAND);
    ASSERT_EQ(rightHand.size(), 4u);
    EXPECT_NEAR(rightHand[1], sixteenth, 1e-9);
    EXPECT_NEAR(rightHand[3], sixteenth, 1e-9);

    // sixteenths inside even eighths, a syncopation and triplets under quarters keep the width of their own length
    for (int i = 0; i < 3; ++i) {
        measure = measure->nextMeasure();
        ASSERT_TRUE(measure);
        for (const Segment* segment = measure->first(SegmentType::ChordRest); segment;
             segment = segment->next(SegmentType::ChordRest)) {
            EXPECT_NEAR(segment->stretch(), durationStretch(score, segment->ticks()), 1e-9)
                << "measure " << measure->no() + 1 << ", tick " << segment->rtick().toString().toStdString();
        }
    }

    delete score;
}

// The first system holds the 9:6 tuplet alone and is stretched to the full width: its notes sit evenly
TEST_F(Engraving_SpacingTests, tupletAgainstSixteenthsIsEvenlyPlaced)
{
    MasterScore* score = ScoreRW::readScore(SPACING_DATA_DIR + u"crossRhythms.mscx");
    ASSERT_TRUE(score);
    score->style().set(Sid::keepEqualDurationSpacing, true);
    score->setLayoutAll();
    score->doLayout();

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);
    ASSERT_TRUE(measure->system());
    ASSERT_NE(measure->system(), measure->nextMeasure()->system());

    std::vector<double> positions;
    for (const Segment* segment = measure->first(SegmentType::ChordRest); segment;
         segment = segment->next(SegmentType::ChordRest)) {
        if (segment->element(LEFT_HAND)) {
            positions.push_back(measure->x() + segment->x());
        }
    }
    ASSERT_EQ(positions.size(), 9u);

    const double gap = positions[1] - positions[0];
    EXPECT_GT(gap, 0.0);
    for (size_t i = 2; i < positions.size(); ++i) {
        EXPECT_NEAR(positions[i] - positions[i - 1], gap, 1e-6 * gap) << "note " << i;
    }

    delete score;
}
