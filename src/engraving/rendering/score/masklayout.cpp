/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "masklayout.h"

#include <algorithm>

#include "dom/barline.h"
#include "dom/chord.h"
#include "dom/jump.h"
#include "dom/keysig.h"
#include "dom/lyrics.h"
#include "dom/marker.h"
#include "dom/measure.h"
#include "dom/measurenumber.h"
#include "dom/note.h"
#include "dom/parenthesis.h"
#include "dom/segment.h"
#include "dom/staff.h"
#include "dom/stafflines.h"
#include "dom/stafftype.h"
#include "dom/system.h"
#include "dom/page.h"
#include "dom/textbase.h"
#include "dom/textlinebase.h"
#include "dom/tie.h"
#include "dom/timesig.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

namespace {
struct SignatureMaskCandidate
{
    Shape pageShape;
    staff_idx_t staffIdx = muse::nidx;
    bool spansAllStaves = false;
};

void clearTieMasks(const System* system)
{
    for (SpannerSegment* spannerSegment : system->spannerSegments()) {
        if (!spannerSegment || !spannerSegment->isTieSegment()) {
            continue;
        }
        if (!spannerSegment->ldata()->mask().empty()) {
            spannerSegment->mutldata()->setMask(Shape());
        }
    }
}

bool tieTouchesStaff(const TieSegment* tieSeg, staff_idx_t staffIdx)
{
    if (tieSeg->staffIdx() == muse::nidx) {
        return false;
    }

    staff_idx_t minStaff = tieSeg->staffIdx();
    staff_idx_t maxStaff = tieSeg->staffIdx();
    if (tieSeg->vStaffIdx() != muse::nidx) {
        minStaff = std::min(minStaff, tieSeg->vStaffIdx());
        maxStaff = std::max(maxStaff, tieSeg->vStaffIdx());
    }

    if (const Tie* tie = tieSeg->tie()) {
        if (const Note* startNote = tie->startNote()) {
            if (const Chord* startChord = startNote->chord()) {
                minStaff = std::min(minStaff, startChord->vStaffIdx());
                maxStaff = std::max(maxStaff, startChord->vStaffIdx());
            }
        }
        if (const Note* endNote = tie->endNote()) {
            if (const Chord* endChord = endNote->chord()) {
                minStaff = std::min(minStaff, endChord->vStaffIdx());
                maxStaff = std::max(maxStaff, endChord->vStaffIdx());
            }
        }
    }

    return staffIdx >= minStaff && staffIdx <= maxStaff;
}

std::vector<SignatureMaskCandidate> collectSignatureMaskCandidates(const System* system, bool maskAtKeySig,
                                                                   bool maskAtTimeSig)
{
    std::vector<SignatureMaskCandidate> candidates;

    for (const MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        const Measure* measure = toMeasure(mb);

        for (const Segment& seg : measure->segments()) {
            if (!seg.enabled()) {
                continue;
            }
            const bool isKeySigSeg = maskAtKeySig && seg.isType(SegmentType::KeySigType);
            const bool isTimeSigSeg = maskAtTimeSig && seg.isType(SegmentType::TimeSigType);
            if (!isKeySigSeg && !isTimeSigSeg) {
                continue;
            }

            for (const EngravingItem* el : seg.elist()) {
                if (!el || !el->visible() || !el->ldata()->isValid() || el->ldata()->isSkipDraw()) {
                    continue;
                }
                const SysStaff* sysStaff = system->staff(el->staffIdx());
                if (!sysStaff || !sysStaff->show()) {
                    continue;
                }

                bool spansAllStaves = false;
                Sid paddingSid = Sid::NOSTYLE;
                if (isKeySigSeg) {
                    if (!el->isKeySig()) {
                        continue;
                    }
                    const KeySig* ks = toKeySig(el);
                    if (ks->staff() && !ks->staff()->staffType(ks->tick())->genKeysig()) {
                        continue;
                    }
                    paddingSid = Sid::tieGapAtKeySigPadding;
                } else {
                    if (!el->isTimeSig()) {
                        continue;
                    }
                    const TimeSig* ts = toTimeSig(el);
                    if ((ts->staff() && !ts->staff()->staffType(ts->tick())->genTimesig())
                        || !ts->showOnThisStaff()) {
                        continue;
                    }
                    spansAllStaves = ts->isAcrossStaves();
                    paddingSid = Sid::tieGapAtTimeSigPadding;
                }

                const Shape rawShape = el->shape();
                if (rawShape.empty()) {
                    continue;
                }

                const double padding = std::max(0.0, el->style().styleS(paddingSid).val()) * el->spatium();
                candidates.push_back({
                    rawShape.translated(el->pagePos()).padded(padding),
                    el->staffIdx(),
                    spansAllStaves,
                });
            }
        }
    }

    return candidates;
}

void maskTieForSignatures(TieSegment* tieSeg, const std::vector<SignatureMaskCandidate>& candidates)
{
    Shape mask;
    if (tieSeg->ldata()->isValid() && !tieSeg->ldata()->isSkipDraw() && !candidates.empty()) {
        const PointF tiePos = tieSeg->pagePos();
        const Shape tieShape = tieSeg->shape().translated(tiePos);

        for (const SignatureMaskCandidate& sig : candidates) {
            if (!sig.spansAllStaves && !tieTouchesStaff(tieSeg, sig.staffIdx)) {
                continue;
            }
            if (tieShape.intersects(sig.pageShape)) {
                mask.add(sig.pageShape.bbox().translated(-tiePos));
            }
        }
    }

    if (!mask.empty() || !tieSeg->ldata()->mask().empty()) {
        tieSeg->mutldata()->setMask(mask);
    }
}
}

void MaskLayout::computeMasks(LayoutContext& ctx, Page* page)
{
    TRACEFUNC;

    const bool maskAllText = ctx.conf().styleB(Sid::maskBarlinesForText);
    const bool maskTieAtKeySig = ctx.conf().styleB(Sid::tieGapAtKeySig);
    const bool maskTieAtTimeSig = ctx.conf().styleB(Sid::tieGapAtTimeSig);

    for (const System* system : page->systems()) {
        std::vector<TextBase*> allSystemText = collectAllSystemText(system);
        std::vector<TextBase*> maskingText;
        maskingText.reserve(allSystemText.size());
        for (TextBase* text : allSystemText) {
            if (maskAllText || text->maskBarlines()) {
                maskingText.push_back(text);
            }
        }

        for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* measure = toMeasure(mb);

            for (const Segment& seg : measure->segments()) {
                if (seg.isType(SegmentType::BarLineType)) {
                    computeBarlineMasks(&seg, system, maskingText, ctx);
                }
            }
        }

        computeTieSignatureMasks(system, maskTieAtKeySig, maskTieAtTimeSig);
    }
}

void MaskLayout::computeTieSignatureMasks(const System* system, bool maskAtKeySig, bool maskAtTimeSig)
{
    TRACEFUNC;

    if (!maskAtKeySig && !maskAtTimeSig) {
        clearTieMasks(system);
        return;
    }

    std::vector<TieSegment*> tieSegments;
    for (SpannerSegment* spannerSegment : system->spannerSegments()) {
        if (spannerSegment && spannerSegment->isTieSegment()) {
            tieSegments.push_back(static_cast<TieSegment*>(spannerSegment));
        }
    }
    if (tieSegments.empty()) {
        return;
    }

    const std::vector<SignatureMaskCandidate> candidates = collectSignatureMaskCandidates(system, maskAtKeySig,
                                                                                          maskAtTimeSig);
    for (TieSegment* tieSeg : tieSegments) {
        maskTieForSignatures(tieSeg, candidates);
    }
}

void MaskLayout::computeBarlineMasks(const Segment* barlineSement, const System* system, const std::vector<TextBase*>& allSystemText,
                                     LayoutContext& ctx)
{
    if (barlineSement->measure()->isLastInSystem() && barlineSement == barlineSement->measure()->lastEnabled()) {
        return;
    }

    staff_idx_t nstaves = ctx.dom().nstaves();

    std::vector<BarLine*> barlines;
    barlines.reserve(nstaves);

    for (staff_idx_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        if (!system->staff(staffIdx)->show()) {
            continue;
        }
        BarLine* barline = toBarLine(barlineSement->element(staff2track(staffIdx)));
        if (!barline) {
            continue;
        }
        if (barline->spanStaff() != 0) {
            maskBarlineForText(barline, allSystemText);
        }
        if (BarLine* connector = barline->spanConnector()) {
            if (connector->ldata() && !connector->ldata()->isSkipDraw()) {
                maskBarlineForText(connector, allSystemText);
            }
        }
    }
}

void MaskLayout::maskBarlineForText(BarLine* barline, const std::vector<TextBase*>& allSystemText)
{
    TRACEFUNC;

    PointF barlinePos = barline->pagePos();
    Shape barlineShape = barline->shape().translated(barlinePos);

    Shape mask;
    const double spatium = barline->spatium();
    const MStyle& style = barline->style();
    const double collisionPaddingSp = style.styleS(Sid::maskBarlinesCollisionPadding).val();
    const double maskPaddingSp = style.styleS(Sid::maskBarlinesPadding).val();
    const double minPadding = style.styleS(Sid::maskBarlinesPaddingMin).val() * spatium;
    const double maxPadding = std::max(minPadding, style.styleS(Sid::maskBarlinesPaddingMax).val() * spatium);

    for (TextBase* text : allSystemText) {
        const double fontSizeScaleFactor = text->size() / 10.0;
        const double collisionPadding = collisionPaddingSp * spatium * fontSizeScaleFactor;
        const bool hasFrame = text->frameType() != FrameType::NO_FRAME;
        const bool useHighResShape = !text->isDynamic() && !text->hasFrame();
        const double maskPadding = hasFrame ? 0.0
                                   : std::clamp(maskPaddingSp * spatium * fontSizeScaleFactor, minPadding, maxPadding);

        PointF textPos = text->pagePos();
        if (!barlineShape.intersects(text->ldata()->bbox().translated(textPos).padded(collisionPadding))) {
            continue;
        }

        Shape textShape = (useHighResShape ? text->ldata()->highResShape() : text->ldata()->shape()).translated(textPos);

        Shape filteredTextShape;
        filteredTextShape.elements().reserve(textShape.elements().size());
        for (const ShapeElement& el : textShape.elements()) {
            if (barlineShape.intersects(el.padded(collisionPadding))) {
                filteredTextShape.add(el);
            }
        }
        if (filteredTextShape.empty()) {
            continue;
        }

        filteredTextShape = filteredTextShape.bbox();
        filteredTextShape.pad(maskPadding);

        mask.add(filteredTextShape.translate(-barlinePos));
    }

    if (mask.empty()) {
        barline->mutldata()->setMask(mask);
        return;
    }

    // Ensure that we don't leave tiny barline fragments. If two masking
    // elements are too close to each other we extend them to join.
    barlineShape.translate(-barlinePos);
    const double minFragmentLengh = style.styleS(Sid::maskBarlinesMinFragmentLength).val() * spatium;
    cleanupMask(barlineShape, mask, minFragmentLengh);

    barline->mutldata()->setMask(mask);
}

void MaskLayout::cleanupMask(const Shape& itemShape, Shape& mask, double minFragmentLength)
{
    for (size_t i = 0; i < mask.size(); ++i) {
        ShapeElement& el = mask.elements()[i];

        if (el.top() - itemShape.top() < minFragmentLength) {
            el.adjust(0.0, -minFragmentLength, 0.0, 0.0);
        }
        if (itemShape.bottom() - el.bottom() < minFragmentLength) {
            el.adjust(0.0, 0.0, 0.0, minFragmentLength);
        }

        if (el.left() + itemShape.left() < minFragmentLength) {
            el.adjust(-minFragmentLength, 0.0, 0.0, 0.0);
        }
        if (itemShape.right() - el.right() < minFragmentLength) {
            el.adjust(0.0, 0.0, minFragmentLength, 0.0);
        }

        for (size_t j = i + 1; j < mask.size(); ++j) {
            ShapeElement& otherEl = mask.elements()[j];
            if (intersects(el.left(), el.right(), otherEl.left(), otherEl.right())) {
                bool otherIsBelow = otherEl.y() > el.y();
                double vertClearance = otherIsBelow ? otherEl.top() - el.bottom() : el.top() - otherEl.bottom();
                if (vertClearance < minFragmentLength) {
                    (otherIsBelow ? el : otherEl).adjust(0.0, 0.0, 0.0, minFragmentLength);
                }
            }
            if (intersects(el.top(), el.bottom(), otherEl.top(), el.bottom())) {
                bool otherIsRight = otherEl.x() > el.x();
                double horClearance = otherIsRight ? otherEl.left() - el.right() : el.left() - otherEl.right();
                if (horClearance < minFragmentLength) {
                    (otherIsRight ? el : otherEl).adjust(0.0, 0.0, minFragmentLength, 0.0);
                }
            }
        }
    }
}

std::vector<TextBase*> MaskLayout::collectAllSystemText(const System* system)
{
    TRACEFUNC;

    std::vector<TextBase*> allText;

    for (const MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        const Measure* measure = toMeasure(mb);
        for (staff_idx_t i = 0; i < measure->mstaves().size(); ++i) {
            if (measure->showMeasureNumberOnStaff(i)) {
                if (MeasureNumber* measureNumber = measure->measureNumber(i)) {
                    allText.push_back(measureNumber);
                }
            }
        }
        for (EngravingItem* item : measure->el()) {
            const SysStaff* staff = system ? system->staff(item->staffIdx()) : nullptr;
            const bool staffVisible = staff && staff->show();
            if ((item->isMarker() || item->isJump()) && item->visible() && staffVisible) {
                allText.push_back(toTextBase(item));
            }
        }
        for (const Segment& s : measure->segments()) {
            if (!s.isType(Segment::CHORD_REST_OR_TIME_TICK_TYPE) || !s.enabled()) {
                continue;
            }
            for (EngravingItem* annotation : s.annotations()) {
                const SysStaff* staff = system ? system->staff(annotation->staffIdx()) : nullptr;
                const bool staffVisible = staff && staff->show();
                if (annotation->isTextBase() && annotation->visible() && staffVisible) {
                    allText.push_back(toTextBase(annotation));
                }
            }
            if (!s.isChordRestType()) {
                continue;
            }
            for (EngravingItem* chordRest : s.elist()) {
                if (!chordRest || !system->staff(chordRest->staffIdx())->show()) {
                    continue;
                }
                for (Lyrics* lyr : toChordRest(chordRest)->lyrics()) {
                    if (lyr->visible()) {
                        allText.push_back(lyr);
                    }
                }
            }
        }
    }

    for (SpannerSegment* spannerSegment : system->spannerSegments()) {
        if (!spannerSegment->isTextLineBaseSegment() || !system->staff(spannerSegment->staffIdx())->show()
            || !spannerSegment->getProperty(Pid::VISIBLE).toBool()) {
            continue;
        }
        TextLineBaseSegment* textLineBaseSegment = static_cast<TextLineBaseSegment*>(spannerSegment);
        Text* beginText = textLineBaseSegment->text();
        Text* endText = textLineBaseSegment->endText();
        if (beginText && !beginText->empty()) {
            allText.push_back(beginText);
        }
        if (endText && !endText->empty()) {
            allText.push_back(endText);
        }
    }

    return allText;
}

void MaskLayout::maskTABStringLinesForFrets(StaffLines* staffLines, const LayoutContext& ctx)
{
    staff_idx_t staffIdx = staffLines->staffIdx();
    bool linesThrough = ctx.dom().staff(staffIdx)->staffType(Fraction())->linesThrough();

    PointF staffLinesPos = staffLines->pagePos();

    double padding = ctx.conf().styleMM(Sid::tabFretPadding);

    track_idx_t startTrack = staff2track(staffIdx);
    track_idx_t endTrack = startTrack + VOICES;

    Shape mask;

    auto maskFret = [&mask, linesThrough, padding, staffLinesPos] (Chord* chord) {
        for (Note* note : chord->notes()) {
            if (note->visible() && !note->shouldHideFret() && (!linesThrough || note->fretConflict())) {
                Shape noteShape = note->ldata()->bbox();
                const Parenthesis* leftParen = note->leftParen();
                if (leftParen && leftParen->addToSkyline()) {
                    noteShape.add(leftParen->ldata()->bbox().translated(leftParen->pos()));
                }
                const Parenthesis* rightParen = note->rightParen();
                if (rightParen && rightParen->addToSkyline()) {
                    noteShape.add(rightParen->ldata()->bbox().translated(rightParen->pos()));
                }
                noteShape.translate(note->pagePos());

                noteShape.pad(padding);
                mask.add(noteShape.translated(-staffLinesPos));
            }
        }
    };

    const Measure* measure = staffLines->measure();
    for (Segment* seg = measure->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            EngravingItem* el = seg->element(track);
            if (!el || !el->isChord()) {
                continue;
            }
            maskFret(toChord(el));
            for (Chord* grace : toChord(el)->graceNotes()) {
                maskFret(grace);
            }
        }
    }

    staffLines->mutldata()->setMask(mask);
}
