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
#pragma once

#include <vector>

#include "draw/types/geometry.h"
#include "draw/types/painterpath.h"

class QPainterPath;

namespace mu::fontdesign {
//! 字形轮廓模型：三次贝塞尔为基础的可编辑表示。
//! 坐标为字体单位（font units），y 向上。
//! 不变式：每个 contour 以 OnCurve 点开头；相邻 OnCurve 点之间要么直连（直线段），
//! 要么恰好隔两个 Control 点（三次曲线段）；contour 隐式闭合（末点接回首点）。
class GlyphOutline
{
public:
    enum class PointType {
        OnCurve,
        Control
    };

    struct Point {
        muse::PointF pos;
        PointType type = PointType::OnCurve;
        bool smooth = false;

        Point() = default;
        Point(const muse::PointF& pos, PointType type)
            : pos(pos), type(type) {}

        bool operator==(const Point& o) const { return pos == o.pos && type == o.type && smooth == o.smooth; }
        bool operator!=(const Point& o) const { return !(*this == o); }
    };

    struct Contour {
        std::vector<Point> points;

        bool operator==(const Contour& o) const { return points == o.points; }
        bool operator!=(const Contour& o) const { return !(*this == o); }
    };

    bool operator==(const GlyphOutline& o) const { return m_contours == o.m_contours; }
    bool operator!=(const GlyphOutline& o) const { return !(*this == o); }

    std::vector<Contour>& contours() { return m_contours; }
    const std::vector<Contour>& contours() const { return m_contours; }

    bool isEmpty() const;

    muse::draw::PainterPath toPainterPath() const;
    muse::RectF boundingRect() const;

    //! 整体平移（字体单位）
    void translate(const muse::PointF& delta);
    //! 以原点为中心整体缩放（跨 upem 粘贴时换算）
    void scale(double factor);

    //! 矩形轮廓（4 个角点，直线段，顺时针）
    static Contour rectContour(const muse::RectF& rect);
    //! 椭圆轮廓（4 段三次贝塞尔近似，kappa=0.5522847498）
    static Contour ellipseContour(const muse::RectF& rect);

    //! QPainterPath → 轮廓集（布尔运算结果 / QRawFont 文本字形导入用）。
    //! 全部节点标为角点；隐式闭合（去掉与首点重合的末点）。
    static GlyphOutline fromQPainterPath(const QPainterPath& path);

private:
    std::vector<Contour> m_contours;
};
}
