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

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "dom/factory.h"
#include "dom/masterscore.h"
#include "dom/slur.h"

#include "draw/types/painterpath.h"
#include "engraving/compat/scoreaccess.h"
#include "rendering/score/slurtielayout.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;
using muse::draw::PainterPath;

namespace {
static constexpr double POINT_TOLERANCE = 1e-6;

void expectPointNear(const PointF& actual, const PointF& expected)
{
    EXPECT_NEAR(actual.x(), expected.x(), POINT_TOLERANCE);
    EXPECT_NEAR(actual.y(), expected.y(), POINT_TOLERANCE);
}

struct PathCubic
{
    PointF start;
    PointF c1;
    PointF c2;
    PointF end;
};

std::vector<PathCubic> pathCubics(const PainterPath& path)
{
    std::vector<PathCubic> cubics;
    PointF current;
    for (size_t i = 0; i < path.elementCount(); ++i) {
        const PainterPath::Element element = path.elementAt(i);
        if (element.isMoveTo()) {
            current = PointF(element);
        } else if (element.isCurveTo()) {
            if (i + 2 >= path.elementCount()) {
                break;
            }
            const PointF c1(element);
            const PointF c2(path.elementAt(i + 1));
            const PointF end(path.elementAt(i + 2));
            cubics.push_back({ current, c1, c2, end });
            current = end;
            i += 2;
        } else if (element.isLineTo()) {
            current = PointF(element);
        }
    }
    return cubics;
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

TEST_F(Engraving_SlurTests, uneditedMultiBezierMatchesSingleCubic)
{
    MasterScore* score = nullptr;
    SlurSegment* segment = createMultiBezierSegment(score);
    ASSERT_NE(score, nullptr);

    segment->slur()->setMultiBezierEnabled(false);
    SlurTieLayout::computeBezier(segment);

    const CubicBezier singleCubic(segment->ups(Grip::START).pos(),
                                  segment->ups(Grip::BEZIER1).pos(),
                                  segment->ups(Grip::BEZIER2).pos(),
                                  segment->ups(Grip::END).pos());

    segment->slur()->setMultiBezierEnabled(true);
    segment->slur()->setMultiBezierKnotCount(2);
    SlurTieLayout::computeBezier(segment);

    ASSERT_EQ(segment->multiBezierKnotData().size(), 2u);
    expectPointNear(segment->multiBezierKnotData()[0].knot.pos(), singleCubic.pointAtPercent(1.0 / 3.0));
    expectPointNear(segment->multiBezierKnotData()[1].knot.pos(), singleCubic.pointAtPercent(2.0 / 3.0));
}

TEST_F(Engraving_SlurTests, multiBezierThicknessFollowsArcLengthFraction)
{
    MasterScore* score = nullptr;
    SlurSegment* segment = createMultiBezierSegment(score);
    ASSERT_NE(score, nullptr);

    SlurTieLayout::computeBezier(segment);
    const double midThickness = segment->ldata()->midThickness();
    ASSERT_GT(midThickness, 1e-9);
    // The solid outline also carries half the end width on each side
    const double endHalfWidth = 0.5 * segment->endWidth();

    auto halfThicknessAtFirstKnot = [endHalfWidth](SlurSegment* slurSeg) {
        const std::vector<PathCubic> cubics = pathCubics(slurSeg->ldata()->path());
        if (cubics.empty() || slurSeg->multiBezierKnotData().empty()) {
            return -1.0;
        }
        const PointF knot = slurSeg->multiBezierKnotData()[0].knot.pos();
        double nearest = 1e9;
        for (const PathCubic& cubic : cubics) {
            nearest = std::min(nearest, std::hypot(cubic.start.x() - knot.x(), cubic.start.y() - knot.y()));
            nearest = std::min(nearest, std::hypot(cubic.end.x() - knot.x(), cubic.end.y() - knot.y()));
        }
        return nearest - endHalfWidth;
    };

    const double halfNearThird = halfThicknessAtFirstKnot(segment);
    ASSERT_GT(halfNearThird, 0.0);
    // First default knot sits near s=1/3, where 3s(1-s) is about 2/3.
    EXPECT_NEAR(halfNearThird, (2.0 / 3.0) * midThickness, 0.08 * midThickness);

    const PointF towardStart(-35.0, 0.0);
    segment->multiBezierKnotData()[0].knot.off += towardStart;
    segment->multiBezierKnotData()[0].inHandle.off += towardStart;
    segment->multiBezierKnotData()[0].outHandle.off += towardStart;
    SlurTieLayout::computeBezier(segment);

    const double halfNearStart = halfThicknessAtFirstKnot(segment);
    ASSERT_GT(halfNearStart, 0.0);
    // Index-based Y offset would keep ~2/3 midThickness. Near the slur tip the
    // arc-length fraction must taper.
    EXPECT_LT(halfNearStart, 0.45 * midThickness);
    EXPECT_LT(halfNearStart, halfNearThird * 0.85);
}

TEST_F(Engraving_SlurTests, solidMultiBezierPathUsesCubicLens)
{
    MasterScore* score = nullptr;
    SlurSegment* segment = createMultiBezierSegment(score);
    ASSERT_NE(score, nullptr);

    SlurTieLayout::computeBezier(segment);

    const int knotCount = segment->multiBezierKnotCount();
    ASSERT_EQ(knotCount, 2);

    const PainterPath& path = segment->ldata()->path();
    size_t cubicCount = 0;
    size_t lineCount = 0;
    for (size_t i = 0; i < path.elementCount(); ++i) {
        const PainterPath::Element element = path.elementAt(i);
        if (element.isCurveTo()) {
            ++cubicCount;
        } else if (element.isLineTo()) {
            ++lineCount;
        }
    }

    EXPECT_GE(cubicCount, size_t(2 * (knotCount + 1)));
    EXPECT_EQ(lineCount, 2u); // the two square end caps

    segment->multiBezierKnotData()[0].knot.off += PointF(10.0, -8.0);
    segment->multiBezierKnotData()[0].inHandle.off += PointF(10.0, -8.0);
    segment->multiBezierKnotData()[0].outHandle.off += PointF(10.0, -8.0);
    SlurTieLayout::computeBezier(segment);

    cubicCount = 0;
    lineCount = 0;
    const PainterPath& editedPath = segment->ldata()->path();
    for (size_t i = 0; i < editedPath.elementCount(); ++i) {
        const PainterPath::Element element = editedPath.elementAt(i);
        if (element.isCurveTo()) {
            ++cubicCount;
        } else if (element.isLineTo()) {
            ++lineCount;
        }
    }

    EXPECT_GE(cubicCount, size_t(2 * (knotCount + 1)));
    EXPECT_EQ(lineCount, 2u); // the two square end caps
}

TEST_F(Engraving_SlurTests, solidMultiBezierZShapeKeepsThicknessAtBends)
{
    MasterScore* score = nullptr;
    SlurSegment* segment = createMultiBezierSegment(score);
    ASSERT_NE(score, nullptr);

    SlurTieLayout::computeBezier(segment);
    ASSERT_EQ(segment->multiBezierKnotData().size(), 2u);

    const PointF up(0.0, 50.0);
    const PointF down(0.0, -50.0);
    segment->multiBezierKnotData()[0].knot.off += up;
    segment->multiBezierKnotData()[0].inHandle.off += up;
    segment->multiBezierKnotData()[0].outHandle.off += up;
    segment->multiBezierKnotData()[1].knot.off += down;
    segment->multiBezierKnotData()[1].inHandle.off += down;
    segment->multiBezierKnotData()[1].outHandle.off += down;
    SlurTieLayout::computeBezier(segment);

    const double midThickness = segment->ldata()->midThickness();
    ASSERT_GT(midThickness, 1e-9);

    const std::vector<PathCubic> cubics = pathCubics(segment->ldata()->path());
    ASSERT_GE(cubics.size(), 8u);
    const size_t upperCount = cubics.size() / 2;
    const size_t sampleCount = upperCount + 1;
    const size_t begin = std::max(size_t(1), size_t(0.2 * double(sampleCount)));
    const size_t end = std::min(sampleCount - 2, size_t(0.8 * double(sampleCount)));
    ASSERT_LT(begin, end);

    double minWidth = 1e9;
    for (size_t j = begin; j <= end; ++j) {
        const PointF upper = cubics[j - 1].end;
        const size_t lowerIndex = upperCount + (sampleCount - 2 - j);
        ASSERT_LT(lowerIndex, cubics.size());
        const PointF lower = cubics[lowerIndex].end;
        minWidth = std::min(minWidth, std::hypot(upper.x() - lower.x(), upper.y() - lower.y()));
    }

    // Knot-only offsets pinch the steep Z folds toward zero width. A sampled
    // parallel curve must keep a lens envelope through those bends.
    EXPECT_GT(minWidth, 0.6 * midThickness);
}

TEST_F(Engraving_SlurTests, solidSlurOutlineIncludesEndWidth)
{
    MasterScore* score = nullptr;
    SlurSegment* segment = createMultiBezierSegment(score);
    ASSERT_NE(score, nullptr);

    segment->slur()->setMultiBezierEnabled(false);
    SlurTieLayout::computeBezier(segment);

    // Filled without a pen, so the end width must be part of the closed outline:
    // moveTo, cubic, end cap, cubic, start cap
    const PainterPath& path = segment->ldata()->path();
    EXPECT_EQ(path.fillRule(), PainterPath::FillRule::WindingFill);
    ASSERT_EQ(path.elementCount(), 9u);
    EXPECT_TRUE(path.elementAt(0).isMoveTo());
    EXPECT_TRUE(path.elementAt(4).isLineTo());
    EXPECT_TRUE(path.elementAt(8).isLineTo());
    expectPointNear(PointF(path.elementAt(8)), PointF(path.elementAt(0)));

    const double endWidth = segment->endWidth();
    ASSERT_GT(endWidth, 1e-9);

    const PointF startOuter(path.elementAt(0));
    const PointF startInner(path.elementAt(7));
    const PointF endOuter(path.elementAt(3));
    const PointF endInner(path.elementAt(4));
    EXPECT_NEAR(std::hypot(startOuter.x() - startInner.x(), startOuter.y() - startInner.y()), endWidth, 1e-6);
    EXPECT_NEAR(std::hypot(endOuter.x() - endInner.x(), endOuter.y() - endInner.y()), endWidth, 1e-6);
    expectPointNear(0.5 * (startOuter + startInner), segment->ups(Grip::START).pos());
    expectPointNear(0.5 * (endOuter + endInner), segment->ups(Grip::END).pos());
}

TEST_F(Engraving_SlurTests, slurGripAlignmentGuidesUseDragPoint)
{
    MasterScore* score = nullptr;
    SlurSegment* segment = createMultiBezierSegment(score);
    ASSERT_NE(segment, nullptr);

    const std::vector<LineF> startGuides = segment->gripAlignmentGuideLines(Grip::START);
    ASSERT_EQ(startGuides.size(), 2);
    const PointF startGrip = segment->gripsPositions().at(size_t(int(Grip::START)));
    const PointF startOrigin = startGrip - segment->pagePos() + segment->canvasPos();
    EXPECT_NEAR(startGuides.at(0).y1(), startOrigin.y(), POINT_TOLERANCE);
    EXPECT_NEAR(startGuides.at(1).x1(), startOrigin.x(), POINT_TOLERANCE);

    const std::vector<LineF> dragGuides = segment->gripAlignmentGuideLines(Grip::DRAG);
    ASSERT_EQ(dragGuides.size(), 2);
    const PointF dragGrip = segment->gripsPositions().at(size_t(int(Grip::DRAG)));
    const PointF dragOrigin = dragGrip - segment->pagePos() + segment->canvasPos();
    EXPECT_NEAR(dragGuides.at(0).y1(), dragOrigin.y(), POINT_TOLERANCE);
    EXPECT_NEAR(dragGuides.at(1).x1(), dragOrigin.x(), POINT_TOLERANCE);
}
