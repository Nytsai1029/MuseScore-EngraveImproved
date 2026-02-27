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
#include "slur.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>

#include "arpeggio.h"
#include "beam.h"
#include "chord.h"
#include "measure.h"
#include "mscoreview.h"
#include "navigate.h"
#include "part.h"
#include "score.h"
#include "stem.h"
#include "system.h"
#include "tremolotwochord.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace muse::draw;

namespace mu::engraving {
SlurSegment::SlurSegment(System* parent, ElementType type)
    : SlurTieSegment(type, parent)
{
}

SlurSegment::SlurSegment(const SlurSegment& ss)
    : SlurTieSegment(ss)
{
    m_multiBezierData = ss.m_multiBezierData;
    m_multiBezierKnotData = ss.m_multiBezierKnotData;
}

bool SlurSegment::useMultiBezier() const
{
    const Slur* parentSlur = slur();
    return parentSlur && !parentSlur->isFlatMiddleCurve() && parentSlur->multiBezierEnabled() && multiBezierKnotCount() > 0;
}

bool SlurSegment::useFlatCurve() const
{
    const Slur* parentSlur = slur();
    return parentSlur && parentSlur->isFlatMiddleCurve();
}

bool SlurSegment::useBezierKnotControls() const
{
    return (useMultiBezier() || useFlatCurve()) && multiBezierKnotCount() > 0;
}

int SlurSegment::multiBezierKnotCount() const
{
    const Slur* parentSlur = slur();
    if (!parentSlur) {
        return 0;
    }

    if (parentSlur->isFlatMiddleCurve()) {
        return 2;
    }

    return std::clamp(parentSlur->multiBezierKnotCount(), 0, 16);
}

int SlurSegment::multiBezierDragGripIndex() const
{
    return int(Grip::DRAG);
}

bool SlurSegment::isMultiBezierControlGripIndex(int gripIndex) const
{
    return gripIndex >= multiBezierFirstGripIndex() && gripIndex < multiBezierControlGripEndIndex();
}

int SlurSegment::multiBezierKnotIndexForGrip(int gripIndex) const
{
    if (!isMultiBezierControlGripIndex(gripIndex)) {
        return -1;
    }

    return (gripIndex - multiBezierFirstGripIndex()) / 3;
}

SlurSegment::MultiBezierGripType SlurSegment::multiBezierGripTypeForGrip(int gripIndex) const
{
    if (!isMultiBezierControlGripIndex(gripIndex)) {
        return MultiBezierGripType::None;
    }

    switch ((gripIndex - multiBezierFirstGripIndex()) % 3) {
    case 0: return MultiBezierGripType::InHandle;
    case 1: return MultiBezierGripType::Knot;
    case 2: return MultiBezierGripType::OutHandle;
    default: return MultiBezierGripType::None;
    }
}

void SlurSegment::parseMultiBezierData()
{
    const int knotCount = multiBezierKnotCount();
    m_multiBezierKnotData.assign(knotCount, MultiBezierKnot());

    if (knotCount == 0 || m_multiBezierData.empty()) {
        return;
    }

    std::istringstream knotStream(m_multiBezierData);
    std::string knotToken;
    int knotIdx = 0;

    while (std::getline(knotStream, knotToken, ';') && knotIdx < knotCount) {
        std::istringstream valueStream(knotToken);
        std::string valueToken;
        std::array<double, 6> values {};
        int valueIdx = 0;

        while (std::getline(valueStream, valueToken, ',') && valueIdx < static_cast<int>(values.size())) {
            try {
                values[valueIdx] = std::stod(valueToken);
            } catch (...) {
                values[valueIdx] = 0.0;
            }
            ++valueIdx;
        }

        if (valueIdx >= static_cast<int>(values.size())) {
            MultiBezierKnot& knot = m_multiBezierKnotData[knotIdx];
            knot.knot.off = PointF(values[0], values[1]);
            knot.inHandle.off = PointF(values[2], values[3]);
            knot.outHandle.off = PointF(values[4], values[5]);
        }

        ++knotIdx;
    }
}

void SlurSegment::ensureMultiBezierKnotData()
{
    if (int(m_multiBezierKnotData.size()) == multiBezierKnotCount()) {
        return;
    }

    parseMultiBezierData();
}

void SlurSegment::syncMultiBezierDataProperty()
{
    if (m_multiBezierKnotData.empty()) {
        m_multiBezierData.clear();
        return;
    }

    bool hasOffsets = false;
    for (const MultiBezierKnot& knot : m_multiBezierKnotData) {
        if (!knot.inHandle.off.isNull() || !knot.knot.off.isNull() || !knot.outHandle.off.isNull()) {
            hasOffsets = true;
            break;
        }
    }

    if (!hasOffsets) {
        m_multiBezierData.clear();
        return;
    }

    std::ostringstream stream;
    stream << std::setprecision(12);
    for (size_t i = 0; i < m_multiBezierKnotData.size(); ++i) {
        if (i > 0) {
            stream << ';';
        }

        const MultiBezierKnot& knot = m_multiBezierKnotData[i];
        stream << knot.knot.off.x() << ',' << knot.knot.off.y() << ','
               << knot.inHandle.off.x() << ',' << knot.inHandle.off.y() << ','
               << knot.outHandle.off.x() << ',' << knot.outHandle.off.y();
    }

    m_multiBezierData = stream.str();
}

bool SlurSegment::resetMultiBezierGrip(Grip grip)
{
    ensureMultiBezierKnotData();

    const int gripIndex = int(grip);
    const int knotIndex = multiBezierKnotIndexForGrip(gripIndex);
    if (knotIndex < 0 || knotIndex >= int(m_multiBezierKnotData.size())) {
        return false;
    }

    MultiBezierKnot& knot = m_multiBezierKnotData[knotIndex];
    switch (multiBezierGripTypeForGrip(gripIndex)) {
    case MultiBezierGripType::InHandle:
        knot.inHandle.off = PointF();
        break;
    case MultiBezierGripType::Knot:
        knot.knot.off = PointF();
        break;
    case MultiBezierGripType::OutHandle:
        knot.outHandle.off = PointF();
        break;
    case MultiBezierGripType::None:
        return false;
    }

    syncMultiBezierDataProperty();
    return true;
}

//---------------------------------------------------------
//   searchCR
//---------------------------------------------------------

static ChordRest* searchCR(Segment* segment, track_idx_t startTrack, track_idx_t endTrack)
{
    for (Segment* s = segment; s; s = s->next(SegmentType::ChordRest)) {       // restrict search to measure
        if (startTrack > endTrack) {
            for (int t = static_cast<int>(startTrack) - 1; t >= static_cast<int>(endTrack); --t) {
                if (s->element(t)) {
                    return toChordRest(s->element(t));
                }
            }
        } else {
            for (track_idx_t t = startTrack; t < endTrack; ++t) {
                if (s->element(t)) {
                    return toChordRest(s->element(t));
                }
            }
        }
    }
    return 0;
}

bool SlurSegment::isEditAllowed(EditData& ed) const
{
    if (SlurTieSegment::isEditAllowed(ed)) {
        return true;
    }

    if (useBezierKnotControls() && ed.key != Key_Home && isMultiBezierControlGripIndex(int(ed.curGrip))) {
        return false;
    }

    const bool moveStart = ed.curGrip == Grip::START;
    const bool moveEnd = ed.curGrip == Grip::END || ed.curGrip == Grip::DRAG;

    if (!((ed.modifiers & ShiftModifier) && (isSingleType()
                                             || (isBeginType() && moveStart) || (isEndType() && moveEnd)))) {
        return false;
    }

    static const std::set<int> navigationKeys {
        Key_Left,
        Key_Up,
        Key_Down,
        Key_Right
    };

    return muse::contains(navigationKeys, ed.key);
}

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool SlurSegment::edit(EditData& ed)
{
    if (!isEditAllowed(ed)) {
        return false;
    }

    if (useBezierKnotControls() && ed.key == Key_Home && ed.hasCurrentGrip()) {
        const int gripIndex = int(ed.curGrip);
        if (isMultiBezierControlGripIndex(gripIndex) || gripIndex == multiBezierDragGripIndex()
            || gripIndex == int(Grip::SHOULDER)) {
            startEditDrag(ed);
            if (isMultiBezierControlGripIndex(gripIndex)) {
                resetMultiBezierGrip(ed.curGrip);
            } else if (gripIndex == int(Grip::SHOULDER)) {
                ensureMultiBezierKnotData();
                if (!m_multiBezierKnotData.empty()) {
                    const int middleKnotIndex = std::clamp(multiBezierKnotCount() / 2, 0, int(m_multiBezierKnotData.size()) - 1);
                    MultiBezierKnot& knot = m_multiBezierKnotData[middleKnotIndex];
                    knot.inHandle.off = PointF();
                    knot.knot.off = PointF();
                    knot.outHandle.off = PointF();
                    syncMultiBezierDataProperty();
                }
            } else {
                roffset() = PointF();
            }
            renderer()->layoutItem(spanner());
            endEditDrag(ed);
            return true;
        }
    }

    if (SlurTieSegment::edit(ed)) {
        return true;
    }

    Slur* sl = slur();

    ChordRest* cr = nullptr;
    ChordRest* e;
    ChordRest* e1;
    const bool start = ed.curGrip == Grip::START;
    if (start) {
        e  = sl->startCR();
        e1 = sl->endCR();
    } else {
        e  = sl->endCR();
        e1 = sl->startCR();
    }

    const bool altMod = ed.modifiers & AltModifier;
    const bool shiftMod = ed.modifiers & ShiftModifier;
    const bool extendToBarLine = shiftMod && altMod;
    const bool isPartialSlur = sl->isIncoming() || sl->isOutgoing();

    ChordRestNavigateOptions options;
    options.disableOverRepeats = true;

    if (ed.key == Key_Left) {
        if (extendToBarLine) {
            const Measure* measure = e->measure();
            if (start) {
                cr = measure->firstChordRest(e->track());
                if (!cr->hasPrecedingJumpItem()) {
                    return false;
                }
                sl->undoSetIncoming(true);
            } else if (e->hasFollowingJumpItem()) {
                sl->undoSetOutgoing(false);
            }
        } else {
            if (start && sl->isIncoming()) {
                sl->undoSetIncoming(false);
                cr = prevChordRest(e, options);
            } else if (!start && sl->isOutgoing()) {
                sl->undoSetOutgoing(false);
            } else {
                cr = prevChordRest(e, options);
            }
        }
    } else if (ed.key == Key_Right) {
        if (extendToBarLine) {
            const Measure* measure = e->measure();
            if (start && e->hasPrecedingJumpItem()) {
                sl->undoSetIncoming(false);
            } else if (!start) {
                cr = measure->lastChordRest(e->track());
                if (!cr->hasFollowingJumpItem()) {
                    return false;
                }
                sl->undoSetOutgoing(true);
            }
        } else {
            if (start && sl->isIncoming()) {
                sl->undoSetIncoming(false);
            } else if (!start && sl->isOutgoing()) {
                sl->undoSetOutgoing(false);
                cr = nextChordRest(e, options);
            } else {
                cr = nextChordRest(e, options);
            }
        }
    } else if (ed.key == Key_Up) {
        Part* part     = e->part();
        track_idx_t startTrack = part->startTrack();
        track_idx_t endTrack   = e->track();
        cr = searchCR(e->segment(), endTrack, startTrack);
    } else if (ed.key == Key_Down) {
        track_idx_t startTrack = e->track() + 1;
        Part* part     = e->part();
        track_idx_t endTrack   = part->endTrack();
        cr = searchCR(e->segment(), startTrack, endTrack);
    } else {
        return false;
    }
    if (cr && (cr != e1 || isPartialSlur)) {
        if (cr->staff() != e->staff() && (cr->staffType()->isTabStaff() || e->staffType()->isTabStaff())) {
            return false; // Cross-staff slurs don't make sense for TAB staves
        }
        if (cr->staff()->isLinked(e->staff())) {
            return false; // Don't allow slur to cross into staff that's linked to this
        }
        changeAnchor(ed, cr);
    }
    return true;
}

//---------------------------------------------------------
//   changeAnchor
//---------------------------------------------------------

void SlurSegment::changeAnchor(EditData& ed, EngravingItem* element)
{
    ChordRest* cr = element->isChordRest() ? toChordRest(element) : nullptr;
    ChordRest* scr = spanner()->startCR();
    ChordRest* ecr = spanner()->endCR();
    if (!cr || !scr || !ecr) {
        return;
    }

    // save current start/end elements
    for (EngravingObject* e : spanner()->linkList()) {
        Spanner* sp = toSpanner(e);
        score()->undoStack()->pushWithoutPerforming(new ChangeStartEndSpanner(sp, sp->startElement(), sp->endElement()));
    }

    if (ed.curGrip == Grip::START) {
        spanner()->undoChangeProperty(Pid::SPANNER_TICK, cr->tick());
        Fraction ticks = ecr->tick() - cr->tick();
        spanner()->undoChangeProperty(Pid::SPANNER_TICKS, ticks);
        int diff = static_cast<int>(cr->track() - spanner()->track());
        for (auto e : spanner()->linkList()) {
            Spanner* s = toSpanner(e);
            s->undoChangeProperty(Pid::TRACK, s->track() + diff);
        }
        scr = cr;
    } else {
        Fraction ticks = cr->tick() - scr->tick();
        spanner()->undoChangeProperty(Pid::SPANNER_TICKS, ticks);
        int diff = static_cast<int>(cr->track() - spanner()->track());
        for (auto e : spanner()->linkList()) {
            Spanner* s = toSpanner(e);
            s->undoChangeProperty(Pid::SPANNER_TRACK2, s->track() + diff);
        }
        ecr = cr;
    }

    // update start/end elements (which could be grace notes)
    slur()->undoChangeStartEndElements(scr, ecr);

    const size_t segments  = spanner()->spannerSegments().size();
    ups(ed.curGrip).off = PointF();
    renderer()->layoutItem(spanner());
    if (spanner()->spannerSegments().size() != segments) {
        const std::vector<SpannerSegment*>& ss = spanner()->spannerSegments();
        const bool moveEnd = ed.curGrip == Grip::END || ed.curGrip == Grip::DRAG;
        SlurSegment* newSegment = toSlurSegment(moveEnd ? ss.back() : ss.front());
        ed.view()->changeEditElement(newSegment);
        triggerLayout();
    }
}

void SlurSegment::editDrag(EditData& ed)
{
    Grip g = ed.curGrip;

    if (useBezierKnotControls()) {
        const int gripIndex = int(g);
        if (isMultiBezierControlGripIndex(gripIndex)) {
            ensureMultiBezierKnotData();
            const int knotIndex = multiBezierKnotIndexForGrip(gripIndex);
            if (knotIndex < 0 || knotIndex >= int(m_multiBezierKnotData.size())) {
                return;
            }

            MultiBezierKnot& knot = m_multiBezierKnotData[knotIndex];
            switch (multiBezierGripTypeForGrip(gripIndex)) {
            case MultiBezierGripType::InHandle:
                knot.inHandle.off += ed.delta;
                {
                    const PointF knotPos = knot.knot.pos();
                    const PointF inPos = knot.inHandle.pos();
                    PointF direction = inPos - knotPos;
                    const double directionLen = std::hypot(direction.x(), direction.y());
                    if (directionLen > 1e-6) {
                        const PointF outPos = knot.outHandle.pos();
                        double outLen = std::hypot(outPos.x() - knotPos.x(), outPos.y() - knotPos.y());
                        if (outLen < 1e-6) {
                            outLen = directionLen;
                        }
                        direction /= directionLen;
                        const PointF newOutPos = knotPos - direction * outLen;
                        knot.outHandle.off = newOutPos - knot.outHandle.p;
                    }
                }
                break;
            case MultiBezierGripType::Knot:
                knot.inHandle.off += ed.delta;
                knot.knot.off += ed.delta;
                knot.outHandle.off += ed.delta;
                break;
            case MultiBezierGripType::OutHandle:
                knot.outHandle.off += ed.delta;
                {
                    const PointF knotPos = knot.knot.pos();
                    const PointF outPos = knot.outHandle.pos();
                    PointF direction = outPos - knotPos;
                    const double directionLen = std::hypot(direction.x(), direction.y());
                    if (directionLen > 1e-6) {
                        const PointF inPos = knot.inHandle.pos();
                        double inLen = std::hypot(inPos.x() - knotPos.x(), inPos.y() - knotPos.y());
                        if (inLen < 1e-6) {
                            inLen = directionLen;
                        }
                        direction /= directionLen;
                        const PointF newInPos = knotPos - direction * inLen;
                        knot.inHandle.off = newInPos - knot.inHandle.p;
                    }
                }
                break;
            case MultiBezierGripType::None:
                return;
            }

            syncMultiBezierDataProperty();
            renderer()->computeBezier(this);
            triggerLayout();
            return;
        }

        if (gripIndex == multiBezierDragGripIndex()) {
            roffset() += ed.delta;
            triggerLayout();
            return;
        }

        if (gripIndex == int(Grip::SHOULDER)) {
            ensureMultiBezierKnotData();
            if (m_multiBezierKnotData.empty()) {
                return;
            }

            if (useFlatCurve()) {
                // In flat-middle mode, shoulder drag controls only the middle segment height.
                const PointF startPos = ups(Grip::START).pos();
                const PointF endPos = ups(Grip::END).pos();
                PointF axis = endPos - startPos;
                const double axisLen = std::hypot(axis.x(), axis.y());
                const PointF normal = axisLen > 1e-6 ? PointF(-axis.y() / axisLen, axis.x() / axisLen) : PointF(0.0, 1.0);
                const double deltaOnNormal = ed.delta.x() * normal.x() + ed.delta.y() * normal.y();
                const PointF projectedDelta = normal * deltaOnNormal;

                for (MultiBezierKnot& knot : m_multiBezierKnotData) {
                    knot.inHandle.off += projectedDelta;
                    knot.knot.off += projectedDelta;
                    knot.outHandle.off += projectedDelta;
                }
            } else {
                const int middleKnotIndex = std::clamp(multiBezierKnotCount() / 2, 0, int(m_multiBezierKnotData.size()) - 1);
                MultiBezierKnot& knot = m_multiBezierKnotData[middleKnotIndex];
                knot.inHandle.off += ed.delta;
                knot.knot.off += ed.delta;
                knot.outHandle.off += ed.delta;
            }

            syncMultiBezierDataProperty();
            renderer()->computeBezier(this);
            triggerLayout();
            return;
        }

        if (g != Grip::START && g != Grip::END && g != Grip::BEZIER1 && g != Grip::BEZIER2) {
            return;
        }
    }

    switch (g) {
    case Grip::START:
    case Grip::END:
        ups(g).off += ed.delta;
        //
        // move anchor for slurs/ties
        //
        if ((g == Grip::START && isSingleBeginType()) || (g == Grip::END && isSingleEndType())) {
            Slur* slr = slur();
            KeyboardModifiers km = ed.modifiers;
            EngravingItem* e = ed.view()->elementNear(ed.pos);
            if (e && e->isNote()) {
                Note* note = toNote(e);
                Fraction tick = note->chord()->tick();
                if ((g == Grip::END && tick > slr->tick()) || (g == Grip::START && tick < slr->tick2())) {
                    if (km != (ShiftModifier | ControlModifier)) {
                        Chord* c = note->chord();
                        ed.view()->setDropTarget(note);
                        if (c->part() == slr->part() && c != slr->endCR()) {
                            changeAnchor(ed, c);
                        }
                    }
                }
            } else {
                ed.view()->setDropTarget(0);
            }
        }
        renderer()->computeBezier(this);
        break;
    case Grip::BEZIER1:
    case Grip::BEZIER2:
        ups(g).off += ed.delta;
        renderer()->computeBezier(this);
        break;
    case Grip::SHOULDER:
        ups(g).off = PointF();
        renderer()->computeBezier(this, ed.delta);
        break;
    case Grip::DRAG:
        ups(Grip::DRAG).off = PointF();
        roffset() += ed.delta;
        break;
    default:
        return;
    }

    triggerLayout();
}

PropertyValue SlurSegment::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SLUR_MULTI_BEZIER_DATA:
        return muse::String::fromStdString(m_multiBezierData);
    default:
        return SlurTieSegment::getProperty(propertyId);
    }
}

bool SlurSegment::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SLUR_MULTI_BEZIER_DATA:
        m_multiBezierData = v.value<muse::String>().toStdString();
        parseMultiBezierData();
        break;
    default:
        return SlurTieSegment::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

PropertyValue SlurSegment::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SLUR_MULTI_BEZIER_DATA:
        return muse::String();
    default:
        return SlurTieSegment::propertyDefault(id);
    }
}

void SlurSegment::reset()
{
    SlurTieSegment::reset();
    undoResetProperty(Pid::SLUR_MULTI_BEZIER_DATA);
}

int SlurSegment::gripsCount() const
{
    if (useBezierKnotControls()) {
        return multiBezierControlGripEndIndex();
    }

    return SlurTieSegment::gripsCount();
}

Grip SlurSegment::defaultGrip() const
{
    if (useBezierKnotControls()) {
        return Grip::DRAG;
    }

    return SlurTieSegment::defaultGrip();
}

std::vector<PointF> SlurSegment::gripsPositions(const EditData& ed) const
{
    if (!useBezierKnotControls()) {
        return SlurTieSegment::gripsPositions(ed);
    }

    std::vector<PointF> grips;
    grips.reserve(gripsCount());

    const PointF pagePosition(pagePos());
    for (int i = 0; i < int(Grip::GRIPS); ++i) {
        grips.push_back(ups(Grip(i)).pos() + pagePosition);
    }

    const int knotCount = multiBezierKnotCount();
    const PointF fallback = ups(Grip::DRAG).pos() + pagePosition;
    for (int i = 0; i < knotCount; ++i) {
        if (i < int(m_multiBezierKnotData.size())) {
            const MultiBezierKnot& knot = m_multiBezierKnotData[size_t(i)];
            grips.push_back(knot.inHandle.pos() + pagePosition);
            grips.push_back(knot.knot.pos() + pagePosition);
            grips.push_back(knot.outHandle.pos() + pagePosition);
        } else {
            grips.push_back(fallback);
            grips.push_back(fallback);
            grips.push_back(fallback);
        }
    }

    return grips;
}

//---------------------------------------------------------
//   isEdited
//---------------------------------------------------------

bool SlurSegment::isEdited() const
{
    for (int i = 0; i < int(Grip::GRIPS); ++i) {
        if (!m_ups[i].off.isNull()) {
            return true;
        }
    }

    for (const MultiBezierKnot& knot : m_multiBezierKnotData) {
        if (!knot.inHandle.off.isNull() || !knot.knot.off.isNull() || !knot.outHandle.off.isNull()) {
            return true;
        }
    }

    if (!m_multiBezierData.empty()) {
        return true;
    }

    return false;
}

bool SlurSegment::isUserModified() const
{
    if (SlurTieSegment::isUserModified()) {
        return true;
    }

    return !m_multiBezierData.empty();
}

bool SlurSegment::isEndPointsEdited() const
{
    return !(m_ups[int(Grip::START)].off.isNull() && m_ups[int(Grip::END)].off.isNull());
}

double SlurSegment::endWidth() const
{
    return style().styleMM(Sid::slurEndWidth);
}

double SlurSegment::midWidth() const
{
    return style().styleMM(Sid::slurMidWidth);
}

double SlurSegment::dottedWidth() const
{
    return style().styleMM(Sid::slurDottedWidth);
}

Color SlurSegment::curColor() const
{
    return EngravingItem::curColor(getProperty(Pid::VISIBLE).toBool(), getProperty(Pid::COLOR).value<Color>());
}

Slur::Slur(const Slur& s)
    : SlurTie(s)
{
    _connectedElement = s._connectedElement;
    _partialSpannerDirection = s._partialSpannerDirection;
    _multiBezierEnabled = s._multiBezierEnabled;
    _multiBezierKnotCount = s._multiBezierKnotCount;
    _curveMode = s._curveMode;
}

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

Slur::Slur(EngravingItem* parent, ElementType type)
    : SlurTie(type, parent)
{
    setAnchor(Anchor::CHORD);
}

double Slur::scalingFactor() const
{
    Chord* startC = startElement() && startElement()->isChord() ? toChord(startElement()) : nullptr;
    Chord* endC = endElement() && endElement()->isChord() ? toChord(endElement()) : nullptr;

    if (!startC || !endC) {
        return 1.0;
    }

    if ((startC->isGraceBefore() && startC->parent() == endC)
        || (endC->isGraceAfter() && endC->parent() == startC)) {
        return style().styleD(Sid::graceNoteMag);
    }

    if (startC->isGrace()) {
        startC = toChord(startC->parent());
    }
    if (endC->isGrace()) {
        endC = toChord(endC->parent());
    }

    return 0.5 * (startC->intrinsicMag() + endC->intrinsicMag());
}

void Slur::undoSetIncoming(bool incoming)
{
    if (incoming == isIncoming()) {
        return;
    }

    undoChangeProperty(Pid::PARTIAL_SPANNER_DIRECTION, calcIncomingDirection(incoming), PropertyFlags::UNSTYLED);
}

void Slur::undoSetOutgoing(bool outgoing)
{
    if (outgoing == isOutgoing()) {
        return;
    }

    undoChangeProperty(Pid::PARTIAL_SPANNER_DIRECTION, calcOutgoingDirection(outgoing), PropertyFlags::UNSTYLED);
}

void Slur::setIncoming(bool incoming)
{
    if (incoming == isIncoming()) {
        return;
    }

    _partialSpannerDirection = calcIncomingDirection(incoming);
}

void Slur::setOutgoing(bool outgoing)
{
    if (outgoing == isOutgoing()) {
        return;
    }

    _partialSpannerDirection = calcOutgoingDirection(outgoing);
}

PartialSpannerDirection Slur::calcIncomingDirection(bool incoming)
{
    PartialSpannerDirection dir = PartialSpannerDirection::INCOMING;
    if (incoming) {
        SlurSegment* firstSeg = nsegments() > 0 ? frontSegment() : nullptr;
        if (firstSeg) {
            firstSeg->setSlurOffset(Grip::START, PointF(0, 0));
        }
        dir = _partialSpannerDirection
              == PartialSpannerDirection::OUTGOING ? PartialSpannerDirection::BOTH : PartialSpannerDirection::INCOMING;
    } else {
        dir = _partialSpannerDirection == PartialSpannerDirection::BOTH ? PartialSpannerDirection::OUTGOING : PartialSpannerDirection::NONE;
    }
    return dir;
}

PartialSpannerDirection Slur::calcOutgoingDirection(bool outgoing)
{
    PartialSpannerDirection dir = PartialSpannerDirection::OUTGOING;
    if (outgoing) {
        SlurSegment* lastSeg = nsegments() > 0 ? backSegment() : nullptr;
        if (lastSeg) {
            lastSeg->setSlurOffset(Grip::END, PointF(0, 0));
        }
        dir = _partialSpannerDirection
              == PartialSpannerDirection::INCOMING ? PartialSpannerDirection::BOTH : PartialSpannerDirection::OUTGOING;
    } else {
        dir = _partialSpannerDirection == PartialSpannerDirection::BOTH ? PartialSpannerDirection::INCOMING : PartialSpannerDirection::NONE;
    }
    return dir;
}

bool Slur::isIncoming() const
{
    return _partialSpannerDirection == PartialSpannerDirection::BOTH || _partialSpannerDirection == PartialSpannerDirection::INCOMING;
}

bool Slur::isOutgoing() const
{
    return _partialSpannerDirection == PartialSpannerDirection::BOTH || _partialSpannerDirection == PartialSpannerDirection::OUTGOING;
}

void Slur::undoChangeStartEndElements(ChordRest* scr, ChordRest* ecr)
{
    for (EngravingObject* lsp : linkList()) {
        Spanner* sp = static_cast<Spanner*>(lsp);
        if (sp == this) {
            score()->undo(new ChangeSpannerElements(this, scr, ecr));
        } else {
            EngravingItem* se = 0;
            EngravingItem* ee = 0;
            if (scr) {
                std::list<EngravingObject*> sel = scr->linkList();
                for (EngravingObject* lcr : sel) {
                    EngravingItem* le = toEngravingItem(lcr);
                    if (le->score() == sp->score() && le->track() == sp->track()) {
                        se = le;
                        break;
                    }
                }
            }
            if (ecr) {
                std::list<EngravingObject*> sel = ecr->linkList();
                for (EngravingObject* lcr : sel) {
                    EngravingItem* le = toEngravingItem(lcr);
                    if (le->score() == sp->score() && le->track() == sp->track2()) {
                        ee = le;
                        break;
                    }
                }
            }
            if (se && ee) {
                score()->undo(new ChangeStartEndSpanner(sp, se, ee));
                renderer()->layoutItem(sp);
            }
        }
    }
}

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Slur::setTrack(track_idx_t n)
{
    EngravingItem::setTrack(n);
    for (SpannerSegment* ss : spannerSegments()) {
        ss->setTrack(n);
    }
}

bool Slur::isCrossStaff()
{
    return startCR() && endCR()
           && (startCR()->staffMove() != 0 || endCR()->staffMove() != 0
               || startCR()->vStaffIdx() != endCR()->vStaffIdx());
}

bool Slur::hasCrossBeams()
{
    return (startCR() && startCR()->beam() && startCR()->beam()->cross()) || (endCR() && endCR()->beam() && endCR()->beam()->cross());
}

PropertyValue Slur::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::PARTIAL_SPANNER_DIRECTION:
        return partialSpannerDirection();
    case Pid::SLUR_MULTI_BEZIER_ENABLED:
        return multiBezierEnabled();
    case Pid::SLUR_MULTI_BEZIER_KNOT_COUNT:
        return multiBezierKnotCount();
    case Pid::SLUR_CURVE_MODE:
        return static_cast<int>(curveMode());
    default:
        return SlurTie::getProperty(propertyId);
    }
}

PropertyValue Slur::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::PARTIAL_SPANNER_DIRECTION:
        return PartialSpannerDirection::NONE;
    case Pid::SLUR_MULTI_BEZIER_ENABLED:
        return false;
    case Pid::SLUR_MULTI_BEZIER_KNOT_COUNT:
        return 2;
    case Pid::SLUR_CURVE_MODE:
        return static_cast<int>(CurveMode::Normal);
    default:
        return SlurTie::propertyDefault(id);
    }
}

bool Slur::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::PARTIAL_SPANNER_DIRECTION:
        setPartialSpannerDirection(v.value<PartialSpannerDirection>());
        break;
    case Pid::SLUR_MULTI_BEZIER_ENABLED:
        if (isFlatMiddleCurve()) {
            setMultiBezierEnabled(false);
        } else {
            setMultiBezierEnabled(v.toBool());
        }
        if (multiBezierEnabled() && multiBezierKnotCount() <= 0) {
            setMultiBezierKnotCount(2);
        }
        break;
    case Pid::SLUR_MULTI_BEZIER_KNOT_COUNT:
        setMultiBezierKnotCount(std::clamp(v.toInt(), 0, 16));
        break;
    case Pid::SLUR_CURVE_MODE:
    {
        const int mode = std::clamp(v.toInt(), int(CurveMode::Normal), int(CurveMode::FlatMiddle));
        setCurveMode(static_cast<CurveMode>(mode));
        if (isFlatMiddleCurve()) {
            setMultiBezierEnabled(false);
        }
        break;
    }
    default:
        return SlurTie::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}
}
