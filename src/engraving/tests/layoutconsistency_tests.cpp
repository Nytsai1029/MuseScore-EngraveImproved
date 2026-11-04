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

#include <gtest/gtest.h>

#include <filesystem>
#include <vector>

#include "dom/articulation.h"
#include "dom/barline.h"
#include "dom/chord.h"
#include "dom/chordline.h"
#include "dom/editdata.h"
#include "dom/factory.h"
#include "dom/hairpin.h"
#include "dom/hammeronpulloff.h"
#include "dom/ledgerline.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/mscore.h"
#include "dom/mscoreview.h"
#include "dom/note.h"
#include "dom/page.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/slur.h"
#include "dom/staff.h"
#include "dom/stafftext.h"
#include "dom/stafftype.h"
#include "dom/symbol.h"
#include "dom/system.h"
#include "dom/tuplet.h"
#include "dom/undo.h"

#include "utils/scorerw.h"

using namespace mu;
using namespace mu::engraving;

static const String LAYOUTCONSISTENCY_DATA_DIR(u"layoutconsistency_data/");

namespace {
// A view that finds nothing, so dragging a slur end never re-anchors it
class NoTargetView : public MuseScoreView
{
public:
    void dataChanged(const RectF&) override {}
    void updateAll() override {}
    void drawBackground(muse::draw::Painter*, const RectF&) const override {}
};

MasterScore* saveAndReload(Score* score, const String& name)
{
    EXPECT_TRUE(ScoreRW::saveScore(score, name));
    const std::string path = std::filesystem::absolute(name.toStdString()).string();
    return ScoreRW::readScore(String::fromStdString(path), true);
}

Spanner* spannerAt(Score* score, ElementType type, const Fraction& tick)
{
    for (const auto& pair : score->spanner()) {
        Spanner* spanner = pair.second;
        if (spanner->type() == type && spanner->tick() == tick) {
            return spanner;
        }
    }
    return nullptr;
}

SlurSegment* slurSegmentAt(Score* score, const Fraction& tick)
{
    Spanner* slur = spannerAt(score, ElementType::SLUR, tick);
    return slur ? toSlurSegment(slur->frontSegment()) : nullptr;
}

std::vector<PointF> slurPathPoints(const SlurSegment* segment)
{
    std::vector<PointF> points;
    const muse::draw::PainterPath& path = segment->ldata()->path.value();
    for (size_t i = 0; i < path.elementCount(); ++i) {
        points.push_back(segment->pagePos() + PointF(path.elementAt(i)));
    }
    return points;
}

double maxDistance(const std::vector<PointF>& a, const std::vector<PointF>& b)
{
    if (a.size() != b.size()) {
        return std::numeric_limits<double>::infinity();
    }
    double result = 0.0;
    for (size_t i = 0; i < a.size(); ++i) {
        result = std::max(result, std::hypot(a[i].x() - b[i].x(), a[i].y() - b[i].y()));
    }
    return result;
}

Note* noteAt(Score* score, const Fraction& tick)
{
    Segment* segment = score->tick2segment(tick, true, SegmentType::ChordRest);
    EngravingItem* item = segment ? segment->element(0) : nullptr;
    return item && item->isChord() ? toChord(item)->upNote() : nullptr;
}

void changePitch(Score* score, Note* note, int pitch, int tpc)
{
    score->startCmd(TranslatableString::untranslatable("Change pitch test"));
    score->undoChangePitch(note, pitch, tpc, tpc);
    note->triggerLayout();
    score->endCmd();
}

void dragGrip(Score* score, SlurSegment* segment, int grip, const PointF& delta)
{
    NoTargetView view;
    view.setScore(score);
    EditData ed(&view);
    ed.pos = PointF(-1.0e6, -1.0e6);

    score->startCmd(TranslatableString::untranslatable("Drag slur grip test"));
    segment->startEdit(ed);
    ed.curGrip = Grip(grip);
    ed.delta = delta;
    segment->startEditDrag(ed);
    segment->editDrag(ed);
    segment->endEditDrag(ed);
    segment->endEdit(ed);
    score->endCmd();
}

// Page rectangles of fermatas and of each chord's articulations and ledger lines, in score order
std::vector<RectF> chordMarkRects(Score* score)
{
    std::vector<RectF> rects;
    auto collect = [&rects](const Chord* chord) {
        for (const Articulation* a : chord->articulations()) {
            rects.push_back(a->pageBoundingRect());
        }
        for (const LedgerLine* line : chord->ledgerLines()) {
            rects.push_back(line->pageBoundingRect());
        }
    };
    for (Segment* s = score->firstMeasure()->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        for (const EngravingItem* annotation : s->annotations()) {
            if (annotation->isFermata()) {
                rects.push_back(annotation->pageBoundingRect());
            }
        }
        EngravingItem* item = s->element(0);
        if (item && item->isChord()) {
            for (const Chord* grace : toChord(item)->graceNotes()) {
                collect(grace);
            }
            collect(toChord(item));
        }
    }
    return rects;
}

// Edits a note in the first system, then checks the range layout against a full layout
void expectRangeLayoutMatchesFullLayout(Score* score, const Fraction& tick)
{
    ASSERT_GT(score->systems().size(), 1u);
    changePitch(score, noteAt(score, tick), noteAt(score, tick)->pitch() + 2, noteAt(score, tick)->tpc1() + 2);
    const std::vector<RectF> range = chordMarkRects(score);
    score->doLayout();
    const std::vector<RectF> full = chordMarkRects(score);
    ASSERT_EQ(range.size(), full.size());
    for (size_t i = 0; i < range.size(); ++i) {
        EXPECT_NEAR(range[i].x(), full[i].x(), 1e-3) << "mark " << i << " after edit at " << tick.toString().toStdString();
        EXPECT_NEAR(range[i].y(), full[i].y(), 1e-3) << "mark " << i << " after edit at " << tick.toString().toStdString();
        EXPECT_NEAR(range[i].width(), full[i].width(), 1e-3) << "mark " << i << " after edit at " << tick.toString().toStdString();
    }
}

BarLine* endBarLineAt(Measure* measure, staff_idx_t staffIdx)
{
    Segment* seg = measure->findSegment(SegmentType::EndBarLine, measure->endTick());
    EngravingItem* item = seg ? seg->element(staffIdx * VOICES) : nullptr;
    return item && item->isBarLine() ? toBarLine(item) : nullptr;
}
}

class Engraving_LayoutConsistencyTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_useRead302 = MScore::useRead302InTestMode;
        MScore::useRead302InTestMode = false;
    }

    void TearDown() override
    {
        MScore::useRead302InTestMode = m_useRead302;
    }

private:
    bool m_useRead302 = false;
};

//---------------------------------------------------------
//   timeSigBracketKeepsItsOwnValues
//   Style refreshes used to push keyboard bracket values into a time signature
//   bracket and leave them styled, so they were drawn but never saved.
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, timeSigBracketKeepsItsOwnValues)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"slurs.mscx");
    ASSERT_TRUE(score);

    Symbol* bracket = Factory::createSymbol(score->dummy());
    bracket->setSym(SymId::timeSigBracketLeft);
    bracket->styleChanged();
    EXPECT_FALSE(bracket->isStyled(Pid::SYMBOL_LONG_SIDE_LENGTH));
    EXPECT_NEAR(bracket->timeSigBracketHeight().val(), 4.944, 1e-9);

    bracket->setProperty(Pid::SYMBOL_LONG_SIDE_LENGTH, PropertyValue::fromValue(Spatium(6.0)));
    EngravingItem* copy = ScoreRW::writeReadElement(bracket);
    ASSERT_TRUE(copy && copy->isSymbol());
    EXPECT_NEAR(toSymbol(copy)->timeSigBracketHeight().val(), 6.0, 1e-9);

    Symbol* handBracket = Factory::createSymbol(score->dummy());
    handBracket->setSym(SymId::keyboardPlayWithLH);
    EXPECT_TRUE(handBracket->isStyled(Pid::SYMBOL_LONG_SIDE_LENGTH));

    delete copy;
    delete bracket;
    delete handBracket;
    delete score;
}

//---------------------------------------------------------
//   hammerOnPullOffKeepsCurveMode
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, hammerOnPullOffKeepsCurveMode)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"slurs.mscx");
    ASSERT_TRUE(score);

    Spanner* hopo = spannerAt(score, ElementType::HAMMER_ON_PULL_OFF, Fraction(3, 1));
    ASSERT_TRUE(hopo);
    score->startCmd(TranslatableString::untranslatable("Flat middle test"));
    hopo->undoChangeProperty(Pid::SLUR_CURVE_MODE, int(Slur::CurveMode::FlatMiddle));
    score->endCmd();

    MasterScore* reloaded = saveAndReload(score, u"hopoCurveMode-test.mscx");
    ASSERT_TRUE(reloaded);
    Spanner* reloadedHopo = spannerAt(reloaded, ElementType::HAMMER_ON_PULL_OFF, Fraction(3, 1));
    ASSERT_TRUE(reloadedHopo);
    EXPECT_TRUE(toSlur(reloadedHopo)->isFlatMiddleCurve());

    delete reloaded;
    delete score;
}

//---------------------------------------------------------
//   spanConnectorEditSurvivesSaveReload
//   A connector is written inside its parent bar line, which used to be skipped
//   while it was still generated.
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, spanConnectorEditSurvivesSaveReload)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"twoStaves.mscx");
    ASSERT_TRUE(score);

    score->startCmd(TranslatableString::untranslatable("Enable barline span"));
    score->undo(new ChangeProperty(score->staff(0), Pid::STAFF_BARLINE_SPAN, 1));
    score->endCmd();
    score->doLayout();

    int measureIdx = 0;
    BarLine* parent = nullptr;
    for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure(), ++measureIdx) {
        BarLine* bl = endBarLineAt(m, 0);
        if (bl && bl->generated() && bl->spanConnector()) {
            parent = bl;
            break;
        }
    }
    ASSERT_TRUE(parent);

    score->startCmd(TranslatableString::untranslatable("Change connector barline type"));
    parent->spanConnector()->undoChangeProperty(Pid::BARLINE_TYPE, PropertyValue::fromValue(BarLineType::DOTTED));
    score->endCmd();

    MasterScore* reloaded = saveAndReload(score, u"spanConnector-test.mscx");
    ASSERT_TRUE(reloaded);
    Measure* m = reloaded->firstMeasure();
    for (int i = 0; i < measureIdx && m; ++i) {
        m = m->nextMeasure();
    }
    ASSERT_TRUE(m);
    BarLine* reloadedParent = endBarLineAt(m, 0);
    ASSERT_TRUE(reloadedParent && reloadedParent->spanConnector());
    EXPECT_EQ(reloadedParent->spanConnector()->barLineType(), BarLineType::DOTTED);

    delete reloaded;
    delete score;
}

//---------------------------------------------------------
//   acciaccaturaSlashOffSurvivesSaveReload
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, acciaccaturaSlashOffSurvivesSaveReload)
{
    MasterScore* score = ScoreRW::readScore(u"note_data/grace.mscx");
    ASSERT_TRUE(score);
    Chord* chord = score->firstMeasure()->findChord(Fraction(0, 1), 0);
    ASSERT_TRUE(chord);

    score->setGraceNote(chord, chord->upNote()->pitch(), NoteType::ACCIACCATURA, Constants::DIVISION / 2);
    Chord* grace = chord->graceNotes().front();
    EXPECT_TRUE(grace->showStemSlash());
    score->startCmd(TranslatableString::untranslatable("Hide stem slash test"));
    grace->undoChangeProperty(Pid::SHOW_STEM_SLASH, false);
    score->endCmd();
    EXPECT_FALSE(grace->showStemSlash());

    MasterScore* reloaded = saveAndReload(score, u"acciaccaturaSlash-test.mscx");
    ASSERT_TRUE(reloaded);
    Chord* reloadedChord = reloaded->firstMeasure()->findChord(Fraction(0, 1), 0);
    ASSERT_TRUE(reloadedChord && !reloadedChord->graceNotes().empty());
    EXPECT_FALSE(reloadedChord->graceNotes().front()->showStemSlash());

    delete reloaded;
    delete score;
}

//---------------------------------------------------------
//   chordLineOnSmallStaffKeepsSize
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, chordLineOnSmallStaffKeepsSize)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"slurs.mscx");
    ASSERT_TRUE(score);
    score->staff(0)->staffType(Fraction(0, 1))->setUserMag(0.5);
    score->doLayout();

    Note* note = noteAt(score, Fraction(3, 1));
    Chord* chord = note->chord();
    ChordLine* chordLine = Factory::createChordLine(chord);
    chordLine->setChordLineType(ChordLineType::FALL);
    chordLine->setTrack(chord->track());
    chordLine->setParent(chord);
    chordLine->setNote(note);
    score->startCmd(TranslatableString::untranslatable("Add fall test"));
    score->undoAddElement(chordLine);
    score->endCmd();
    chordLine->setModified(true);

    const muse::draw::PainterPath path = chordLine->ldata()->path;
    ASSERT_GT(path.elementCount(), 1u);

    MasterScore* reloaded = saveAndReload(score, u"chordLineSmallStaff-test.mscx");
    ASSERT_TRUE(reloaded);
    ChordLine* reloadedLine = noteAt(reloaded, Fraction(3, 1))->chord()->chordLine();
    ASSERT_TRUE(reloadedLine);
    const muse::draw::PainterPath& reloadedPath = reloadedLine->ldata()->path;
    ASSERT_EQ(reloadedPath.elementCount(), path.elementCount());
    for (size_t i = 0; i < path.elementCount(); ++i) {
        EXPECT_NEAR(reloadedPath.elementAt(i).x, path.elementAt(i).x, 1e-3);
        EXPECT_NEAR(reloadedPath.elementAt(i).y, path.elementAt(i).y, 1e-3);
    }

    delete reloaded;
    delete score;
}

//---------------------------------------------------------
//   copiesKeepLetterSpacingAndFixedSlant
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, copiesKeepLetterSpacingAndFixedSlant)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"slurs.mscx");
    ASSERT_TRUE(score);

    StaffText* text = Factory::createStaffText(score->dummy()->segment());
    text->setTextLetterSpacing(0.25);
    StaffText* textCopy = text->clone();
    EXPECT_DOUBLE_EQ(textCopy->textLetterSpacing(), 0.25);

    Tuplet* tuplet = Factory::createTuplet(score->dummy()->measure());
    tuplet->setBracketFixedSlant(true);
    Tuplet* tupletCopy = tuplet->clone();
    EXPECT_TRUE(tupletCopy->bracketFixedSlant());

    delete textCopy;
    delete text;
    delete tupletCopy;
    delete tuplet;
    delete score;
}

//---------------------------------------------------------
//   freezingKeepsTheShapeAtTheFirstEdit
//   Freezing must not move the slur, in normal and multi-Bézier mode.
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, freezingKeepsTheShapeAtTheFirstEdit)
{
    for (bool multiBezier : { false, true }) {
        MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"slurs.mscx");
        ASSERT_TRUE(score);
        SlurSegment* segment = slurSegmentAt(score, Fraction(0, 1));
        ASSERT_TRUE(segment);
        segment->slur()->setMultiBezierEnabled(multiBezier);
        score->doLayout();
        const std::vector<PointF> automatic = slurPathPoints(segment);

        EditData ed;
        segment->startEdit(ed);
        ed.curGrip = Grip::BEZIER1;
        segment->startEditDrag(ed);
        EXPECT_TRUE(segment->isShapeFrozen());
        score->doLayout();
        EXPECT_LT(maxDistance(automatic, slurPathPoints(segment)), 1e-6) << "multiBezier " << multiBezier;

        // Selecting a grip without moving it does not freeze the slur
        segment->endEditDrag(ed);
        EXPECT_FALSE(segment->hasFrozenShape());

        delete score;
    }
}

//---------------------------------------------------------
//   editedSlurIgnoresChangesUnderIt
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, editedSlurIgnoresChangesUnderIt)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"slurs.mscx");
    ASSERT_TRUE(score);
    SlurSegment* segment = slurSegmentAt(score, Fraction(0, 1));
    ASSERT_TRUE(segment);
    Note* middle = noteAt(score, Fraction(1, 4));
    ASSERT_TRUE(middle);
    const double sp = score->style().spatium();

    // An unedited slur still avoids a raised note
    const std::vector<PointF> automatic = slurPathPoints(segment);
    changePitch(score, middle, 79, 15);
    EXPECT_GT(maxDistance(automatic, slurPathPoints(segment)), 0.1 * sp);
    score->undoRedo(true, nullptr);
    EXPECT_LT(maxDistance(automatic, slurPathPoints(segment)), 1e-6);

    dragGrip(score, segment, int(Grip::BEZIER1), PointF(0.0, -0.5 * sp));
    EXPECT_TRUE(segment->isShapeFrozen());
    const std::vector<PointF> edited = slurPathPoints(segment);

    changePitch(score, middle, 79, 15);
    EXPECT_LT(maxDistance(edited, slurPathPoints(segment)), 1e-6);

    // Undoing the edit makes the slur automatic again
    score->undoRedo(true, nullptr);
    score->undoRedo(true, nullptr);
    EXPECT_FALSE(segment->hasFrozenShape());
    EXPECT_LT(maxDistance(automatic, slurPathPoints(segment)), 1e-6);

    delete score;
}

//---------------------------------------------------------
//   frozenSlurSurvivesSaveReload
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, frozenSlurSurvivesSaveReload)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"slurs.mscx");
    ASSERT_TRUE(score);
    SlurSegment* segment = slurSegmentAt(score, Fraction(0, 1));
    ASSERT_TRUE(segment);
    const double sp = score->style().spatium();

    dragGrip(score, segment, int(Grip::BEZIER2), PointF(0.3 * sp, -0.7 * sp));
    changePitch(score, noteAt(score, Fraction(2, 4)), 79, 15);
    const std::vector<PointF> edited = slurPathPoints(segment);

    MasterScore* reloaded = saveAndReload(score, u"frozenSlur-test.mscx");
    ASSERT_TRUE(reloaded);
    SlurSegment* reloadedSegment = slurSegmentAt(reloaded, Fraction(0, 1));
    ASSERT_TRUE(reloadedSegment);
    EXPECT_TRUE(reloadedSegment->isShapeFrozen());
    EXPECT_LT(maxDistance(edited, slurPathPoints(reloadedSegment)), 1e-3 * sp);

    delete reloaded;
    delete score;
}

//---------------------------------------------------------
//   endpointDragUndoRedoIsStable
//   Undo/redo of an end point drag used to add the automatic adjustment again each time.
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, endpointDragUndoRedoIsStable)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"slurs.mscx");
    ASSERT_TRUE(score);
    SlurSegment* segment = slurSegmentAt(score, Fraction(0, 1));
    ASSERT_TRUE(segment);
    const double sp = score->style().spatium();

    dragGrip(score, segment, int(Grip::START), PointF(0.0, -1.0 * sp));
    const PointF startOffset = segment->ups(Grip::START).off;
    const PointF endOffset = segment->ups(Grip::END).off;
    const std::vector<PointF> edited = slurPathPoints(segment);

    for (int i = 0; i < 3; ++i) {
        score->undoRedo(true, nullptr);
        score->undoRedo(false, nullptr);
        EXPECT_LT(std::hypot(segment->ups(Grip::START).off.x() - startOffset.x(),
                             segment->ups(Grip::START).off.y() - startOffset.y()), 1e-9);
        EXPECT_LT(std::hypot(segment->ups(Grip::END).off.x() - endOffset.x(),
                             segment->ups(Grip::END).off.y() - endOffset.y()), 1e-9);
        EXPECT_LT(maxDistance(edited, slurPathPoints(segment)), 1e-6);
    }

    delete score;
}

//---------------------------------------------------------
//   knotDataFollowsStaffSize
//   The knot string is in staff spatium units; a staff size change after the edit used to
//   leave it stale, so the knots rescaled on reopen.
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, knotDataFollowsStaffSize)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"slurs.mscx");
    ASSERT_TRUE(score);
    SlurSegment* segment = slurSegmentAt(score, Fraction(0, 1));
    ASSERT_TRUE(segment);
    segment->slur()->setMultiBezierEnabled(true);
    score->doLayout();
    const double sp = score->style().spatium();

    // The knot grip of the first knot follows the in-handle grip
    dragGrip(score, segment, int(Grip::GRIPS) + 1, PointF(0.0, -0.8 * sp));
    ASSERT_FALSE(segment->multiBezierKnotData().empty());
    const PointF knotOffset = segment->multiBezierKnotData().front().knot.off;
    ASSERT_FALSE(knotOffset.isNull());

    score->staff(0)->staffType(Fraction(0, 1))->setUserMag(0.5);
    score->doLayout();
    EXPECT_EQ(segment->multiBezierKnotData().front().knot.off, knotOffset);

    MasterScore* reloaded = saveAndReload(score, u"knotStaffSize-test.mscx");
    ASSERT_TRUE(reloaded);
    SlurSegment* reloadedSegment = slurSegmentAt(reloaded, Fraction(0, 1));
    ASSERT_TRUE(reloadedSegment && !reloadedSegment->multiBezierKnotData().empty());
    const PointF reloadedOffset = reloadedSegment->multiBezierKnotData().front().knot.off;
    EXPECT_NEAR(reloadedOffset.x(), knotOffset.x(), 1e-6);
    EXPECT_NEAR(reloadedOffset.y(), knotOffset.y(), 1e-6);

    delete reloaded;
    delete score;
}

//---------------------------------------------------------
//   rangeLayoutKeepsShapesInTheNextSystem
//   A range layout reuses the next system unchanged; spanner segments crossing into it
//   must keep their shapes and match a full layout.
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, rangeLayoutKeepsShapesInTheNextSystem)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"slurs.mscx");
    ASSERT_TRUE(score);
    Spanner* slur = spannerAt(score, ElementType::SLUR, Fraction(6, 4));
    Spanner* hairpin = spannerAt(score, ElementType::HAIRPIN, Fraction(6, 4));
    ASSERT_TRUE(slur && hairpin);
    ASSERT_EQ(slur->spannerSegments().size(), 2u);
    ASSERT_EQ(hairpin->spannerSegments().size(), 2u);

    changePitch(score, noteAt(score, Fraction(1, 4)), 76, 18);

    for (Spanner* spanner : { slur, hairpin }) {
        SpannerSegment* second = spanner->backSegment();
        EXPECT_TRUE(second->ldata()->isValid()) << spanner->typeName();
        const RectF rangeRect = second->pageBoundingRect();
        score->doLayout();
        const RectF fullRect = spanner->backSegment()->pageBoundingRect();
        EXPECT_NEAR(rangeRect.x(), fullRect.x(), 1e-3) << spanner->typeName();
        EXPECT_NEAR(rangeRect.y(), fullRect.y(), 1e-3) << spanner->typeName();
        EXPECT_NEAR(rangeRect.width(), fullRect.width(), 1e-3) << spanner->typeName();
        EXPECT_NEAR(rangeRect.height(), fullRect.height(), 1e-3) << spanner->typeName();
    }

    delete score;
}

//---------------------------------------------------------
//   rangeLayoutMatchesFullLayoutForArticulations
//   Collecting a system lays out the next measure in that system to test its width, then gives
//   it back to the next system, which a range layout reuses unchanged; its articulations
//   must stay where that system's own layout put them.
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, rangeLayoutMatchesFullLayoutForArticulations)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"articulations.mscx");
    ASSERT_TRUE(score);

    expectRangeLayoutMatchesFullLayout(score, Fraction(1, 8));
    expectRangeLayoutMatchesFullLayout(score, Fraction(9, 8));

    delete score;
}

//---------------------------------------------------------
//   rangeLayoutKeepsLedgerLineCuts
//   The same for automatically shortened ledger lines, which the width test rebuilds uncut.
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, rangeLayoutKeepsLedgerLineCuts)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"ledgerLines.mscx");
    ASSERT_TRUE(score);
    score->style().set(Sid::ledgerLineAutoCut, true);
    score->doLayout();

    expectRangeLayoutMatchesFullLayout(score, Fraction(1, 16));
    expectRangeLayoutMatchesFullLayout(score, Fraction(17, 16));

    delete score;
}

//---------------------------------------------------------
//   openedLayoutMatchesRelayout
//   On a freshly opened score every measure is still at x = 0 while its articulations are first
//   placed, so they stack up; fermatas and the system height used to follow that stale stack
//   until the next edit, and came back after save and reopen.
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, openedLayoutMatchesRelayout)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"articulations.mscx");
    ASSERT_TRUE(score);

    std::vector<double> systemsY;
    for (const System* system : score->systems()) {
        systemsY.push_back(system->y());
    }
    const std::vector<RectF> opened = chordMarkRects(score);

    score->doLayout();

    ASSERT_EQ(score->systems().size(), systemsY.size());
    for (size_t i = 0; i < systemsY.size(); ++i) {
        EXPECT_NEAR(score->systems().at(i)->y(), systemsY.at(i), 1e-3) << "system " << i;
    }
    const std::vector<RectF> relaid = chordMarkRects(score);
    ASSERT_EQ(opened.size(), relaid.size());
    for (size_t i = 0; i < opened.size(); ++i) {
        EXPECT_NEAR(opened[i].y(), relaid[i].y(), 1e-3) << "mark " << i;
    }

    delete score;
}
