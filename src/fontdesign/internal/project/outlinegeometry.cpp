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
#include "outlinegeometry.h"

#include <algorithm>
#include <cmath>

#include <QPainterPath>

using namespace muse;

namespace mu::fontdesign::outlinegeom {
using PointType = GlyphOutline::PointType;

static std::vector<int> onCurveIndices(const GlyphOutline::Contour& c)
{
    std::vector<int> result;
    for (int i = 0; i < static_cast<int>(c.points.size()); ++i) {
        if (c.points[i].type == PointType::OnCurve) {
            result.push_back(i);
        }
    }
    return result;
}

static PointF cubicEval(const PointF& p0, const PointF& p1, const PointF& p2, const PointF& p3, double t)
{
    const double u = 1.0 - t;
    const double w0 = u * u * u;
    const double w1 = 3 * u * u * t;
    const double w2 = 3 * u * t * t;
    const double w3 = t * t * t;
    return PointF(w0 * p0.x() + w1 * p1.x() + w2 * p2.x() + w3 * p3.x(),
                  w0 * p0.y() + w1 * p1.y() + w2 * p2.y() + w3 * p3.y());
}

std::vector<PointF> sampleContour(const GlyphOutline::Contour& c)
{
    std::vector<PointF> samples;
    std::vector<int> onc = onCurveIndices(c);
    if (onc.size() < 2) {
        return samples;
    }

    const auto& pts = c.points;
    const int n = static_cast<int>(pts.size());
    constexpr int SEGMENT_SAMPLES = 8;

    for (size_t s = 0; s < onc.size(); ++s) {
        int aIdx = onc[s];
        int bIdx = onc[(s + 1) % onc.size()];
        int between = (aIdx + 1) % n;
        const PointF a = pts[aIdx].pos;
        const PointF b = pts[bIdx].pos;

        if (pts[between].type == PointType::Control) {
            const PointF c1 = pts[between].pos;
            const PointF c2 = pts[(between + 1) % n].pos;
            for (int k = 0; k < SEGMENT_SAMPLES; ++k) {
                samples.push_back(cubicEval(a, c1, c2, b, static_cast<double>(k) / SEGMENT_SAMPLES));
            }
        } else {
            samples.push_back(a);
        }
    }
    return samples;
}

double signedArea(const std::vector<PointF>& poly)
{
    double area = 0.0;
    const int n = static_cast<int>(poly.size());
    for (int i = 0; i < n; ++i) {
        const PointF& p = poly[i];
        const PointF& q = poly[(i + 1) % n];
        area += p.x() * q.y() - q.x() * p.y();
    }
    return area / 2.0;
}

void appendContourToQPath(QPainterPath& path, const GlyphOutline::Contour& c)
{
    std::vector<int> onc = onCurveIndices(c);
    if (onc.size() < 2) {
        return;
    }

    const auto& pts = c.points;
    const int n = static_cast<int>(pts.size());

    path.moveTo(pts[onc[0]].pos.x(), pts[onc[0]].pos.y());
    for (size_t s = 0; s < onc.size(); ++s) {
        int aIdx = onc[s];
        int bIdx = onc[(s + 1) % onc.size()];
        int between = (aIdx + 1) % n;
        if (pts[between].type == PointType::Control) {
            const PointF c1 = pts[between].pos;
            const PointF c2 = pts[(between + 1) % n].pos;
            path.cubicTo(c1.x(), c1.y(), c2.x(), c2.y(), pts[bIdx].pos.x(), pts[bIdx].pos.y());
        } else {
            path.lineTo(pts[bIdx].pos.x(), pts[bIdx].pos.y());
        }
    }
    path.closeSubpath();
}

std::optional<PointF> contourInteriorPoint(const std::vector<PointF>& samples)
{
    const int n = static_cast<int>(samples.size());
    if (n < 3) {
        return std::nullopt;
    }

    double minY = samples[0].y();
    double maxY = samples[0].y();
    for (const PointF& p : samples) {
        minY = std::min(minY, p.y());
        maxY = std::max(maxY, p.y());
    }
    if (maxY - minY < 1e-9) {
        return std::nullopt;
    }

    //! 从中线开始上下微调，避开恰好穿过顶点/水平边的高度
    for (int attempt = 0; attempt < 9; ++attempt) {
        const double offset = 0.061 * ((attempt + 1) / 2) * (attempt % 2 == 0 ? 1.0 : -1.0);
        const double yStar = minY + (maxY - minY) * (0.5 + offset);

        std::vector<double> xs;
        for (int i = 0; i < n; ++i) {
            const PointF& p = samples[i];
            const PointF& q = samples[(i + 1) % n];
            if ((p.y() > yStar) != (q.y() > yStar)) {
                xs.push_back(p.x() + (yStar - p.y()) * (q.x() - p.x()) / (q.y() - p.y()));
            }
        }
        if (xs.size() < 2 || xs.size() % 2 != 0) {
            continue;
        }
        std::sort(xs.begin(), xs.end());
        for (size_t k = 0; k + 1 < xs.size(); k += 2) {
            if (xs[k + 1] - xs[k] > 1e-6) {
                return PointF((xs[k] + xs[k + 1]) / 2.0, yStar);
            }
        }
    }
    return std::nullopt;
}

int contourNestingDepth(const std::vector<GlyphOutline::Contour>& contours, int index,
                        const std::vector<PointF>& samples)
{
    if (samples.size() < 3) {
        return 0;
    }

    //! "j 包含 i" 用 i 的**边界**采样多数表决判定。
    //! 不能用 i 的内部点：外轮廓（如数字 0 的外圈）自身 odd-even 区域覆盖整个盘面，
    //! 其内部点可能落在自己的子孔里，把外轮廓误判为孔（会抹掉镂空）；
    //! 边界点不会落进 i 的子轮廓，且多数表决对个别切点/公共边稳健。
    constexpr size_t MAX_PROBES = 16;
    const size_t step = std::max<size_t>(1, samples.size() / MAX_PROBES);

    int depth = 0;
    for (int j = 0; j < static_cast<int>(contours.size()); ++j) {
        if (j == index) {
            continue;
        }
        QPainterPath other;
        appendContourToQPath(other, contours[j]);
        if (other.isEmpty()) {
            continue;
        }

        int inside = 0;
        int total = 0;
        for (size_t k = 0; k < samples.size(); k += step) {
            ++total;
            if (other.contains(QPointF(samples[k].x(), samples[k].y()))) {
                ++inside;
            }
        }
        if (inside * 2 > total) {
            ++depth;
        }
    }
    return depth;
}

void reverseContour(GlyphOutline::Contour& c)
{
    std::reverse(c.points.begin(), c.points.end());
    auto isOn = [](const GlyphOutline::Point& p) { return p.type == PointType::OnCurve; };
    auto it = std::find_if(c.points.begin(), c.points.end(), isOn);
    if (it != c.points.begin() && it != c.points.end()) {
        std::rotate(c.points.begin(), it, c.points.end());
    }
}

bool orientContourByDepth(std::vector<GlyphOutline::Contour>& contours, int index)
{
    std::vector<PointF> samples = sampleContour(contours[index]);
    if (samples.size() < 3) {
        return false;
    }

    const int depth = contourNestingDepth(contours, index, samples);
    const double area = signedArea(samples);
    const bool shouldBeCcw = (depth % 2) == 0;
    const bool isCcw = area > 0;

    if (shouldBeCcw != isCcw) {
        reverseContour(contours[index]);
        return true;
    }
    return false;
}

bool contourDirectionIsCorrect(const std::vector<GlyphOutline::Contour>& contours, int index)
{
    std::vector<PointF> samples = sampleContour(contours[index]);
    if (samples.size() < 3) {
        return true;
    }

    const int depth = contourNestingDepth(contours, index, samples);
    const bool shouldBeCcw = (depth % 2) == 0;
    return shouldBeCcw == (signedArea(samples) > 0);
}
}
