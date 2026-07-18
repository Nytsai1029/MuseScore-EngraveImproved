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

#include <string>
#include <vector>

#include "io/path.h"
#include "types/ret.h"

namespace mu::fontdesign {
class FontDesignProject;

//! 组装 OTF(CFF) 并写盘，随后用 FreeType 回读校验。
class FontExporter
{
public:
    struct Report {
        bool ok = false;
        std::string message;
        int numGlyphs = 0;
        int cmapEntries = 0;
        std::vector<std::string> warnings;
    };

    static muse::Ret exportFont(const FontDesignProject& project, const muse::io::path_t& path, Report* report = nullptr);

    //! 仅生成字节（便于测试）
    static muse::Ret buildFontBytes(const FontDesignProject& project, std::vector<uint8_t>& out, Report* report = nullptr);
};
}
