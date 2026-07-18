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
#include "fontdesigntypes.h"

#include "translation.h"

namespace mu::fontdesign {
const std::map<std::string, AnchorId>& anchorIdsByName()
{
    static const std::map<std::string, AnchorId> map {
        { "stemUpSE", AnchorId::stemUpSE },
        { "stemUpNW", AnchorId::stemUpNW },
        { "stemDownNW", AnchorId::stemDownNW },
        { "stemDownSW", AnchorId::stemDownSW },
        { "splitStemUpSE", AnchorId::splitStemUpSE },
        { "splitStemUpSW", AnchorId::splitStemUpSW },
        { "splitStemDownNE", AnchorId::splitStemDownNE },
        { "splitStemDownNW", AnchorId::splitStemDownNW },
        { "cutOutNE", AnchorId::cutOutNE },
        { "cutOutNW", AnchorId::cutOutNW },
        { "cutOutSE", AnchorId::cutOutSE },
        { "cutOutSW", AnchorId::cutOutSW },
        { "opticalCenter", AnchorId::opticalCenter },
        { "noteheadOrigin", AnchorId::noteheadOrigin },
        { "nominalWidth", AnchorId::nominalWidth },
        { "numeralTop", AnchorId::numeralTop },
        { "numeralBottom", AnchorId::numeralBottom },
        { "graceNoteSlashSW", AnchorId::graceNoteSlashSW },
        { "graceNoteSlashNE", AnchorId::graceNoteSlashNE },
        { "graceNoteSlashSE", AnchorId::graceNoteSlashSE },
        { "graceNoteSlashNW", AnchorId::graceNoteSlashNW },
        { "repeatOffset", AnchorId::repeatOffset },
    };

    return map;
}

const std::string& anchorNameById(AnchorId id)
{
    for (const auto& pair : anchorIdsByName()) {
        if (pair.second == id) {
            return pair.first;
        }
    }

    static const std::string dummy;
    return dummy;
}

const char* anchorDescriptionKey(AnchorId id)
{
    // 返回可翻译源文案（英文）。此处用 QT_TRANSLATE_NOOP 标记以便 lupdate 抽取到
    // "fontdesign" 上下文，真正的翻译在使用处（如 AnchorsModel）以 qtrc("fontdesign", key) 完成。
    switch (id) {
    case AnchorId::stemUpSE:
        return QT_TRANSLATE_NOOP("fontdesign", "Attachment point for an upward stem on the south-east of the glyph (typical notehead right side).");
    case AnchorId::stemUpNW:
        return QT_TRANSLATE_NOOP("fontdesign", "Attachment point for an upward stem on the north-west of the glyph.");
    case AnchorId::stemDownNW:
        return QT_TRANSLATE_NOOP("fontdesign", "Attachment point for a downward stem on the north-west of the glyph (typical notehead left side).");
    case AnchorId::stemDownSW:
        return QT_TRANSLATE_NOOP("fontdesign", "Attachment point for a downward stem on the south-west of the glyph.");
    case AnchorId::splitStemUpSE:
        return QT_TRANSLATE_NOOP("fontdesign", "Outer (south-east) point when a stem splits upward around the glyph.");
    case AnchorId::splitStemUpSW:
        return QT_TRANSLATE_NOOP("fontdesign", "Inner (south-west) point when a stem splits upward around the glyph.");
    case AnchorId::splitStemDownNE:
        return QT_TRANSLATE_NOOP("fontdesign", "Outer (north-east) point when a stem splits downward around the glyph.");
    case AnchorId::splitStemDownNW:
        return QT_TRANSLATE_NOOP("fontdesign", "Inner (north-west) point when a stem splits downward around the glyph.");
    case AnchorId::cutOutNE:
        return QT_TRANSLATE_NOOP("fontdesign", "North-east cut-out corner of the glyph bounding box for collision avoidance.");
    case AnchorId::cutOutNW:
        return QT_TRANSLATE_NOOP("fontdesign", "North-west cut-out corner of the glyph bounding box for collision avoidance.");
    case AnchorId::cutOutSE:
        return QT_TRANSLATE_NOOP("fontdesign", "South-east cut-out corner of the glyph bounding box for collision avoidance.");
    case AnchorId::cutOutSW:
        return QT_TRANSLATE_NOOP("fontdesign", "South-west cut-out corner of the glyph bounding box for collision avoidance.");
    case AnchorId::opticalCenter:
        return QT_TRANSLATE_NOOP("fontdesign", "Optical (visual) center used for alignment of dynamics, ornaments, and similar marks.");
    case AnchorId::noteheadOrigin:
        return QT_TRANSLATE_NOOP("fontdesign", "Horizontal origin of the notehead for positioning relative to the stem or rhythmic slot.");
    case AnchorId::nominalWidth:
        return QT_TRANSLATE_NOOP("fontdesign", "Nominal advance width marker (x = width in staff spaces); used when width differs from the outline.");
    case AnchorId::numeralTop:
        return QT_TRANSLATE_NOOP("fontdesign", "Top attachment for multi-digit time-signature or similar stacked numerals.");
    case AnchorId::numeralBottom:
        return QT_TRANSLATE_NOOP("fontdesign", "Bottom attachment for multi-digit time-signature or similar stacked numerals.");
    case AnchorId::graceNoteSlashSW:
        return QT_TRANSLATE_NOOP("fontdesign", "South-west end of a grace-note slash through the stem or flag.");
    case AnchorId::graceNoteSlashNE:
        return QT_TRANSLATE_NOOP("fontdesign", "North-east end of a grace-note slash through the stem or flag.");
    case AnchorId::graceNoteSlashSE:
        return QT_TRANSLATE_NOOP("fontdesign", "South-east end of a grace-note slash through the stem or flag.");
    case AnchorId::graceNoteSlashNW:
        return QT_TRANSLATE_NOOP("fontdesign", "North-west end of a grace-note slash through the stem or flag.");
    case AnchorId::repeatOffset:
        return QT_TRANSLATE_NOOP("fontdesign", "Horizontal offset for aligning repeat-dot pairs or similar repeated marks.");
    }
    return "";
}
}
