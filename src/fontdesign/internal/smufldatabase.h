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
#include <string>
#include <vector>

namespace mu::fontdesign {
//! SMuFL 规范参考数据（:/fonts/smufl/{glyphnames,ranges,classes}.json）。
//! 与 engraving 的 Smufl 类不同，这里以开放的 名称↔码位 映射工作，
//! 不受固定 SymId 枚举限制，可表达任意推荐/可选字形。
class SmuflDatabase
{
public:
    struct GlyphInfo {
        std::string name;
        char32_t codepoint = 0;
        std::string description;
    };

    struct Range {
        std::string id;
        std::string description;
        char32_t start = 0;
        char32_t end = 0;
        std::vector<std::string> glyphNames;
    };

    //! dir 为含 glyphnames/ranges/classes.json 的目录；默认 Qt 资源前缀
    void init(const std::string& dir = ":/fonts/smufl");
    bool isInited() const;

    const std::vector<Range>& ranges() const;

    const GlyphInfo* infoByName(const std::string& name) const;
    const GlyphInfo* infoByCodepoint(char32_t code) const;

    const std::vector<std::string>& classNames() const;
    std::vector<std::string> classesOfGlyph(const std::string& glyphName) const;

    //! "U+E0A4" -> 0xE0A4；解析失败返回 0
    static char32_t codepointFromString(const std::string& str);

private:
    bool m_inited = false;
    std::vector<Range> m_ranges;
    std::map<std::string, GlyphInfo> m_glyphsByName;
    std::map<char32_t, std::string> m_namesByCode;
    std::vector<std::string> m_classNames;
    std::map<std::string, std::vector<std::string>> m_classesByGlyph;
};
}
