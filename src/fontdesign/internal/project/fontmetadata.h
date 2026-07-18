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
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "serialization/json.h"

namespace mu::fontdesign {
struct AlternateInfo {
    std::string name;
    char32_t codepoint = 0;
};

struct LigatureInfo {
    char32_t codepoint = 0;
    std::vector<std::string> componentGlyphs;
    std::string description;
};

struct OptionalGlyphInfo {
    char32_t codepoint = 0;
    std::vector<std::string> classes;
    std::string description;
};

struct SetGlyphInfo {
    std::string alternateFor;
    char32_t codepoint = 0;
    std::string name;
    std::string description;
};

struct SetInfo {
    std::string description;
    std::string type;
    std::vector<SetGlyphInfo> glyphs;
};

//! SMuFL 字体级元数据（per-glyph 的 anchors/advance 分布在 GlyphItem 上）。
//! 无法识别的顶层键原样保存在 passthrough 中，写出时透传，保证「读入→保存」不丢数据。
struct FontMetadata {
    std::string fontName;
    double fontVersion = 1.0;

    std::optional<int> designSize;                     // 整数 decipoints
    std::optional<std::pair<int, int>> sizeRange;

    std::map<std::string, double> engravingDefaults;   // 数值键（sp）
    std::string textFontFamily;

    std::map<std::string, std::vector<AlternateInfo>> alternates;  // 基字形名 → 替代形列表
    std::map<std::string, LigatureInfo> ligatures;
    std::map<std::string, OptionalGlyphInfo> optionalGlyphs;
    std::map<std::string, SetInfo> sets;

    muse::JsonObject passthrough;                      // 未识别的顶层键
    muse::JsonObject passthroughAnchors;               // 无法解析的 glyphsWithAnchors 条目
};
}
