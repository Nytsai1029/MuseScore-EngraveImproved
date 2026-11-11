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
#include "layoutsnapshot.h"

#include <algorithm>
#include <unordered_set>

#include "dom/bracket.h"
#include "dom/engravingitem.h"
#include "dom/instrumentname.h"
#include "dom/measure.h"
#include "dom/measurenumber.h"
#include "dom/mmrestrange.h"
#include "dom/page.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/slurtie.h"
#include "dom/spanner.h"
#include "dom/stem.h"
#include "dom/system.h"
#include "dom/systemdivider.h"
#include "dom/tuplet.h"

using namespace mu::engraving;

static const std::array<Grip, 4> CURVE_GRIPS { Grip::START, Grip::BEZIER1, Grip::BEZIER2, Grip::END };

static size_t spannerSegmentIndex(const SpannerSegment* segment)
{
    const std::vector<SpannerSegment*>& segments = segment->spanner()->spannerSegments();
    return size_t(std::find(segments.begin(), segments.end(), segment) - segments.begin());
}

static bool sameItem(const ItemGeometry& geometry, const EngravingItem* item)
{
    return geometry.type == item->type() && geometry.parent == item->explicitParent() && geometry.track == item->track();
}

LayoutSnapshot LayoutSnapshot::capture(Score* score)
{
    LayoutSnapshot snapshot;
    for (const EngravingItem* item : laidOutItems(score)) {
        snapshot.set(item, geometryOf(item));
    }

    for (const System* system : score->systems()) {
        if (!system->first()) {
            continue;
        }
        SystemGeometry geometry;
        geometry.page = system->page() ? system->page()->no() : 0;
        geometry.y = system->y();
        geometry.height = system->height();
        for (const SysStaff* staff : system->staves()) {
            geometry.staffY.push_back(staff->y());
        }
        snapshot.m_systems[system->first()] = geometry;
    }

    return snapshot;
}

const SystemGeometry* LayoutSnapshot::findSystem(const System* system) const
{
    auto it = m_systems.find(system->first());
    return it != m_systems.end() && it->second.staffY.size() == system->staves().size() ? &it->second : nullptr;
}

const ItemGeometry* LayoutSnapshot::find(const EngravingItem* item) const
{
    if (item->isSpannerSegment()) {
        const SpannerSegment* segment = toSpannerSegment(item);
        if (!segment->spanner()) {
            return nullptr;
        }
        auto it = m_spannerSegments.find({ segment->spanner(), spannerSegmentIndex(segment) });
        return it != m_spannerSegments.end() && it->second.type == item->type() ? &it->second : nullptr;
    }

    auto it = m_items.find(item);
    return it != m_items.end() && sameItem(it->second, item) ? &it->second : nullptr;
}

void LayoutSnapshot::set(const EngravingItem* item, const ItemGeometry& geometry)
{
    if (item->isSpannerSegment()) {
        const SpannerSegment* segment = toSpannerSegment(item);
        if (segment->spanner()) {
            m_spannerSegments[{ segment->spanner(), spannerSegmentIndex(segment) }] = geometry;
        }
        return;
    }
    m_items[item] = geometry;
}

void LayoutSnapshot::setSpannerSegment(const Spanner* spanner, size_t index, const ItemGeometry& geometry)
{
    m_spannerSegments[{ spanner, index }] = geometry;
}

void LayoutSnapshot::removeSpanner(const Spanner* spanner)
{
    auto first = m_spannerSegments.lower_bound({ spanner, 0 });
    auto last = first;
    while (last != m_spannerSegments.end() && last->first.first == spanner) {
        ++last;
    }
    m_spannerSegments.erase(first, last);
}

ItemGeometry LayoutSnapshot::geometryOf(const EngravingItem* item)
{
    ItemGeometry geometry;
    geometry.type = item->type();
    geometry.parent = item->explicitParent();
    geometry.track = item->track();
    geometry.pagePos = item->pagePos();

    if (item->isSpannerSegment()) {
        geometry.pos2 = toSpannerSegment(item)->pos2();
    }
    if (item->isSlurTieSegment()) {
        const SlurTieSegment* segment = toSlurTieSegment(item);
        for (size_t i = 0; i < CURVE_GRIPS.size(); ++i) {
            geometry.curve[i] = geometry.pagePos + segment->ups(CURVE_GRIPS[i]).pos();
        }
    }
    if (item->isStem()) {
        geometry.length = toStem(item)->length();
    }
    if (item->isTuplet()) {
        const Tuplet* tuplet = toTuplet(item);
        geometry.p1 = geometry.pagePos + tuplet->p1();
        geometry.p2 = geometry.pagePos + tuplet->p2();
    }

    return geometry;
}

namespace {
class ItemCollector
{
public:
    void add(EngravingItem* item)
    {
        if (!item || !m_seen.insert(item).second) {
            return;
        }
        // A spanner has no place of its own: its segments are the systems' children
        if (!item->isSpanner()) {
            items.push_back(item);
        }
        for (EngravingObject* child : item->scanChildren()) {
            if (child && child->isEngravingItem()) {
                add(toEngravingItem(child));
            }
        }
    }

    std::vector<EngravingItem*> items;

private:
    std::unordered_set<const EngravingItem*> m_seen;
};
}

static int depthOf(const EngravingItem* item)
{
    int depth = 0;
    for (const EngravingItem* parent = item->parentItem(); parent; parent = parent->parentItem()) {
        ++depth;
    }
    return depth;
}

std::vector<EngravingItem*> LayoutSnapshot::laidOutItems(Score* score)
{
    ItemCollector collector;
    const size_t staffCount = score->nstaves();

    for (const Page* page : score->pages()) {
        for (System* system : page->systems()) {
            for (Bracket* bracket : system->brackets()) {
                collector.add(bracket);
            }
            collector.add(system->systemDividerLeft());
            collector.add(system->systemDividerRight());
            for (const SysStaff* staff : system->staves()) {
                for (InstrumentName* name : staff->instrumentNames) {
                    collector.add(name);
                }
            }
            for (SpannerSegment* segment : system->spannerSegments()) {
                collector.add(segment);
            }

            for (MeasureBase* measureBase : system->measures()) {
                for (EngravingItem* item : measureBase->el()) {
                    collector.add(item);
                }
                if (!measureBase->isMeasure()) {
                    continue;
                }

                // The segments of the measure as it is shown: for a multimeasure rest, its own
                Measure* measure = toMeasure(measureBase);
                for (staff_idx_t staffIdx = 0; staffIdx < staffCount; ++staffIdx) {
                    collector.add(measure->measureNumber(staffIdx));
                    collector.add(measure->mmRangeText(staffIdx));
                }
                for (Segment& segment : measure->segments()) {
                    if (!segment.enabled()) {
                        continue;
                    }
                    for (EngravingItem* item : segment.elist()) {
                        collector.add(item);
                    }
                    for (EngravingItem* item : segment.annotations()) {
                        collector.add(item);
                    }
                }
            }
        }
    }

    std::vector<std::pair<int, EngravingItem*> > byDepth;
    byDepth.reserve(collector.items.size());
    for (EngravingItem* item : collector.items) {
        byDepth.emplace_back(depthOf(item), item);
    }
    std::stable_sort(byDepth.begin(), byDepth.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

    std::vector<EngravingItem*> items;
    items.reserve(byDepth.size());
    for (const auto& [depth, item] : byDepth) {
        items.push_back(item);
    }
    return items;
}
