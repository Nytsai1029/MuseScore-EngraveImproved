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

namespace mu::fontdesign {
//! SMuFL glyphsWithAnchors 锚点全集
//! https://w3c-cg.github.io/smufl/latest/specification/glyphswithanchors.html
enum class AnchorId {
    stemUpSE = 0,
    stemUpNW,
    stemDownNW,
    stemDownSW,
    splitStemUpSE,
    splitStemUpSW,
    splitStemDownNE,
    splitStemDownNW,
    cutOutNE,
    cutOutNW,
    cutOutSE,
    cutOutSW,
    opticalCenter,
    noteheadOrigin,
    nominalWidth,
    numeralTop,
    numeralBottom,
    graceNoteSlashSW,
    graceNoteSlashNE,
    graceNoteSlashSE,
    graceNoteSlashNW,
    repeatOffset
};

const std::map<std::string, AnchorId>& anchorIdsByName();
const std::string& anchorNameById(AnchorId id);
//! 简要用途说明（英文键名旁悬浮提示用；文案走 qtrc）
const char* anchorDescriptionKey(AnchorId id);

constexpr char32_t SMUFL_RECOMMENDED_START = 0xE000;
constexpr char32_t SMUFL_RECOMMENDED_END = 0xF3FF;
constexpr char32_t SMUFL_OPTIONAL_START = 0xF400;
}
