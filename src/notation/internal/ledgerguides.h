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

#include <vector>

#include "draw/types/geometry.h"

namespace mu::engraving {
class Page;
}

namespace mu::notation {
//! NOTE The ledger guides are lines which continue the staff above and below it: one at every position
//! of a ledger line and one in every space between them. They are a working aid of the Engrave mode,
//! which is drawn behind the score and is not a part of it

//! NOTE No more ledger lines than this on one side of a staff, whatever is there
inline constexpr int MAX_LEDGER_GUIDES = 16;

struct LedgerGuide
{
    muse::LineF line;           // in the coordinates of the page
    bool isSpace = false;       // between two ledger lines (otherwise at the position of a ledger line)
    size_t colorIndex = 0;      // the staves of a system take two colours in turn
    double width = 0.0;         // the width of the staff lines
    double dash = 0.0;          // the distance between the staff lines
};

//! NOTE How many ledger lines the guides cover on one side of a staff: up to the first one
//! which is further out than the outermost element.
//! The distance is from the outer staff line to the outer edge of that element (negative when it is inside)
int ledgerGuideCount(double distanceBeyondStaff, double lineDistance);

//! NOTE The guides of the staves of a page.
//! How far they reach is found for every staff of every system, above and below, from its notes,
//! stems, flags, beams and rests, so it follows what is changed in the score.
//! If the visible rect is valid, only what can be seen in it is returned
std::vector<LedgerGuide> ledgerGuides(const mu::engraving::Page* page, const muse::RectF& visibleRect = muse::RectF());
}
