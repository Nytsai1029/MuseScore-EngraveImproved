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
#include "puaallocator.h"

#include <cstdio>

#include "fontdesigntypes.h"

using namespace mu::fontdesign;

std::set<char32_t> PuaAllocator::collectUsedCodepoints(const FontDesignProject& project)
{
    std::set<char32_t> used;

    for (const auto& pair : project.glyphs()) {
        used.insert(pair.first);
    }

    const FontMetadata& metadata = project.metadata();

    for (const auto& pair : metadata.alternates) {
        for (const AlternateInfo& alt : pair.second) {
            if (alt.codepoint != 0) {
                used.insert(alt.codepoint);
            }
        }
    }

    for (const auto& pair : metadata.ligatures) {
        if (pair.second.codepoint != 0) {
            used.insert(pair.second.codepoint);
        }
    }

    for (const auto& pair : metadata.optionalGlyphs) {
        if (pair.second.codepoint != 0) {
            used.insert(pair.second.codepoint);
        }
    }

    for (const auto& pair : metadata.sets) {
        for (const SetGlyphInfo& glyph : pair.second.glyphs) {
            if (glyph.codepoint != 0) {
                used.insert(glyph.codepoint);
            }
        }
    }

    return used;
}

char32_t PuaAllocator::nextFreePua(const FontDesignProject& project)
{
    std::set<char32_t> used = collectUsedCodepoints(project);

    for (char32_t code = SMUFL_OPTIONAL_START; code <= 0xF8FF; ++code) {
        if (used.find(code) == used.end()) {
            return code;
        }
    }

    return 0;
}

bool PuaAllocator::isUsed(const FontDesignProject& project, char32_t code)
{
    if (code == 0) {
        return false;
    }

    return collectUsedCodepoints(project).count(code) > 0;
}

std::string PuaAllocator::toHex(char32_t code)
{
    char buf[16];
    std::snprintf(buf, sizeof(buf), "U+%04X", static_cast<unsigned int>(code));
    return buf;
}

char32_t PuaAllocator::fromHex(const std::string& text)
{
    return SmuflDatabase::codepointFromString(text);
}
