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

#include "dom/barline.h"
#include "dom/bracket.h"
#include "dom/editdata.h"
#include "dom/factory.h"
#include "dom/layoutbreak.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/score.h"
#include "dom/staff.h"
#include "dom/system.h"
#include "dom/timesig.h"
#include "dom/undo.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String BARLINE_DATA_DIR(u"barline_data/");

//---------------------------------------------------------
//   BarlineTests
//---------------------------------------------------------

class Engraving_BarlineTests : public ::testing::Test
{
public:
};

//---------------------------------------------------------
//  barline01
//  Check bar line and brackets presence and length with hidden empty staves:
//    A score with:
//          3 staves,
//          bracket across all 3 staves
//          bar lines across all 3 staves
//          systems with each staff hidden in turn because empty
//    is loaded, laid out and bracket/bar line sizes are checked.
//
//    NO REFERENCE SCORE IS USED: the test has to do with layout/formatting,
//    not with edit or read/save operations.
//---------------------------------------------------------

// actual 3-staff bracket should be high 28.6 SP ca.: allow for some layout margin
static const double BRACKET0_HEIGHT_MIN     = 27;
static const double BRACKET0_HEIGHT_MAX     = 30;
// actual 2-staff bracket should be high 18.1 SP ca.
static const double BRACKET_HEIGHT_MIN      = 17;
static const double BRACKET_HEIGHT_MAX      = 20;

TEST_F(Engraving_BarlineTests, barline01)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barline01.mscx");
    EXPECT_TRUE(score);

    double height, heightMin, heightMax;
    double spatium = score->style().spatium();
    int sysNo = 0;
    for (System* sys : score->systems()) {
        // check number of the brackets of each system
        EXPECT_EQ(sys->brackets().size(), 1);

        // check height of the bracket of each system
        // (bracket height is different between first system (3 staves) and other systems (2 staves) )
        Bracket* bracket = sys->brackets().at(0);
        height      = bracket->ldata()->bbox().height() / spatium;
        heightMin   = (sysNo == 0) ? BRACKET0_HEIGHT_MIN : BRACKET_HEIGHT_MIN;
        heightMax   = (sysNo == 0) ? BRACKET0_HEIGHT_MAX : BRACKET_HEIGHT_MAX;

        EXPECT_GT(height, heightMin);
        EXPECT_LT(height, heightMax);

        // check presence and height of the bar line of each measure of each system
        // (2 measure for each system)
        for (int msrNo=0; msrNo < 2; ++msrNo) {
            Measure* msr = toMeasure(sys->measure(msrNo));
            Segment* seg = msr->findSegment(SegmentType::EndBarLine, msr->tick() + msr->ticks());
            EXPECT_TRUE(seg);

            BarLine* bar = toBarLine(seg->element(0));
            EXPECT_TRUE(bar);
        }
        sysNo++;
    }

    delete score;
}

//---------------------------------------------------------
//   barline02
//   add a 3/4 time signature in the second measure and check bar line 'generated' status
//
//    NO REFERENCE SCORE IS USED.
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, barline02)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barline02.mscx");
    EXPECT_TRUE(score);

    Measure* msr = score->firstMeasure()->nextMeasure();
    TimeSig* ts  = Factory::createTimeSig(score->dummy()->segment());
    ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

    score->cmdAddTimeSig(msr, 0, ts, false);
    score->doLayout();

    msr = score->firstMeasure();
    while ((msr = msr->nextMeasure())) {
        Segment* seg = msr->findSegment(SegmentType::EndBarLine, msr->tick() + msr->ticks());
        EXPECT_TRUE(seg);

        BarLine* bar = static_cast<BarLine*>(seg->element(0));
        EXPECT_TRUE(bar);

        // bar line should be generated if NORMAL, except the END one at the end
        bool test = bar->generated();
        EXPECT_TRUE(test);
    }

    delete score;
}

//---------------------------------------------------------
//   barline03
//   Sets a staff bar line span involving spanFrom and spanTo and
//   check that it is properly applied to start-repeat
//
//   NO REFERENCE SCORE IS USED.
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, barline03)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barline03.mscx");
    EXPECT_TRUE(score);

    score->startCmd(TranslatableString::untranslatable("Engraving barline tests"));
    score->undo(new ChangeProperty(score->staff(0), Pid::STAFF_BARLINE_SPAN, 1));
    score->undo(new ChangeProperty(score->staff(0), Pid::STAFF_BARLINE_SPAN_FROM, 2));
    score->undo(new ChangeProperty(score->staff(0), Pid::STAFF_BARLINE_SPAN_TO, -2));
    score->endCmd();

    // 'go' to 5th measure
    Measure* msr = score->firstMeasure();
    for (int i=0; i < 4; i++) {
        msr = msr->nextMeasure();
    }
    // check span data of measure-initial start-repeat bar line
    Segment* seg = msr->findSegment(SegmentType::StartRepeatBarLine, msr->tick());
    EXPECT_TRUE(seg);

    BarLine* bar = toBarLine(seg->element(0));
    EXPECT_TRUE(bar);

    delete score;
}

//---------------------------------------------------------
//   barline04
//   Sets custom span parameters to a system-initial start-repeat bar line and
//   check that it is properly applied to it and to the start-repeat bar lines of staves below.
//
//   NO REFERENCE SCORE IS USED.
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, barline04)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barline04.mscx");
    EXPECT_TRUE(score);

    score->doLayout();

    score->startCmd(TranslatableString::untranslatable("Engraving barline tests"));
    // 'go' to 5th measure
    Measure* msr = score->firstMeasure();
    for (int i=0; i < 4; i++) {
        msr = msr->nextMeasure();
    }
    // check span data of measure-initial start-repeat bar line
    Segment* seg = msr->findSegment(SegmentType::StartRepeatBarLine, msr->tick());
    EXPECT_TRUE(seg);

    BarLine* bar = static_cast<BarLine*>(seg->element(0));
    EXPECT_TRUE(bar);

    bar->undoChangeProperty(Pid::BARLINE_SPAN, 2);
    bar->undoChangeProperty(Pid::BARLINE_SPAN_FROM, 2);
    bar->undoChangeProperty(Pid::BARLINE_SPAN_TO, 6);
    score->endCmd();

    EXPECT_GT(bar->spanStaff(), 0);
    EXPECT_EQ(bar->spanFrom(), 2);
    EXPECT_EQ(bar->spanTo(), 6);

    // check start-repeat bar ine in second staff is gone
    EXPECT_EQ(seg->element(1), nullptr);

    delete score;
}

//---------------------------------------------------------
//   barline05
//   Adds a line break in the middle of a end-start-repeat bar line and then checks the two resulting
//   bar lines (an end-repeat and a start-repeat) are not marked as generated.
//
//   NO REFERENCE SCORE IS USED.
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, barline05)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barline05.mscx");
    EXPECT_TRUE(score);

    score->doLayout();

    // 'go' to 4th measure
    Measure* msr = score->firstMeasure();
    for (int i=0; i < 3; i++) {
        msr = msr->nextMeasure();
    }
    // create and add a LineBreak element
    LayoutBreak* lb = Factory::createLayoutBreak(msr);
    lb->setLayoutBreakType(LayoutBreakType::LINE);
    lb->setTrack(0);
    lb->setParent(msr);
    score->undoAddElement(lb);
    score->doLayout();

    // check an end-repeat bar line has been created at the end of this measure and it is generated
    Segment* seg = msr->findSegment(SegmentType::EndBarLine, msr->tick() + msr->ticks());
    EXPECT_TRUE(seg);

    BarLine* bar = static_cast<BarLine*>(seg->element(0));
    EXPECT_TRUE(bar);

    EXPECT_EQ(bar->barLineType(), BarLineType::END_REPEAT);
    EXPECT_TRUE(bar->generated());

    // // check an end-repeat bar line has been created at the beginning of the next measure and it is not generated
    // check an end-repeat bar line has been created at the beginning of the next measure and it is generated
    msr = msr->nextMeasure();
    seg = msr->findSegment(SegmentType::StartRepeatBarLine, msr->tick());
    EXPECT_TRUE(seg);

    bar = static_cast<BarLine*>(seg->element(0));
    EXPECT_TRUE(bar);
    EXPECT_TRUE(bar->generated());

    delete score;
}

//---------------------------------------------------------
//   barline06
//   Read a score with 3 staves and custom bar line sub-types for staff i-th at measure i-th
//   and check the custom syb-types are applied only to their respective bar lines,
//   rather than to whole measures.
//
//   NO REFERENCE SCORE IS USED.
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, barline06)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barline06.mscx");
    EXPECT_TRUE(score);

    score->doLayout();

    // scan each measure
    Measure* msr   = score->firstMeasure();
    for (int i=0; i < 3; i++) {
        // locate end-measure bar line segment
        Segment* seg = msr->findSegment(SegmentType::EndBarLine, msr->tick() + msr->ticks());
        EXPECT_TRUE(seg);

        // check only i-th staff has custom bar line type
        for (int j=0; j < 3; j++) {
            BarLine* bar = static_cast<BarLine*>(seg->element(j * VOICES));
            EXPECT_TRUE(bar);

            // if not the i-th staff, bar should be normal and not custom
            if (j != i) {
                EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);
            }
            // in the i-th staff, the bar line should be of type DOUBLE and custom type should be true
            else {
                EXPECT_EQ(bar->barLineType(), BarLineType::DOUBLE);
            }
        }

        msr = msr->nextMeasure();
    }

    delete score;
}

//---------------------------------------------------------
//   endBarLineAt
//---------------------------------------------------------

static BarLine* endBarLineAt(Measure* measure, staff_idx_t staffIdx)
{
    Segment* seg = measure->findSegment(SegmentType::EndBarLine, measure->endTick());
    if (!seg) {
        return nullptr;
    }
    EngravingItem* el = seg->element(staffIdx * VOICES);
    return el && el->isBarLine() ? toBarLine(el) : nullptr;
}

//---------------------------------------------------------
//   changeTypeAffectsOnlySelectedStaff
//   Changing an end-barline type from the inspector / property path
//   (and dropping a type onto a selected barline) must not rewrite
//   other staves on the same vertical line.
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, changeTypeAffectsOnlySelectedStaff)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + u"barline06.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    BarLine* staff0 = endBarLineAt(measure, 0);
    BarLine* staff1 = endBarLineAt(measure, 1);
    BarLine* staff2 = endBarLineAt(measure, 2);
    ASSERT_TRUE(staff0);
    ASSERT_TRUE(staff1);
    ASSERT_TRUE(staff2);

    EXPECT_EQ(staff0->barLineType(), BarLineType::DOUBLE);
    EXPECT_EQ(staff1->barLineType(), BarLineType::NORMAL);
    EXPECT_EQ(staff2->barLineType(), BarLineType::NORMAL);

    score->startCmd(TranslatableString::untranslatable("Engraving barline tests"));
    staff1->undoChangeProperty(Pid::BARLINE_TYPE, PropertyValue::fromValue(BarLineType::DOTTED));
    score->endCmd();

    EXPECT_EQ(staff0->barLineType(), BarLineType::DOUBLE);
    EXPECT_EQ(staff1->barLineType(), BarLineType::DOTTED);
    EXPECT_EQ(staff2->barLineType(), BarLineType::NORMAL);

    EditData dropData(0);
    BarLine* heavy = Factory::createBarLine(score->dummy()->segment());
    heavy->setBarLineType(BarLineType::HEAVY);
    dropData.dropElement = heavy;
    score->startCmd(TranslatableString::untranslatable("Drop heavy barline test"));
    staff2->drop(dropData);
    score->endCmd();

    EXPECT_EQ(staff0->barLineType(), BarLineType::DOUBLE);
    EXPECT_EQ(staff1->barLineType(), BarLineType::DOTTED);
    EXPECT_EQ(staff2->barLineType(), BarLineType::HEAVY);

    score->startCmd(TranslatableString::untranslatable("Engraving barline tests"));
    score->undoChangeBarLineType(staff0, BarLineType::BROKEN, true);
    score->endCmd();

    EXPECT_EQ(staff0->barLineType(), BarLineType::BROKEN);
    EXPECT_EQ(staff1->barLineType(), BarLineType::BROKEN);
    EXPECT_EQ(staff2->barLineType(), BarLineType::BROKEN);

    delete score;
}

//---------------------------------------------------------
//   spanningBarlineSplitsStaffAndConnector
//   A spanning barline is three independently selectable pieces:
//   this staff, the gap connector, and the next staff.
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, spanningBarlineSplitsStaffAndConnector)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + u"barline01.mscx");
    ASSERT_TRUE(score);

    score->doLayout();

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);
    System* system = measure->system();
    ASSERT_TRUE(system);

    BarLine* staff0 = endBarLineAt(measure, 0);
    BarLine* staff1 = endBarLineAt(measure, 1);
    ASSERT_TRUE(staff0);
    ASSERT_TRUE(staff1);
    ASSERT_TRUE(staff0->spanStaff());

    BarLine* connector = staff0->spanConnector();
    ASSERT_TRUE(connector);
    EXPECT_TRUE(connector->isSpanConnector());
    EXPECT_FALSE(staff0->isSpanConnector());
    EXPECT_FALSE(staff1->isSpanConnector());
    EXPECT_EQ(connector->spanParent(), staff0);
    EXPECT_EQ(connector->scanParent(), staff0);

    const BarLine::LayoutData* staffData = staff0->ldata();
    const BarLine::LayoutData* connectorData = connector->ldata();
    ASSERT_TRUE(staffData);
    ASSERT_TRUE(connectorData);
    ASSERT_FALSE(connectorData->isSkipDraw());

    const double spatium = score->style().spatium();
    EXPECT_NEAR(connectorData->y1, staffData->y2, 0.01);
    EXPECT_GT(connectorData->y2, connectorData->y1);
    EXPECT_LT(staffData->y2 - staffData->y1, 6.0 * spatium);

    const double nextStaffTop = system->staff(1)->y() - system->staff(0)->y();
    EXPECT_NEAR(connectorData->y2, nextStaffTop, 0.5 * spatium);
    EXPECT_LT(connectorData->y2, nextStaffTop + spatium);

    delete score;
}

//---------------------------------------------------------
//   spanningBarlineTypesAreIndependent
//   Staff, connector, and next-staff barlines keep independent
//   appearance types after inspector / drop changes.
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, spanningBarlineTypesAreIndependent)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + u"barline06.mscx");
    ASSERT_TRUE(score);

    score->startCmd(TranslatableString::untranslatable("Enable barline span"));
    score->undo(new ChangeProperty(score->staff(0), Pid::STAFF_BARLINE_SPAN, 1));
    score->endCmd();
    score->doLayout();

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    BarLine* staff0 = endBarLineAt(measure, 0);
    BarLine* staff1 = endBarLineAt(measure, 1);
    ASSERT_TRUE(staff0);
    ASSERT_TRUE(staff1);

    BarLine* connector = staff0->spanConnector();
    ASSERT_TRUE(connector);

    EXPECT_EQ(staff0->barLineType(), BarLineType::DOUBLE);
    EXPECT_EQ(connector->barLineType(), BarLineType::DOUBLE);
    EXPECT_EQ(staff1->barLineType(), BarLineType::NORMAL);

    score->startCmd(TranslatableString::untranslatable("Change staff barline type"));
    staff0->undoChangeProperty(Pid::BARLINE_TYPE, PropertyValue::fromValue(BarLineType::HEAVY));
    score->endCmd();

    EXPECT_EQ(staff0->barLineType(), BarLineType::HEAVY);
    EXPECT_EQ(connector->barLineType(), BarLineType::DOUBLE);
    EXPECT_EQ(staff1->barLineType(), BarLineType::NORMAL);

    score->startCmd(TranslatableString::untranslatable("Change connector barline type"));
    connector->undoChangeProperty(Pid::BARLINE_TYPE, PropertyValue::fromValue(BarLineType::DOTTED));
    score->endCmd();

    EXPECT_EQ(staff0->barLineType(), BarLineType::HEAVY);
    EXPECT_EQ(connector->barLineType(), BarLineType::DOTTED);
    EXPECT_EQ(staff1->barLineType(), BarLineType::NORMAL);

    EditData dropData(0);
    BarLine* broken = Factory::createBarLine(score->dummy()->segment());
    broken->setBarLineType(BarLineType::BROKEN);
    dropData.dropElement = broken;
    score->startCmd(TranslatableString::untranslatable("Drop dashed barline on connector"));
    connector->drop(dropData);
    score->endCmd();

    EXPECT_EQ(staff0->barLineType(), BarLineType::HEAVY);
    EXPECT_EQ(connector->barLineType(), BarLineType::BROKEN);
    EXPECT_EQ(staff1->barLineType(), BarLineType::NORMAL);

    delete score;
}

//---------------------------------------------------------
//   dropNormalBarline
//   helper for barline179726()
//---------------------------------------------------------

void dropNormalBarline(EngravingItem* e)
{
    EditData dropData(0);
    BarLine* barLine = Factory::createBarLine(e->score()->dummy()->segment());
    barLine->setBarLineType(BarLineType::NORMAL);
    dropData.dropElement = barLine;

    e->score()->startCmd(TranslatableString::untranslatable("Drop normal barline test"));
    e->drop(dropData);
    e->score()->endCmd();
}

//---------------------------------------------------------
//   barline179726
//   Drop a normal barline onto measures and barlines of each type of barline
//
//    NO REFERENCE SCORE IS USED.
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, barline179726)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barline179726.mscx");
    EXPECT_TRUE(score);

    score->doLayout();

    Measure* m = score->firstMeasure();

    // drop NORMAL onto initial START_REPEAT barline will remove that START_REPEAT
    dropNormalBarline(m->findSegment(SegmentType::StartRepeatBarLine, m->tick())->elementAt(0));
    EXPECT_EQ(m->findSegment(SegmentType::StartRepeatBarLine, Fraction(0, 1)), nullptr);

    // drop NORMAL onto END_START_REPEAT will turn into NORMAL
    dropNormalBarline(m->findSegment(SegmentType::EndBarLine, m->endTick())->elementAt(0));
    BarLine* bar = static_cast<BarLine*>(m->findSegment(SegmentType::EndBarLine, m->endTick())->elementAt(0));
    EXPECT_TRUE(bar);
    EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);

    m = m->nextMeasure();

    // drop NORMAL onto the END_REPEAT part of an END_START_REPEAT straddling a newline will turn into NORMAL at the end of this meas
    dropNormalBarline(m->findSegment(SegmentType::EndBarLine, m->endTick())->elementAt(0));
    bar = static_cast<BarLine*>(m->findSegment(SegmentType::EndBarLine, m->endTick())->elementAt(0));
    EXPECT_TRUE(bar);
    EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);

    m = m->nextMeasure();

    // but leave START_REPEAT at the beginning of the newline
    bar = static_cast<BarLine*>(m->findSegment(SegmentType::StartRepeatBarLine, m->tick())->elementAt(0));
    EXPECT_TRUE(bar);

    // drop NORMAL onto the meas ending with an END_START_REPEAT straddling a newline will turn into NORMAL at the end of this meas
    // but note I'm not verifying what happens to the START_REPEAT at the beginning of the newline...I'm not sure that behavior is well-defined yet
    dropNormalBarline(m);
    bar = static_cast<BarLine*>(m->findSegment(SegmentType::EndBarLine, m->endTick())->elementAt(0));
    EXPECT_TRUE(bar);
    EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);

    m = m->nextMeasure();
    m = m->nextMeasure();

    // drop NORMAL onto the START_REPEAT part of an END_START_REPEAT straddling a newline will remove the START_REPEAT at the beginning of this measure
    dropNormalBarline(m->findSegment(SegmentType::StartRepeatBarLine, m->tick())->elementAt(0));
    EXPECT_EQ(m->findSegment(SegmentType::StartRepeatBarLine, m->tick()), nullptr);

    // but leave END_REPEAT at the end of previous line
    bar = static_cast<BarLine*>(m->prevMeasure()->findSegment(SegmentType::EndBarLine, m->tick())->elementAt(0));
    EXPECT_TRUE(bar);
    EXPECT_EQ(bar->barLineType(), BarLineType::END_REPEAT);

    for (int i = 0; i < 4; i++, m = m->nextMeasure()) {
        // drop NORMAL onto END_REPEAT, BROKEN, DOTTED, DOUBLE at the end of this meas will turn into NORMAL
        dropNormalBarline(m->findSegment(SegmentType::EndBarLine, m->endTick())->elementAt(0));
        bar = static_cast<BarLine*>(m->findSegment(SegmentType::EndBarLine, m->endTick())->elementAt(0));
        EXPECT_TRUE(bar);
        EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);
    }

    m = m->nextMeasure();

    // drop NORMAL onto a START_REPEAT in middle of a line will remove the START_REPEAT at the beginning of this measure
    dropNormalBarline(m->findSegment(SegmentType::StartRepeatBarLine, m->tick())->elementAt(0));
    EXPECT_EQ(m->findSegment(SegmentType::StartRepeatBarLine, m->tick()), nullptr);

    // drop NORMAL onto final END_REPEAT at end of score will turn into NORMAL
    dropNormalBarline(m->findSegment(SegmentType::EndBarLine, m->endTick())->elementAt(0));
    bar = static_cast<BarLine*>(m->findSegment(SegmentType::EndBarLine, m->endTick())->elementAt(0));
    EXPECT_TRUE(bar);
    EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);

    delete score;
}

//---------------------------------------------------------
//   deleteSkipBarlines
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, deleteSkipBarlines)
{
    MasterScore* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barlinedelete.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    EXPECT_TRUE(m1);

    score->startCmd(TranslatableString::untranslatable("Engraving barline tests"));
    score->cmdSelectAll();
    score->cmdDeleteSelection();
    score->endCmd();

    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String("barlinedelete.mscx"), BARLINE_DATA_DIR + String("barlinedelete-ref.mscx")));

    delete score;
}
