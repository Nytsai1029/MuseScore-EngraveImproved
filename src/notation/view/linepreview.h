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

#include "draw/painter.h"
#include "draw/types/geometry.h"

#include "notation/inotation.h"

namespace mu::engraving {
class Score;
class System;
}

//! NOTE A quick look at lines (systems) as they will be printed, without going to the preview:
//! the lines which are selected whole are drawn over the score on white cards, the way the preview draws them
namespace mu::notation {
//! NOTE The lines which the range selection covers whole (each line which has a part of the range has all of it
//! in the range), in their order; none if the selection is not such a range
std::vector<const engraving::System*> wholeSelectedLines(const engraving::Score* score);

//! NOTE The card of a line: the line with what is above and below it, and a margin; it stays on the page
muse::RectF linePreviewCardRect(const engraving::System* line);

void paintLinePreview(muse::draw::Painter* painter, const INotationPtr& notation,
                      const std::vector<const engraving::System*>& lines);
}
