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
#include "ledgerguides.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "engraving/dom/beam.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/hook.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/page.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafflines.h"
#include "engraving/dom/stem.h"
#include "engraving/dom/system.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;

namespace {
//! NOTE The top and the bottom of what counts on a staff of a system, in the coordinates of the page
struct LedgerExtent
{
    double top = std::numeric_limits<double>::max();
    double bottom = std::numeric_limits<double>::lowest();

    void add(double y)
    {
        top = std::min(top, y);
        bottom = std::max(bottom, y);
    }
};

//! NOTE Measures which follow each other with the same staff lines
struct StaffLinesRun
{
    double x1 = 0.0;
    double x2 = 0.0;
    double top = 0.0;
    double bottom = 0.0;
    double lineDistance = 0.0;
    double lineWidth = 0.0;
    bool isValid = false;
};

class ExtentCollector
{
public:
    ExtentCollector(const System* system, std::vector<LedgerExtent>& extents)
        : m_system(system), m_extents(extents), m_isShowInvisible(system->score()->isShowInvisible())
    {
    }

    void addChordRest(const ChordRest* chordRest)
    {
        const staff_idx_t staffIdx = chordRest->vStaffIdx();
        if (staffIdx >= m_extents.size() || !m_system->staff(staffIdx)->show()) {
            return;
        }

        LedgerExtent& extent = m_extents[staffIdx];

        if (chordRest->isChord()) {
            const Chord* chord = toChord(chordRest);

            for (const Chord* grace : chord->graceNotes()) {
                addChordRest(grace);
            }

            //! NOTE A notehead counts by the line or the space it is on, not by its outline
            for (const Note* note : chord->notes()) {
                if (isShown(note)) {
                    extent.add(note->pagePos().y());
                }
            }

            addBox(extent, chord->stem());
            addBox(extent, chord->hook());
        } else if (chordRest->isRest()) {
            addBox(extent, chordRest);
        }

        //! NOTE A beam which goes on in another system has its position and its outline from there
        const Beam* beam = chordRest->beam();
        if (beam && beam->system() == m_system) {
            addBox(extent, beam);
        }
    }

private:
    bool isShown(const EngravingItem* item) const
    {
        return item->visible() || m_isShowInvisible;
    }

    void addBox(LedgerExtent& extent, const EngravingItem* item) const
    {
        if (!item || !isShown(item)) {
            return;
        }

        const RectF box = item->pageBoundingRect();
        extent.add(box.top());
        extent.add(box.bottom());
    }

    const System* m_system = nullptr;
    std::vector<LedgerExtent>& m_extents;
    bool m_isShowInvisible = false;
};

std::vector<LedgerExtent> collectExtents(const System* system)
{
    std::vector<LedgerExtent> extents(system->staves().size());
    ExtentCollector collector(system, extents);

    const track_idx_t trackCount = extents.size() * VOICES;

    for (const MeasureBase* measureBase : system->measures()) {
        if (!measureBase->isMeasure()) {
            continue;
        }

        const Measure* measure = toMeasure(measureBase);
        for (const Segment* segment = measure->first(SegmentType::ChordRest); segment;
             segment = segment->next(SegmentType::ChordRest)) {
            for (track_idx_t track = 0; track < trackCount; ++track) {
                //! NOTE What is on a hidden staff has no layout
                if (!system->staff(track2staff(track))->show()) {
                    continue;
                }

                const EngravingItem* element = segment->element(track);
                if (element && element->isChordRest()) {
                    collector.addChordRest(toChordRest(element));
                }
            }
        }
    }

    return extents;
}

void addGuidesOfSide(std::vector<LedgerGuide>& guides, const StaffLinesRun& run, double x1, double x2,
                     double staffEdge, double direction, int count, size_t colorIndex)
{
    const double step = run.lineDistance / 2.0;

    for (int i = 1; i <= count * 2; ++i) {
        const double y = staffEdge + direction * step * i;

        LedgerGuide guide;
        guide.line = LineF(x1, y, x2, y);
        guide.isSpace = (i % 2) == 1;
        guide.colorIndex = colorIndex;
        guide.width = run.lineWidth;
        guide.dash = run.lineDistance;
        guides.push_back(guide);
    }
}

void addGuidesOfRun(std::vector<LedgerGuide>& guides, const StaffLinesRun& run, const LedgerExtent& extent,
                    size_t colorIndex, const RectF& visibleRect)
{
    if (!run.isValid || run.lineDistance <= 0.0 || run.x2 <= run.x1) {
        return;
    }

    double x1 = run.x1;
    double x2 = run.x2;

    if (visibleRect.isValid()) {
        //! NOTE The dashes begin where the staff begins, so a line is cut by whole periods of them
        const double period = run.lineDistance * 2.0;
        if (visibleRect.left() > x1) {
            x1 += std::floor((visibleRect.left() - x1) / period) * period;
        }

        x2 = std::min(x2, visibleRect.right());

        if (x2 <= x1) {
            return;
        }
    }

    const int above = ledgerGuideCount(run.top - extent.top, run.lineDistance);
    const int below = ledgerGuideCount(extent.bottom - run.bottom, run.lineDistance);

    addGuidesOfSide(guides, run, x1, x2, run.top, -1.0, above, colorIndex);
    addGuidesOfSide(guides, run, x1, x2, run.bottom, 1.0, below, colorIndex);
}

void addGuidesOfStaff(std::vector<LedgerGuide>& guides, const System* system, staff_idx_t staffIdx,
                      const LedgerExtent& extent, size_t colorIndex, const RectF& visibleRect)
{
    const Staff* staff = system->score()->staff(staffIdx);
    if (!staff) {
        return;
    }

    StaffLinesRun run;

    auto closeRun = [&]() {
        addGuidesOfRun(guides, run, extent, colorIndex, visibleRect);
        run = StaffLinesRun();
    };

    for (MeasureBase* measureBase : system->measures()) {
        if (!measureBase->isMeasure()) {
            closeRun();
            continue;
        }

        Measure* measure = toMeasure(measureBase);
        const StaffLines* staffLines = measure->visible(staffIdx) ? measure->staffLines(staffIdx) : nullptr;

        if (!staffLines || !staffLines->visible() || staffLines->lines().empty()
            || staff->isTabStaff(measure->tick())) {
            closeRun();
            continue;
        }

        const std::vector<LineF>& lines = staffLines->lines();
        const PointF pos = staffLines->pagePos();

        StaffLinesRun next;
        next.x1 = pos.x() + lines.front().x1();
        next.x2 = pos.x() + lines.front().x2();
        next.top = pos.y() + lines.front().y1();
        next.bottom = pos.y() + lines.back().y1();
        next.lineDistance = lines.size() > 1 ? lines.at(1).y1() - lines.at(0).y1()
                            : staff->lineDistance(measure->tick()) * staffLines->spatium();
        next.lineWidth = staffLines->lw();
        next.isValid = true;

        const double tolerance = next.lineDistance * 0.01;
        auto isSame = [tolerance](double a, double b) {
            return std::abs(a - b) <= tolerance;
        };

        const bool isContinued = run.isValid && isSame(run.x2, next.x1) && isSame(run.top, next.top)
                                 && isSame(run.bottom, next.bottom) && isSame(run.lineDistance, next.lineDistance);
        if (isContinued) {
            run.x2 = next.x2;
        } else {
            closeRun();
            run = next;
        }
    }

    closeRun();
}
}

int mu::notation::ledgerGuideCount(double distanceBeyondStaff, double lineDistance)
{
    if (lineDistance <= 0.0) {
        return 0;
    }

    if (distanceBeyondStaff <= 0.0) {
        return 1;
    }

    //! NOTE What ends exactly at a ledger line gets the next one, in spite of the rounding of the layout
    static constexpr double ROUNDING = 0.001;

    const double count = std::floor(distanceBeyondStaff / lineDistance + ROUNDING) + 1.0;
    return static_cast<int>(std::min(count, static_cast<double>(MAX_LEDGER_GUIDES)));
}

std::vector<LedgerGuide> mu::notation::ledgerGuides(const Page* page, const RectF& visibleRect)
{
    std::vector<LedgerGuide> guides;
    if (!page) {
        return guides;
    }

    for (const System* system : page->systems()) {
        if (system->staves().empty() || system->measures().empty()) {
            continue;
        }

        if (visibleRect.isValid()) {
            const double reach = (MAX_LEDGER_GUIDES + 1) * system->spatium() * 2.0;
            const RectF systemRect = system->pageBoundingRect().adjusted(0.0, -reach, 0.0, reach);
            if (!systemRect.intersects(visibleRect)) {
                continue;
            }
        }

        const std::vector<LedgerExtent> extents = collectExtents(system);

        size_t colorIndex = 0;
        for (staff_idx_t staffIdx = 0; staffIdx < extents.size(); ++staffIdx) {
            if (!system->staff(staffIdx)->show()) {
                continue;
            }

            addGuidesOfStaff(guides, system, staffIdx, extents.at(staffIdx), colorIndex % 2, visibleRect);
            ++colorIndex;
        }
    }

    return guides;
}
