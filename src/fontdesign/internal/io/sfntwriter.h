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

#include "types/ret.h"

namespace mu::fontdesign {
//! sfnt / OpenType 容器（'OTTO' + CFF）。表按 tag 排序写入，最后回填 head.checkSumAdjustment。
class SfntWriter
{
public:
    struct NameStrings {
        std::string family;
        std::string subfamily = "Regular";
        std::string fullName;
        std::string postScriptName;
        std::string version = "Version 1.000";
        //! 从源字体保留的法律/署名记录（nameID → UTF-8）：版权(0)、许可证(13/14) 等。
        //! 与上面重新生成的 1-6 号记录不冲突（1-6 会被跳过）。
        std::map<uint16_t, std::string> legalRecords;
    };

    struct Metrics {
        int upem = 1000;
        int ascender = 800;
        int descender = -200;
        int lineGap = 0;
        int xMin = 0;
        int yMin = 0;
        int xMax = 0;
        int yMax = 0;
        uint16_t fsType = 0;
    };

    struct GlyphMetrics {
        int advance = 0;
        int lsb = 0;
    };

    struct Input {
        Metrics metrics;
        NameStrings names;
        std::vector<GlyphMetrics> glyphMetrics; // 与 CFF 字形顺序一致
        std::map<char32_t, uint16_t> cmap;      // codepoint → glyph index
        std::vector<uint8_t> cffTable;
    };

    static muse::Ret write(const Input& input, std::vector<uint8_t>& out);
};
}
