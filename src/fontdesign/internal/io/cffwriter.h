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

#include <cstdint>
#include <string>
#include <vector>

#include "types/ret.h"

#include "../project/glyphoutline.h"

namespace mu::fontdesign {
class FontDesignProject;

//! 写出 CFF 表（Type2 charstrings）。坐标为字体单位，y 向上。
class CffWriter
{
public:
    struct GlyphInput {
        std::string name;          // PostScript / SMuFL 名；.notdef 固定为 glyph 0
        char32_t codepoint = 0;    // 仅用于命名回退
        GlyphOutline outline;
        int advance = 0;           // 字体单位，整数
    };

    struct Input {
        std::string fontName;
        double fontVersion = 1.0;
        int upem = 1000;
        int fontBBoxXMin = 0;
        int fontBBoxYMin = 0;
        int fontBBoxXMax = 0;
        int fontBBoxYMax = 0;
        std::vector<GlyphInput> glyphs; // glyphs[0] 必须是 .notdef
    };

    //! 成功时 out 为完整 CFF 表字节
    static muse::Ret write(const Input& input, std::vector<uint8_t>& out);

    //! 从项目组装 Input（排序码位、保证 .notdef）
    static Input fromProject(const FontDesignProject& project);
};
}
