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

#include "dom/chordrest.h"
#include "dom/hairpin.h"
#include "dom/masterscore.h"
#include "dom/segment.h"
#include "dom/text.h"

#include "engraving/compat/scoreaccess.h"
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
