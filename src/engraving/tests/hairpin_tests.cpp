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

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "dom/anchors.h"
#include "dom/articulation.h"
#include "dom/chord.h"
#include "dom/chordrest.h"
#include "dom/dynamic.h"
#include "dom/editdata.h"
#include "dom/expression.h"
#include "dom/factory.h"
#include "dom/fermata.h"
#include "dom/fingering.h"
#include "dom/hairpin.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/noteval.h"
#include "dom/ottava.h"
#include "dom/page.h"
#include "dom/segment.h"
#include "dom/slur.h"
#include "dom/stafftext.h"
#include "dom/stem.h"
#include "dom/system.h"
#include "dom/text.h"
#include "dom/textbase.h"

#include "types/symid.h"

#include "engraving/compat/scoreaccess.h"
#include "infrastructure/shape.h"
#include "types/translatablestring.h"
#include "utils/scorerw.h"

using namespace mu;
using namespace mu::engraving;

class Engraving_HairpinTests : public ::testing::Test
{
};

TEST_F(Engraving_HairpinTests, hairpin)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    Hairpin* hp = new Hairpin(score->dummy()->segment());

    // subtype
    hp->setHairpinType(HairpinType::DIM_HAIRPIN);
    Hairpin* hp2 = static_cast<Hairpin*>(ScoreRW::writeReadElement(hp));
    EXPECT_EQ(hp2->hairpinType(), HairpinType::DIM_HAIRPIN);
    delete hp2;

    hp->setHairpinType(HairpinType::CRESC_HAIRPIN);
    hp2 = static_cast<Hairpin*>(ScoreRW::writeReadElement(hp));
    EXPECT_EQ(hp2->hairpinType(), HairpinType::CRESC_HAIRPIN);
    delete hp2;
}

static PointF canvasOriginFromPageGrip(const EngravingItem* item, Grip grip)
{
    const std::vector<PointF> grips = item->gripsPositions();
    const PointF gripPage = grips.at(size_t(int(grip)));
    return gripPage - item->pagePos() + item->canvasPos();
}

TEST_F(Engraving_HairpinTests, hairpinGripAlignmentGuidesUseDragPoint)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ChordRest* cr1 = score->firstSegment(SegmentType::ChordRest)->nextChordRest(0);
    ASSERT_TRUE(cr1);

    Hairpin* hp = score->addHairpin(HairpinType::CRESC_HAIRPIN, cr1);
    score->doLayout();
    ASSERT_TRUE(hp);
    ASSERT_FALSE(hp->segmentsEmpty());

    HairpinSegment* seg = toHairpinSegment(hp->frontSegment());
    ASSERT_TRUE(seg);

    const std::vector<LineF> startGuides = seg->gripAlignmentGuideLines(Grip::START);
    ASSERT_EQ(startGuides.size(), 2);
    const PointF startOrigin = canvasOriginFromPageGrip(seg, Grip::START);
    EXPECT_NEAR(startGuides.at(0).y1(), startOrigin.y(), 1e-6);
    EXPECT_NEAR(startGuides.at(1).x1(), startOrigin.x(), 1e-6);

    const std::vector<LineF> endGuides = seg->gripAlignmentGuideLines(Grip::END);
    ASSERT_EQ(endGuides.size(), 2);
    const PointF endOrigin = canvasOriginFromPageGrip(seg, Grip::END);
    EXPECT_NEAR(endGuides.at(0).y1(), endOrigin.y(), 1e-6);
    EXPECT_NEAR(endGuides.at(1).x1(), endOrigin.x(), 1e-6);
}

TEST_F(Engraving_HairpinTests, crescLineAlignmentGuidesUseTextOrigin)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ChordRest* cr1 = score->firstSegment(SegmentType::ChordRest)->nextChordRest(0);
    ASSERT_TRUE(cr1);

    Hairpin* hp = score->addHairpin(HairpinType::CRESC_LINE, cr1);
    score->doLayout();
    ASSERT_TRUE(hp);
    ASSERT_FALSE(hp->segmentsEmpty());

    HairpinSegment* seg = toHairpinSegment(hp->frontSegment());
    ASSERT_TRUE(seg);
    ASSERT_TRUE(seg->text());
    ASSERT_FALSE(seg->text()->empty());

    const std::vector<LineF> guides = seg->gripAlignmentGuideLines(Grip::MIDDLE);
    ASSERT_EQ(guides.size(), 2);

    PointF localOrigin;
    ASSERT_TRUE(seg->text()->dragReferenceOrigin(localOrigin));
    const PointF origin = seg->text()->canvasPos() + localOrigin;
    EXPECT_NEAR(guides.at(0).y1(), origin.y(), 1e-6);
    EXPECT_NEAR(guides.at(1).x1(), origin.x(), 1e-6);
}

namespace {
int countTimeTickSegments(const Score* score)
{
    int count = 0;
    for (const Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (const Segment* segment = measure->first(SegmentType::TimeTick); segment;
             segment = segment->next(SegmentType::TimeTick)) {
            ++count;
        }
    }
    return count;
}

Segment* findTimeTickSegment(Score* score, const Fraction& tick)
{
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(SegmentType::TimeTick); segment;
             segment = segment->next(SegmentType::TimeTick)) {
            if (segment->tick() == tick) {
                return segment;
            }
        }
    }
    return nullptr;
}

Segment* firstTimeTickSegment(Score* score)
{
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        if (Segment* segment = measure->first(SegmentType::TimeTick)) {
            return segment;
        }
    }
    return nullptr;
}

std::vector<double> chordRestXs(const Score* score)
{
    std::vector<double> xs;
    const Measure* measure = score->firstMeasure();
    if (!measure) {
        return xs;
    }
    for (const Segment* segment = measure->first(SegmentType::ChordRest); segment;
         segment = segment->next(SegmentType::ChordRest)) {
        xs.push_back(segment->x());
    }
    return xs;
}

std::vector<std::string> staffShapeTypeNames(const Segment* segment, staff_idx_t staffIdx)
{
    std::vector<std::string> names;
    for (const ShapeElement& shapeEl : segment->staffShape(staffIdx).elements()) {
        if (shapeEl.item()) {
            names.emplace_back(shapeEl.item()->typeName());
        }
    }
    std::sort(names.begin(), names.end());
    return names;
}

std::string joinNames(const std::vector<std::string>& names)
{
    std::ostringstream out;
    for (size_t i = 0; i < names.size(); ++i) {
        if (i) {
            out << ',';
        }
        out << names[i];
    }
    return out.str();
}

Slur* firstSlur(Score* score)
{
    for (auto& pair : score->spanner()) {
        if (pair.second->isSlur()) {
            return toSlur(pair.second);
        }
    }
    return nullptr;
}

struct SlurGeometrySnapshot {
    PointF start;
    PointF end;
    PointF bezier1;
    PointF bezier2;
};

SlurGeometrySnapshot captureSlurGeometry(const Slur* slur)
{
    const SlurSegment* slurSeg = slur->frontSegment();
    SlurGeometrySnapshot snapshot;
    snapshot.start = slurSeg->ups(Grip::START).p;
    snapshot.end = slurSeg->ups(Grip::END).p;
    snapshot.bezier1 = slurSeg->ups(Grip::BEZIER1).p;
    snapshot.bezier2 = slurSeg->ups(Grip::BEZIER2).p;
    return snapshot;
}

void expectPointNear(const PointF& actual, const PointF& expected, double epsilon, const char* label)
{
    EXPECT_NEAR(actual.x(), expected.x(), epsilon) << label << ".x";
    EXPECT_NEAR(actual.y(), expected.y(), epsilon) << label << ".y";
}

void expectXsNear(const std::vector<double>& actual, const std::vector<double>& expected, double epsilon)
{
    ASSERT_EQ(actual.size(), expected.size());
    for (size_t i = 0; i < actual.size(); ++i) {
        EXPECT_NEAR(actual[i], expected[i], epsilon) << "ChordRest x[" << i << "]";
    }
}

Hairpin* addHairpinOverFirstMeasure(MasterScore* score)
{
    Measure* measure = score->firstMeasure();
    if (!measure) {
        return nullptr;
    }
    Segment* firstCR = measure->first(SegmentType::ChordRest);
    Segment* lastCR = measure->last(SegmentType::ChordRest);
    if (!firstCR || !lastCR) {
        return nullptr;
    }
    ChordRest* cr1 = toChordRest(firstCR->element(0));
    ChordRest* cr2 = toChordRest(lastCR->element(0));
    if (!cr1 || !cr2) {
        return nullptr;
    }
    return score->addHairpin(HairpinType::CRESC_HAIRPIN, cr1, cr2);
}

std::string timeTickTicksDebug(const Score* score)
{
    std::ostringstream out;
    for (const Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (const Segment* segment = measure->first(SegmentType::TimeTick); segment;
             segment = segment->next(SegmentType::TimeTick)) {
            out << segment->tick().numerator() << '/' << segment->tick().denominator() << ' ';
        }
    }
    return out.str();
}

Ottava* addOttavaOnStaff2ToBeatTwo(MasterScore* score)
{
    Measure* measure = score->firstMeasure();
    if (!measure) {
        return nullptr;
    }

    // Staff 2 is a full-measure rest, so 2/4 has no ChordRest there. Create the same
    // TimeTick Spanner::endSegment() would insert, then attach the ottava to it.
    EditTimeTickAnchors::createTimeTickAnchor(measure, Fraction(1, 2), 1);
    EditTimeTickAnchors::updateLayout(measure);

    Ottava* ottava = Factory::createOttava(score->dummy());
    ottava->setOttavaType(OttavaType::OTTAVA_8VA);
    ottava->setTrack(staff2track(1));
    ottava->setTrack2(staff2track(1));
    ottava->setTick(Fraction(0, 1));
    ottava->setTick2(Fraction(1, 2));
    score->addSpanner(ottava);
    return ottava;
}

int countTimeTickAnchorsInPageBsp(Page* page)
{
    int count = 0;
    if (!page) {
        return count;
    }
    for (EngravingItem* item : page->items(page->pageBoundingRect())) {
        if (item && item->isTimeTickAnchor()) {
            ++count;
        }
    }
    return count;
}

Expression* addExpressionOnFirstChord(MasterScore* score)
{
    Measure* measure = score->firstMeasure();
    if (!measure) {
        return nullptr;
    }
    Segment* firstCR = measure->first(SegmentType::ChordRest);
    if (!firstCR) {
        return nullptr;
    }
    Expression* expression = Factory::createExpression(firstCR, true);
    expression->setTrack(0);
    expression->setXmlText(u"dolce");
    firstCR->add(expression);
    return expression;
}

StaffText* addStaffTextOnFirstChord(MasterScore* score)
{
    Measure* measure = score->firstMeasure();
    if (!measure) {
        return nullptr;
    }
    Segment* firstCR = measure->first(SegmentType::ChordRest);
    if (!firstCR) {
        return nullptr;
    }
    StaffText* text = Factory::createStaffText(firstCR, TextStyleType::STAFF, true);
    text->setTrack(0);
    text->setXmlText(u"solo");
    firstCR->add(text);
    return text;
}

Dynamic* addDynamicOnFirstChord(MasterScore* score)
{
    Measure* measure = score->firstMeasure();
    if (!measure) {
        return nullptr;
    }
    Segment* firstCR = measure->first(SegmentType::ChordRest);
    if (!firstCR) {
        return nullptr;
    }
    Dynamic* dynamic = Factory::createDynamic(firstCR, true);
    dynamic->setTrack(0);
    dynamic->setDynamicType(u"f");
    firstCR->add(dynamic);
    return dynamic;
}

struct HairpinAnchorSnapshot {
    PointF posRelToStartCR;
    PointF pos2;
    PointF startGripRel;
    PointF endGripRel;
};

HairpinAnchorSnapshot captureHairpinAnchorsRel(const HairpinSegment* hairpinSeg, const EngravingItem* startCR)
{
    HairpinAnchorSnapshot snapshot;
    const PointF origin = startCR->canvasPos();
    snapshot.posRelToStartCR = hairpinSeg->canvasPos() - origin;
    snapshot.pos2 = hairpinSeg->pos2();
    snapshot.startGripRel = canvasOriginFromPageGrip(hairpinSeg, Grip::START) - origin;
    snapshot.endGripRel = canvasOriginFromPageGrip(hairpinSeg, Grip::END) - origin;
    return snapshot;
}

Articulation* addChordArticulation(Chord* chord, SymId id)
{
    Articulation* articulation = Factory::createArticulation(chord);
    articulation->setSymId(id);
    articulation->setParent(chord);
    articulation->setTrack(chord->track());
    chord->add(articulation);
    return articulation;
}

Fermata* addFermataOnChord(Chord* chord)
{
    Segment* segment = chord->segment();
    Fermata* fermata = Factory::createFermata(segment);
    fermata->setTrack(chord->track());
    fermata->setSymId(SymId::fermataAbove);
    fermata->setPlacement(PlacementV::ABOVE);
    segment->add(fermata);
    return fermata;
}

Fingering* addFingeringOnNote(Note* note, const String& text)
{
    Fingering* fingering = Factory::createFingering(note);
    fingering->setXmlText(text);
    note->add(fingering);
    return fingering;
}

struct NextMeasureMarksSnapshot {
    PointF staccatoRel;
    PointF tenutoRel;
    PointF accentRel;
    PointF marcatoRel;
    PointF fermataRel;
    PointF fingeringRel;
    double stemLength = 0.0;
    PointF notePos;
    PointF noteCanvas;
};

NextMeasureMarksSnapshot captureNextMeasureMarks(const Articulation* staccato, const Articulation* tenuto,
                                                 const Articulation* accent, const Articulation* marcato,
                                                 const Fermata* fermata, const Fingering* fingering,
                                                 const Chord* startChord, const Chord* endChord)
{
    NextMeasureMarksSnapshot snapshot;
    const Note* startNote = startChord->upNote();
    const Note* endNote = endChord->upNote();
    const PointF startOrigin = startNote->canvasPos();
    const PointF endOrigin = endNote->canvasPos();
    snapshot.staccatoRel = staccato->canvasPos() - startOrigin;
    snapshot.tenutoRel = tenuto->canvasPos() - startOrigin;
    snapshot.fingeringRel = fingering->canvasPos() - startOrigin;
    snapshot.accentRel = accent->canvasPos() - endOrigin;
    snapshot.marcatoRel = marcato->canvasPos() - endOrigin;
    snapshot.fermataRel = fermata->canvasPos() - endOrigin;
    snapshot.notePos = startNote->pos();
    snapshot.noteCanvas = startOrigin;
    if (const Stem* stem = startChord->stem()) {
        snapshot.stemLength = stem->length();
    }
    return snapshot;
}

void expectNextMeasureMarksStable(const NextMeasureMarksSnapshot& after, const NextMeasureMarksSnapshot& before)
{
    expectPointNear(after.staccatoRel, before.staccatoRel, 1e-4, "next-measure staccato");
    expectPointNear(after.tenutoRel, before.tenutoRel, 1e-4, "next-measure tenuto");
    expectPointNear(after.accentRel, before.accentRel, 1e-4, "next-measure accent");
    expectPointNear(after.marcatoRel, before.marcatoRel, 1e-4, "next-measure marcato");
    expectPointNear(after.fermataRel, before.fermataRel, 1e-4, "next-measure fermata");
    expectPointNear(after.fingeringRel, before.fingeringRel, 1e-4, "next-measure fingering");
    EXPECT_NEAR(after.stemLength, before.stemLength, 1e-4)
        << "next-measure stem length (note canvas y before=" << before.noteCanvas.y()
        << " after=" << after.noteCanvas.y() << ")";
    expectPointNear(after.notePos, before.notePos, 1e-4, "next-measure note pos");
}

Measure* appendMeasureWithTwoQuarters(MasterScore* score)
{
    score->startCmd(TranslatableString::untranslatable("Hairpin tests"));
    score->appendMeasures(1);
    score->endCmd();

    Measure* measure = score->lastMeasure();
    if (!measure) {
        return nullptr;
    }

    Segment* firstCR = measure->first(SegmentType::ChordRest);
    if (!firstCR) {
        return nullptr;
    }

    score->startCmd(TranslatableString::untranslatable("Hairpin tests"));
    score->setNoteRest(firstCR, 0, NoteVal(64), Fraction(1, 4));
    Segment* secondCR = firstCR->next(SegmentType::ChordRest);
    if (!secondCR) {
        secondCR = measure->findSegmentR(SegmentType::ChordRest, Fraction(1, 4));
    }
    if (secondCR) {
        score->setNoteRest(secondCR, 0, NoteVal(65), Fraction(1, 4));
    }
    score->endCmd();
    return measure;
}

void prepareElementEditData(EditData& ed, EngravingItem* item)
{
    auto eed = std::make_shared<ElementEditData>();
    eed->e = item;
    ed.addData(eed);
}

void expectStableSlurAfterOffsetOnlyMove(MasterScore* score, const Slur* slur, const Segment* firstCR,
                                         int timeTicksBefore, const std::vector<double>& noteXsBefore,
                                         const SlurGeometrySnapshot& slurBefore,
                                         const std::vector<std::string>& staffShapeBefore)
{
    const int timeTicksAfter = countTimeTickSegments(score);
    const std::vector<double> noteXsAfter = chordRestXs(score);
    const SlurGeometrySnapshot slurAfter = captureSlurGeometry(slur);
    const std::vector<std::string> staffShapeAfter = staffShapeTypeNames(firstCR, 0);

    EXPECT_EQ(timeTicksAfter, timeTicksBefore)
        << "TimeTick count before=" << timeTicksBefore << " after=" << timeTicksAfter;
    expectXsNear(noteXsAfter, noteXsBefore, 1e-4);
    expectPointNear(slurAfter.start, slurBefore.start, 1e-4, "slur START");
    expectPointNear(slurAfter.end, slurBefore.end, 1e-4, "slur END");
    expectPointNear(slurAfter.bezier1, slurBefore.bezier1, 1e-4, "slur BEZIER1");
    expectPointNear(slurAfter.bezier2, slurBefore.bezier2, 1e-4, "slur BEZIER2");
    EXPECT_EQ(staffShapeAfter, staffShapeBefore)
        << "staffShape before=[" << joinNames(staffShapeBefore) << "] after=["
        << joinNames(staffShapeAfter) << "]";
}
}

TEST_F(Engraving_HairpinTests, verticalHairpinDragKeepsSlurGeometry)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ASSERT_TRUE(score);

    Hairpin* hp = addHairpinOverFirstMeasure(score);
    ASSERT_TRUE(hp);
    score->doLayout();
    ASSERT_FALSE(hp->segmentsEmpty());

    Slur* slur = firstSlur(score);
    ASSERT_TRUE(slur);
    ASSERT_FALSE(slur->segmentsEmpty());

    HairpinSegment* hairpinSeg = toHairpinSegment(hp->frontSegment());
    ASSERT_TRUE(hairpinSeg);

    const int timeTicksBefore = countTimeTickSegments(score);
    const std::vector<double> noteXsBefore = chordRestXs(score);
    const SlurGeometrySnapshot slurBefore = captureSlurGeometry(slur);
    const Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);
    const Segment* firstCR = firstMeasure->first(SegmentType::ChordRest);
    ASSERT_TRUE(firstCR);
    const std::vector<std::string> staffShapeBefore = staffShapeTypeNames(firstCR, 0);

    EditData ed;
    ed.curGrip = Grip::MIDDLE;
    ed.evtDelta = PointF(0.0, hairpinSeg->spatium());
    prepareElementEditData(ed, hairpinSeg);
    static_cast<EngravingItem*>(hairpinSeg)->startEditDrag(ed);
    static_cast<EngravingItem*>(hairpinSeg)->editDrag(ed);
    score->doLayout();

    const int timeTicksAfterDrag = countTimeTickSegments(score);
    EXPECT_EQ(timeTicksAfterDrag, timeTicksBefore)
        << "startEditDrag + vertical middle drag must not insert a TimeTick grid"
        << " (before=" << timeTicksBefore << " afterDrag=" << timeTicksAfterDrag << ")";

    score->hideAnchors();
    score->doLayout();

    expectStableSlurAfterOffsetOnlyMove(score, slur, firstCR, timeTicksBefore, noteXsBefore, slurBefore, staffShapeBefore);

    delete score;
}

TEST_F(Engraving_HairpinTests, verticalHairpinEndGripDragKeepsSlurGeometry)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ASSERT_TRUE(score);

    Hairpin* hp = addHairpinOverFirstMeasure(score);
    ASSERT_TRUE(hp);
    score->doLayout();
    ASSERT_FALSE(hp->segmentsEmpty());

    Slur* slur = firstSlur(score);
    ASSERT_TRUE(slur);
    ASSERT_FALSE(slur->segmentsEmpty());

    HairpinSegment* hairpinSeg = toHairpinSegment(hp->frontSegment());
    ASSERT_TRUE(hairpinSeg);

    const int timeTicksBefore = countTimeTickSegments(score);
    const std::vector<double> noteXsBefore = chordRestXs(score);
    const SlurGeometrySnapshot slurBefore = captureSlurGeometry(slur);
    const Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);
    const Segment* firstCR = firstMeasure->first(SegmentType::ChordRest);
    ASSERT_TRUE(firstCR);
    const std::vector<std::string> staffShapeBefore = staffShapeTypeNames(firstCR, 0);

    EditData ed;
    ed.curGrip = Grip::END;
    ed.evtDelta = PointF(0.0, hairpinSeg->spatium());
    ed.pos = canvasOriginFromPageGrip(hairpinSeg, Grip::END);
    prepareElementEditData(ed, hairpinSeg);
    static_cast<EngravingItem*>(hairpinSeg)->startEditDrag(ed);
    static_cast<EngravingItem*>(hairpinSeg)->editDrag(ed);
    score->doLayout();

    const int timeTicksAfterDrag = countTimeTickSegments(score);
    EXPECT_EQ(timeTicksAfterDrag, timeTicksBefore)
        << "startEditDrag + vertical end-grip drag must not insert a TimeTick grid"
        << " (before=" << timeTicksBefore << " afterDrag=" << timeTicksAfterDrag << ")";

    score->hideAnchors();
    score->doLayout();

    expectStableSlurAfterOffsetOnlyMove(score, slur, firstCR, timeTicksBefore, noteXsBefore, slurBefore, staffShapeBefore);

    delete score;
}

TEST_F(Engraving_HairpinTests, verticalExpressionDragKeepsSlurGeometry)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ASSERT_TRUE(score);

    Expression* expression = addExpressionOnFirstChord(score);
    ASSERT_TRUE(expression);
    score->doLayout();

    Slur* slur = firstSlur(score);
    ASSERT_TRUE(slur);
    ASSERT_FALSE(slur->segmentsEmpty());

    const int timeTicksBefore = countTimeTickSegments(score);
    const std::vector<double> noteXsBefore = chordRestXs(score);
    const SlurGeometrySnapshot slurBefore = captureSlurGeometry(slur);
    const Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);
    const Segment* firstCR = firstMeasure->first(SegmentType::ChordRest);
    ASSERT_TRUE(firstCR);
    const std::vector<std::string> staffShapeBefore = staffShapeTypeNames(firstCR, 0);

    EditData ed;
    const double spatium = expression->spatium();
    ed.evtDelta = PointF(0.3 * spatium, spatium);
    ed.moveDelta = ed.evtDelta;
    static_cast<TextBase*>(expression)->drag(ed);
    score->doLayout();

    const int timeTicksAfterDrag = countTimeTickSegments(score);
    EXPECT_EQ(timeTicksAfterDrag, timeTicksBefore)
        << "vertical expression drag must not insert a TimeTick grid"
        << " (before=" << timeTicksBefore << " afterDrag=" << timeTicksAfterDrag << ")";

    score->hideAnchors();
    score->doLayout();

    expectStableSlurAfterOffsetOnlyMove(score, slur, firstCR, timeTicksBefore, noteXsBefore, slurBefore, staffShapeBefore);

    delete score;
}

TEST_F(Engraving_HairpinTests, staffTextDragDoesNotShiftHairpinOrDynamicY)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ASSERT_TRUE(score);

    Hairpin* hp = addHairpinOverFirstMeasure(score);
    ASSERT_TRUE(hp);
    Dynamic* dynamic = addDynamicOnFirstChord(score);
    ASSERT_TRUE(dynamic);
    StaffText* staffText = addStaffTextOnFirstChord(score);
    ASSERT_TRUE(staffText);
    score->doLayout();
    ASSERT_FALSE(hp->segmentsEmpty());

    HairpinSegment* hairpinSeg = toHairpinSegment(hp->frontSegment());
    ASSERT_TRUE(hairpinSeg);

    Slur* slur = firstSlur(score);
    ASSERT_TRUE(slur);
    ASSERT_FALSE(slur->segmentsEmpty());

    const int timeTicksBefore = countTimeTickSegments(score);
    const std::vector<double> noteXsBefore = chordRestXs(score);
    const SlurGeometrySnapshot slurBefore = captureSlurGeometry(slur);
    const Segment* staffTextParentBefore = staffText->segment();

    const Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);
    const Segment* firstCR = firstMeasure->first(SegmentType::ChordRest);
    ASSERT_TRUE(firstCR);
    const std::vector<std::string> staffShapeBefore = staffShapeTypeNames(firstCR, 0);

    const double spatium = staffText->spatium();
    EditData ed;
    ed.evtDelta = PointF(0.3 * spatium, spatium);
    ed.moveDelta = ed.evtDelta;
    static_cast<EngravingItem*>(staffText)->startDrag(ed);
    static_cast<EngravingItem*>(staffText)->drag(ed);
    score->doLayout();

    EXPECT_EQ(staffText->segment(), staffTextParentBefore);
    EXPECT_EQ(countTimeTickSegments(score), timeTicksBefore)
        << "offset-only staff text drag must not insert a TimeTick grid";

    const double hairpinYDuringDrag = hairpinSeg->canvasPos().y();
    const double dynamicYDuringDrag = dynamic->canvasPos().y();

    static_cast<EngravingItem*>(staffText)->endDrag(ed);
    score->doLayout();

    EXPECT_EQ(countTimeTickSegments(score), timeTicksBefore);
    EXPECT_NEAR(hairpinSeg->canvasPos().y(), hairpinYDuringDrag, 1e-4) << "hairpin y jumped on mouse-up";
    EXPECT_NEAR(dynamic->canvasPos().y(), dynamicYDuringDrag, 1e-4) << "dynamic y jumped on mouse-up";

    expectStableSlurAfterOffsetOnlyMove(score, slur, firstCR, timeTicksBefore, noteXsBefore, slurBefore, staffShapeBefore);

    delete score;
}

TEST_F(Engraving_HairpinTests, dynamicDragDoesNotJumpHairpinYOnRelease)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ASSERT_TRUE(score);

    Hairpin* hp = addHairpinOverFirstMeasure(score);
    ASSERT_TRUE(hp);
    Dynamic* dynamic = addDynamicOnFirstChord(score);
    ASSERT_TRUE(dynamic);
    score->doLayout();
    ASSERT_FALSE(hp->segmentsEmpty());

    HairpinSegment* hairpinSeg = toHairpinSegment(hp->frontSegment());
    ASSERT_TRUE(hairpinSeg);

    const int timeTicksBefore = countTimeTickSegments(score);
    const Segment* dynamicParentBefore = dynamic->segment();

    const double spatium = dynamic->spatium();
    EditData ed;
    ed.evtDelta = PointF(0.3 * spatium, spatium);
    ed.moveDelta = ed.evtDelta;
    static_cast<EngravingItem*>(dynamic)->startDrag(ed);
    static_cast<EngravingItem*>(dynamic)->drag(ed);
    score->doLayout();

    EXPECT_EQ(dynamic->segment(), dynamicParentBefore);
    EXPECT_EQ(countTimeTickSegments(score), timeTicksBefore)
        << "offset-only dynamic drag must not insert a TimeTick grid";

    const double hairpinYDuringDrag = hairpinSeg->canvasPos().y();
    const double dynamicYDuringDrag = dynamic->canvasPos().y();

    static_cast<EngravingItem*>(dynamic)->endDrag(ed);
    score->doLayout();

    EXPECT_EQ(countTimeTickSegments(score), timeTicksBefore);
    EXPECT_NEAR(hairpinSeg->canvasPos().y(), hairpinYDuringDrag, 1e-4) << "hairpin y jumped on mouse-up";
    EXPECT_NEAR(dynamic->canvasPos().y(), dynamicYDuringDrag, 1e-4) << "dynamic y jumped on mouse-up";

    delete score;
}

TEST_F(Engraving_HairpinTests, dynamicDragRangeLayoutKeepsNextMeasureSlurHairpinAnchors)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ASSERT_TRUE(score);

    Measure* secondMeasure = appendMeasureWithTwoQuarters(score);
    ASSERT_TRUE(secondMeasure);

    Segment* m2FirstCRSeg = secondMeasure->first(SegmentType::ChordRest);
    Segment* m2SecondCRSeg = m2FirstCRSeg ? m2FirstCRSeg->next(SegmentType::ChordRest) : nullptr;
    ASSERT_TRUE(m2FirstCRSeg);
    ASSERT_TRUE(m2SecondCRSeg);
    ChordRest* m2StartCR = toChordRest(m2FirstCRSeg->element(0));
    ChordRest* m2EndCR = toChordRest(m2SecondCRSeg->element(0));
    ASSERT_TRUE(m2StartCR);
    ASSERT_TRUE(m2EndCR);
    ASSERT_TRUE(m2StartCR->isChord());
    ASSERT_TRUE(m2EndCR->isChord());
    Chord* m2StartChord = toChord(m2StartCR);
    Chord* m2EndChord = toChord(m2EndCR);
    Note* m2StartNote = m2StartChord->upNote();
    ASSERT_TRUE(m2StartNote);

    Articulation* staccato = addChordArticulation(m2StartChord, SymId::articStaccatoAbove);
    Articulation* tenuto = addChordArticulation(m2StartChord, SymId::articTenutoAbove);
    Fingering* fingering = addFingeringOnNote(m2StartNote, u"3");
    Articulation* accent = addChordArticulation(m2EndChord, SymId::articAccentAbove);
    Articulation* marcato = addChordArticulation(m2EndChord, SymId::articMarcatoAbove);
    Fermata* fermata = addFermataOnChord(m2EndChord);
    ASSERT_TRUE(staccato);
    ASSERT_TRUE(tenuto);
    ASSERT_TRUE(fingering);
    ASSERT_TRUE(accent);
    ASSERT_TRUE(marcato);
    ASSERT_TRUE(fermata);

    Slur* nextSlur = score->addSlur(m2StartCR, m2EndCR, nullptr);
    Hairpin* nextHairpin = score->addHairpin(HairpinType::CRESC_HAIRPIN, m2StartCR, m2EndCR);
    Dynamic* dynamic = addDynamicOnFirstChord(score);
    ASSERT_TRUE(nextSlur);
    ASSERT_TRUE(nextHairpin);
    ASSERT_TRUE(dynamic);

    score->doLayout();
    ASSERT_FALSE(nextSlur->segmentsEmpty());
    ASSERT_FALSE(nextHairpin->segmentsEmpty());
    ASSERT_TRUE(score->firstMeasure()->system());
    ASSERT_EQ(score->firstMeasure()->system(), secondMeasure->system());

    HairpinSegment* nextHairpinSeg = toHairpinSegment(nextHairpin->frontSegment());
    ASSERT_TRUE(nextHairpinSeg);

    const int timeTicksBefore = countTimeTickSegments(score);
    const Segment* dynamicParentBefore = dynamic->segment();
    const SlurGeometrySnapshot slurBefore = captureSlurGeometry(nextSlur);
    const HairpinAnchorSnapshot hairpinBefore = captureHairpinAnchorsRel(nextHairpinSeg, m2StartCR);
    const NextMeasureMarksSnapshot marksBefore = captureNextMeasureMarks(staccato, tenuto, accent, marcato,
                                                                         fermata, fingering, m2StartChord, m2EndChord);

    const double spatium = dynamic->spatium();
    EditData ed;
    ed.evtDelta = PointF(0.3 * spatium, spatium);
    ed.moveDelta = ed.evtDelta;
    static_cast<EngravingItem*>(dynamic)->startDrag(ed);
    static_cast<EngravingItem*>(dynamic)->drag(ed);

    EXPECT_EQ(dynamic->segment(), dynamicParentBefore);
    EXPECT_EQ(countTimeTickSegments(score), timeTicksBefore)
        << "offset-only dynamic drag must not insert a TimeTick grid";

    static_cast<EngravingItem*>(dynamic)->endDrag(ed);
    score->doLayoutRange(dynamic->tick(), dynamic->tick());

    EXPECT_EQ(dynamic->segment(), dynamicParentBefore);
    EXPECT_EQ(countTimeTickSegments(score), timeTicksBefore);

    ASSERT_FALSE(nextSlur->segmentsEmpty());
    ASSERT_FALSE(nextHairpin->segmentsEmpty());
    nextHairpinSeg = toHairpinSegment(nextHairpin->frontSegment());
    ASSERT_TRUE(nextHairpinSeg);

    const SlurGeometrySnapshot slurAfter = captureSlurGeometry(nextSlur);
    const HairpinAnchorSnapshot hairpinAfter = captureHairpinAnchorsRel(nextHairpinSeg, m2StartCR);
    expectPointNear(slurAfter.start, slurBefore.start, 1e-4, "next-measure slur START");
    expectPointNear(slurAfter.end, slurBefore.end, 1e-4, "next-measure slur END");
    expectPointNear(slurAfter.bezier1, slurBefore.bezier1, 1e-4, "next-measure slur BEZIER1");
    expectPointNear(slurAfter.bezier2, slurBefore.bezier2, 1e-4, "next-measure slur BEZIER2");
    expectPointNear(hairpinAfter.posRelToStartCR, hairpinBefore.posRelToStartCR, 1e-4,
                    "next-measure hairpin pos");
    expectPointNear(hairpinAfter.pos2, hairpinBefore.pos2, 1e-4, "next-measure hairpin pos2");
    expectPointNear(hairpinAfter.startGripRel, hairpinBefore.startGripRel, 1e-4,
                    "next-measure hairpin START grip");
    expectPointNear(hairpinAfter.endGripRel, hairpinBefore.endGripRel, 1e-4,
                    "next-measure hairpin END grip");

    const NextMeasureMarksSnapshot marksAfter = captureNextMeasureMarks(staccato, tenuto, accent, marcato,
                                                                        fermata, fingering, m2StartChord, m2EndChord);
    expectNextMeasureMarksStable(marksAfter, marksBefore);

    delete score;
}

TEST_F(Engraving_HairpinTests, timeTickAnchorsAreNotStaffShapeObstacles)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ASSERT_TRUE(score);

    Hairpin* hp = addHairpinOverFirstMeasure(score);
    ASSERT_TRUE(hp);
    score->doLayout();
    ASSERT_FALSE(hp->segmentsEmpty());

    HairpinSegment* hairpinSeg = toHairpinSegment(hp->frontSegment());
    ASSERT_TRUE(hairpinSeg);

    EditTimeTickAnchors::updateAnchors(hairpinSeg);
    score->doLayout();
    EXPECT_GT(countTimeTickSegments(score), 0);

    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(SegmentType::TimeTick); segment;
             segment = segment->next(SegmentType::TimeTick)) {
            for (EngravingItem* item : segment->elist()) {
                if (item && item->isTimeTickAnchor()) {
                    EXPECT_FALSE(item->addToSkyline());
                    EXPECT_FALSE(item->autoplace());
                }
            }
        }
        for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            const std::vector<std::string> names = staffShapeTypeNames(segment, 0);
            for (const std::string& name : names) {
                EXPECT_STRNE(name.c_str(), "TimeTickAnchor") << "staffShape must not include TimeTickAnchor";
            }
        }
    }

    delete score;
}

TEST_F(Engraving_HairpinTests, hideAnchorsRemovesUnusedTimeTickGrid)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ASSERT_TRUE(score);

    Hairpin* hp = addHairpinOverFirstMeasure(score);
    ASSERT_TRUE(hp);
    score->doLayout();
    ASSERT_FALSE(hp->segmentsEmpty());

    HairpinSegment* hairpinSeg = toHairpinSegment(hp->frontSegment());
    ASSERT_TRUE(hairpinSeg);

    const int timeTicksBefore = countTimeTickSegments(score);
    EditTimeTickAnchors::updateAnchors(hairpinSeg);
    EXPECT_GT(countTimeTickSegments(score), timeTicksBefore);

    score->hideAnchors();
    EXPECT_EQ(countTimeTickSegments(score), timeTicksBefore);

    delete score;
}

TEST_F(Engraving_HairpinTests, hideAnchorsKeepsNeededTimeTick)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ASSERT_TRUE(score);

    Hairpin* hp = addHairpinOverFirstMeasure(score);
    ASSERT_TRUE(hp);
    score->doLayout();
    ASSERT_FALSE(hp->segmentsEmpty());

    HairpinSegment* hairpinSeg = toHairpinSegment(hp->frontSegment());
    ASSERT_TRUE(hairpinSeg);

    const int timeTicksBefore = countTimeTickSegments(score);
    EditTimeTickAnchors::updateAnchors(hairpinSeg);
    EXPECT_GT(countTimeTickSegments(score), timeTicksBefore);

    Segment* kept = firstTimeTickSegment(score);
    ASSERT_TRUE(kept);
    const Fraction keptTick = kept->tick();

    Expression* expression = Factory::createExpression(kept, true);
    expression->setTrack(0);
    expression->setXmlText(u"dolce");
    kept->add(expression);

    score->hideAnchors();

    Segment* stillThere = findTimeTickSegment(score, keptTick);
    ASSERT_TRUE(stillThere);
    EXPECT_FALSE(stillThere->annotations().empty());
    EXPECT_EQ(countTimeTickSegments(score), timeTicksBefore + 1);

    delete score;
}

TEST_F(Engraving_HairpinTests, hideAnchorsKeepsTimeTickNeededByOttava)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ASSERT_TRUE(score);

    Ottava* ottava = addOttavaOnStaff2ToBeatTwo(score);
    ASSERT_TRUE(ottava);
    score->doLayout();

    ASSERT_EQ(score->nstaves(), 2u);
    EXPECT_EQ(ottava->staffIdx(), 1u);
    EXPECT_EQ(ottava->tick(), Fraction(0, 1));
    EXPECT_EQ(ottava->tick2(), Fraction(1, 2));

    const Fraction endTick = Fraction(1, 2);
    Segment* endTimeTick = findTimeTickSegment(score, endTick);
    ASSERT_TRUE(endTimeTick)
        << "Ottava ending mid-rest must keep a TimeTick at 2/4; existing ticks=["
        << timeTickTicksDebug(score) << "] endElement="
        << (ottava->endElement() ? ottava->endElement()->typeName() : "null");

    EngravingItem* timeTickAnchor = endTimeTick->element(staff2track(1));
    ASSERT_TRUE(timeTickAnchor);
    ASSERT_TRUE(timeTickAnchor->isTimeTickAnchor());

    score->hideAnchors();

    Segment* stillThere = findTimeTickSegment(score, endTick);
    ASSERT_TRUE(stillThere) << "hideAnchors must not drop a TimeTick required by Ottava";
    EXPECT_EQ(stillThere, endTimeTick);
    EngravingItem* stillAnchor = stillThere->element(staff2track(1));
    ASSERT_TRUE(stillAnchor);
    ASSERT_TRUE(stillAnchor->isTimeTickAnchor());
    EXPECT_EQ(toSegment(stillAnchor->explicitParent())->element(stillAnchor->track()), stillAnchor);

    delete score;
}

TEST_F(Engraving_HairpinTests, hideAnchorsInvalidatesPageBspTree)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ASSERT_TRUE(score);

    Hairpin* hp = addHairpinOverFirstMeasure(score);
    ASSERT_TRUE(hp);
    score->doLayout();
    ASSERT_FALSE(hp->segmentsEmpty());

    HairpinSegment* hairpinSeg = toHairpinSegment(hp->frontSegment());
    ASSERT_TRUE(hairpinSeg);

    EditTimeTickAnchors::updateAnchors(hairpinSeg);
    score->doLayout();

    ASSERT_FALSE(score->pages().empty());
    Page* page = score->pages().front();
    ASSERT_TRUE(page);

    const int anchorsBeforeCleanup = countTimeTickAnchorsInPageBsp(page);
    EXPECT_GT(anchorsBeforeCleanup, 0);

    score->hideAnchors();

    const int anchorsAfterCleanup = countTimeTickAnchorsInPageBsp(page);
    EXPECT_LT(anchorsAfterCleanup, anchorsBeforeCleanup);

    for (EngravingItem* item : page->items(page->pageBoundingRect())) {
        if (!item || !item->isTimeTickAnchor()) {
            continue;
        }
        ASSERT_TRUE(item->explicitParent());
        ASSERT_TRUE(item->explicitParent()->isSegment());
        EXPECT_EQ(toSegment(item->explicitParent())->element(item->track()), item);
    }

    delete score;
}
