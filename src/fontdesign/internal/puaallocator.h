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

#include <set>
#include <string>

#include "project/fontdesignproject.h"
#include "smufldatabase.h"

namespace mu::fontdesign {
//! PUA 码位占用与分配（可选字形自 U+F400 起）
class PuaAllocator
{
public:
    static std::set<char32_t> collectUsedCodepoints(const FontDesignProject& project);

    //! 从 U+F400 起找第一个未占用码位；失败返回 0
    static char32_t nextFreePua(const FontDesignProject& project);

    static bool isUsed(const FontDesignProject& project, char32_t code);

    static std::string toHex(char32_t code);
    static char32_t fromHex(const std::string& text);
};
}
