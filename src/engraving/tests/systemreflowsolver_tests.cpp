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

#include <gtest/gtest.h>

#include <algorithm>
#include <functional>
#include <limits>
#include <numeric>
#include <vector>

#include "dom/systemreflowsolver.h"

using namespace mu::engraving;

using Segment = SystemReflowSolver::Segment;

class Engraving_SystemReflowSolverTests : public ::testing::Test
{
protected:
    static double segLoad(const std::vector<double>& w, const Segment& s)
    {
        double sum = 0.0;
        for (size_t i = s.startIndex; i <= s.endIndex; ++i) {
            sum += w[i];
        }
        return sum;
    }

    static double maxLoad(const std::vector<double>& w, const std::vector<Segment>& segs)
    {
        double m = 0.0;
        for (const Segment& s : segs) {
            m = std::max(m, segLoad(w, s));
        }
        return m;
    }

    // Asserts Property 1 (completeness) and Property 2 (exactly k, non-empty).
    static void checkPartitionValid(const std::vector<double>& w, int k, const std::vector<Segment>& segs)
    {
        ASSERT_EQ(segs.size(), static_cast<size_t>(k));
        ASSERT_FALSE(segs.empty());

        EXPECT_EQ(segs.front().startIndex, size_t(0));
        EXPECT_EQ(segs.back().endIndex, w.size() - 1);

        for (size_t i = 0; i < segs.size(); ++i) {
            // Non-empty.
            EXPECT_LE(segs[i].startIndex, segs[i].endIndex);
            // Contiguous and gapless.
            if (i + 1 < segs.size()) {
                EXPECT_EQ(segs[i].endIndex + 1, segs[i + 1].startIndex);
            }
        }
    }

    // Brute-force optimal min-max over all ways to place k-1 cut points.
    static double bruteForceOptimalMaxLoad(const std::vector<double>& w, int k)
    {
        const size_t n = w.size();
        std::vector<double> prefix(n + 1, 0.0);
        for (size_t i = 0; i < n; ++i) {
            prefix[i + 1] = prefix[i] + w[i];
        }

        double best = std::numeric_limits<double>::max();

        // Choose k-1 internal cut positions from {1..n-1}.
        std::vector<size_t> cuts(static_cast<size_t>(k - 1));
        std::function<void(size_t, size_t)> rec = [&](size_t idx, size_t startPos) {
            if (idx == cuts.size()) {
                std::vector<size_t> bounds;
                bounds.push_back(0);
                for (size_t c : cuts) {
                    bounds.push_back(c);
                }
                bounds.push_back(n);
                double m = 0.0;
                for (size_t b = 0; b + 1 < bounds.size(); ++b) {
                    m = std::max(m, prefix[bounds[b + 1]] - prefix[bounds[b]]);
                }
                best = std::min(best, m);
                return;
            }
            for (size_t p = startPos; p <= n - (cuts.size() - idx); ++p) {
                cuts[idx] = p;
                rec(idx + 1, p + 1);
            }
        };

        if (k == 1) {
            return prefix[n];
        }
        rec(0, 1);
        return best;
    }
};

TEST_F(Engraving_SystemReflowSolverTests, EqualWidthsDivideEvenly)
{
    std::vector<double> w(12, 10.0);
    std::vector<Segment> segs = SystemReflowSolver::solve(w, 3, false);

    checkPartitionValid(w, 3, segs);
    for (const Segment& s : segs) {
        EXPECT_EQ(s.endIndex - s.startIndex + 1, size_t(4));
    }
}

TEST_F(Engraving_SystemReflowSolverTests, SingleSystemContainsAllCells)
{
    std::vector<double> w{ 3.0, 1.0, 4.0, 1.0, 5.0 };
    std::vector<Segment> segs = SystemReflowSolver::solve(w, 1, false);

    checkPartitionValid(w, 1, segs);
    EXPECT_EQ(segs.front().startIndex, size_t(0));
    EXPECT_EQ(segs.front().endIndex, w.size() - 1);
}

TEST_F(Engraving_SystemReflowSolverTests, KEqualsNGivesOneCellPerSystem)
{
    std::vector<double> w{ 3.0, 1.0, 4.0, 1.0, 5.0 };
    std::vector<Segment> segs = SystemReflowSolver::solve(w, static_cast<int>(w.size()), false);

    checkPartitionValid(w, static_cast<int>(w.size()), segs);
    for (const Segment& s : segs) {
        EXPECT_EQ(s.startIndex, s.endIndex);
    }
}

TEST_F(Engraving_SystemReflowSolverTests, MinMaxMatchesBruteForce)
{
    const std::vector<std::vector<double> > cases = {
        { 3, 1, 4, 1, 5, 9, 2, 6 },
        { 10, 2, 2, 2, 10, 2, 2 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 7, 3, 3, 7, 1, 1, 8, 2 },
    };

    for (const std::vector<double>& w : cases) {
        for (int k = 1; k <= static_cast<int>(w.size()); ++k) {
            std::vector<Segment> segs = SystemReflowSolver::solve(w, k, false);
            checkPartitionValid(w, k, segs);
            EXPECT_NEAR(maxLoad(w, segs), bruteForceOptimalMaxLoad(w, k), 1e-9)
                << "n=" << w.size() << " k=" << k;
        }
    }
}

TEST_F(Engraving_SystemReflowSolverTests, NEqualsTwo)
{
    std::vector<double> w{ 4.0, 6.0 };
    std::vector<Segment> segs = SystemReflowSolver::solve(w, 2, false);
    checkPartitionValid(w, 2, segs);
    EXPECT_EQ(segs[0].startIndex, size_t(0));
    EXPECT_EQ(segs[0].endIndex, size_t(0));
    EXPECT_EQ(segs[1].startIndex, size_t(1));
}

TEST_F(Engraving_SystemReflowSolverTests, ZeroWidthCellsHandled)
{
    std::vector<double> w{ 0.0, 5.0, 0.0, 5.0, 0.0 };
    std::vector<Segment> segs = SystemReflowSolver::solve(w, 2, false);
    checkPartitionValid(w, 2, segs);
    EXPECT_NEAR(maxLoad(w, segs), bruteForceOptimalMaxLoad(w, 2), 1e-9);
}

TEST_F(Engraving_SystemReflowSolverTests, SmoothingKeepsSegmentCountAndDoesNotWorsenBalance)
{
    // A region where the raw min-max partition can leave an imbalanced pair.
    std::vector<double> w{ 5, 5, 5, 1, 1, 1, 1, 1, 9 };

    for (int k = 2; k <= 4; ++k) {
        std::vector<Segment> raw = SystemReflowSolver::solve(w, k, false);
        std::vector<Segment> smoothed = SystemReflowSolver::solve(w, k, true);

        checkPartitionValid(w, k, smoothed);

        // Property 5: segment count unchanged.
        EXPECT_EQ(raw.size(), smoothed.size());

        // Sum of adjacent load differences should not increase after smoothing.
        auto adjacentDiffSum = [&](const std::vector<Segment>& segs) {
            double total = 0.0;
            for (size_t i = 0; i + 1 < segs.size(); ++i) {
                total += std::abs(segLoad(w, segs[i]) - segLoad(w, segs[i + 1]));
            }
            return total;
        };
        EXPECT_LE(adjacentDiffSum(smoothed), adjacentDiffSum(raw) + 1e-9) << "k=" << k;
    }
}

TEST_F(Engraving_SystemReflowSolverTests, InvalidInputsReturnEmpty)
{
    std::vector<double> w{ 1.0, 2.0, 3.0 };
    EXPECT_TRUE(SystemReflowSolver::solve(w, 0, false).empty());
    EXPECT_TRUE(SystemReflowSolver::solve(w, 4, false).empty());
    EXPECT_TRUE(SystemReflowSolver::solve({}, 1, false).empty());
}
