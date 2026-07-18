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

#include "io/path.h"
#include "types/ret.h"

namespace mu::fontdesign {
class FontDesignProject;

//! SMuFL 元数据 JSON 写出。
//! 自研序列化（不走 picojson）以保证：段按 SMuFL 惯例顺序、键序稳定（字典序）、
//! 数值格式可控（最多 5 位小数并去尾零，避免浮点噪声），空段省略、未知键透传。
//! glyphAdvanceWidths / glyphBBoxes 由当前字体数据自动生成。
class MetadataWriter
{
public:
    static muse::Ret write(const FontDesignProject& project, const muse::io::path_t& path);

    //! 便于测试：直接产出 JSON 文本
    static std::string toJsonText(const FontDesignProject& project);
};
}
