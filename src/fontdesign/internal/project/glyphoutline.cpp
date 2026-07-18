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
#include "glyphoutline.h"

#include <cmath>

#include <QPainterPath>

using namespace mu::fontdesign;
using namespace muse;
using namespace muse::draw;

bool GlyphOutline::isEmpty() const
{
    for (const Contour& contour : m_contours) {
        if (!contour.points.empty()) {
            return false;
        }
    }

    return true;
}

PainterPath GlyphOutline::toPainterPath() const
{
    PainterPath path;
    path.setFillRule(PainterPath::FillRule::WindingFill);

    for (const Contour& contour : m_contours) {
        const std::vector<Point>& pts = contour.points;
        if (pts.size() < 2) {
            continue;
        }

        path.moveTo(pts[0].pos);

        size_t i = 1;
        while (i < pts.size()) {
            if (pts[i].type == PointType::OnCurve) {
                path.lineTo(pts[i].pos);
                i += 1;
            } else if (i + 1 < pts.size()) {
                const PointF& c1 = pts[i].pos;
                const PointF& c2 = pts[i + 1].pos;
                const PointF end = (i + 2 < pts.size()) ? pts[i + 2].pos : pts[0].pos;
                path.cubicTo(c1, c2, end);
                i += 3;
            } else {
                break;
            }
        }

        path.closeSubpath();
    }

    return path;
}

RectF GlyphOutline::boundingRect() const
{
    return toPainterPath().boundingRect();
}

void GlyphOutline::translate(const PointF& delta)
{
    for (Contour& contour : m_contours) {
        for (Point& p : contour.points) {
            p.pos += delta;
        }
    }
}

void GlyphOutline::scale(double factor)
{
    for (Contour& contour : m_contours) {
        for (Point& p : contour.points) {
            p.pos = PointF(p.pos.x() * factor, p.pos.y() * factor);
        }
    }
}

GlyphOutline::Contour GlyphOutline::rectContour(const RectF& rect)
{
    // y 向上：顺序取 左下→左上→右上→右下，形成一个闭合直线轮廓
    Contour c;
    c.points.emplace_back(PointF(rect.left(), rect.top()), PointType::OnCurve);
    c.points.emplace_back(PointF(rect.left(), rect.bottom()), PointType::OnCurve);
    c.points.emplace_back(PointF(rect.right(), rect.bottom()), PointType::OnCurve);
    c.points.emplace_back(PointF(rect.right(), rect.top()), PointType::OnCurve);
    return c;
}

GlyphOutline::Contour GlyphOutline::ellipseContour(const RectF& rect)
{
    constexpr double kappa = 0.5522847498307936;
    const double cx = rect.center().x();
    const double cy = rect.center().y();
    const double rx = rect.width() / 2.0;
    const double ry = rect.height() / 2.0;
    const double ox = rx * kappa;
    const double oy = ry * kappa;

    // 4 个象限点（上/右/下/左）以三次贝塞尔连接，每个 on-curve 点标记 smooth
    Contour c;
    auto onCurve = [](const PointF& p) {
        Point pt(p, PointType::OnCurve);
        pt.smooth = true;
        return pt;
    };
    auto control = [](const PointF& p) { return Point(p, PointType::Control); };

    const PointF top(cx, cy + ry);
    const PointF right(cx + rx, cy);
    const PointF bottom(cx, cy - ry);
    const PointF left(cx - rx, cy);

    c.points.push_back(onCurve(top));
    c.points.push_back(control(PointF(cx + ox, cy + ry)));
    c.points.push_back(control(PointF(cx + rx, cy + oy)));
    c.points.push_back(onCurve(right));
    c.points.push_back(control(PointF(cx + rx, cy - oy)));
    c.points.push_back(control(PointF(cx + ox, cy - ry)));
    c.points.push_back(onCurve(bottom));
    c.points.push_back(control(PointF(cx - ox, cy - ry)));
    c.points.push_back(control(PointF(cx - rx, cy - oy)));
    c.points.push_back(onCurve(left));
    c.points.push_back(control(PointF(cx - rx, cy + oy)));
    c.points.push_back(control(PointF(cx - ox, cy + ry)));
    // 末两个 Control 之后隐式接回首点 top，闭合最后一段
    return c;
}

GlyphOutline GlyphOutline::fromQPainterPath(const QPainterPath& path)
{
    GlyphOutline result;
    Contour current;

    auto onCurveCount = [](const Contour& c) {
        size_t count = 0;
        for (const Point& p : c.points) {
            if (p.type == PointType::OnCurve) {
                ++count;
            }
        }
        return count;
    };

    auto finalize = [&]() {
        auto& pts = current.points;
        if (pts.size() >= 2) {
            const PointF first = pts.front().pos;
            const PointF last = pts.back().pos;
            if (pts.back().type == PointType::OnCurve
                && std::abs(first.x() - last.x()) < 1e-6 && std::abs(first.y() - last.y()) < 1e-6) {
                pts.pop_back();
            }
        }
        if (onCurveCount(current) >= 2) {
            result.m_contours.push_back(current);
        }
        current = Contour();
    };

    const int count = path.elementCount();
    for (int i = 0; i < count; ++i) {
        const QPainterPath::Element e = path.elementAt(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:
            finalize();
            current.points.emplace_back(PointF(e.x, e.y), PointType::OnCurve);
            break;
        case QPainterPath::LineToElement:
            current.points.emplace_back(PointF(e.x, e.y), PointType::OnCurve);
            break;
        case QPainterPath::CurveToElement: {
            const QPainterPath::Element c2 = path.elementAt(i + 1);
            const QPainterPath::Element end = path.elementAt(i + 2);
            current.points.emplace_back(PointF(e.x, e.y), PointType::Control);
            current.points.emplace_back(PointF(c2.x, c2.y), PointType::Control);
            current.points.emplace_back(PointF(end.x, end.y), PointType::OnCurve);
            i += 2;
            break;
        }
        default:
            break;
        }
    }
    finalize();
    return result;
}
