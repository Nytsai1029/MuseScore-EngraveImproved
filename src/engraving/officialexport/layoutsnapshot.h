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

#include <array>
#include <functional>
#include <map>
#include <unordered_map>
#include <vector>

#include "draw/types/geometry.h"

#include "types/types.h"

namespace mu::engraving {
class EngravingItem;
class EngravingObject;
class MeasureBase;
class Score;
class Spanner;
class SpannerSegment;
class System;

//! Where layout put one item, in the coordinates of its page
struct ItemGeometry
{
    ElementType type = ElementType::INVALID;
    const EngravingObject* parent = nullptr;
    track_idx_t track = muse::nidx;

    PointF pagePos;

    PointF pos2;                  // line segments: the end point, relative to the start
    std::array<PointF, 4> curve;  // slur and tie segments: start, the two bezier points, end
    double length = 0.0;          // stems
    PointF p1;                    // tuplets: the two ends of the bracket
    PointF p2;
};

//! Where layout put one system on its page, and its staves in it
struct SystemGeometry
{
    size_t page = 0;
    double y = 0.0;
    double height = 0.0;
    std::vector<double> staffY;
};

//! The layout of a score at one moment: what an export for official MuseScore Studio has to reproduce.
//! Items are found again by address, checked against their type, parent and track, because a later
//! layout may have put another item at the address of one it deleted. Spanner segments are handed out
//! anew by every layout, so they are found by their spanner and their place in its list of segments.
class LayoutSnapshot
{
public:
    static LayoutSnapshot capture(Score* score);

    const ItemGeometry* find(const EngravingItem* item) const;
    void set(const EngravingItem* item, const ItemGeometry& geometry);
    void setSpannerSegment(const Spanner* spanner, size_t index, const ItemGeometry& geometry);
    void removeSpanner(const Spanner* spanner);

    //! A system is found again by the first of its measures: breaks are locked before any other layout
    const SystemGeometry* findSystem(const System* system) const;

    static ItemGeometry geometryOf(const EngravingItem* item);

    //! Every item of the laid out pages, each one after the item it is positioned against
    static std::vector<EngravingItem*> laidOutItems(Score* score);

private:
    std::unordered_map<const EngravingItem*, ItemGeometry> m_items;
    std::map<std::pair<const Spanner*, size_t>, ItemGeometry> m_spannerSegments;
    std::unordered_map<const MeasureBase*, SystemGeometry> m_systems;
};
}
