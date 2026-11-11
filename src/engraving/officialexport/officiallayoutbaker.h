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
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "draw/types/geometry.h"

#include "dom/masterscore.h"

#include "layoutsnapshot.h"
#include "officialexportreport.h"

namespace mu::engraving {
class Note;
class Segment;
class System;

//! Prepares a score for official MuseScore Studio, which lays out by upstream's rules and knows none of
//! the fork's settings: whatever the fork's layout decided is written into the score as the explicit values
//! official has for it (locked systems, beam positions, leading space, offsets, curve points, stem lengths),
//! so that official's own layout of the saved file comes out where the fork's did.
//!
//! Official keeps nearly all of these values relative to its automatic layout, so each one is found as
//! "where the fork put the item" minus "where upstream's rules put it". The second term comes from laying
//! the score out here with the fork's rules switched off (MasterScore::officialCompat), and since moving
//! one item moves others, that is repeated until nothing moves any more.
//!
//! Horizontally, what all items of a segment have in common goes into the segment's leading space, not into
//! their offsets: upstream spaces segments by the shapes of their items, offsets included, so an offset
//! cannot bring two notes closer than its collision rules allow, while leading space is added after them.
//! The leading space is not found by trying: the segments are given pins, and spacing, which knows what the
//! leading space of each segment does, works it out as it goes (HorizontalSpacing::pinSegment).
//!
//! The score is changed for good and is of no use to the fork afterwards: bake a copy, never the open score.
class OfficialLayoutBaker
{
public:
    //! masterScore and its parts must be laid out
    static OfficialExportReport bake(MasterScore* masterScore);

private:
    //! An accidental as the fork shows it
    struct ShownAccidental {
        Note* note = nullptr;
        AccidentalType type = AccidentalType::NONE;
        AccidentalBracket bracket = AccidentalBracket::NONE;
        ItemGeometry geometry;
    };

    OfficialLayoutBaker(Score* score, LayoutSnapshot& target, OfficialExportReport& report);

    static std::vector<ShownAccidental> shownAccidentals(Score* score);
    void restoreAccidentals(const std::vector<ShownAccidental>& accidentals);

    struct SegmentEntry {
        Segment* segment = nullptr;
        EngravingItem* anchor = nullptr; // the item that says where the segment is
        double delta = 0.0;              // how far that is from where it should be
    };

    void pin(const std::vector<ShownAccidental>& accidentals);
    void relayout();
    std::vector<SegmentEntry> segmentEntries(const System* system) const;
    double tensionForMMRests(const System* system, const std::vector<SegmentEntry>& entries, double& farthest) const;
    double pinSegments(bool write);
    double pinVertical(bool write);
    double pinItems(bool write);
    double correct(EngravingItem* item, const ItemGeometry& want, bool write);
    void addOffset(EngravingItem* item, const PointF& delta);
    EngravingItem* anchorItem(const Segment& segment) const;

    Score* m_score = nullptr;
    LayoutSnapshot& m_target;
    OfficialExportReport& m_report;

    double m_tolerance = 0.0;
    int m_farItems = 0;
    std::map<ElementType, int> m_farByType;
    int m_verticalPasses = 0;
    std::unordered_set<size_t> m_fixedPages; // pages that have the distances of their systems fixed
    std::unordered_set<const EngravingObject*> m_pinned;
    std::unordered_map<const EngravingItem*, int> m_stubborn; // passes in a row an item stayed away

    SegmentPins m_pins; // where spacing is to put the segments: see pinSegments

    struct SystemState {
        bool pinned = false;        // upstream's own spacing does not put its segments right
        bool givenUp = false;       // nor do the pins: its items take what is left
        bool widestTension = false; // stretched as far as the pins allow, not as far as it was
        int passes = 0;             // corrections of its pins
        int tensionPasses = 0;      // corrections of its tension
        double give = 0.0;          // by how much its end is pinned short of where it belongs
    };
    std::unordered_map<const MeasureBase*, SystemState> m_systemStates; // by the system's first measure
};
}
