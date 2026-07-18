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
#include "curvefitter.h"

#include <algorithm>
#include <array>
#include <cmath>

#include "outlinegeometry.h"

using namespace mu::fontdesign;
using namespace muse;

namespace {
using Bezier = std::array<PointF, 4>;

//! 拟合输出段：直线（只用端点）或三次贝塞尔
struct Segment {
    bool isLine = false;
    Bezier b;
};

double dot(const PointF& a, const PointF& b)
{
    return a.x() * b.x() + a.y() * b.y();
}

double length(const PointF& v)
{
    return std::hypot(v.x(), v.y());
}

PointF scaled(const PointF& v, double f)
{
    return PointF(v.x() * f, v.y() * f);
}

PointF normalized(const PointF& v)
{
    const double len = length(v);
    return len > 1e-12 ? scaled(v, 1.0 / len) : PointF(0.0, 0.0);
}

PointF bezierEval(const Bezier& b, double t)
{
    const double u = 1.0 - t;
    const double b0 = u * u * u;
    const double b1 = 3 * u * u * t;
    const double b2 = 3 * u * t * t;
    const double b3 = t * t * t;
    return PointF(b0 * b[0].x() + b1 * b[1].x() + b2 * b[2].x() + b3 * b[3].x(),
                  b0 * b[0].y() + b1 * b[1].y() + b2 * b[2].y() + b3 * b[3].y());
}

// ---------------------------------------------------------------------------
// Ramer–Douglas–Peucker 折线简化（开放段）
// ---------------------------------------------------------------------------

double pointLineDistance(const PointF& p, const PointF& a, const PointF& b)
{
    const PointF ab = b - a;
    const double len2 = dot(ab, ab);
    if (len2 < 1e-12) {
        return length(p - a);
    }
    const double t = std::clamp(dot(p - a, ab) / len2, 0.0, 1.0);
    return length(p - (a + scaled(ab, t)));
}

void rdpRecurse(const std::vector<PointF>& pts, int first, int last, double eps, std::vector<bool>& keep)
{
    if (last - first < 2) {
        return;
    }
    double maxDist = -1.0;
    int maxIdx = first;
    for (int i = first + 1; i < last; ++i) {
        const double d = pointLineDistance(pts[i], pts[first], pts[last]);
        if (d > maxDist) {
            maxDist = d;
            maxIdx = i;
        }
    }
    if (maxDist > eps) {
        keep[maxIdx] = true;
        rdpRecurse(pts, first, maxIdx, eps, keep);
        rdpRecurse(pts, maxIdx, last, eps, keep);
    }
}

//! 闭环简化：以 0 与最远点两处为锚分两半 RDP
std::vector<PointF> simplifyRing(const std::vector<PointF>& ring, double eps)
{
    const int n = static_cast<int>(ring.size());
    if (n < 4) {
        return ring;
    }

    int farIdx = n / 2;
    double farDist = -1.0;
    for (int i = 1; i < n; ++i) {
        const double d = length(ring[i] - ring[0]);
        if (d > farDist) {
            farDist = d;
            farIdx = i;
        }
    }

    std::vector<bool> keep(n, false);
    keep[0] = true;
    keep[farIdx] = true;
    rdpRecurse(ring, 0, farIdx, eps, keep);
    {
        // 后半段带回绕：拼一个临时开放段 [farIdx..n-1, 0]
        std::vector<PointF> tail(ring.begin() + farIdx, ring.end());
        tail.push_back(ring[0]);
        std::vector<bool> tailKeep(tail.size(), false);
        tailKeep.front() = true;
        tailKeep.back() = true;
        rdpRecurse(tail, 0, static_cast<int>(tail.size()) - 1, eps, tailKeep);
        for (size_t i = 1; i + 1 < tail.size(); ++i) {
            if (tailKeep[i]) {
                keep[farIdx + i] = true;
            }
        }
    }

    std::vector<PointF> result;
    for (int i = 0; i < n; ++i) {
        if (keep[i]) {
            result.push_back(ring[i]);
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// Schneider 三次贝塞尔拟合（Graphics Gems: An Algorithm for Automatically
// Fitting Digitized Curves）
// ---------------------------------------------------------------------------

std::vector<double> chordLengthParameterize(const std::vector<PointF>& pts, int first, int last)
{
    std::vector<double> u(last - first + 1, 0.0);
    for (int i = first + 1; i <= last; ++i) {
        u[i - first] = u[i - first - 1] + length(pts[i] - pts[i - 1]);
    }
    const double total = u.back();
    if (total > 1e-12) {
        for (double& v : u) {
            v /= total;
        }
    }
    return u;
}

Bezier generateBezier(const std::vector<PointF>& pts, int first, int last,
                      const std::vector<double>& uPrime, const PointF& tHat1, const PointF& tHat2)
{
    const int nPts = last - first + 1;
    const PointF p0 = pts[first];
    const PointF p3 = pts[last];

    double c00 = 0.0, c01 = 0.0, c11 = 0.0;
    double x0 = 0.0, x1 = 0.0;

    for (int i = 0; i < nPts; ++i) {
        const double u = uPrime[i];
        const double omu = 1.0 - u;
        const double b0 = omu * omu * omu;
        const double b1 = 3 * u * omu * omu;
        const double b2 = 3 * u * u * omu;
        const double b3 = u * u * u;

        const PointF a0 = scaled(tHat1, b1);
        const PointF a1 = scaled(tHat2, b2);

        c00 += dot(a0, a0);
        c01 += dot(a0, a1);
        c11 += dot(a1, a1);

        const PointF tmp = pts[first + i]
                           - (scaled(p0, b0 + b1) + scaled(p3, b2 + b3));
        x0 += dot(a0, tmp);
        x1 += dot(a1, tmp);
    }

    const double detC0C1 = c00 * c11 - c01 * c01;
    const double detC0X = c00 * x1 - c01 * x0;
    const double detXC1 = x0 * c11 - x1 * c01;

    double alphaL = std::abs(detC0C1) > 1e-12 ? detXC1 / detC0C1 : 0.0;
    double alphaR = std::abs(detC0C1) > 1e-12 ? detC0X / detC0C1 : 0.0;

    //! α 非正/过小：退化为 Wu/Barsky 启发（弦长三分）
    const double segLength = length(p3 - p0);
    const double epsilon = 1e-6 * segLength;
    if (alphaL < epsilon || alphaR < epsilon) {
        alphaL = alphaR = segLength / 3.0;
    }

    return { p0, p0 + scaled(tHat1, alphaL), p3 + scaled(tHat2, alphaR), p3 };
}

double computeMaxError(const std::vector<PointF>& pts, int first, int last,
                       const Bezier& bez, const std::vector<double>& u, int& splitPoint)
{
    splitPoint = (last + first + 1) / 2;
    double maxDist = 0.0;
    for (int i = first + 1; i < last; ++i) {
        const PointF p = bezierEval(bez, u[i - first]);
        const PointF diff = p - pts[i];
        const double dist = dot(diff, diff);
        if (dist >= maxDist) {
            maxDist = dist;
            splitPoint = i;
        }
    }
    return maxDist;
}

double newtonRaphsonRootFind(const Bezier& q, const PointF& p, double u)
{
    Bezier q1;
    for (int i = 0; i < 3; ++i) {
        q1[i] = scaled(q[i + 1] - q[i], 3.0);
    }
    std::array<PointF, 2> q2;
    for (int i = 0; i < 2; ++i) {
        q2[i] = scaled(q1[i + 1] - q1[i], 2.0);
    }

    const PointF qu = bezierEval(q, u);
    const double omu = 1.0 - u;
    const PointF q1u = scaled(q1[0], omu * omu) + scaled(q1[1], 2 * u * omu) + scaled(q1[2], u * u);
    const PointF q2u = scaled(q2[0], omu) + scaled(q2[1], u);

    const PointF diff = qu - p;
    const double numerator = dot(diff, q1u);
    const double denominator = dot(q1u, q1u) + dot(diff, q2u);
    if (std::abs(denominator) < 1e-12) {
        return u;
    }
    return u - numerator / denominator;
}

void fitCubicRecurse(const std::vector<PointF>& pts, int first, int last,
                     PointF tHat1, PointF tHat2, double errorSq, std::vector<Segment>& out, int depth)
{
    //! 两点：直线段
    if (last - first == 1) {
        Segment seg;
        seg.isLine = true;
        seg.b = { pts[first], PointF(), PointF(), pts[last] };
        out.push_back(seg);
        return;
    }

    std::vector<double> u = chordLengthParameterize(pts, first, last);
    Bezier bez = generateBezier(pts, first, last, u, tHat1, tHat2);

    int splitPoint = 0;
    double maxError = computeMaxError(pts, first, last, bez, u, splitPoint);
    if (maxError < errorSq) {
        Segment seg;
        seg.b = bez;
        out.push_back(seg);
        return;
    }

    //! 误差不大：重参数化再试几轮
    if (maxError < errorSq * 16.0) {
        for (int iteration = 0; iteration < 4; ++iteration) {
            for (int i = 0; i <= last - first; ++i) {
                u[i] = std::clamp(newtonRaphsonRootFind(bez, pts[first + i], u[i]), 0.0, 1.0);
            }
            bez = generateBezier(pts, first, last, u, tHat1, tHat2);
            maxError = computeMaxError(pts, first, last, bez, u, splitPoint);
            if (maxError < errorSq) {
                Segment seg;
                seg.b = bez;
                out.push_back(seg);
                return;
            }
        }
    }

    if (depth > 24) {      // 保底：不再细分
        Segment seg;
        seg.b = bez;
        out.push_back(seg);
        return;
    }

    //! 在最大误差处分割，中心切线保证接点平滑
    splitPoint = std::clamp(splitPoint, first + 1, last - 1);
    PointF centerTangent = normalized(pts[splitPoint - 1] - pts[splitPoint + 1]);
    if (length(centerTangent) < 1e-9) {
        centerTangent = normalized(pts[splitPoint - 1] - pts[splitPoint]);
    }
    fitCubicRecurse(pts, first, splitPoint, tHat1, centerTangent, errorSq, out, depth + 1);
    fitCubicRecurse(pts, splitPoint, last, scaled(centerTangent, -1.0), tHat2, errorSq, out, depth + 1);
}

//! 开放段拟合（端点切线单侧估计）
void fitSection(const std::vector<PointF>& pts, double errorSq, std::vector<Segment>& out)
{
    const int n = static_cast<int>(pts.size());
    if (n < 2) {
        return;
    }
    if (n == 2) {
        Segment seg;
        seg.isLine = true;
        seg.b = { pts[0], PointF(), PointF(), pts[1] };
        out.push_back(seg);
        return;
    }
    const PointF tHat1 = normalized(pts[1] - pts[0]);
    const PointF tHat2 = normalized(pts[n - 2] - pts[n - 1]);
    fitCubicRecurse(pts, 0, n - 1, tHat1, tHat2, errorSq, out, 0);
}
}

GlyphOutline::Contour CurveFitter::refitContour(const GlyphOutline::Contour& contour, double upem)
{
    const double scale = upem > 0 ? upem / 1000.0 : 1.0;
    const double simplifyEps = 0.25 * scale;
    const double fitTol = 0.75 * scale;
    const double errorSq = fitTol * fitTol;
    constexpr double CORNER_ANGLE_DEG = 30.0;

    //! 展平为折线环（布尔结果基本已是直线段；兜底也处理残留曲线）
    std::vector<PointF> ring = outlinegeom::sampleContour(contour);
    if (ring.size() < 4) {
        return contour;
    }

    ring = simplifyRing(ring, simplifyEps);
    const int n = static_cast<int>(ring.size());
    if (n < 3) {
        return contour;
    }

    //! 角点检测（相邻边方向变化超过阈值）
    std::vector<int> corners;
    const double cornerCos = std::cos(CORNER_ANGLE_DEG * M_PI / 180.0);
    for (int i = 0; i < n; ++i) {
        const PointF inDir = normalized(ring[i] - ring[(i - 1 + n) % n]);
        const PointF outDir = normalized(ring[(i + 1) % n] - ring[i]);
        if (dot(inDir, outDir) < cornerCos) {
            corners.push_back(i);
        }
    }
    //! 全平滑闭环（如圆的并集）：取两处人工分割，接点切线用中心差分保持平滑
    const bool syntheticCorners = corners.empty();
    if (syntheticCorners) {
        corners.push_back(0);
        corners.push_back(n / 2);
    }

    //! 逐分段拟合
    std::vector<Segment> segments;
    const int cornerCount = static_cast<int>(corners.size());
    for (int c = 0; c < cornerCount; ++c) {
        const int start = corners[c];
        const int end = corners[(c + 1) % cornerCount];

        std::vector<PointF> section;
        for (int i = start; ; i = (i + 1) % n) {
            section.push_back(ring[i]);
            if (i == end) {
                break;
            }
        }
        if (section.size() < 2) {
            continue;
        }

        if (syntheticCorners) {
            //! 平滑闭环：端点切线用环上的中心差分（保证两段在接点相切）
            const int startPrev = (start - 1 + n) % n;
            const int endNext = (end + 1) % n;
            std::vector<Segment> local;
            const PointF tHat1 = normalized(ring[(start + 1) % n] - ring[startPrev]);
            const PointF tHat2 = normalized(ring[(end - 1 + n) % n] - ring[endNext]);
            fitCubicRecurse(section, 0, static_cast<int>(section.size()) - 1, tHat1, tHat2, errorSq, local, 0);
            segments.insert(segments.end(), local.begin(), local.end());
        } else {
            fitSection(section, errorSq, segments);
        }
    }

    if (segments.empty()) {
        return contour;
    }

    //! 组装轮廓：接点 on-curve；切线连续处标 smooth
    GlyphOutline::Contour result;
    const int segCount = static_cast<int>(segments.size());
    for (int s = 0; s < segCount; ++s) {
        const Segment& seg = segments[s];
        const Segment& prev = segments[(s - 1 + segCount) % segCount];

        GlyphOutline::Point on(seg.b[0], GlyphOutline::PointType::OnCurve);
        //! smooth：入方向与出方向共线
        const PointF inDir = normalized(prev.isLine ? prev.b[3] - prev.b[0] : prev.b[3] - prev.b[2]);
        const PointF outDir = normalized(seg.isLine ? seg.b[3] - seg.b[0] : seg.b[1] - seg.b[0]);
        on.smooth = dot(inDir, outDir) > 0.985;
        result.points.push_back(on);

        if (!seg.isLine) {
            result.points.emplace_back(seg.b[1], GlyphOutline::PointType::Control);
            result.points.emplace_back(seg.b[2], GlyphOutline::PointType::Control);
        }
    }
    return result;
}

GlyphOutline CurveFitter::refit(const GlyphOutline& flattened, double upem)
{
    GlyphOutline result;
    for (const GlyphOutline::Contour& contour : flattened.contours()) {
        GlyphOutline::Contour fitted = refitContour(contour, upem);
        if (!fitted.points.empty()) {
            result.contours().push_back(fitted);
        }
    }
    return result;
}
