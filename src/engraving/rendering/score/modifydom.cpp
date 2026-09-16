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
#include "modifydom.h"

#include <array>

#include "dom/measure.h"
#include "dom/staff.h"
#include "dom/spannermap.h"
#include "dom/timesig.h"
#include "dom/trill.h"
#include "dom/ornament.h"
#include "dom/note.h"
#include "dom/utils.h"
#include "dom/chord.h"
#include "dom/keysig.h"
#include "dom/hook.h"
#include "dom/part.h"
#include "dom/pitchspelling.h"
#include "dom/score.h"
#include "rendering/score/chordlayout.h"
#include "style/style.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

static void applyCautionaryAccidentalsFromPreviousMeasure(AccidentalState& as, const Measure* measure, const Staff* staff)
{
    if (!measure->score()->style().styleB(Sid::showCautionaryAccidentals)) {
        return;
    }

    const Measure* prev = measure->prevMeasure();
    if (!prev) {
        return;
    }

    for (const MeasureBase* mb = prev; mb && mb != measure; mb = mb->next()) {
        if (mb->sectionBreak()) {
            return;
        }
    }

    const KeySigEvent prevKey = staff->keySigEvent(prev->endTick() - Fraction::fromTicks(1));
    const KeySigEvent curKey = staff->keySigEvent(measure->tick());
    if (prevKey != curKey) {
        return;
    }

    std::array<bool, STEP_DELTA_OCTAVE> seen{};
    std::array<AccidentalVal, STEP_DELTA_OCTAVE> lastAlter{};

    auto considerChord = [&](const Chord* chord) {
        for (const Note* note : chord->notes()) {
            const int step = tpc2step(note->tpc());
            if (step < 0 || step >= STEP_DELTA_OCTAVE) {
                continue;
            }
            seen[static_cast<size_t>(step)] = true;
            lastAlter[static_cast<size_t>(step)] = tpc2alter(note->tpc());
        }
    };

    const track_idx_t startTrack = staff->part()->startTrack();
    const track_idx_t endTrack = staff->part()->endTrack();

    for (const Segment& segment : prev->segments()) {
        if (!segment.isJustType(SegmentType::ChordRest)) {
            continue;
        }
        for (track_idx_t t = startTrack; t < endTrack; ++t) {
            Chord* chord = item_cast<Chord*>(segment.element(t), CastMode::MAYBE_BAD);
            if (!chord) {
                continue;
            }
            for (Chord* grace : chord->graceNotesBefore()) {
                considerChord(grace);
            }
            considerChord(chord);
            for (Chord* grace : chord->graceNotesAfter()) {
                considerChord(grace);
            }
        }
    }

    for (int step = 0; step < STEP_DELTA_OCTAVE; ++step) {
        if (!seen[static_cast<size_t>(step)]) {
            continue;
        }
        if (lastAlter[static_cast<size_t>(step)] == as.accidentalVal(step)) {
            continue;
        }
        for (int line = step; line < MAX_ACC_STATE; line += STEP_DELTA_OCTAVE) {
            as.setForceRestateAccidental(line, true);
        }
    }
}

void ModifyDom::setCrossMeasure(const Measure* measure, LayoutContext& ctx)
{
    bool crossMeasure = ctx.conf().styleB(Sid::crossMeasureValues);
    const DomAccessor& dom = ctx.dom();
    for (staff_idx_t staffIdx = 0; staffIdx < dom.nstaves(); ++staffIdx) {
        const Staff* staff = dom.staff(staffIdx);
        if (!staff->show()) {
            continue;
        }

        track_idx_t startTrack = staffIdx * VOICES;
        track_idx_t endTrack  = startTrack + VOICES;

        for (const Segment& segment : measure->segments()) {
            if (!segment.isJustType(SegmentType::ChordRest)) {
                continue;
            }

            for (track_idx_t t = startTrack; t < endTrack; ++t) {
                ChordRest* cr = segment.cr(t);
                if (!cr) {
                    continue;
                }
                if (cr->isChord()) {
                    Chord* chord = toChord(cr);
                    if (!chord->isGrace()) {
                        ChordLayout::crossMeasureSetup(chord, crossMeasure, ctx);
                    }
                }
            }
        }
    }
}

void ModifyDom::connectTremolo(Measure* m)
{
    m->connectTremolo();
}

void ModifyDom::cmdUpdateNotes(const Measure* measure, const DomAccessor& dom)
{
    for (staff_idx_t staffIdx = 0; staffIdx < dom.nstaves(); ++staffIdx) {
        const Staff* staff = dom.staff(staffIdx);
        if (!staff->show()) {
            continue;
        }

        AccidentalState as;          // list of already set accidentals for this measure
        // initAccidentalState
        {
            as.init(staff->keySigEvent(measure->tick()));

            // Trills may carry an accidental into this measure that requires a force-restate
            int ticks = measure->tick().ticks();
            auto spanners = dom.spannerMap().findOverlapping(ticks, ticks, true);
            for (auto iter : spanners) {
                Spanner* spanner = iter.value;
                if (spanner->staffIdx() != staffIdx || !spanner->isTrill()
                    || spanner->tick() == measure->tick() || spanner->tick2() == measure->tick()) {
                    continue;
                }
                Ornament* ornament = toTrill(spanner)->ornament();
                Note* trillNote = ornament ? ornament->noteAbove() : nullptr;
                if (trillNote && trillNote->accidental() && ornament->showAccidental() == OrnamentShowAccidental::DEFAULT) {
                    int line = absStep(trillNote->tpc(), trillNote->epitch());
                    as.setForceRestateAccidental(line, true);
                }
            }

            applyCautionaryAccidentalsFromPreviousMeasure(as, measure, staff);
        }

        track_idx_t startTrack = staff->part()->startTrack();
        track_idx_t mainTrack = staffIdx * VOICES;
        track_idx_t endTrack = staff->part()->endTrack();

        for (const Segment& segment : measure->segments()) {
            if (segment.isJustType(SegmentType::KeySig) && !segment.header() && !segment.trailer()) {
                KeySig* ks = item_cast<KeySig*>(segment.element(mainTrack));
                if (ks) {
                    Fraction tick = segment.tick();
                    as.init(staff->keySigEvent(tick));
                }
            } else if (segment.isJustType(SegmentType::ChordRest)) {
                for (track_idx_t t = startTrack; t < endTrack; ++t) {
                    Chord* chord = item_cast<Chord*>(segment.element(t), CastMode::MAYBE_BAD); // maybe Rest
                    if (chord) {
                        chord->cmdUpdateNotes(&as, staffIdx);
                    }
                }
            }
        }
    }
}

void ModifyDom::createStems(const Measure* measure, LayoutContext& ctx)
{
    const DomAccessor& dom = ctx.dom();
    for (staff_idx_t staffIdx = 0; staffIdx < dom.nstaves(); ++staffIdx) {
        const Staff* staff = dom.staff(staffIdx);
        if (!staff->show()) {
            continue;
        }

        track_idx_t startTrack = staffIdx * VOICES;
        track_idx_t endTrack  = startTrack + VOICES;

        for (const Segment& segment : measure->segments()) {
            if (!segment.isJustType(SegmentType::ChordRest)) {
                continue;
            }

            for (track_idx_t t = startTrack; t < endTrack; ++t) {
                ChordRest* cr = segment.cr(t);
                if (!cr) {
                    continue;
                }

                auto createStems = [](Chord* chord) {
                    if (!chord->shouldHaveStem()) {
                        chord->removeStem();
                        return;
                    }

                    if (!chord->stem()) {
                        chord->createStem();
                    }
                };

                if (cr->isChord()) {
                    Chord* chord = toChord(cr);

                    for (Chord* c : chord->graceNotes()) {
                        createStems(c);
                    }

                    createStems(chord);     // create stems needed to calculate spacing
                    // stem direction can change later during beam processing
                }
            }
        }
    }
}

void ModifyDom::setTrackForChordGraceNotes(Measure* measure, const DomAccessor& dom)
{
    for (staff_idx_t staffIdx = 0; staffIdx < dom.nstaves(); ++staffIdx) {
        track_idx_t startTrack = staffIdx * VOICES;
        track_idx_t endTrack  = startTrack + VOICES;

        for (const Segment& segment : measure->segments()) {
            if (!segment.isJustType(SegmentType::ChordRest)) {
                continue;
            }

            for (track_idx_t t = startTrack; t < endTrack; ++t) {
                ChordRest* cr = segment.cr(t);
                if (!cr) {
                    continue;
                }

                if (cr->isChord()) {
                    Chord* chord = toChord(cr);
                    for (Chord* c : chord->graceNotes()) {
                        c->setTrack(t);
                    }
                }
            }
        }
    }
}

void ModifyDom::sortMeasureSegments(Measure* measure, LayoutContext& ctx)
{
    measure = measure->coveringMMRestOrThis();

    // Move segments between measure which need to move

    // Move key and time signature segments to the correct measure.
    // Depending on configuration, this could mean moving segments at the end of this measure to the start of the following measure
    // or moving segments from the start of the following measure to the end of this measure

    // Loop through clef, key, time sigs in this bar.  If any are found and shouldn't be in this bar, move to the next

    auto changeAppliesToRepeatAndContinuation = [&](const Segment& seg) -> bool {
        // Check if the change applies to the beginning of the repeat section as well as the continuation
        const std::vector<Measure*> measures = findFollowingRepeatMeasures(measure);
        for (const Measure* repeatMeasure : measures) {
            if (repeatMeasure == measure->nextMeasureMM()) {
                continue;
            }
            const Fraction startTick = repeatMeasure->tick();
            for (track_idx_t track = 0; track < ctx.dom().nstaves(); track += VOICES) {
                const Staff* staff = ctx.dom().staff(track2staff(track));

                if (seg.isKeySigType()) {
                    const Key continuationKey = staff->key(seg.tick());
                    const Key repeatKey = staff->key(startTick);
                    const Key keyBeforeContinuation = staff->key(seg.tick() - Fraction::eps());
                    return keyBeforeContinuation != continuationKey && continuationKey == repeatKey;
                }

                if (seg.isTimeSigType()) {
                    const TimeSig* continuationTimeSig = staff->timeSig(seg.tick());
                    const TimeSig* repeatTimeSig = staff->timeSig(startTick);
                    const TimeSig* timeSigBeforeContinuation = staff->timeSig(seg.tick() - Fraction::eps());
                    return continuationTimeSig && repeatTimeSig && timeSigBeforeContinuation
                           && timeSigBeforeContinuation->sig() != continuationTimeSig->sig()
                           && continuationTimeSig->sig() == repeatTimeSig->sig();
                }

                if (seg.isClefType()) {
                    const ClefType continuationClef = staff->clef(seg.tick());
                    const ClefType repeatClef = staff->clef(startTick);
                    const ClefType clefBeforeContinuation = staff->clef(seg.tick() - Fraction::eps());
                    if (clefBeforeContinuation != continuationClef && continuationClef == repeatClef) {
                        return true;
                    }
                }
            }
        }
        return false;
    };

    auto clefSegBarlinePosition = [&](const Segment& seg) -> ClefToBarlinePosition
    {
        for (EngravingItem* el : seg.elist()) {
            if (!el || !el->isClef()) {
                continue;
            }
            const Clef* clef = toClef(el);
            if (clef->clefToBarlinePosition() != ClefToBarlinePosition::AUTO) {
                return clef->clefToBarlinePosition();
            }
        }
        return ClefToBarlinePosition::AUTO;
    };

    MeasureBase* nextMb = measure ? measure->nextMM() : nullptr;

    if (!nextMb || !nextMb->isMeasure()) {
        return;
    }

    Measure* nextMeasure = toMeasure(nextMb);

    const bool sigsShouldBeInThisMeasure = ((measure->repeatEnd() && ctx.conf().styleB(Sid::changesBeforeBarlineRepeats))
                                            || (measure->repeatJump() && ctx.conf().styleB(Sid::changesBeforeBarlineOtherJumps)));
    std::vector<Segment*> segsToRemove;

    std::vector<Segment*> segsToMoveToNextMeasure;
    for (Segment& seg : measure->segments()) {
        if (seg.tick() != measure->endTick() || seg.isChordRestType()) {
            continue;
        }

        // Move clefs at the end of this measure into the next measure
        if (seg.isClefType()) {
            ClefToBarlinePosition pos = clefSegBarlinePosition(seg);
            // Clef position explicitly set by user
            if (pos == ClefToBarlinePosition::BEFORE) {
                continue;
            } else if (pos == ClefToBarlinePosition::AFTER) {
                segsToMoveToNextMeasure.push_back(&seg);
                continue;
            }

            // AUTO on normal barline
            if (!measure->repeatEnd()) {
                continue;
            }

            // Otherwise, pos is AUTO
            // Place changes before barline & clefs before repeats style settings
            if ((!sigsShouldBeInThisMeasure && !changeAppliesToRepeatAndContinuation(seg))
                || (measure->repeatEnd() && !ctx.conf().styleB(Sid::placeClefsBeforeRepeats))) {
                segsToMoveToNextMeasure.push_back(&seg);
                continue;
            }
        }

        // Move key sigs and time sigs at the end of this measure into the next measure
        if ((seg.isKeySigType() || seg.isTimeSigType()) && !seg.header() && !seg.trailer()
            && (!sigsShouldBeInThisMeasure || !changeAppliesToRepeatAndContinuation(seg))) {
            if (nextMeasure && nextMeasure->findSegmentR(seg.segmentType(), Fraction(0, 1))) {
                segsToRemove.push_back(&seg);
                continue;
            }
            segsToMoveToNextMeasure.push_back(&seg);
        }
    }

    // No next measure, so these segments are useless
    if (!nextMeasure) {
        for (Segment* seg : segsToMoveToNextMeasure) {
            measure->remove(seg);
        }
        return;
    }

    std::vector<Segment*> segsToMoveToThisMeasure;
    for (Segment& seg : nextMeasure->segments()) {
        if (seg.tick() != nextMeasure->tick() || seg.isChordRestType()) {
            continue;
        }

        // Move clefs at the beginning of the next measure into this measure
        if (seg.isClefType()) {
            ClefToBarlinePosition pos = clefSegBarlinePosition(seg);
            // Clef position explicitly set by user
            if (pos == ClefToBarlinePosition::BEFORE) {
                segsToMoveToThisMeasure.push_back(&seg);
                continue;
            } else if (pos == ClefToBarlinePosition::AFTER) {
                continue;
            }

            if (!measure->repeatEnd()) {
                segsToMoveToThisMeasure.push_back(&seg);
                continue;
            }

            // Otherwise, pos is AUTO
            // Place changes before barline & clefs before repeats style settings
            if ((sigsShouldBeInThisMeasure && changeAppliesToRepeatAndContinuation(seg))
                || !measure->repeatEnd() || ctx.conf().styleB(Sid::placeClefsBeforeRepeats)) {
                segsToMoveToThisMeasure.push_back(&seg);
            }
        }

        // Move key sigs and time sigs at the start of the next measure to the end of this measure
        if ((seg.isTimeSigType() || seg.isKeySigType()) && !seg.header() && !seg.trailer()
            && sigsShouldBeInThisMeasure && changeAppliesToRepeatAndContinuation(seg)) {
            if (measure->findSegmentR(seg.segmentType(), measure->ticks())) {
                segsToRemove.push_back(&seg);
                continue;
            }
            segsToMoveToThisMeasure.push_back(&seg);
        }
    }

    for (Segment* seg : segsToRemove) {
        // Don't add duplicate segs to a measure
        ctx.mutDom().doUndoRemoveElement(seg);
    }

    for (Segment* seg : segsToMoveToNextMeasure) {
        seg->setRtick(Fraction(0, 1));
        seg->setEndOfMeasureChange(false);
        measure->segments().remove(seg);
        nextMeasure->add(seg);
    }

    for (Segment* seg : segsToMoveToThisMeasure) {
        seg->setRtick(measure->ticks());
        seg->setEndOfMeasureChange(true);
        nextMeasure->segments().remove(seg);
        measure->add(seg);
    }

    measure->checkEndOfMeasureChange();

    // Sort segments at start of next measure
    removeAndAddBeginSegments(nextMeasure);

    // Sort segments at start of first measure
    Measure* prevMeasure = measure->prevMeasure();
    if (prevMeasure && prevMeasure == ctx.dom().firstMeasure()) {
        removeAndAddBeginSegments(prevMeasure);
    }
}

void ModifyDom::removeAndAddBeginSegments(Measure* measure)
{
    std::vector<Segment*> segsToSort;
    for (Segment& seg : measure->segments()) {
        if (seg.rtick() != Fraction(0, 1) || seg.isChordRestType() || seg.isEndBarLineType()) {
            continue;
        }

        segsToSort.push_back(&seg);
    }

    // Re-add the segments. They will be placed in their correct positions
    for (Segment* seg : segsToSort) {
        measure->segments().remove(seg);
    }
    for (Segment* seg : segsToSort) {
        measure->add(seg);
    }
}
