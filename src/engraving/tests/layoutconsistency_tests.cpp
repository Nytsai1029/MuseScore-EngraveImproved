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

#include "dom/arpeggio.h"
#include "dom/articulation.h"
#include "dom/barline.h"
#include "dom/box.h"
#include "dom/chord.h"
#include "dom/chordline.h"
#include "dom/editdata.h"
#include "dom/factory.h"
#include "dom/glissando.h"
#include "dom/guitarbend.h"
#include "dom/hairpin.h"
#include "dom/hammeronpulloff.h"
#include "dom/ledgerline.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/mscore.h"
#include "dom/mscoreview.h"
#include "dom/note.h"
#include "dom/noteline.h"
#include "dom/page.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/slur.h"
#include "dom/staff.h"
#include "dom/stafftext.h"
#include "dom/stafftype.h"
#include "dom/stem.h"
#include "dom/symbol.h"
#include "dom/system.h"
#include "dom/trill.h"
#include "dom/tuplet.h"
#include "dom/undo.h"

#include "compat/engravingcompat.h"
#include "compat/mscxcompat.h"
#include "engravingproject.h"

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

//! A score as the application has it: set up by its project, with the compat steps of
//! NotationProject::doLoad before and after the first layout (its style migration dialog aside)
MasterScore* openLikeTheApp(const String& name)
{
    static std::vector<EngravingProjectPtr> keepAlive;
    EngravingProjectPtr project = EngravingProject::create(nullptr);
    if (!compat::loadMsczOrMscx(project, ScoreRW::rootPath() + u"/" + name, false)) {
        return nullptr;
    }
    MasterScore* score = project->masterScore();
    score->lockUpdates(true);
    if (!project->setupMasterScore(true)) {
        score->lockUpdates(false);
        return nullptr;
    }
    compat::EngravingCompat::doPreLayoutCompatIfNeeded(score);
    score->lockUpdates(false);
    score->setLayoutAll();
    score->update();
    compat::EngravingCompat::doPostLayoutCompatIfNeeded(score);
    keepAlive.push_back(project);
    return score;
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

//---------------------------------------------------------
//   openedLayoutMatchesRelayoutForTupletsOnCrossStaffBeams
//   A tuplet whose notes stay on their staff while its beam reaches the other staff took its number
//   position from the stems as they were before the page set the staff distances. On a freshly opened
//   score those stems are not the ones a later layout starts from, so the number moved at the first edit
//   and came back after save and reopen. The second tuplet starts with a rest, before its beam does.
//   Here the beam alone sets the distance between the staves, and that distance is not the same on a
//   freshly opened score either, so each number is measured from its beam.
//---------------------------------------------------------

namespace {
// The top left corner of each tuplet number, relative to the tip of the first stem under the tuplet
std::vector<PointF> tupletNumbersFromStems(Score* score)
{
    std::vector<const Tuplet*> tuplets;
    for (Segment* s = score->firstMeasure()->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        for (const EngravingItem* item : s->elist()) {
            if (!item || !item->isChordRest()) {
                continue;
            }
            for (const Tuplet* t = toChordRest(item)->tuplet(); t; t = t->tuplet()) {
                if (std::find(tuplets.begin(), tuplets.end(), t) == tuplets.end()) {
                    tuplets.push_back(t);
                }
            }
        }
    }
    std::vector<PointF> offsets;
    for (const Tuplet* t : tuplets) {
        auto firstChord = std::find_if(t->elements().begin(), t->elements().end(),
                                       [](const DurationElement* d) { return d->isChord(); });
        if (!t->number() || firstChord == t->elements().end() || !toChord(*firstChord)->stem()) {
            continue;
        }
        const Chord* chord = toChord(*firstChord);
        const RectF stem = chord->stem()->pageBoundingRect();
        const PointF tip(stem.left(), chord->up() ? stem.top() : stem.bottom());
        offsets.push_back(t->number()->pageBoundingRect().topLeft() - tip);
    }
    return offsets;
}
}

TEST_F(Engraving_LayoutConsistencyTests, openedLayoutMatchesRelayoutForTupletsOnCrossStaffBeams)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"crossStaffBeamTuplet.mscx");
    ASSERT_TRUE(score);

    const std::vector<PointF> opened = tupletNumbersFromStems(score);
    ASSERT_EQ(opened.size(), 2u);

    score->doLayout();
    const std::vector<PointF> relaid = tupletNumbersFromStems(score);
    ASSERT_EQ(relaid.size(), opened.size());
    for (size_t i = 0; i < opened.size(); ++i) {
        EXPECT_NEAR(opened[i].x(), relaid[i].x(), 1e-3) << "tuplet " << i;
        EXPECT_NEAR(opened[i].y(), relaid[i].y(), 1e-3) << "tuplet " << i;
    }

    // Moving the tuplet towards the staff moves its number by just as much, with no correction worked out
    // against the earlier stems
    Segment* tupletStart = score->tick2segment(Fraction(1, 8), true, SegmentType::ChordRest);
    ASSERT_TRUE(tupletStart && tupletStart->element(4) && tupletStart->element(4)->isChordRest());
    Tuplet* tuplet = toChordRest(tupletStart->element(4))->tuplet();
    ASSERT_TRUE(tuplet);
    const double delta = 0.5 * tuplet->spatium();
    score->startCmd(TranslatableString::untranslatable("Move tuplet test"));
    tuplet->undoChangeProperty(Pid::OFFSET, tuplet->offset() + PointF(0.0, delta));
    tuplet->triggerLayout();
    score->endCmd();
    const std::vector<PointF> moved = tupletNumbersFromStems(score);
    ASSERT_EQ(moved.size(), relaid.size());
    EXPECT_NEAR(moved[0].x(), relaid[0].x(), 1e-3);
    EXPECT_NEAR(moved[0].y() - relaid[0].y(), delta, 1e-3);

    // An edit elsewhere in the system lays its range out again
    changePitch(score, noteAt(score, Fraction(1, 1)), 74, 16);
    const std::vector<PointF> edited = tupletNumbersFromStems(score);
    score->doLayout();
    const std::vector<PointF> full = tupletNumbersFromStems(score);
    ASSERT_EQ(edited.size(), full.size());
    for (size_t i = 0; i < edited.size(); ++i) {
        EXPECT_NEAR(edited[i].x(), full[i].x(), 1e-3) << "tuplet " << i << " after edit";
        EXPECT_NEAR(edited[i].y(), full[i].y(), 1e-3) << "tuplet " << i << " after edit";
    }

    delete score;
}

//---------------------------------------------------------
//   openedLayoutMatchesRelayoutForStaffDistanceSetByCrossStaffBeam
//   A beam between the staves can need more room than the skylines leave. When it alone sets the distance
//   between the staves, the system took it from the beam as laid out before the system had its final spacing,
//   which on a freshly opened score was still without room for the beam and at no horizontal positions, so the
//   lower staff sat 0.9 sp higher than after any later layout. With beams across the staves in two bars of the
//   system, even a later layout depended on positions left by the one before it.
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, openedLayoutMatchesRelayoutForStaffDistanceSetByCrossStaffBeam)
{
    for (const char16_t* file : { u"crossStaffBeamTuplet.mscx", u"crossStaffBeamsInTwoMeasures.mscx" }) {
        SCOPED_TRACE(String(file).toStdString());
        MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + file);
        ASSERT_TRUE(score);
        ASSERT_EQ(score->systems().size(), 1u);
        const double sp = score->style().spatium();
        auto lowerStaffY = [score, sp]() { return score->systems().front()->staff(1)->y() / sp; };

        const double opened = lowerStaffY();

        score->doLayout();
        const double relaid = lowerStaffY();
        EXPECT_NEAR(opened, relaid, 1e-3);

        score->doLayout();
        EXPECT_NEAR(lowerStaffY(), relaid, 1e-3);

        // An edit elsewhere in the system lays its range out again
        changePitch(score, noteAt(score, Fraction(1, 1)), 74, 16);
        const double edited = lowerStaffY();
        score->doLayout();
        EXPECT_NEAR(edited, lowerStaffY(), 1e-3);

        delete score;
    }
}

//---------------------------------------------------------
//   spannerToTheEndOfTheScoreSurvivesUnderAMultimeasureRest
//   A spanner which runs to the last tick of the score has its end written at the last segment of the
//   last measure. When that measure is under a multimeasure rest no segment at the end tick is
//   visited, so the end tag was lost and the volta disappeared on reopen.
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, spannerToTheEndOfTheScoreSurvivesUnderAMultimeasureRest)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"voltaToTheEndUnderMMRest.mscz");
    ASSERT_TRUE(score);

    const Fraction voltaTick(1, 1);                // measure 2
    const Fraction scoreEnd(6, 1);
    EXPECT_EQ(score->endTick(), scoreEnd);
    EXPECT_TRUE(score->style().styleB(Sid::createMultiMeasureRests));

    auto voltaRunsToTheEnd = [voltaTick, scoreEnd](Score* s) {
        Spanner* volta = spannerAt(s, ElementType::VOLTA, voltaTick);
        return volta && volta->tick2() == scoreEnd;
    };

    EXPECT_TRUE(voltaRunsToTheEnd(score));

    Measure* m3 = score->crMeasure(2);
    ASSERT_TRUE(m3 && m3->mmRest() && m3->mmRest()->isMMRest());
    EXPECT_EQ(m3->mmRest()->mmRestCount(), 4);      // measures 3-6 under one multimeasure rest

    // Why the end was lost: under the rest the last measure has no segment at the end of the score,
    // so nothing which is written visits the tick where the volta ends
    Measure* lastMeasure = score->lastMeasure();
    ASSERT_TRUE(lastMeasure && lastMeasure->last());
    EXPECT_LT(lastMeasure->last()->tick(), scoreEnd);

    MasterScore* reloaded = saveAndReload(score, u"voltaMMRest-test.mscx");
    ASSERT_TRUE(reloaded);
    EXPECT_TRUE(voltaRunsToTheEnd(reloaded));
    delete reloaded;

    // Control: without a multimeasure rest the end is written (before and after the fix)
    score->style().set(Sid::createMultiMeasureRests, false);
    score->doLayout();

    MasterScore* controlReloaded = saveAndReload(score, u"voltaMMRest-control-test.mscx");
    ASSERT_TRUE(controlReloaded);
    EXPECT_TRUE(voltaRunsToTheEnd(controlReloaded));
    delete controlReloaded;

    delete score;
}

//---------------------------------------------------------
//   arpeggioIsHiddenOnTheStandardStaffWhenTheTablatureShowsIt
//   Sid::arpeggioHiddenInStdIfTab: layoutArpeggio set isSkipDraw(true) and then unconditionally
//   back to false, so the option never did anything. A hidden arpeggio is not drawn, takes no
//   room in the chord shape and must not be selectable.
//---------------------------------------------------------

TEST_F(Engraving_LayoutConsistencyTests, arpeggioIsHiddenOnTheStandardStaffWhenTheTablatureShowsIt)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"arpeggioWithLinkedTab.mscx");
    ASSERT_TRUE(score);
    ASSERT_EQ(score->staves().size(), 2u);
    ASSERT_TRUE(score->staff(0)->isPitchedStaff(Fraction(0, 1)));
    ASSERT_TRUE(score->staff(1)->isTabStaff(Fraction(0, 1)));

    Measure* m = score->firstMeasure();
    ASSERT_TRUE(m);
    Segment* firstChordRest = m->first(SegmentType::ChordRest);
    ASSERT_TRUE(firstChordRest);
    Segment* secondChordRest = firstChordRest->next(SegmentType::ChordRest);
    ASSERT_TRUE(secondChordRest);

    Chord* stdChordWithArp = toChord(firstChordRest->element(0));
    Chord* stdChordWithout = toChord(secondChordRest->element(0));
    Chord* tabChordWithArp = toChord(firstChordRest->element(4));
    ASSERT_TRUE(stdChordWithArp && stdChordWithout && tabChordWithArp);
    Arpeggio* stdArp = stdChordWithArp->arpeggio();
    Arpeggio* tabArp = tabChordWithArp->arpeggio();
    ASSERT_TRUE(stdArp && tabArp);
    ASSERT_FALSE(stdChordWithout->arpeggio());

    // Option off: both arpeggios are drawn
    EXPECT_FALSE(stdArp->ldata()->isSkipDraw());
    EXPECT_FALSE(tabArp->ldata()->isSkipDraw());
    //! NOTE Shape::left() is the distance the shape protrudes left of the chord's origin,
    //! so it is bigger when the arpeggio takes room there
    const double leftWithArpeggio = stdChordWithArp->shape().left();
    EXPECT_GT(leftWithArpeggio, stdChordWithout->shape().left());

    // Option on: the standard-staff arpeggio is skipped, the tablature's is not
    score->style().set(Sid::arpeggioHiddenInStdIfTab, true);
    score->doLayout();

    EXPECT_TRUE(stdArp->ldata()->isSkipDraw());
    EXPECT_FALSE(stdArp->selectable());
    EXPECT_FALSE(tabArp->ldata()->isSkipDraw());
    EXPECT_TRUE(tabArp->selectable());

    // No room: the chord's shape is the same shape the spacing uses (chordlayout.cpp adds the
    // arpeggio's shape to it when it addToSkyline()), so it protrudes left no more than the
    // shape of a chord without an arpeggio now
    EXPECT_LT(stdChordWithArp->shape().left(), leftWithArpeggio);
    EXPECT_NEAR(stdChordWithArp->shape().left(), stdChordWithout->shape().left(), 1e-5);

    // Tablature staff hidden: the arpeggio comes back on the standard staff
    score->staff(1)->setVisible(false);
    score->doLayout();

    EXPECT_FALSE(stdArp->ldata()->isSkipDraw());
    EXPECT_TRUE(stdArp->selectable());
    EXPECT_NEAR(stdChordWithArp->shape().left(), leftWithArpeggio, 1e-5);

    delete score;
}

//! What an old file says about a glissando — its kind of line, whether it shows its text — is the glissando's own,
//! also where the reader sets it without reading a value (no text written: none shown; a straight line without
//! text on tablature). A styled value is not written, so a save would hand it back to the style: a wavy
//! glissando came back straight, one without text came back with it.
TEST_F(Engraving_LayoutConsistencyTests, glissandoOfAnOldFileKeepsItsLineAndText)
{
    auto glissandos = [](Score* s) {
        std::vector<std::pair<GlissandoType, bool> > result;
        s->scanElements(&result, [](void* d, EngravingItem* e) {
            if (!e->isNote()) {
                return;
            }
            for (Spanner* spanner : toNote(e)->spannerFor()) {
                if (spanner->isGlissando()) {
                    const Glissando* glissando = toGlissando(spanner);
                    static_cast<std::vector<std::pair<GlissandoType, bool> >*>(d)->emplace_back(
                        glissando->glissandoType(), glissando->showText());
                }
            }
        });
        return result;
    };

    bool anyWavy = false;
    bool anyWithoutText = false;
    for (const String& fixture : { LAYOUTCONSISTENCY_DATA_DIR + u"glissandos-400.mscx",
                                   LAYOUTCONSISTENCY_DATA_DIR + u"glissandos-420.mscx",
                                   String(u"../../../vtest/scores/tablature-1.mscx") }) {
        MasterScore* score = openLikeTheApp(fixture);
        ASSERT_TRUE(score) << fixture.toStdString();
        const auto opened = glissandos(score);
        ASSERT_FALSE(opened.empty()) << fixture.toStdString();
        for (const auto& [type, showText] : opened) {
            anyWavy |= type == GlissandoType::WAVY;
            anyWithoutText |= !showText;
        }

        MasterScore* reloaded = saveAndReload(score, u"oldGlissandos.mscx");
        ASSERT_TRUE(reloaded) << fixture.toStdString();
        EXPECT_EQ(glissandos(reloaded), opened) << fixture.toStdString();
    }
    EXPECT_TRUE(anyWavy);
    EXPECT_TRUE(anyWithoutText);
}

//! A frame of a file before 4.6 keeps the distances it had: no padding to the music (there was none), and the
//! two gaps of frames which follow each other added up in the first one's bottom gap (as they were laid out)
TEST_F(Engraving_LayoutConsistencyTests, framesOfAnOldFileKeepTheirDistances)
{
    auto distances = [](Score* s) {
        std::vector<std::tuple<double, double, double> > result;
        for (MeasureBase* mb = s->first(); mb; mb = mb->next()) {
            if (mb->isVBox()) {
                const VBox* box = toVBox(mb);
                result.emplace_back(box->getProperty(Pid::PADDING_TO_NOTATION_ABOVE).value<Spatium>().val(),
                                    box->getProperty(Pid::PADDING_TO_NOTATION_BELOW).value<Spatium>().val(),
                                    box->bottomGap().val());
            }
        }
        return result;
    };

    MasterScore* score = openLikeTheApp(LAYOUTCONSISTENCY_DATA_DIR + u"framesAboveTheMusic-440.mscx");
    ASSERT_TRUE(score);
    const auto opened = distances(score);
    ASSERT_EQ(opened.size(), 2u);
    EXPECT_EQ(std::get<0>(opened[0]), 0.0);
    EXPECT_EQ(std::get<1>(opened[0]), 0.0);
    EXPECT_DOUBLE_EQ(std::get<2>(opened[0]),
                     (score->style().styleS(Sid::frameSystemDistance) + score->style().styleS(Sid::systemFrameDistance)).val());

    MasterScore* reloaded = saveAndReload(score, u"oldFrames.mscx");
    ASSERT_TRUE(reloaded);
    EXPECT_EQ(distances(reloaded), opened);
}

//! A segment's end offset — what a drag of the end grip changes, leaving the segment's position styled — is
//! written also when nothing else about its line is user-modified (the writer used to leave the whole Segment
//! block out then, so the drag was lost on save)
TEST_F(Engraving_LayoutConsistencyTests, spannerSegmentEndOffsetSurvivesSaveReload)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"twoStaves.mscx");
    ASSERT_TRUE(score);

    Trill* trill = Factory::createTrill(score->dummy());
    trill->setTrillType(TrillType::TRILL_LINE);
    trill->setTrack(0);
    trill->setTrack2(0);
    trill->setTick(Fraction(0, 1));
    trill->setTick2(Fraction(1, 1));
    score->addSpanner(trill);
    score->setLayoutAll();
    score->update();

    ASSERT_FALSE(trill->isUserModified());
    ASSERT_FALSE(trill->segmentsEmpty());
    const PointF off2(2.5 * trill->spatium(), 0.0);
    trill->frontSegment()->setUserOff2(off2);

    MasterScore* reloaded = saveAndReload(score, u"trillOff2.mscx");
    ASSERT_TRUE(reloaded);

    Spanner* reloadedTrill = spannerAt(reloaded, ElementType::TRILL, Fraction(0, 1));
    ASSERT_TRUE(reloadedTrill);
    ASSERT_FALSE(reloadedTrill->segmentsEmpty());
    EXPECT_EQ(toLineSegment(reloadedTrill->frontSegment())->userOff2(), off2);
}

//! A note line keeps where it ends. The note-anchored text lines of an older file become note lines which end at the
//! note's left edge, as they were laid out; the writer never wrote that, so after a save they ended at the offset end
//! (the default). The conversion did not give them the track they end on either.
TEST_F(Engraving_LayoutConsistencyTests, noteLineKeepsWhereItEnds)
{
    auto noteLines = [](Score* s) {
        std::vector<std::tuple<Fraction, NoteLineEndPlacement, track_idx_t> > result;
        s->scanElements(&result, [](void* d, EngravingItem* e) {
            if (!e->isNote()) {
                return;
            }
            for (Spanner* spanner : toNote(e)->spannerFor()) {
                if (spanner->isNoteLine()) {
                    static_cast<std::vector<std::tuple<Fraction, NoteLineEndPlacement, track_idx_t> >*>(d)->emplace_back(
                        spanner->tick(), toNoteLine(spanner)->lineEndPlacement(), spanner->track2());
                }
            }
        });
        return result;
    };

    MasterScore* score = openLikeTheApp(LAYOUTCONSISTENCY_DATA_DIR + u"note-line.mscz");
    ASSERT_TRUE(score);
    const auto opened = noteLines(score);
    ASSERT_FALSE(opened.empty());
    bool anyAtTheLeftEdge = false;
    for (const auto& [tick, placement, track2] : opened) {
        anyAtTheLeftEdge |= placement == NoteLineEndPlacement::LEFT_EDGE;
        EXPECT_NE(track2, muse::nidx) << tick.toString().toStdString();
    }
    EXPECT_TRUE(anyAtTheLeftEdge);

    MasterScore* reloaded = saveAndReload(score, u"noteLines.mscx");
    ASSERT_TRUE(reloaded);
    EXPECT_EQ(noteLines(reloaded), opened);
}

//! What an item owns stands on its track: the parentheses of a note (here those of a trill's cue note, which
//! are made while the note is read, before its chord gets its track) and the accidentals which an ornament
//! shows above and below it (a 4.0 file gives them a track of their own). Else they are on another voice
//! than their note, and another one again after a save and a reopen.
TEST_F(Engraving_LayoutConsistencyTests, ornamentPartsStandOnTheTrackOfTheirItem)
{
    struct Owned {
        size_t parentheses = 0;
        size_t ornamentAccidentals = 0;
        std::vector<std::string> offTrack;
    };
    auto check = [](Score* s) {
        Owned owned;
        s->scanElements(&owned, [](void* d, EngravingItem* e) {
            Owned* o = static_cast<Owned*>(d);
            const EngravingItem* owner = e->parentItem();
            if (e->isParenthesis()) {
                ++o->parentheses;
            } else if (e->isAccidental() && owner && owner->isOrnament()) {
                ++o->ornamentAccidentals;
            } else {
                return;
            }
            if (e->track() != owner->track()) {
                o->offTrack.push_back(e->typeName() + std::string(" at ") + e->tick().toString().toStdString());
            }
        });
        return owned;
    };

    MasterScore* score = openLikeTheApp(LAYOUTCONSISTENCY_DATA_DIR + u"ornament-2.mscz");
    ASSERT_TRUE(score);
    const Owned opened = check(score);
    ASSERT_GE(opened.parentheses, 2u);
    ASSERT_GE(opened.ornamentAccidentals, 1u);
    EXPECT_TRUE(opened.offTrack.empty()) << ::testing::PrintToString(opened.offTrack);

    MasterScore* reloaded = saveAndReload(score, u"ornamentTracks.mscx");
    ASSERT_TRUE(reloaded);
    const Owned reopened = check(reloaded);
    EXPECT_EQ(reopened.parentheses, opened.parentheses);
    EXPECT_EQ(reopened.ornamentAccidentals, opened.ornamentAccidentals);
    EXPECT_TRUE(reopened.offTrack.empty()) << ::testing::PrintToString(reopened.offTrack);
}

//! An empty bend text lays out to the same (empty) box in every pass, also in the first pass of a segment
//! which the layout makes itself (as for a new bend, or any bend of a reopened file)
TEST_F(Engraving_LayoutConsistencyTests, emptyBendTextLaysOutToAnEmptyBox)
{
    MasterScore* score = ScoreRW::readScore(LAYOUTCONSISTENCY_DATA_DIR + u"twoStaves.mscx");
    ASSERT_TRUE(score);
    Note* note = noteAt(score, Fraction(0, 1));
    Note* endNote = noteAt(score, Fraction(1, 1));
    ASSERT_TRUE(note && endNote);

    score->startCmd(TranslatableString::untranslatable("Add bend test"));
    GuitarBend* bend = score->addGuitarBend(GuitarBendType::BEND, note, endNote);
    score->endCmd();
    ASSERT_TRUE(bend);
    ASSERT_FALSE(bend->segmentsEmpty());

    const GuitarBendText* text = toGuitarBendSegment(bend->frontSegment())->bendText();
    ASSERT_TRUE(text->empty());
    const RectF firstPass = text->ldata()->bbox();
    EXPECT_TRUE(firstPass.isNull());

    score->setLayoutAll();
    score->update();
    EXPECT_EQ(toGuitarBendSegment(bend->frontSegment())->bendText()->ldata()->bbox(), firstPass);
}
