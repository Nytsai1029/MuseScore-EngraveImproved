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
#include "officiallayoutbaker.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <list>

#include "dom/accidental.h"
#include "dom/box.h"
#include "dom/chord.h"
#include "dom/factory.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/page.h"
#include "dom/rest.h"
#include "dom/segment.h"
#include "dom/slurtie.h"
#include "dom/spacer.h"
#include "dom/spanner.h"
#include "dom/staff.h"
#include "dom/stem.h"
#include "dom/system.h"
#include "dom/tuplet.h"

#include "types/typesconv.h"

#include "forkfeatureconverter.h"

#include "log.h"

using namespace mu::engraving;

static constexpr int MAX_PASSES = 30;
static constexpr int SEGMENT_PASSES = 8;  // per system: corrections of its pins
static constexpr int TENSION_PASSES = 3;  // per system: corrections of its tension
static constexpr int MAX_VERTICAL_PASSES = 6;
static constexpr int PASSES_BEFORE_AUTOPLACE_OFF = 3;
static constexpr double TOLERANCE_SP = 0.01;
static constexpr double ROOM_SP = 0.1; // by how much the measures of a system should at least fall short of its width

static const std::array<Grip, 4> CURVE_GRIPS { Grip::START, Grip::BEZIER1, Grip::BEZIER2, Grip::END };

static double length(const PointF& p)
{
    return std::hypot(p.x(), p.y());
}

//! Items whose place follows from other items, or that the file cannot hold a place for
static bool followsOthers(const EngravingItem* item)
{
    switch (item->type()) {
    case ElementType::BEAM:              // pinned outright by ForkFeatureConverter
    case ElementType::HOOK:
    case ElementType::STEM_SLASH:
    case ElementType::LEDGER_LINE:
    case ElementType::STAFF_LINES:
    case ElementType::BRACKET:
    case ElementType::SYSTEM_DIVIDER:
    case ElementType::INSTRUMENT_NAME:
    case ElementType::LAYOUT_BREAK:
    case ElementType::SPACER:
    case ElementType::SYSTEM_LOCK_INDICATOR:
    case ElementType::STAFFTYPE_CHANGE:
    case ElementType::TIME_TICK_ANCHOR:
    case ElementType::TAB_DURATION_SYMBOL:
        return true;
    default:
        break;
    }

    // A generated item is made anew by every layout and not saved. Barlines and measure numbers are the
    // exception: taking the mark off turns them into the user's own, as editing one would. So are stems,
    // which every layout makes but which are saved, mark or no mark, once they have a length of their own.
    if (item->generated()) {
        return !(item->isBarLine() || item->isMeasureNumber() || item->isMMRestRange() || item->isStem());
    }

    return false;
}

//! Items that only sit somewhere, so that switching automatic placement off leaves them at their
//! default place plus offset. The others take part in spacing and have to stay in it. A rest does too, but
//! automatic placement is also what moves it up or down out of the way of another voice, to the next
//! free line each time, which an offset cannot steer.
static bool canLeaveAutoplace(const EngravingItem* item)
{
    return !(item->isChord() || item->isNote() || item->isStem() || item->isBarLine()
             || item->isKeySig() || item->isTimeSig() || item->isMMRest() || item->isAmbitus());
}

OfficialExportReport OfficialLayoutBaker::bake(MasterScore* masterScore)
{
    OfficialExportReport report;

    const std::list<Score*>& scoreList = masterScore->scoreList();
    const std::vector<Score*> scores(scoreList.begin(), scoreList.end());

    // What has to be reproduced, and what the fork has to say about it, while its layout still stands
    std::vector<LayoutSnapshot> targets;
    std::vector<std::vector<ShownAccidental> > accidentals;
    targets.reserve(scores.size());
    for (Score* score : scores) {
        targets.push_back(LayoutSnapshot::capture(score));
        accidentals.push_back(shownAccidentals(score));
    }
    for (size_t i = 0; i < scores.size(); ++i) {
        ForkFeatureConverter(scores[i], targets[i], report).convert();
    }

    masterScore->setOfficialCompat(true);

    for (size_t i = 0; i < scores.size(); ++i) {
        OfficialLayoutBaker(scores[i], targets[i], report).pin(accidentals[i]);
    }

    return report;
}

OfficialLayoutBaker::OfficialLayoutBaker(Score* score, LayoutSnapshot& target, OfficialExportReport& report)
    : m_score(score), m_target(target), m_report(report)
{
    m_tolerance = TOLERANCE_SP * score->style().spatium();
}

std::vector<OfficialLayoutBaker::ShownAccidental> OfficialLayoutBaker::shownAccidentals(Score* score)
{
    std::vector<ShownAccidental> accidentals;
    for (EngravingItem* item : LayoutSnapshot::laidOutItems(score)) {
        if (item->isAccidental() && item->explicitParent() && item->explicitParent()->isNote()) {
            const Accidental* accidental = toAccidental(item);
            accidentals.push_back({ toNote(item->explicitParent()), accidental->accidentalType(), accidental->bracket(),
                                    LayoutSnapshot::geometryOf(item) });
        }
    }
    return accidentals;
}

//! Gives the notes that have lost their accidental one of their own again.
//!
//! The fork shows accidentals that upstream's rules do not ask for: the cautionary ones of its style.
//! Layout makes and removes the automatic accidentals, so what is missing shows once the score has been
//! laid out by upstream's rules.
void OfficialLayoutBaker::restoreAccidentals(const std::vector<ShownAccidental>& accidentals)
{
    for (const ShownAccidental& shown : accidentals) {
        if (shown.note->accidental() || shown.type == AccidentalType::NONE) {
            continue;
        }
        Accidental* accidental = Factory::createAccidental(shown.note);
        accidental->setAccidentalType(shown.type);
        accidental->setRole(AccidentalRole::USER);
        accidental->setBracket(shown.bracket);
        accidental->setTrack(shown.note->track());
        accidental->setParent(shown.note);
        shown.note->add(accidental);
        m_target.set(accidental, shown.geometry);
        m_pinned.insert(accidental);
    }
}

//! Takes out the segments that only mark a moment in time for a line to end at.
//!
//! Layout makes these when it lays out the line, they stay for as long as the score is open, and they are
//! not saved. Upstream's spacing keeps the leading space of a segment out of the stretching only if the
//! segment follows its note directly, so one of these in between changes what the leading space does. What
//! counts is how the score is laid out when it is opened, and then they are made as layout goes along.
static void removeTimeAnchors(Score* score)
{
    std::unordered_set<const EngravingItem*> removed;
    std::vector<Segment*> segments;
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (const Measure* shown : { measure, measure->mmRest() }) {
            if (!shown) {
                continue;
            }
            for (Segment* segment = shown->first(SegmentType::TimeTick); segment; segment = segment->next(SegmentType::TimeTick)) {
                bool onlyMarks = segment->annotations().empty();
                for (const EngravingItem* item : segment->elist()) {
                    onlyMarks = onlyMarks && (!item || item->isTimeTickAnchor());
                }
                if (!onlyMarks) {
                    continue; // something is attached to it: that is saved, and the segment comes back with it
                }
                segments.push_back(segment);
                removed.insert(segment);
                for (const EngravingItem* item : segment->elist()) {
                    removed.insert(item);
                }
            }
        }
    }
    if (segments.empty()) {
        return;
    }

    // Layout looks up where a line starts and ends before it lays it out
    auto forget = [&removed](Spanner* spanner) {
        if (removed.count(spanner->startElement())) {
            spanner->setStartElement(nullptr);
        }
        if (removed.count(spanner->endElement())) {
            spanner->setEndElement(nullptr);
        }
    };
    for (const auto& pair : score->spanner()) {
        forget(pair.second);
    }
    for (Spanner* spanner : score->unmanagedSpanners()) {
        forget(spanner);
    }

    for (Segment* segment : segments) {
        segment->measure()->remove(segment);
        delete segment;
    }
}

void OfficialLayoutBaker::relayout()
{
    removeTimeAnchors(m_score);
    m_score->setLayoutAll();
    m_score->doLayout();
}

void OfficialLayoutBaker::pin(const std::vector<ShownAccidental>& accidentals)
{
    relayout();
    restoreAccidentals(accidentals);

    MasterScore* masterScore = m_score->masterScore();
    masterScore->setSegmentPins(&m_pins);

    int passes = 0;
    for (; passes < MAX_PASSES; ++passes) {
        relayout();

        // The items are looked at once the segments stand: nearly everything follows the notes
        if (pinSegments(true) >= m_tolerance) {
            continue;
        }
        // The same goes for the staves
        if (pinVertical(true) >= m_tolerance && ++m_verticalPasses <= MAX_VERTICAL_PASSES) {
            continue;
        }
        if (pinItems(true) < m_tolerance) {
            break;
        }
    }

    // What official is going to make of it: its own spacing, with the leading space that was found
    masterScore->setSegmentPins(nullptr);
    relayout();
    const double spatium = m_score->style().spatium();
    const double segments = pinSegments(false);
    const double staves = pinVertical(false);
    const double items = pinItems(false);
    const double residual = std::max({ segments, staves, items });
    if (residual >= m_tolerance) {
        std::string kinds;
        for (const auto& [type, count] : m_farByType) {
            kinds += std::string(kinds.empty() ? "" : ", ") + std::to_string(count) + " " + TConv::toXml(type).ascii();
        }
        LOGW() << "official export: not where the fork has them: segments by up to " << segments / spatium << " sp, staves by "
               << staves / spatium << " sp, items by " << items / spatium << " sp (" << kinds << ")";
    }

    m_report.passes = std::max(m_report.passes, passes + 1);
    m_report.pinnedItems += int(m_pinned.size());
    m_report.unpinnedItems += m_farItems;
    m_report.maxResidualSp = std::max(m_report.maxResidualSp, residual / spatium);

    LOGI() << "official export: " << m_pinned.size() << " items pinned in " << passes + 1 << " passes, "
           << m_farItems << " still off, the farthest by " << residual / spatium << " sp";
}

//! The item of a segment that says where the segment is: one that is saved, so that the segment's
//! leading space is saved with it
EngravingItem* OfficialLayoutBaker::anchorItem(const Segment& segment) const
{
    EngravingItem* rest = nullptr;
    for (EngravingItem* item : segment.elist()) {
        if (!item || followsOthers(item) || !m_target.find(item)) {
            continue;
        }
        if (!item->isRestFamily()) {
            return item;
        }
        // A rest that fills its measure stands in the middle of it, wherever the segment is
        if (!rest && item->isRest() && !toRest(item)->isFullMeasureRest()) {
            rest = item;
        }
    }
    return rest;
}

//! The segments of a system that leading space can move, from the left, with how far each is from its target
std::vector<OfficialLayoutBaker::SegmentEntry> OfficialLayoutBaker::segmentEntries(const System* system) const
{
    std::vector<SegmentEntry> entries;
    for (MeasureBase* measureBase : system->measures()) {
        if (!measureBase->isMeasure() || toMeasure(measureBase)->isMMRest()) {
            continue; // a multimeasure rest is not saved, nor are its segments
        }
        for (Segment& segment : toMeasure(measureBase)->segments()) {
            // Clefs and breaths are pushed against the segment after them once spacing is done,
            // whatever their leading space: their items are moved by their offsets instead
            if (!segment.enabled() || segment.isRightAligned()) {
                continue;
            }
            if (EngravingItem* anchor = anchorItem(segment)) {
                entries.push_back({ &segment, anchor, m_target.find(anchor)->pagePos.x() - anchor->pagePos().x() });
            }
        }
    }
    return entries;
}

//! The segments justification stretches: see HorizontalSpacing::justifySystem
static bool isStretched(const Segment& segment)
{
    return segment.isChordRestType() && segment.ticks() > Fraction(0, 1) && segment.visible() && segment.enabled()
           && !segment.allElementsInvisible() && segment.stretch() > 0.0;
}

//! The width per unit of stretch that justification has given the notes of a system; 0 if it has none
static double tensionOf(const System* system)
{
    double tension = 0.0;
    for (MeasureBase* measureBase : system->measures()) {
        if (!measureBase->isMeasure()) {
            continue;
        }
        for (Segment& segment : toMeasure(measureBase)->segments()) {
            if (isStretched(segment)) {
                const double own = (segment.width() - segment.widthOffset()) / segment.stretch();
                tension = tension > 0.0 ? std::min(tension, own) : own;
            }
        }
    }
    return tension;
}

//! The tension that makes the multimeasure rests of a system as wide as the target has them, or 0 if it
//! has none. Their segments are not saved, so their width can only come from how far the system is stretched.
//! farthest takes how far the barline after one of them is from where it should be.
double OfficialLayoutBaker::tensionForMMRests(const System* system, const std::vector<SegmentEntry>& entries,
                                              double& farthest) const
{
    double sum = 0.0;
    int count = 0;
    double before = 0.0; // how far the last segment before the rest is from its target
    size_t next = 0;
    farthest = 0.0;

    for (MeasureBase* measureBase : system->measures()) {
        if (!measureBase->isMeasure()) {
            continue;
        }
        const Measure* measure = toMeasure(measureBase);
        if (!measure->isMMRest()) {
            for (; next < entries.size() && entries[next].segment->measure() == measure; ++next) {
                before = entries[next].delta;
            }
            continue;
        }

        const Segment* rest = measure->first(SegmentType::ChordRest);
        const Segment* end = measure->findSegmentR(SegmentType::EndBarLine, measure->ticks());
        const ItemGeometry* want = nullptr;
        const EngravingItem* barLine = nullptr;
        if (end) {
            for (const EngravingItem* item : end->elist()) {
                if (item && (want = m_target.find(item))) {
                    barLine = item;
                    break;
                }
            }
        }
        if (!rest || !barLine || !isStretched(*rest)) {
            continue;
        }

        const double delta = want->pagePos.x() - barLine->pagePos().x();
        farthest = std::max(farthest, std::abs(delta - before));
        sum += (rest->width() - rest->widthOffset() + delta - before) / rest->stretch();
        ++count;
        before = delta;
    }

    return count > 0 ? sum / count : 0.0;
}

//! Pins the segments of the systems whose notes upstream's own spacing does not put where the target has
//! them, and corrects the pins by what the layout made of them. Returns how far the farthest segment was.
//!
//! The leading space itself is found by spacing, where everything it depends on is at hand: see
//! HorizontalSpacing::pinSegment. A pin starts as the place the target has, and spacing reaches that
//! at once wherever upstream's spacing is as simple as pinSegment takes it to be. Where it is not, the pin
//! is moved by as much as the segment came out wrong.
double OfficialLayoutBaker::pinSegments(bool write)
{
    const double spatium = m_score->style().spatium();
    const double least = 0.25 * m_tolerance;
    const double room = ROOM_SP * spatium;
    double farthest = 0.0;

    for (const System* system : m_score->systems()) {
        const std::vector<SegmentEntry> entries = segmentEntries(system);
        const size_t count = entries.size();

        double worst = 0.0;
        for (const SegmentEntry& entry : entries) {
            worst = std::max(worst, std::abs(entry.delta));
        }
        if (!write || count < 2) {
            farthest = std::max(farthest, worst);
            continue;
        }

        SystemState& state = m_systemStates[system->first()];
        if (state.givenUp) {
            continue; // what is left of this system goes into the offsets of its items
        }

        if (!state.pinned) {
            farthest = std::max(farthest, worst);
            if (worst < m_tolerance) {
                continue; // upstream's own spacing does it
            }
            state.pinned = true;
            // Stretched as far as it is now, so that leading space only makes up for what differs.
            // A system that was squeezed says nothing about that.
            const double tension = system->spacingExcess() < 0.0 ? tensionOf(system) : 0.0;
            if (tension > 0.0) {
                m_pins.tension[system->first()] = tension;
            } else {
                state.widestTension = true;
            }
            for (const SegmentEntry& entry : entries) {
                m_pins.x[entry.segment] = entry.segment->xPosInSystemCoords() + entry.delta;
            }
            continue;
        }

        for (const SegmentEntry& entry : entries) {
            if (entry.segment->extraLeadingSpace().val() == 0.0) {
                continue;
            }
            // The leading space of a segment is saved with its items, and a generated item is not saved
            if (entry.anchor->generated()) {
                entry.anchor->setGenerated(false);
            }
            m_pinned.insert(entry.segment);
        }

        // A system whose measures fill it to the last bit is one that official squeezes if it finds them
        // the least bit wider, and squeezing goes in coarse steps
        const double excess = system->spacingExcess();
        if (excess > -0.5 * room) {
            if (!state.widestTension) {
                // The more the notes are stretched, the more of the width is not leading space
                state.widestTension = true;
                m_pins.tension.erase(system->first());
            } else {
                const double more = excess + room;
                state.give += more;
                m_pins.x[entries.back().segment] -= more;
            }
            farthest = std::max(farthest, m_tolerance); // not there yet, whatever the distances say
            if (excess > 0.0) {
                continue; // squeezed here as well: where the segments are says nothing
            }
        }

        if (!state.widestTension && state.tensionPasses < TENSION_PASSES) {
            double rests = 0.0;
            const double tension = tensionForMMRests(system, entries, rests);
            if (tension > 0.0 && rests >= m_tolerance) {
                m_pins.tension[system->first()] = tension;
                ++state.tensionPasses;
                worst = std::max(worst, rests);
            }
        }

        farthest = std::max(farthest, worst);
        if (worst < m_tolerance) {
            continue;
        }
        if (++state.passes > SEGMENT_PASSES) {
            state.givenUp = true;
            continue;
        }
        for (size_t i = 0; i < count; ++i) {
            const SegmentEntry& entry = entries[i];
            if (state.give > 0.0 && i + 1 == count) {
                continue; // the end of the system is held back on purpose
            }
            auto pin = m_pins.x.find(entry.segment);
            if (pin == m_pins.x.end()) {
                m_pins.x[entry.segment] = entry.segment->xPosInSystemCoords() + entry.delta;
            } else if (std::abs(entry.delta) >= least) {
                pin->second += entry.delta;
            }
        }
    }

    return farthest;
}

static Measure* spacerHost(const System* system)
{
    // A multimeasure rest is not saved, and the spacers of the measures it stands for do not count
    for (MeasureBase* measureBase : system->measures()) {
        if (measureBase->isMeasure() && !toMeasure(measureBase)->isMMRest()) {
            return toMeasure(measureBase);
        }
    }
    return nullptr;
}

static staff_idx_t visibleStaff(const System* system, bool last)
{
    staff_idx_t found = muse::nidx;
    for (staff_idx_t staffIdx = 0; staffIdx < system->staves().size(); ++staffIdx) {
        if (system->staff(staffIdx)->show()) {
            found = staffIdx;
            if (!last) {
                break;
            }
        }
    }
    return found;
}

static void setSpacer(Measure* measure, staff_idx_t staffIdx, SpacerType type, double gap)
{
    Spacer* spacer = type == SpacerType::UP ? measure->vspacerUp(staffIdx) : measure->vspacerDown(staffIdx);
    if (!spacer) {
        spacer = Factory::createSpacer(measure);
        spacer->setSpacerType(type);
        spacer->setTrack(staff2track(staffIdx));
        measure->add(spacer);
    }
    spacer->setSpacerType(type);
    spacer->setGap(Spatium(gap / spacer->spatium()));
}

//! Puts the systems and their staves where the target has them.
//! Returns how far the farthest was.
//!
//! Upstream finds these distances from what sticks out of the staves, and here that is not always what
//! the fork found. A distance that differs is fixed with a spacer, which sets it outright; only a system at
//! the top of a page or below a frame cannot be moved up, as its distance there is a least one.
//!
//! A page is as a rule filled to the last bit: what room its systems leave is shared out between them.
//! Whether a system still fits is decided before that, by the least distances, and a spacer makes the
//! distance it fixes the least one. So with some of the distances of a page fixed and the others not, the
//! last system may not fit any more. Between two systems with a fixed distance that question is not
//! asked, hence a page gets all its distances fixed or none.
double OfficialLayoutBaker::pinVertical(bool write)
{
    double farthest = 0.0;

    struct Row {
        const System* system = nullptr;
        const SystemGeometry* want = nullptr;
        const System* above = nullptr;          // on the page the target has the system on
        const SystemGeometry* wantAbove = nullptr;
    };
    std::vector<Row> rows;
    for (const System* system : m_score->systems()) {
        const SystemGeometry* want = system->first() ? m_target.findSystem(system) : nullptr;
        if (!want || !system->page()) {
            continue;
        }
        Row row { system, want };
        if (!rows.empty() && rows.back().want->page == want->page) {
            row.above = rows.back().system;
            row.wantAbove = rows.back().want;
        }
        rows.push_back(row);
    }

    // The staves in their systems, and which pages do not come out right
    for (const Row& row : rows) {
        const System* system = row.system;
        if (system->vbox()) {
            continue; // a frame keeps the height it was given
        }
        Measure* host = spacerHost(system);
        staff_idx_t upper = muse::nidx;
        for (staff_idx_t staffIdx = 0; staffIdx < system->staves().size(); ++staffIdx) {
            if (!system->staff(staffIdx)->show()) {
                continue;
            }
            if (upper != muse::nidx) {
                const double distance = system->staff(staffIdx)->y() - system->staff(upper)->y();
                const double wanted = row.want->staffY[staffIdx] - row.want->staffY[upper];
                farthest = std::max(farthest, std::abs(wanted - distance));
                if (write && host && std::abs(wanted - distance) >= m_tolerance) {
                    setSpacer(host, upper, SpacerType::FIXED, wanted - m_score->staff(upper)->staffHeight());
                    m_pinned.insert(host->vspacerDown(upper));
                    m_fixedPages.insert(row.want->page); // the system takes more of its page than it must
                }
            }
            upper = staffIdx;
        }
    }

    for (const Row& row : rows) {
        const bool onItsPage = row.system->page()->no() == row.want->page;
        // On another page, the system is off by more than any distance on a page
        const double delta = onItsPage ? row.want->y - row.system->y() : row.system->page()->height();
        farthest = std::max(farthest, std::abs(delta));
        if (write && std::abs(delta) >= m_tolerance) {
            m_fixedPages.insert(row.want->page);
        }
    }
    if (!write) {
        return farthest;
    }

    // The systems on their pages
    for (const Row& row : rows) {
        if (m_fixedPages.count(row.want->page) == 0) {
            continue;
        }
        const System* system = row.system;
        const System* above = row.above;
        const double delta = row.want->y - system->y();

        if (above && !above->vbox() && !system->vbox()) {
            Measure* hostAbove = spacerHost(above);
            const staff_idx_t last = visibleStaff(above, true);
            if (hostAbove && last != muse::nidx) {
                setSpacer(hostAbove, last, SpacerType::FIXED, row.want->y - (row.wantAbove->y + row.wantAbove->height));
                m_pinned.insert(hostAbove->vspacerDown(last));
            }
        } else if (above && above->vbox() && !system->vbox()) {
            if (delta >= m_tolerance && system->page()->no() == row.want->page) {
                Box* frame = above->vbox();
                frame->setBottomGap(Spatium((row.want->y - (row.wantAbove->y + row.wantAbove->height)) / frame->spatium()));
                m_pinned.insert(frame);
            }
        } else if (!above && !system->vbox()) {
            Measure* host = spacerHost(system);
            const staff_idx_t first = visibleStaff(system, false);
            if (host && first != muse::nidx && delta >= m_tolerance && system->page()->no() == row.want->page) {
                setSpacer(host, first, SpacerType::UP, row.want->y - system->page()->tm());
                m_pinned.insert(host->vspacerUp(first));
            }
        }
    }

    return farthest;
}

//! Compares the layout as it stands with the target and, if write is set, stores what brings each item
//! there. Returns how far the farthest item was.
double OfficialLayoutBaker::pinItems(bool write)
{
    double farthest = 0.0;
    m_farItems = 0;
    m_farByType.clear();

    for (EngravingItem* item : LayoutSnapshot::laidOutItems(m_score)) {
        if (followsOthers(item)) {
            continue;
        }
        const ItemGeometry* want = m_target.find(item);
        if (!want) {
            continue;
        }

        const double distance = correct(item, *want, write);
        farthest = std::max(farthest, distance);
        if (distance < m_tolerance) {
            m_stubborn.erase(item);
            continue;
        }

        ++m_farItems;
        ++m_farByType[item->type()];
        if (!write) {
            continue;
        }

        m_pinned.insert(item);
        if (++m_stubborn[item] >= PASSES_BEFORE_AUTOPLACE_OFF && item->autoplace() && canLeaveAutoplace(item)) {
            item->setAutoplace(false);
        }
    }

    return farthest;
}

double OfficialLayoutBaker::correct(EngravingItem* item, const ItemGeometry& want, bool write)
{
    // Anything smaller is noise that would only be written to the file
    const double least = 0.25 * m_tolerance;

    if (item->isStem()) {
        Stem* stem = toStem(item);
        const Chord* chord = stem->chord();
        if (chord && (chord->beam() || chord->tremoloTwoChord())) {
            return 0.0; // as long as the beam it reaches
        }
        const double delta = want.length - stem->length();
        if (write && std::abs(delta) > least) {
            stem->setUserLength(stem->userLength() + Spatium(delta / stem->spatium()));
        }
        return std::abs(delta);
    }

    if (item->isSlurTieSegment()) {
        SlurTieSegment* segment = toSlurTieSegment(item);
        const PointF origin = segment->pagePos();
        double farthest = 0.0;
        for (size_t i = 0; i < CURVE_GRIPS.size(); ++i) {
            UP& point = segment->ups(CURVE_GRIPS[i]);
            const PointF delta = want.curve[i] - (origin + point.pos());
            farthest = std::max(farthest, length(delta));
            if (write && length(delta) > least) {
                point.off += delta;
            }
        }
        return farthest;
    }

    if (item->isSpannerSegment()) {
        SpannerSegment* segment = toSpannerSegment(item);
        const PointF delta = want.pagePos - segment->pagePos();
        const PointF delta2 = want.pos2 - segment->pos2();
        if (write && length(delta) > least) {
            addOffset(segment, delta);
        }
        if (write && length(delta2) > least) {
            segment->setUserOff2(segment->userOff2() + delta2);
        }
        return std::max(length(delta), length(delta2));
    }

    if (item->isTuplet()) {
        Tuplet* tuplet = toTuplet(item);
        const PointF origin = tuplet->pagePos();
        const PointF delta1 = want.p1 - (origin + tuplet->p1());
        const PointF delta2 = want.p2 - (origin + tuplet->p2());
        if (write && length(delta1) > least) {
            tuplet->setUserPoint1(tuplet->userP1() + delta1);
        }
        if (write && length(delta2) > least) {
            tuplet->setUserPoint2(tuplet->userP2() + delta2);
        }
        return std::max(length(delta1), length(delta2));
    }

    PointF delta = want.pagePos - item->pagePos();
    if (item->isNote()) {
        delta.setY(0.0); // the pitch decides
    }
    if (write && length(delta) > least) {
        addOffset(item, delta);
    }
    return length(delta);
}

void OfficialLayoutBaker::addOffset(EngravingItem* item, const PointF& delta)
{
    item->setOffset(item->offset() + delta);
    if (item->propertyFlags(Pid::OFFSET) == PropertyFlags::STYLED) {
        item->setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);
    }
    if (item->generated()) {
        item->setGenerated(false);
    }
    // In a part, an item otherwise takes the place it has in the full score
    if (!m_score->isMaster()) {
        item->setPositionLinkedToMaster(false);
    }
}
