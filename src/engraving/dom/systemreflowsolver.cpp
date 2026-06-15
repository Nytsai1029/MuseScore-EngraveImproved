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

#include "systemreflowsolver.h"

#include <algorithm>
#include <cmath>
#include <limits>

using namespace mu::engraving;

std::vector<SystemReflowSolver::Segment> SystemReflowSolver::solve(const std::vector<double>& widths, int k, bool smoothing)
{
    const size_t n = widths.size();
    if (n == 0 || k < 1 || static_cast<size_t>(k) > n) {
        return {};
    }

    std::vector<Segment> segments = partitionMinMax(widths, k);

    if (smoothing) {
        smooth(segments, widths);
    }

    return segments;
}

//---------------------------------------------------------
//   partitionMinMax
//
//   Classic "linear partition" dynamic programming. Splits the contiguous
//   sequence into exactly k non-empty segments minimising the maximum
//   segment load (sum of widths). Runs in O(n^2 * k); n is the number of
//   cells in a user-selected region, so it is comfortably small.
//---------------------------------------------------------

std::vector<SystemReflowSolver::Segment> SystemReflowSolver::partitionMinMax(const std::vector<double>& widths, int k)
{
    const size_t n = widths.size();
    const size_t kk = static_cast<size_t>(k);

    // prefix[i] = sum of widths[0..i-1]
    std::vector<double> prefix(n + 1, 0.0);
    for (size_t i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + widths[i];
    }

    const double kInf = std::numeric_limits<double>::max();

    // dp[i][j] = minimal possible "max segment load" when splitting the
    // first i cells into j segments. cut[i][j] = start index of the j-th
    // (last) segment in that optimal split, used for reconstruction.
    std::vector<std::vector<double> > dp(n + 1, std::vector<double>(kk + 1, kInf));
    std::vector<std::vector<size_t> > cut(n + 1, std::vector<size_t>(kk + 1, 0));

    dp[0][0] = 0.0;
    for (size_t i = 1; i <= n; ++i) {
        const size_t maxJ = std::min(i, kk);
        for (size_t j = 1; j <= maxJ; ++j) {
            // Last segment is widths[m..i-1] for some m in [j-1, i-1].
            for (size_t m = j - 1; m < i; ++m) {
                if (dp[m][j - 1] == kInf) {
                    continue;
                }
                const double lastLoad = prefix[i] - prefix[m];
                const double candidate = std::max(dp[m][j - 1], lastLoad);
                if (candidate < dp[i][j]) {
                    dp[i][j] = candidate;
                    cut[i][j] = m;
                }
            }
        }
    }

    // Reconstruct segment boundaries from the back.
    std::vector<Segment> segments(kk);
    size_t end = n;
    for (size_t j = kk; j >= 1; --j) {
        const size_t start = cut[end][j];
        segments[j - 1] = Segment{ start, end - 1 };
        end = start;
        if (j == 1) {
            break;
        }
    }

    return segments;
}

//---------------------------------------------------------
//   smooth
//
//   Secondary local-optimisation pass that keeps the number of segments
//   fixed. It repeatedly tries to move a single cell across an adjacent
//   boundary when doing so reduces the absolute load difference of that
//   pair, without creating an empty segment. This lowers the variance of
//   system loads (Property 5) even where it does not change the global max.
//---------------------------------------------------------

void SystemReflowSolver::smooth(std::vector<Segment>& segments, const std::vector<double>& widths)
{
    if (segments.size() < 2) {
        return;
    }

    const size_t maxIterations = segments.size() * 4 + 8;

    for (size_t iter = 0; iter < maxIterations; ++iter) {
        bool improved = false;

        for (size_t i = 0; i + 1 < segments.size(); ++i) {
            Segment& left = segments[i];
            Segment& right = segments[i + 1];

            const double leftLoad = segmentLoad(widths, left);
            const double rightLoad = segmentLoad(widths, right);
            const double currentDiff = std::abs(leftLoad - rightLoad);

            // Option A: move the last cell of the left segment into the right
            // segment (left must keep at least one cell).
            if (left.endIndex > left.startIndex) {
                const double moved = widths[left.endIndex];
                const double newDiff = std::abs((leftLoad - moved) - (rightLoad + moved));
                if (newDiff < currentDiff) {
                    left.endIndex -= 1;
                    right.startIndex -= 1;
                    improved = true;
                    continue;
                }
            }

            // Option B: move the first cell of the right segment into the left
            // segment (right must keep at least one cell).
            if (right.endIndex > right.startIndex) {
                const double moved = widths[right.startIndex];
                const double newDiff = std::abs((leftLoad + moved) - (rightLoad - moved));
                if (newDiff < currentDiff) {
                    left.endIndex += 1;
                    right.startIndex += 1;
                    improved = true;
                }
            }
        }

        if (!improved) {
            break;
        }
    }
}

double SystemReflowSolver::segmentLoad(const std::vector<double>& widths, const Segment& seg)
{
    double sum = 0.0;
    for (size_t i = seg.startIndex; i <= seg.endIndex; ++i) {
        sum += widths[i];
    }
    return sum;
}
