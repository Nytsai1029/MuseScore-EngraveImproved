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

#include "glyphoutline.h"

namespace mu::fontdesign {
//! 折线轮廓 → 三次贝塞尔重拟合（Schneider 最小二乘，Graphics Gems "FitCurves"）。
//! Qt 的路径布尔运算会把曲线压平成密集直线段；布尔之后经此重建曲线：
//! RDP 简化去冗余点 → 按转角检测角点分段 → 平滑段递归拟合三次贝塞尔，
//! 共线段保持直线。角点为 corner，拟合分割点为 smooth。
class CurveFitter
{
public:
    //! upem 决定容差尺度（简化 0.5、拟合误差 2 字体单位 @1000 upem）
    static GlyphOutline refit(const GlyphOutline& flattened, double upem);
    static GlyphOutline::Contour refitContour(const GlyphOutline::Contour& contour, double upem);
};
}
