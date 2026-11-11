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

#include <algorithm>
#include <map>

#include "global/types/translatablestring.h"

namespace mu::engraving {
//! What the official MuseScore Studio file format cannot hold, so an export to it leaves out
enum class DroppedFeature {
    LetterSpacing,
    LedgerLineLengths,
    HiddenLedgerLines,
    BrokenOttavaLines,
    SlantedText,
    HairpinVerticalEnds,
    BracketSymbolSizes,
    PerTextBarlineMask,
    KeySigAccidentalDistances,
    TremoloStrokeOffsets,
    ParenthesisEndOffsets,
    SeparateAccidentalParentheses,
    BarlineConnectorEdits,
    MusicSymbolTextCharacters,
    GraceNotesBeforeSystemBreak,
    SlurPiecesNotSplit,
};

struct OfficialExportReport
{
    std::map<DroppedFeature, int> dropped; // how many times each one was met

    int pinnedItems = 0;         // items that got an explicit value
    int unpinnedItems = 0;       // items still away from where the fork laid them out
    double maxResidualSp = 0.0;  // the farthest such item, in spatium
    int passes = 0;

    void addDropped(DroppedFeature feature, int count = 1)
    {
        if (count > 0) {
            dropped[feature] += count;
        }
    }

    void merge(const OfficialExportReport& other)
    {
        for (const auto& [feature, count] : other.dropped) {
            dropped[feature] += count;
        }
        pinnedItems += other.pinnedItems;
        unpinnedItems += other.unpinnedItems;
        maxResidualSp = std::max(maxResidualSp, other.maxResidualSp);
        passes = std::max(passes, other.passes);
    }
};

muse::TranslatableString droppedFeatureTitle(DroppedFeature feature);
}
