/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

namespace mu::engraving {
//---------------------------------------------------------
//   FitMusicOptions
//
//   Parameters for the "fit music" reflow command, passed from the UI
//   (dialog) down to Score::fitMusicReflow.
//
//   - Absolute mode (relativeMode == false): the region is distributed over
//     exactly targetSystemCount systems.
//   - Relative mode (relativeMode == true): the target is the region's
//     current system count plus relativeDelta. A relativeDelta of 0 keeps
//     the current number of systems and only rebalances the cells.
//---------------------------------------------------------

struct FitMusicOptions {
    bool relativeMode = false;
    int targetSystemCount = 1;
    int relativeDelta = 0;
    bool smoothing = true;
};
} // namespace mu::engraving
