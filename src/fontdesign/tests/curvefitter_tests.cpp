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

#include <QPainterPath>

#include "fontdesign/internal/project/curvefitter.h"
#include "fontdesign/internal/project/glyphoutline.h"
#include "fontdesign/internal/project/outlinegeometry.h"

using namespace mu::fontdesign;
using namespace muse;

namespace {
int countType(const GlyphOutline::Contour& c, GlyphOutline::PointType type)
{
    int count = 0;
    for (const GlyphOutline::Point& p : c.points) {
        if (p.type == type) {
            ++count;
        }
    }
    return count;
}

//! 拟合结果对参考路径的最大采样偏差（字体单位）
double maxDeviation(const GlyphOutline::Contour& fitted, const QPainterPath& reference)
{
    double maxDist = 0.0;
    for (const PointF& sample : outlinegeom::sampleContour(fitted)) {
        //! 用点到路径的粗略距离：多方向探测最近边界
        const QPointF p(sample.x(), sample.y());
        double best = 1e9;
        for (double probe = 0.0; probe <= 8.0; probe += 0.5) {
            if (reference.contains(QRectF(p.x() - probe, p.y() - probe, 2 * probe, 2 * probe).center())
                != reference.contains(p)) {
                break;
            }
            // 找到边界的保守下界：以 probe 为半径的方形完全同侧则距离 > probe
            const bool allSame = reference.contains(QPointF(p.x() + probe, p.y()))
                                 == reference.contains(p)
                                 && reference.contains(QPointF(p.x() - probe, p.y())) == reference.contains(p)
                                 && reference.contains(QPointF(p.x(), p.y() + probe)) == reference.contains(p)
                                 && reference.contains(QPointF(p.x(), p.y() - probe)) == reference.contains(p);
            if (!allSame) {
                best = probe;
                break;
            }
        }
        if (best < 1e9) {
            maxDist = std::max(maxDist, best);
        }
    }
    return maxDist;
}
}

TEST(FontDesign_CurveFitterTests, UnionOfEllipsesKeepsCurves)
{
    //! 两个交叠椭圆 → Qt 并集（压平为折线）→ 重拟合应恢复曲线
    GlyphOutline source;
    source.contours().push_back(GlyphOutline::ellipseContour(RectF(0, 0, 400, 300)));
    source.contours().push_back(GlyphOutline::ellipseContour(RectF(250, 50, 400, 300)));

    QPainterPath a;
    a.setFillRule(Qt::WindingFill);
    outlinegeom::appendContourToQPath(a, source.contours()[0]);
    QPainterPath b;
    b.setFillRule(Qt::WindingFill);
    outlinegeom::appendContourToQPath(b, source.contours()[1]);

    const QPainterPath unionPath = a.united(b);

    GlyphOutline flattened = GlyphOutline::fromQPainterPath(unionPath);
    ASSERT_EQ(flattened.contours().size(), size_t(1));
    const int flatOnCurve = countType(flattened.contours()[0], GlyphOutline::PointType::OnCurve);

    GlyphOutline fitted = CurveFitter::refit(flattened, 1000.0);
    ASSERT_EQ(fitted.contours().size(), size_t(1));
    const GlyphOutline::Contour& c = fitted.contours()[0];

    //! 曲线恢复：有控制点，且节点数远小于压平折线
    EXPECT_GT(countType(c, GlyphOutline::PointType::Control), 0) << "curves were not reconstructed";
    const int fittedOnCurve = countType(c, GlyphOutline::PointType::OnCurve);
    EXPECT_LT(fittedOnCurve, flatOnCurve / 3) << "fit did not reduce point count";
    EXPECT_GE(fittedOnCurve, 2);

    //! 形状保真：拟合轮廓离并集边界的最大偏差 ≤ 4 字体单位（容差 2 + 探测粒度）
    EXPECT_LE(maxDeviation(c, unionPath), 4.0);
}

TEST(FontDesign_CurveFitterTests, RectangleUnionStaysPolygonal)
{
    //! 两个矩形并集 → 重拟合结果应保持直线多边形（无控制点）
    GlyphOutline source;
    source.contours().push_back(GlyphOutline::rectContour(RectF(0, 0, 300, 200)));
    source.contours().push_back(GlyphOutline::rectContour(RectF(200, 50, 300, 200)));

    QPainterPath a;
    a.setFillRule(Qt::WindingFill);
    outlinegeom::appendContourToQPath(a, source.contours()[0]);
    QPainterPath b;
    b.setFillRule(Qt::WindingFill);
    outlinegeom::appendContourToQPath(b, source.contours()[1]);

    GlyphOutline fitted = CurveFitter::refit(GlyphOutline::fromQPainterPath(a.united(b)), 1000.0);
    ASSERT_EQ(fitted.contours().size(), size_t(1));

    EXPECT_EQ(countType(fitted.contours()[0], GlyphOutline::PointType::Control), 0)
        << "straight edges should stay straight";
    EXPECT_EQ(countType(fitted.contours()[0], GlyphOutline::PointType::OnCurve), 8);
}
