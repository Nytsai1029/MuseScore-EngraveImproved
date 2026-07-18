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

#include "internal/project/glyphoutline.h"

namespace mu::fontdesign {
//! 画布编辑面（GlyphCanvas）向页面级动作暴露的能力桥：
//! 复制/粘贴/全选动作按当前画布选择集工作，而不是只对整字形操作。
//! 画布创建时经 IFontDesignService::setActiveEditSurface 注册，析构时注销。
class IFontDesignEditSurface
{
public:
    virtual ~IFontDesignEditSurface() = default;

    virtual bool hasOutlineSelection() const = 0;
    //! 选中点所在的整条 contour 集合（复制的粒度为 contour）
    virtual GlyphOutline selectionAsOutline() const = 0;
    //! 追加轮廓到当前字形（可撤销），并选中粘贴的内容
    virtual void pasteOutline(const GlyphOutline& outline) = 0;
    virtual void selectAllPoints() = 0;
};
}
