/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include <gmock/gmock.h>

#include "mocks/notationconfigurationmock.h"
#include "ui/tests/mocks/uiconfigurationmock.h"

#include "draw/painter.h"
#include "draw/bufferedpaintprovider.h"

#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/tests/utils/scorerw.h"

#include "notation/view/notationruler.h"

using ::testing::NiceMock;

using namespace mu;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;
using namespace muse::draw;

static const String ANACRUSIS_SCORE_PATH(u"data/ruler_anacrusis.mscx");

class Notation_RulerTests : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_configuration = std::make_shared<NiceMock<NotationConfigurationMock> >();
        modularity::globalIoc()->registerExport<INotationConfiguration>("utests", m_configuration);

        m_uiConfiguration = std::make_shared<NiceMock<ui::UiConfigurationMock> >();
        modularity::globalIoc()->registerExport<ui::IUiConfiguration>("utests", m_uiConfiguration);
    }

    void TearDown() override
    {
        modularity::globalIoc()->unregister<ui::IUiConfiguration>("utests");
        modularity::globalIoc()->unregister<INotationConfiguration>("utests");
    }

    struct Marks {
        size_t lines = 0;
        size_t cursors = 0;
    };

    //! The ruler fills a rect per beat / subdivision line and draws the cursor as an icon font glyph
    static Marks countMarks(const DrawData::Item& item)
    {
        Marks marks;
        for (const DrawData::Data& data : item.datas) {
            marks.lines += data.paths.size();
            marks.cursors += data.texts.size();
        }

        for (const DrawData::Item& child : item.chilren) {
            const Marks childMarks = countMarks(child);
            marks.lines += childMarks.lines;
            marks.cursors += childMarks.cursors;
        }

        return marks;
    }

    static Marks paintRuler(const NoteInputState& state)
    {
        std::shared_ptr<BufferedPaintProvider> provider = std::make_shared<BufferedPaintProvider>();
        Painter painter(provider, "ruler");

        NotationRuler ruler(modularity::globalCtx());
        ruler.paint(&painter, state);

        painter.endDraw();

        return countMarks(provider->drawData()->item);
    }

    std::shared_ptr<NiceMock<NotationConfigurationMock> > m_configuration;
    std::shared_ptr<NiceMock<ui::UiConfigurationMock> > m_uiConfiguration;
};

/**
 * @TEST_CASE Measure_After_Short_Anacrusis
 * @details A 1/16 pickup shifts every following barline off the half-beat grid of the score's absolute ticks.
 *          The ruler must still find the beats of such a measure (it used to hit an UNREACHABLE on its first line)
 */
TEST_F(Notation_RulerTests, Measure_After_Short_Anacrusis)
{
    // [GIVEN] A 4/4 score with a 1/16 pickup measure, followed by a measure of four quarter notes
    MasterScore* score = ScoreRW::readScore(ANACRUSIS_SCORE_PATH);
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure()->nextMeasure();
    ASSERT_TRUE(measure);
    ASSERT_EQ(measure->tick(), Fraction(1, 16));

    // [GIVEN] The note input position is on the first beat of that measure
    Segment* segment = measure->first(SegmentType::ChordRest);
    ASSERT_TRUE(segment);

    NoteInputState& state = score->inputState();
    state.setTrack(0);
    state.setSegment(segment);

    // [WHEN] Paint the ruler
    const Marks marks = paintRuler(state);

    // [THEN] All 8 half-beats of the measure are marked: the cursor on beat 1, lines on the other 7
    EXPECT_EQ(marks.cursors, 1);
    EXPECT_EQ(marks.lines, 7);

    delete score;
}
