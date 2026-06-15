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

#include <cstddef>
#include <vector>

namespace mu::engraving {
//---------------------------------------------------------
//   SystemReflowSolver
//
//   Pure, DOM-independent solver for the "fit music" reflow feature.
//   Given the (already computed) minimum width of each layout cell and a
//   target number of systems k, it partitions the contiguous cell sequence
//   into exactly k non-empty, non-overlapping, gapless segments that
//   minimise the maximum per-system width sum ("grey level" balancing).
//
//   The solver does not touch any score data; callers map the resulting
//   index segments back to MeasureBase ranges.
//---------------------------------------------------------

class SystemReflowSolver
{
public:
    // Closed index range [startIndex, endIndex] into the input widths vector.
    struct Segment {
        size_t startIndex = 0;
        size_t endIndex = 0;
    };

    // widths:    minimum width of each cell, in score order (size n).
    // k:         target number of systems. Must satisfy 1 <= k <= n.
    // smoothing: when true, run an extra pass that balances adjacent
    //            systems without changing the number of segments.
    //
    // Returns exactly k segments when the input is valid; returns an empty
    // vector when the input is invalid (n == 0, k < 1, or k > n).
    static std::vector<Segment> solve(const std::vector<double>& widths, int k, bool smoothing);

private:
    static std::vector<Segment> partitionMinMax(const std::vector<double>& widths, int k);
    static void smooth(std::vector<Segment>& segments, const std::vector<double>& widths);

    static double segmentLoad(const std::vector<double>& widths, const Segment& seg);
};
} // namespace mu::engraving
