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

#include "io/path.h"
#include "types/ret.h"

#include "draw/types/geometry.h"

#include "../fontdesigntypes.h"
#include "../smufldatabase.h"
#include "../project/fontmetadata.h"

namespace mu::fontdesign {
//! SMuFL 元数据 JSON 读取：全段解析、容错、未知键透传。
//! 锚点按 字形名 → 码位 解析（先查 optionalGlyphs 声明，再查规范 glyphnames），
//! 解析不了的条目进 passthroughAnchors。
class MetadataReader
{
public:
    static muse::Ret read(const muse::io::path_t& path, const SmuflDatabase& db,
                          FontMetadata& out,
                          std::map<char32_t, std::map<AnchorId, muse::PointF>>& anchorsByCode);
};
}
