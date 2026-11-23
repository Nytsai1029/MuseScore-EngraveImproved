/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <QQmlEngine>
#include <QQmlContext>
#include <QObject>

#include "modularity/ioc.h"

#include "notation/view/engravingoptions/engravingoptionsstate.h"

#include "mocks/notationconfigurationmock.h"

using namespace mu;
using namespace mu::notation;
using namespace muse;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace {
//! NOTE A state object, as QML makes it: an object of an engine, which is where it gets the
//! injected services from
class StateInEngine
{
public:
    StateInEngine()
        : m_context(&m_engine), m_state(new EngravingOptionsState())
    {
        QQmlEngine::setContextForObject(m_state, &m_context);
        QQmlEngine::setObjectOwnership(m_state, QQmlEngine::CppOwnership);
    }

    ~StateInEngine() { delete m_state; }

    EngravingOptionsState* get() const { return m_state; }

private:
    QQmlEngine m_engine;
    QQmlContext m_context;
    EngravingOptionsState* m_state = nullptr;
};

EngravingOptionsDialogState storedState()
{
    EngravingOptionsDialogState state;
    state.lastPageCode = QStringLiteral("beams");
    state.showsHandles = false;
    state.sidebarWidth = 200;
    state.previewWidth = 700;
    return state;
}
}

class Notation_EngravingOptionsStateTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_configuration = std::make_shared<NiceMock<NotationConfigurationMock> >();
        modularity::globalIoc()->registerExport<INotationConfiguration>("utests", m_configuration);

        ON_CALL(*m_configuration, engravingOptionsDialogState()).WillByDefault(Return(storedState()));
    }

    void TearDown() override
    {
        modularity::globalIoc()->unregister<INotationConfiguration>("utests");
    }

    std::shared_ptr<NiceMock<NotationConfigurationMock> > m_configuration;
};

/**
 * @TEST_CASE propertiesGiveWhatTheConfigurationReturns
 * @details Every property reads the member of the state which the configuration returns
 */
TEST_F(Notation_EngravingOptionsStateTests, propertiesGiveWhatTheConfigurationReturns)
{
    StateInEngine holder;
    EngravingOptionsState* state = holder.get();

    EXPECT_EQ(state->lastPageCode(), QStringLiteral("beams"));
    EXPECT_EQ(state->showsHandles(), false);
    EXPECT_EQ(state->sidebarWidth(), 200);
    EXPECT_EQ(state->previewWidth(), 700);
}

/**
 * @TEST_CASE settingAPropertyWritesAndEmits
 * @details Setting a property calls setEngravingOptionsDialogState once with the state in which
 *          only that member differs, and emits the property's signal
 */
TEST_F(Notation_EngravingOptionsStateTests, settingAPropertyWritesAndEmits)
{
    StateInEngine holder;
    EngravingOptionsState* state = holder.get();

    EngravingOptionsDialogState written;
    EXPECT_CALL(*m_configuration, setEngravingOptionsDialogState(_)).WillOnce(
        [&written](const EngravingOptionsDialogState& state) {
        written = state;
    });

    int emissions = 0;
    QObject::connect(state, &EngravingOptionsState::sidebarWidthChanged, state, [&emissions]() { ++emissions; });
    state->setSidebarWidth(300);

    EXPECT_EQ(written.lastPageCode, QStringLiteral("beams"));
    EXPECT_EQ(written.showsHandles, false);
    EXPECT_EQ(written.sidebarWidth, 300);
    EXPECT_EQ(written.previewWidth, 700);
    EXPECT_EQ(emissions, 1);
}

/**
 * @TEST_CASE settingTheSameValueWritesNothing
 * @details Setting the value which is already stored calls nothing and emits nothing
 */
TEST_F(Notation_EngravingOptionsStateTests, settingTheSameValueWritesNothing)
{
    StateInEngine holder;
    EngravingOptionsState* state = holder.get();

    EXPECT_CALL(*m_configuration, setEngravingOptionsDialogState(_)).Times(0);

    int emissions = 0;
    QObject::connect(state, &EngravingOptionsState::sidebarWidthChanged, state, [&emissions]() { ++emissions; });
    state->setSidebarWidth(200);

    EXPECT_EQ(emissions, 0);
}
