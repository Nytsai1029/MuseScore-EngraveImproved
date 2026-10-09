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

#include <map>
#include <set>
#include <string>

#include "io/path.h"
#include "types/ret.h"

#include "draw/types/geometry.h"

#include "../fontdesigntypes.h"
#include "../smufldatabase.h"
#include "../project/fontmetadata.h"

namespace mu::fontdesign {
//! SMuFL 元数据 JSON 读取：全段解析、容错、未知键透传。
//! 原则：读不懂或对不上字形的内容一律原样保留（透传），绝不因为「读入→保存」而丢数据。
//! 锚点按 字形名 → 码位 解析：optionalGlyphs 声明 → 规范 glyphnames → 字体自带字形名 → uniXXXX；
//! 解析不了、或字体里没有该字形的条目进 passthroughAnchors。
class MetadataReader
{
public:
    //! 字体侧信息：元数据里的名字要对到字体里实际存在的字形
    struct FontGlyphIndex {
        std::set<char32_t> codepoints;                      // 字体里有字形的码位
        std::map<std::string, char32_t> codepointByName;    // 字体自带的字形名（post/CFF）→ 码位
    };

    struct Output {
        FontMetadata metadata;
        std::map<char32_t, std::map<AnchorId, muse::PointF>> anchorsByCode;
        //! 元数据里用作字形键的名字（glyphBBoxes / glyphAdvanceWidths / glyphsWithAnchors）
        std::set<std::string> glyphKeys;
    };

    static muse::Ret read(const muse::io::path_t& path, const SmuflDatabase& db, const FontGlyphIndex& font,
                          Output& out);

    //! "uniXXXX" / "uXXXX[XX]"（大写十六进制，4–6 位）→ 码位；不是这种名字返回 0
    static char32_t codepointFromUniName(const std::string& name);
};
}
