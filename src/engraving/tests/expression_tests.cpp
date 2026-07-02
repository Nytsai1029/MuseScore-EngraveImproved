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

#include "dom/dynamic.h"
#include "dom/expression.h"
#include "dom/factory.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/timesig.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String EXPRESSION_DATA_DIR(u"expression_data/");

class Engraving_ExpressionTests : public ::testing::Test
{
};

//---------------------------------------------------------
//    Import of old expressions to new Expression items
//---------------------------------------------------------
TEST_F(Engraving_ExpressionTests, expression1)
{
    MasterScore* score = ScoreRW::readScore(EXPRESSION_DATA_DIR + u"expression-1.mscx");
    EXPECT_TRUE(score);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"expression-1.mscx", EXPRESSION_DATA_DIR + u"expression-1-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//    Create and save new expression items
//---------------------------------------------------------
TEST_F(Engraving_ExpressionTests, expression2)
{
    MasterScore* score = ScoreRW::readScore(EXPRESSION_DATA_DIR + u"expression-2.mscx");
    EXPECT_TRUE(score);

    Measure* measure = score->firstMeasure();
    Segment* segment = measure->findSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    Expression* expression = Factory::createExpression(segment, true);
    expression->setTrack(0);
    expression->setXmlText("expression");
    segment->add(expression);

    measure = measure->nextMeasure();
    segment = measure->findSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    expression = Factory::createExpression(segment, true);
    expression->setTrack(0);
    expression->setXmlText("expression");
    segment->add(expression);
    expression->undoChangeProperty(Pid::PLACEMENT, PlacementV::ABOVE, PropertyFlags::UNSTYLED);

    measure = measure->nextMeasure();
    segment = measure->findSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    expression = Factory::createExpression(segment, true);
    expression->setTrack(0);
    expression->setXmlText("expression");
    segment->add(expression);
    expression->undoChangeProperty(Pid::SNAP_TO_DYNAMICS, false, PropertyFlags::UNSTYLED);

    measure = measure->nextMeasure();
    segment = measure->findSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    expression = Factory::createExpression(segment, true);
    expression->setTrack(0);
    expression->setXmlText("<b>expression</b>");
    expression->undoChangeProperty(Pid::SNAP_BEFORE_DYNAMICS, true, PropertyFlags::UNSTYLED);
    segment->add(expression);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"expression-2.mscx", EXPRESSION_DATA_DIR + u"expression-2-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//    Position expression before or after a matching dynamic
//---------------------------------------------------------
TEST_F(Engraving_ExpressionTests, snapBeforeDynamicsLayout)
{
    MasterScore* score = ScoreRW::readScore(EXPRESSION_DATA_DIR + u"expression-2.mscx");
    EXPECT_TRUE(score);

    Measure* measure = score->firstMeasure();
    Segment* segment = measure->findSegmentR(SegmentType::ChordRest, Fraction(0, 1));

    Dynamic* dynamic = Factory::createDynamic(segment, true);
    dynamic->setTrack(0);
    dynamic->setDynamicType(u"f");
    segment->add(dynamic);

    Expression* expression = Factory::createExpression(segment, true);
    expression->setTrack(0);
    expression->setXmlText("dolce");
    segment->add(expression);

    score->doLayout();

    RectF dynamicRect = dynamic->pageBoundingRect();
    RectF expressionRect = expression->pageBoundingRect();
    const double dynamicLeft = dynamicRect.left();
    const double dynamicRight = dynamicRect.right();

    EXPECT_FALSE(expression->snapBeforeDynamics());
    EXPECT_LT(dynamicRight, expressionRect.left());

    expression->undoChangeProperty(Pid::SNAP_BEFORE_DYNAMICS, true, PropertyFlags::UNSTYLED);
    EXPECT_TRUE(expression->snapBeforeDynamics());
    score->setLayoutAll();
    score->doLayout();

    dynamicRect = dynamic->pageBoundingRect();
    expressionRect = expression->pageBoundingRect();

    EXPECT_DOUBLE_EQ(dynamicLeft, dynamicRect.left());
    EXPECT_LT(expressionRect.right(), dynamicRect.left());

    delete score;
}
