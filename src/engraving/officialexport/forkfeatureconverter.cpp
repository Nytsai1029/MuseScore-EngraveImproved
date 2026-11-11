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
#include "forkfeatureconverter.h"

#include <algorithm>
#include <array>

#include <optional>
#include <string_view>
#include <unordered_set>

#include "dom/accidental.h"
#include "dom/barline.h"
#include "dom/beam.h"
#include "dom/chord.h"
#include "dom/factory.h"
#include "dom/hairpin.h"
#include "dom/keysig.h"
#include "dom/layoutbreak.h"
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/page.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/slur.h"
#include "dom/slurtie.h"
#include "dom/spanner.h"
#include "dom/symbol.h"
#include "dom/system.h"
#include "dom/systemlock.h"
#include "dom/textbase.h"
#include "dom/tie.h"
#include "dom/tremolotwochord.h"
#include "dom/tuplet.h"
#include "style/defaultstyle.h"
#include "types/symnames.h"

#include "layoutsnapshot.h"
#include "upstreamformat.h"

using namespace mu::engraving;

ForkFeatureConverter::ForkFeatureConverter(Score* score, LayoutSnapshot& target, OfficialExportReport& report)
    : m_score(score), m_target(target), m_report(report)
{
}

void ForkFeatureConverter::convert()
{
    m_maskAllText = m_score->style().styleB(Sid::maskBarlinesForText);
    m_combinedAccidentals = m_score->style().styleB(Sid::bracketedAccidentalUseSmuflSym);

    pinBreaks();
    pinBeams();
    splitSlurs();
    pinDirections();
    stripStyle();
    stripItems();
}

//! A slur drawn as several curves in a row, each ending where the next begins, becomes as many slurs
//! between the same two notes, one for each curve. Official draws a slur as a single curve, so this is
//! all it can show of such a shape; each of its slurs thins out towards its ends, which the fork's one
//! slur does not do where its curves meet.
void ForkFeatureConverter::splitSlurs()
{
    using Curve = std::array<PointF, 4>;

    std::vector<Slur*> slurs;
    for (const auto& pair : m_score->spanner()) {
        if (pair.second && pair.second->isSlur()) {
            slurs.push_back(toSlur(pair.second));
        }
    }

    for (Slur* slur : slurs) {
        // The curves of each of its segments, in the coordinates of the page
        std::vector<std::vector<Curve> > curves;
        size_t count = 0;
        for (SpannerSegment* spannerSegment : slur->spannerSegments()) {
            SlurSegment* segment = toSlurSegment(spannerSegment);
            const std::vector<SlurSegment::MultiBezierKnot>& knots = segment->multiBezierKnotData();
            if (!segment->useBezierKnotControls() || knots.empty() || !segment->system()
                || (count != 0 && knots.size() + 1 != count)) {
                curves.clear();
                break;
            }
            count = knots.size() + 1;

            const PointF origin = segment->pagePos();
            std::vector<Curve> chain;
            PointF start = origin + segment->ups(Grip::START).pos();
            for (size_t i = 0; i < count; ++i) {
                const bool last = i + 1 == count;
                const PointF end = origin + (last ? segment->ups(Grip::END).pos() : knots[i].knot.pos());
                const PointF control1 = origin + (i == 0 ? segment->ups(Grip::BEZIER1).pos() : knots[i - 1].outHandle.pos());
                const PointF control2 = origin + (last ? segment->ups(Grip::BEZIER2).pos() : knots[i].inHandle.pos());
                chain.push_back({ start, control1, control2, end });
                start = end;
            }
            curves.push_back(chain);
        }
        if (curves.empty()) {
            continue; // an ordinary slur, or one that is not laid out: stripItem() reports the latter
        }

        // One slur for each curve: the slur itself for the first
        std::vector<Slur*> pieces { slur };
        for (size_t i = 1; i < count; ++i) {
            Slur* piece = slur->clone();
            piece->assignNewEID();
            m_score->addElement(piece);
            pieces.push_back(piece);
        }
        const DirectionV direction = slur->up() ? DirectionV::UP : DirectionV::DOWN;
        for (size_t i = 0; i < count; ++i) {
            Slur* piece = pieces[i];
            piece->setSlurDirection(direction);
            // The pieces are in each other's way wherever they meet, and are not to make room for one another
            piece->setAutoplace(false);
            for (SpannerSegment* segment : piece->spannerSegments()) {
                segment->setAutoplace(false);
            }
            piece->resetProperty(Pid::SLUR_MULTI_BEZIER_ENABLED);
            piece->resetProperty(Pid::SLUR_MULTI_BEZIER_KNOT_COUNT);
            piece->resetProperty(Pid::SLUR_CURVE_MODE);
            for (size_t segmentIdx = 0; segmentIdx < curves.size(); ++segmentIdx) {
                const SpannerSegment* segment = slur->spannerSegments()[segmentIdx];
                ItemGeometry geometry = LayoutSnapshot::geometryOf(segment);
                geometry.curve = curves[segmentIdx][i];
                m_target.setSpannerSegment(piece, segmentIdx, geometry);
            }
        }
    }
}

void ForkFeatureConverter::forEachItem(Score* score, const std::function<void(EngravingItem*)>& func)
{
    std::unordered_set<const EngravingItem*> seen;

    std::function<void(EngravingItem*)> visit = [&](EngravingItem* item) {
        if (!item || !seen.insert(item).second) {
            return;
        }
        func(item);

        if (item->isSpanner()) {
            for (SpannerSegment* segment : toSpanner(item)->spannerSegments()) {
                visit(segment);
            }
        }
        if (item->isNote()) {
            visit(toNote(item)->tieFor());
            visit(toNote(item)->tieBack());
        }
        for (EngravingObject* child : item->scanChildren()) {
            if (child && child->isEngravingItem()) {
                visit(toEngravingItem(child));
            }
        }
    };

    auto visitMeasure = [&](Measure* measure) {
        for (EngravingItem* item : measure->el()) {
            visit(item);
        }
        for (Segment& segment : measure->segments()) {
            for (EngravingItem* item : segment.elist()) {
                visit(item);
            }
            for (EngravingItem* item : segment.annotations()) {
                visit(item);
            }
        }
    };

    for (MeasureBase* measureBase = score->first(); measureBase; measureBase = measureBase->next()) {
        if (!measureBase->isMeasure()) {
            for (EngravingItem* item : measureBase->el()) {
                visit(item);
            }
            continue;
        }
        Measure* measure = toMeasure(measureBase);
        visitMeasure(measure);
        if (measure->mmRest()) {
            visitMeasure(measure->mmRest());
        }
    }

    for (const auto& [tick, spanner] : score->spanner()) {
        visit(spanner);
    }
    for (Spanner* spanner : score->unmanagedSpanners()) {
        visit(spanner);
    }
}

//---------------------------------------------------------
//   pinBreaks
//    Every system keeps its measures and every page its systems
//---------------------------------------------------------

void ForkFeatureConverter::pinBreaks()
{
    for (const System* system : m_score->systems()) {
        MeasureBase* first = system->first();
        MeasureBase* last = system->last();
        const bool lockable = first && last && (first->isMeasure() || first->isHBox())
                              && (last->isMeasure() || last->isHBox());
        if (lockable && !m_score->systemLocks()->lockContaining(first) && !m_score->systemLocks()->lockContaining(last)) {
            m_score->addSystemLock(new SystemLock(first, last));
        }
    }

    const std::vector<Page*>& pages = m_score->pages();
    for (size_t i = 0; i + 1 < pages.size(); ++i) {
        const std::vector<System*>& systems = pages[i]->systems();
        MeasureBase* last = systems.empty() ? nullptr : systems.back()->last();
        if (!last) {
            continue;
        }
        // A multimeasure rest is not saved: the break belongs to the last measure it stands for
        if (last->isMeasure() && toMeasure(last)->isMMRest()) {
            last = toMeasure(last)->mmRestLast();
        }
        if (last->pageBreak() || last->sectionBreak()) {
            continue;
        }

        LayoutBreak* pageBreak = Factory::createLayoutBreak(last);
        pageBreak->setLayoutBreakType(LayoutBreakType::PAGE);
        pageBreak->setTrack(0);
        last->add(pageBreak);
    }
}

//---------------------------------------------------------
//   pinBeams
//    A beam keeps the position layout has just given it, as if the user had dragged it there
//---------------------------------------------------------

static void keepFragment(BeamFragment& fragment, int fromIdx, int toIdx)
{
    fragment.py1[toIdx] = fragment.py1[fromIdx];
    fragment.py2[toIdx] = fragment.py2[fromIdx];
}

void ForkFeatureConverter::pinBeams()
{
    for (EngravingItem* item : LayoutSnapshot::laidOutItems(m_score)) {
        if (item->isBeam()) {
            Beam* beam = toBeam(item);
            if (beam->elements().empty() || beam->beamFragments().empty()) {
                continue;
            }

            bool allUp = true;
            bool allDown = true;
            for (const ChordRest* chordRest : beam->elements()) {
                if (chordRest->isChord()) {
                    (chordRest->up() ? allDown : allUp) = false;
                }
            }

            const int layoutIdx = beam->directionIdx();
            if (!beam->cross() && beam->direction() == DirectionV::AUTO && allUp != allDown) {
                const DirectionV direction = allUp ? DirectionV::UP : DirectionV::DOWN;
                beam->doSetDirection(direction);
                for (ChordRest* chordRest : beam->elements()) {
                    if (chordRest->isChord()) {
                        toChord(chordRest)->setStemDirection(direction);
                    }
                }
            }
            for (BeamFragment* fragment : beam->beamFragments()) {
                keepFragment(*fragment, layoutIdx, beam->directionIdx());
            }
            beam->setUserModified(true);
            beam->setGenerated(false);
        } else if (item->type() == ElementType::TREMOLO_TWOCHORD) {
            item_cast<TremoloTwoChord*>(item)->setUserModified(true);
        }
    }
}

//---------------------------------------------------------
//   pinDirections
//    Stems, slurs, ties and tuplets keep the side layout chose for them
//---------------------------------------------------------

void ForkFeatureConverter::pinDirections()
{
    forEachItem(m_score, [](EngravingItem* item) {
        if (item->isChord()) {
            Chord* chord = toChord(item);
            if (!chord->beam() && chord->stemDirection() == DirectionV::AUTO && chord->ldata()->isValid()) {
                chord->setStemDirection(chord->up() ? DirectionV::UP : DirectionV::DOWN);
            }
        } else if (item->isSlur() || item->isTie()) {
            SlurTie* slurTie = static_cast<SlurTie*>(item);
            if (slurTie->slurDirection() == DirectionV::AUTO && !slurTie->spannerSegments().empty()) {
                slurTie->setSlurDirection(slurTie->up() ? DirectionV::UP : DirectionV::DOWN);
            }
        } else if (item->isTuplet()) {
            Tuplet* tuplet = toTuplet(item);
            if (tuplet->direction() == DirectionV::AUTO && tuplet->ldata()->isValid()) {
                tuplet->setDirection(tuplet->isUp() ? DirectionV::UP : DirectionV::DOWN);
            }
        }
    });
}

//---------------------------------------------------------
//   stripStyle
//    Fork-only style keys go back to their defaults, which is upstream's behaviour
//---------------------------------------------------------

static bool endsWith(std::string_view text, std::string_view suffix)
{
    return text.size() >= suffix.size() && text.substr(text.size() - suffix.size()) == suffix;
}

void ForkFeatureConverter::stripStyle()
{
    MStyle& style = m_score->style();

    // Official has one distance for all key signature accidentals
    {
        bool sharps = false;
        bool flats = false;
        bool others = false;
        forEachItem(m_score, [&](EngravingItem* item) {
            if (!item->isKeySig()) {
                return;
            }
            for (const KeySym& keySym : toKeySig(item)->ldata()->keySymbols) {
                const std::string name(SymNames::nameForSymId(keySym.sym).ascii());
                if (name.find("Sharp") != std::string::npos) {
                    sharps = true;
                } else if (name.find("Flat") != std::string::npos) {
                    flats = true;
                } else {
                    others = true;
                }
            }
        });

        const Spatium sharpDistance = style.styleS(Sid::keysigSharpAccidentalDistance);
        const Spatium flatDistance = style.styleS(Sid::keysigFlatAccidentalDistance);
        const Spatium otherDistance = style.styleS(Sid::keysigAccidentalDistance);
        const Spatium distance = sharps ? sharpDistance : flats ? flatDistance : otherDistance;
        if ((flats && flatDistance != distance) || (others && otherDistance != distance)) {
            m_report.addDropped(DroppedFeature::KeySigAccidentalDistances);
        }
        style.set(Sid::keysigAccidentalDistance, distance);
    }

    // Official would move the beams pinned here to the nearest quarter space
    style.set(Sid::snapCustomBeamsToGrid, false);

    const MStyle& defaults = DefaultStyle::resolveStyleDefaults(style.defaultStyleVersion());
    bool letterSpacing = false;
    for (int i = 0; i < int(Sid::STYLES); ++i) {
        const Sid sid = Sid(i);
        if (UpstreamFormat::hasStyle(sid) || style.isDefault(sid)) {
            continue;
        }

        const std::string_view name = MStyle::valueName(sid);
        if (endsWith(name, "LetterSpacing")) {
            letterSpacing = true;
        } else if (sid == Sid::ledgerLineAutoCut) {
            m_report.addDropped(DroppedFeature::LedgerLineLengths);
        }
        style.set(sid, defaults.value(sid));
    }
    if (letterSpacing) {
        m_report.addDropped(DroppedFeature::LetterSpacing);
    }

    m_score->styleChanged();
}

//---------------------------------------------------------
//   stripItems
//    Fork-only properties go back to their defaults
//---------------------------------------------------------

void ForkFeatureConverter::stripItems()
{
    forEachItem(m_score, [this](EngravingItem* item) { stripItem(item); });
}

void ForkFeatureConverter::stripItem(EngravingItem* item)
{
    // Returns whether the property had a value of its own
    auto reset = [item](Pid pid) {
        const PropertyValue value = item->getProperty(pid);
        if (!value.isValid()) {
            return false;
        }
        const bool changed = value != item->propertyDefault(pid);
        item->resetProperty(pid);
        return changed;
    };
    auto drop = [this, &reset](Pid pid, DroppedFeature feature) {
        if (reset(pid)) {
            m_report.addDropped(feature);
        }
    };

    if (item->isNote()) {
        drop(Pid::LEDGER_LINE_LENGTH_OFFSET_LEFT, DroppedFeature::LedgerLineLengths);
        drop(Pid::LEDGER_LINE_LENGTH_OFFSET_RIGHT, DroppedFeature::LedgerLineLengths);
    } else if (item->isChord()) {
        Chord* chord = toChord(item);

        const std::map<int, LedgerLineOffsets> offsets = chord->ledgerLineOffsets();
        for (const auto& [line, lineOffsets] : offsets) {
            chord->setLedgerLineOffsets(line, LedgerLineOffsets());
            m_report.addDropped(DroppedFeature::LedgerLineLengths);
        }
        const std::set<int> hidden = chord->hiddenLedgerLines();
        for (int line : hidden) {
            chord->setLedgerLineHidden(line, false);
            m_report.addDropped(DroppedFeature::HiddenLedgerLines);
        }

        // Grace notes drawn before the barline are pinned there like everything else, but official cannot
        // show them on another system than their main note
        if (chord->graceBeforeBarline() && chord->explicitParent() && chord->explicitParent()->isChord()) {
            const Measure* measure = toChord(chord->explicitParent())->measure();
            if (chord->placeGraceNotesBeforeBarline() && measure && measure->isFirstInSystem()) {
                m_report.addDropped(DroppedFeature::GraceNotesBeforeSystemBreak);
            }
        }
        reset(Pid::GRACE_BEFORE_BARLINE);
        if (chord->isGrace()) {
            reset(Pid::LEADING_SPACE);
        }
    } else if (item->isTextBase()) {
        TextBase* text = toTextBase(item);
        drop(Pid::TEXT_LETTER_SPACING, DroppedFeature::LetterSpacing);
        if (text->isSlanted()) {
            m_report.addDropped(DroppedFeature::SlantedText);
        }
        reset(Pid::DIAGONAL);
        reset(Pid::TEXT_SLANT_ANGLE);
        if (text->maskBarlines() && !m_maskAllText) {
            m_report.addDropped(DroppedFeature::PerTextBarlineMask);
        }
        reset(Pid::MASK_BARLINES);
        if (item->isExpression()) {
            reset(Pid::SNAP_BEFORE_DYNAMICS);
        }
    } else if (item->isOttava()) {
        const bool broken = reset(Pid::OTTAVA_BREAK_POINTS_COUNT);
        if (reset(Pid::OTTAVA_ALLOW_BROKEN_LINE) && broken) {
            m_report.addDropped(DroppedFeature::BrokenOttavaLines);
        }
    } else if (item->isOttavaSegment()) {
        reset(Pid::OTTAVA_BREAK_POINT_1_OFFSET);
        reset(Pid::OTTAVA_BREAK_POINT_2_OFFSET);
        reset(Pid::OTTAVA_BREAK_POINT_3_OFFSET);
        reset(Pid::OTTAVA_BREAK_POINT_4_OFFSET);
    } else if (item->isHairpin()) {
        const Hairpin* hairpin = toHairpin(item);
        if (hairpin->verticalEnds() && hairpin->diagonal()) {
            m_report.addDropped(DroppedFeature::HairpinVerticalEnds);
        }
        reset(Pid::HAIRPIN_VERTICAL_ENDS);
    } else if (item->isSymbol()) {
        if (toSymbol(item)->isEditableBracketSymbol()) {
            bool changed = reset(Pid::SYMBOL_SHORT_SIDE_LENGTH);
            changed |= reset(Pid::SYMBOL_LONG_SIDE_LENGTH);
            changed |= reset(Pid::SYMBOL_TOP_HOOK_LENGTH);
            changed |= reset(Pid::SYMBOL_BOTTOM_HOOK_LENGTH);
            changed |= reset(Pid::LINE_WIDTH);
            if (changed) {
                m_report.addDropped(DroppedFeature::BracketSymbolSizes);
            }
        }
    } else if (item->isFSymbol()) {
        if (toFSymbol(item)->font().type() == muse::draw::Font::Type::MusicSymbolText) {
            m_report.addDropped(DroppedFeature::MusicSymbolTextCharacters);
        }
    } else if (item->isAccidental()) {
        // Official always draws the common accidentals and their parentheses as one glyph
        const Accidental* accidental = toAccidental(item);
        static const std::array<AccidentalType, 5> COMBINED { AccidentalType::FLAT2, AccidentalType::FLAT, AccidentalType::NATURAL,
                                                              AccidentalType::SHARP, AccidentalType::SHARP2 };
        if (!m_combinedAccidentals && accidental->bracket() == AccidentalBracket::PARENTHESIS
            && std::find(COMBINED.begin(), COMBINED.end(), accidental->accidentalType()) != COMBINED.end()) {
            m_report.addDropped(DroppedFeature::SeparateAccidentalParentheses);
        }
    } else if (item->isSlur()) {
        // What splitSlurs() could not turn into slurs of its own
        bool changed = reset(Pid::SLUR_MULTI_BEZIER_ENABLED);
        changed |= reset(Pid::SLUR_CURVE_MODE);
        reset(Pid::SLUR_MULTI_BEZIER_KNOT_COUNT);
        if (changed) {
            m_report.addDropped(DroppedFeature::SlurPiecesNotSplit);
        }
    } else if (item->isSlurSegment()) {
        reset(Pid::SLUR_MULTI_BEZIER_DATA);
        reset(Pid::SLUR_FROZEN_SHAPE);
    } else if (item->isTuplet()) {
        reset(Pid::TUPLET_BRACKET_FIXED_SLANT);
    } else if (item->type() == ElementType::TREMOLO_TWOCHORD) {
        bool changed = reset(Pid::TREMOLO_START_X_OFFSET);
        changed |= reset(Pid::TREMOLO_START_Y_OFFSET);
        changed |= reset(Pid::TREMOLO_END_X_OFFSET);
        changed |= reset(Pid::TREMOLO_END_Y_OFFSET);
        if (changed) {
            m_report.addDropped(DroppedFeature::TremoloStrokeOffsets);
        }
    } else if (item->isParenthesis()) {
        bool changed = reset(Pid::PAREN_START_Y_OFFSET);
        changed |= reset(Pid::PAREN_END_Y_OFFSET);
        if (changed) {
            m_report.addDropped(DroppedFeature::ParenthesisEndOffsets);
        }
    } else if (item->isBarLine()) {
        const BarLine* barLine = toBarLine(item);
        if (const BarLine* connector = barLine->spanConnector()) {
            if (!connector->generated() || connector->barLineType() != barLine->barLineType()) {
                m_report.addDropped(DroppedFeature::BarlineConnectorEdits);
            }
        }
    }
}
