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

#include <string>

#include "dom/factory.h"
#include "dom/masterscore.h"
#include "dom/slur.h"

#include "engraving/compat/scoreaccess.h"
#include "rendering/score/slurtielayout.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

namespace {
static constexpr double POINT_TOLERANCE = 1e-6;

void expectPointNear(const PointF& actual, const PointF& expected)
{
    EXPECT_NEAR(actual.x(), expected.x(), POINT_TOLERANCE);
    EXPECT_NEAR(actual.y(), expected.y(), POINT_TOLERANCE);
}

SlurSegment* createMultiBezierSegment(MasterScore*& score)
{
    score = compat::ScoreAccess::createMasterScore(nullptr);

    Slur* slur = Factory::createSlur(score->dummy());
    slur->setMultiBezierEnabled(true);
    slur->setMultiBezierKnotCount(2);
    slur->setAutoplace(false);
    slur->setUp(true);
    slur->fixupSegments(1);

    SlurSegment* segment = slur->frontSegment();
    segment->setSpannerSegmentType(SpannerSegmentType::SINGLE);
    segment->setAutoplace(false);
    segment->ups(Grip::START).p = PointF(0.0, 0.0);
    segment->ups(Grip::END).p = PointF(120.0, 0.0);
    return segment;
}
}

class Engraving_SlurTests : public ::testing::Test
{
};

TEST_F(Engraving_SlurTests, multiBezierEndpointHandlesDoNotMoveInteriorKnots)
{
    MasterScore* score = nullptr;
    SlurSegment* segment = createMultiBezierSegment(score);
    ASSERT_NE(score, nullptr);

    SlurTieLayout::computeBezier(segment);
    ASSERT_EQ(segment->multiBezierKnotData().size(), 2u);

    const PointF firstKnotBefore = segment->multiBezierKnotData()[0].knot.pos();
    const PointF secondKnotBefore = segment->multiBezierKnotData()[1].knot.pos();

    segment->ups(Grip::BEZIER1).off += PointF(0.0, -20.0);
    SlurTieLayout::computeBezier(segment);

    expectPointNear(segment->multiBezierKnotData()[0].knot.pos(), firstKnotBefore);
    expectPointNear(segment->multiBezierKnotData()[1].knot.pos(), secondKnotBefore);

    segment->ups(Grip::BEZIER2).off += PointF(0.0, 18.0);
    SlurTieLayout::computeBezier(segment);

    expectPointNear(segment->multiBezierKnotData()[0].knot.pos(), firstKnotBefore);
    expectPointNear(segment->multiBezierKnotData()[1].knot.pos(), secondKnotBefore);
}

TEST_F(Engraving_SlurTests, multiBezierDataIsStoredRelativeToSpatium)
{
    MasterScore* score = nullptr;
    SlurSegment* segment = createMultiBezierSegment(score);
    ASSERT_NE(score, nullptr);
    const double sp = segment->spatium();

    segment->setProperty(Pid::SLUR_MULTI_BEZIER_DATA, muse::String::fromAscii("1,-2,0.5,1.5,-1,4;2,3,-2,0,1,-3"));
    ASSERT_EQ(segment->multiBezierKnotData().size(), 2u);

    expectPointNear(segment->multiBezierKnotData()[0].knot.off, PointF(1.0 * sp, -2.0 * sp));
    expectPointNear(segment->multiBezierKnotData()[0].inHandle.off, PointF(0.5 * sp, 1.5 * sp));
    expectPointNear(segment->multiBezierKnotData()[0].outHandle.off, PointF(-1.0 * sp, 4.0 * sp));
    expectPointNear(segment->multiBezierKnotData()[1].knot.off, PointF(2.0 * sp, 3.0 * sp));
    expectPointNear(segment->multiBezierKnotData()[1].inHandle.off, PointF(-2.0 * sp, 0.0));
    expectPointNear(segment->multiBezierKnotData()[1].outHandle.off, PointF(1.0 * sp, -3.0 * sp));

    SlurSegment::MultiBezierKnot& first = segment->multiBezierKnotData()[0];
    first.knot.off = PointF(2.0 * sp, -3.0 * sp);
    first.inHandle.off = PointF(0.25 * sp, 1.25 * sp);
    first.outHandle.off = PointF(-1.5 * sp, 4.5 * sp);

    SlurSegment::MultiBezierKnot& second = segment->multiBezierKnotData()[1];
    second.knot.off = PointF(-2.0 * sp, 3.0 * sp);
    second.inHandle.off = PointF(0.0, -0.5 * sp);
    second.outHandle.off = PointF(1.0 * sp, -3.0 * sp);

    segment->syncMultiBezierDataProperty();
    const std::string stored = segment->getProperty(Pid::SLUR_MULTI_BEZIER_DATA).value<muse::String>().toStdString();
    EXPECT_EQ(stored, "2,-3,0.25,1.25,-1.5,4.5;-2,3,0,-0.5,1,-3");
}
