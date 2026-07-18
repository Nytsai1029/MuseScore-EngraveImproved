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
#include <map>
#include <string>
#include <vector>

#include "io/path.h"
#include "types/ret.h"

#include "../project/glyphoutline.h"

namespace mu::fontdesign {
//! FreeType 包装：读取 OTF/TTF 的 cmap、轮廓（含二次→三次升阶）、advance 与 upem。
//! 所有坐标为字体单位（FT_LOAD_NO_SCALE），y 向上。
class FontFaceReader
{
public:
    struct FaceGlyph {
        char32_t codepoint = 0;
        GlyphOutline outline;
        double advance = 0.0;
    };

    struct FaceData {
        double upem = 1000.0;
        double ascender = 0.0;
        double descender = 0.0;
        std::vector<FaceGlyph> glyphs;

        //! 源字体的 OS/2 fsType 嵌入许可位；导出派生字体时原样保留
        uint16_t fsType = 0;
        //! 源字体 name 表中需保留的法律/署名记录（nameID → UTF-8）：
        //! 版权(0)、商标(7)、制造商(8)、设计师(9)、描述(10)、
        //! 厂商URL(11)、设计师URL(12)、许可证(13)、许可证URL(14)。
        //! OFL 要求派生字体保留版权与许可证；商业字体亦需保留其许可信息。
        std::map<uint16_t, std::string> legalNameRecords;
    };

    static muse::Ret read(const muse::io::path_t& path, FaceData& out);
};
}
