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

#include <optional>
#include <vector>

#include "glyphoutline.h"

class QPainterPath;

namespace mu::fontdesign::outlinegeom {
//! 轮廓几何判定（画布编辑与 SMuFL 校验共用）：
//! 方向（有向面积）、内部代表点、嵌套深度、按深度定向。

//! 轮廓采样为折线（曲线段细分）
std::vector<muse::PointF> sampleContour(const GlyphOutline::Contour& c);

//! 有向面积（shoelace）：y 向上坐标系中 > 0 为逆时针
double signedArea(const std::vector<muse::PointF>& poly);

//! contour → QPainterPath 子路径（字体单位坐标）
void appendContourToQPath(QPainterPath& path, const GlyphOutline::Contour& c);

//! 保证严格位于轮廓实体内部的代表点（扫描线法；质心对螺旋/凹形不可靠）
std::optional<muse::PointF> contourInteriorPoint(const std::vector<muse::PointF>& samples);

//! 轮廓 index 在其余轮廓中的嵌套深度（被多少条轮廓包含）
int contourNestingDepth(const std::vector<GlyphOutline::Contour>& contours, int index,
                        const std::vector<muse::PointF>& samples);

//! 反转点序（保持环绕方向语义），并旋转使首点为 on-curve（模型不变式）
void reverseContour(GlyphOutline::Contour& c);

//! 按嵌套深度定向单条轮廓：偶数深度（外轮廓）逆时针，奇数深度（孔）顺时针。
//! 返回 true = 发生了反转
bool orientContourByDepth(std::vector<GlyphOutline::Contour>& contours, int index);

//! 只检查不修改：轮廓方向是否符合按深度的期望（不符 = 会导致镂空/填充异常）
bool contourDirectionIsCorrect(const std::vector<GlyphOutline::Contour>& contours, int index);
}
