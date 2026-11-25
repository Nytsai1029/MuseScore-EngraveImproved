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

#include <memory>
#include <set>
#include <vector>

#include <QCoreApplication>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

#include "modularity/ioc.h"
#include "ui/inavigation.h"
#include "ui/inavigationcontroller.h"

#include "notation/view/engravingoptions/engravingoptionsnavigation.h"

using namespace mu::notation;
using namespace muse;
using namespace muse::ui;

namespace {
class StubSection : public QObject, public INavigationSection
{
public:
    explicit StubSection(const QString& name)
        : m_name(name) {}

    QString name() const override { return m_name; }
    const Index& index() const override { return m_index; }
    void setIndex(const Index& idx) override { m_index = idx; }
    async::Channel<Index> indexChanged() const override { return {}; }
    bool enabled() const override { return true; }
    async::Channel<bool> enabledChanged() const override { return {}; }
    bool active() const override { return false; }
    void setActive(bool) override {}
    async::Channel<bool> activeChanged() const override { return {}; }
    QWindow* window() const override { return nullptr; }
    QQuickItem* visualItem() const override { return nullptr; }
    void onEvent(EventPtr) override {}

    Type type() const override { return Type::Exclusive; }
    const std::set<INavigationPanel*>& panels() const override { return m_panels; }
    async::Notification panelsListChanged() const override { return {}; }
    void setOnActiveRequested(const OnActiveRequested&) override {}
    void requestActive(INavigationPanel*, INavigationControl*, bool, ActivationType) override {}

private:
    QString m_name;
    Index m_index;
    std::set<INavigationPanel*> m_panels;
};

class StubPanel : public INavigationPanel
{
public:
    StubPanel(const QString& name, INavigationSection* section)
        : m_name(name), m_section(section) {}

    QString name() const override { return m_name; }
    const Index& index() const override { return m_index; }
    void setIndex(const Index& idx) override { m_index = idx; }
    async::Channel<Index> indexChanged() const override { return {}; }
    bool enabled() const override { return true; }
    async::Channel<bool> enabledChanged() const override { return {}; }
    bool active() const override { return false; }
    void setActive(bool) override {}
    async::Channel<bool> activeChanged() const override { return {}; }
    QWindow* window() const override { return nullptr; }
    QQuickItem* visualItem() const override { return nullptr; }
    void onEvent(EventPtr) override {}

    INavigationSection* section() const override { return m_section; }
    Direction direction() const override { return Direction::Both; }
    const std::set<INavigationControl*>& controls() const override { return m_controls; }
    async::Notification controlsListChanged() const override { return {}; }
    void requestActive(INavigationControl*, bool, ActivationType) override {}

private:
    QString m_name;
    INavigationSection* m_section = nullptr;
    Index m_index;
    std::set<INavigationControl*> m_controls;
};

class StubControl : public INavigationControl
{
public:
    StubControl(const QString& name, INavigationPanel* panel, QQuickItem* item)
        : m_name(name), m_panel(panel), m_item(item) {}

    QString name() const override { return m_name; }
    const Index& index() const override { return m_index; }
    void setIndex(const Index& idx) override { m_index = idx; }
    async::Channel<Index> indexChanged() const override { return {}; }
    bool enabled() const override { return true; }
    async::Channel<bool> enabledChanged() const override { return {}; }
    bool active() const override { return false; }
    void setActive(bool) override {}
    async::Channel<bool> activeChanged() const override { return {}; }
    QWindow* window() const override { return nullptr; }
    QQuickItem* visualItem() const override { return m_item; }
    void onEvent(EventPtr) override {}

    INavigationPanel* panel() const override { return m_panel; }
    void trigger() override {}
    async::Notification triggered() const override { return {}; }
    void requestActive(bool) override {}

private:
    QString m_name;
    INavigationPanel* m_panel = nullptr;
    QQuickItem* m_item = nullptr;
    Index m_index;
};

class StubNavigationController : public INavigationController
{
public:
    void reg(INavigationSection*) override {}
    void unreg(INavigationSection*) override {}
    bool requestActivateByName(const std::string&, const std::string&, const std::string&) override { return false; }
    bool requestActivateByIndex(const std::string&, const std::string&, const INavigation::Index&) override
    {
        return false;
    }
    void resetNavigation() override {}

    INavigationSection* activeSection() const override
    {
        return m_activeControl && m_activeControl->panel() ? m_activeControl->panel()->section() : nullptr;
    }
    INavigationPanel* activePanel() const override
    {
        return m_activeControl ? m_activeControl->panel() : nullptr;
    }
    INavigationControl* activeControl() const override { return m_activeControl; }

    const std::set<INavigationSection*>& sections() const override { return m_sections; }
    const INavigationSection* findSection(const std::string&) const override { return nullptr; }
    const INavigationPanel* findPanel(const std::string&, const std::string&) const override { return nullptr; }
    const INavigationControl* findControl(const std::string&, const std::string&, const std::string&) const override
    {
        return nullptr;
    }

    void setDefaultNavigationControl(INavigationControl*) override {}
    async::Notification navigationChanged() const override { return m_navigationChanged; }
    bool isHighlight() const override { return m_isHighlight; }
    void setIsHighlight(bool isHighlight) override
    {
        if (m_isHighlight == isHighlight) {
            return;
        }
        m_isHighlight = isHighlight;
        m_highlightChanged.notify();
    }
    async::Notification highlightChanged() const override { return m_highlightChanged; }
    void setIsResetOnMousePress(bool) override {}
    void dump() const override {}

    void activate(INavigationControl* control, bool highlight)
    {
        m_activeControl = control;
        m_navigationChanged.notify();
        setIsHighlight(highlight);
    }

private:
    std::set<INavigationSection*> m_sections;
    INavigationControl* m_activeControl = nullptr;
    bool m_isHighlight = false;
    async::Notification m_navigationChanged;
    async::Notification m_highlightChanged;
};

class NavigationInEngine
{
public:
    NavigationInEngine()
        : m_context(&m_engine), m_navigation(new EngravingOptionsNavigation())
    {
        QQmlEngine::setContextForObject(m_navigation, &m_context);
        QQmlEngine::setObjectOwnership(m_navigation, QQmlEngine::CppOwnership);
    }

    ~NavigationInEngine() { delete m_navigation; }

    QQmlEngine* engine() { return &m_engine; }
    EngravingOptionsNavigation* get() const { return m_navigation; }

private:
    QQmlEngine m_engine;
    QQmlContext m_context;
    EngravingOptionsNavigation* m_navigation = nullptr;
};
}

class Notation_EngravingOptionsNavigationTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_controller = std::make_shared<StubNavigationController>();
        modularity::globalIoc()->registerExport<INavigationController>("utests", m_controller);
    }

    void TearDown() override
    {
        modularity::globalIoc()->unregister<INavigationController>("utests");
    }

    std::shared_ptr<StubNavigationController> m_controller;
};

TEST_F(Notation_EngravingOptionsNavigationTests, scrollsDownAndUpToRevealContainerWithMargin)
{
    //! NOTE Control + its option row sit far below a 600px viewport: scrolling reveals the full row + 20px margin
    const double down = EngravingOptionsNavigation::scrollOffsetToReveal(
        0.0, 0.0, 2000.0, 600.0,
        1200.0, 1228.0,
        1190.0, 1240.0,
        20.0, false);
    EXPECT_DOUBLE_EQ(down, 660.0);

    //! NOTE Control + its option row sit above the viewport: scrolling reveals the top of the row + 20px margin
    const double up = EngravingOptionsNavigation::scrollOffsetToReveal(
        500.0, 0.0, 2000.0, 600.0,
        -120.0, -92.0,
        -130.0, -80.0,
        20.0, true);
    EXPECT_DOUBLE_EQ(up, 350.0);
}

TEST_F(Notation_EngravingOptionsNavigationTests, visibleControlSkipsScrollOnClickButRevealsClippedTitleOnKeyboard)
{
    //! NOTE Tall option row's title is clipped at y = -30 while the control at y = 90..120 is inside the viewport
    const double onClick = EngravingOptionsNavigation::scrollOffsetToReveal(
        300.0, 0.0, 2000.0, 600.0,
        90.0, 120.0,
        -30.0, 180.0,
        20.0, false);
    EXPECT_DOUBLE_EQ(onClick, 300.0);

    const double onKeyboard = EngravingOptionsNavigation::scrollOffsetToReveal(
        300.0, 0.0, 2000.0, 600.0,
        90.0, 120.0,
        -30.0, 180.0,
        20.0, true);
    EXPECT_DOUBLE_EQ(onKeyboard, 250.0);
}

TEST_F(Notation_EngravingOptionsNavigationTests, fallsBackToControlWhenContainerExceedsViewportAndClampsToBounds)
{
    //! NOTE Option taller than viewport (700px > 600px): reveal the focused tile itself, not the whole container
    const double tall = EngravingOptionsNavigation::scrollOffsetToReveal(
        0.0, 0.0, 2000.0, 600.0,
        980.0, 1080.0,
        400.0, 1100.0,
        20.0, true);
    EXPECT_DOUBLE_EQ(tall, 500.0);

    //! NOTE ListView topMargin gives a negative minOffset (-6); scrolling to the top item clamps cleanly to -6
    const double clampedTop = EngravingOptionsNavigation::scrollOffsetToReveal(
        10.0, -6.0, 1000.0, 500.0,
        -10.0, 24.0,
        -10.0, 24.0,
        20.0, true);
    EXPECT_DOUBLE_EQ(clampedTop, -6.0);

    //! NOTE Non-scrollable viewport (maxOffset <= minOffset) never changes offset
    const double fixed = EngravingOptionsNavigation::scrollOffsetToReveal(
        0.0, 0.0, 0.0, 500.0,
        -50.0, -20.0,
        -50.0, -20.0,
        20.0, true);
    EXPECT_DOUBLE_EQ(fixed, 0.0);
}

TEST_F(Notation_EngravingOptionsNavigationTests, tracksOnlyControlsInItsSectionAndCoalescesHighlight)
{
    NavigationInEngine holder;
    EngravingOptionsNavigation* nav = holder.get();

    StubSection dialogSection(QStringLiteral("EngravingOptionsDialog"));
    StubSection otherSection(QStringLiteral("MainWindow"));
    StubPanel dialogPanel(QStringLiteral("EngravingOptionsPage"), &dialogSection);
    StubPanel otherPanel(QStringLiteral("OtherPanel"), &otherSection);

    QQuickItem itemA;
    QQuickItem itemB;
    StubControl ctrlA(QStringLiteral("Value_a"), &dialogPanel, &itemA);
    StubControl ctrlB(QStringLiteral("Value_b"), &otherPanel, &itemB);

    struct Event {
        QQuickItem* item;
        bool highlighted;
    };
    std::vector<Event> events;
    QObject::connect(nav, &EngravingOptionsNavigation::controlActivated,
                     [&events](QQuickItem* item, bool highlighted) {
        events.push_back({ item, highlighted });
    });

    nav->setSection(&dialogSection);
    QCoreApplication::processEvents();
    EXPECT_TRUE(events.empty());

    //! NOTE Navigate via keyboard: navigationChanged fires before setIsHighlight(true), coalesced into one signal
    m_controller->activate(&ctrlA, true);
    QCoreApplication::processEvents();
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].item, &itemA);
    EXPECT_TRUE(events[0].highlighted);
    EXPECT_EQ(nav->activeItem(), &itemA);
    EXPECT_TRUE(nav->isHighlighted());

    //! NOTE Turning highlight off for the same control updates state without re-emitting controlActivated
    m_controller->setIsHighlight(false);
    QCoreApplication::processEvents();
    EXPECT_EQ(events.size(), 1u);
    EXPECT_FALSE(nav->isHighlighted());

    //! NOTE Turning highlight back on for the same control re-emits so preview/scroll syncs again
    m_controller->setIsHighlight(true);
    QCoreApplication::processEvents();
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[1].item, &itemA);
    EXPECT_TRUE(events[1].highlighted);

    //! NOTE Control in a different section clears activeItem and does not emit controlActivated
    m_controller->activate(&ctrlB, true);
    QCoreApplication::processEvents();
    EXPECT_EQ(events.size(), 2u);
    EXPECT_EQ(nav->activeItem(), nullptr);
    EXPECT_FALSE(nav->isHighlighted());
}

TEST_F(Notation_EngravingOptionsNavigationTests, ensureVisibleScrollsNestedHorizontalAndVerticalFlickables)
{
    NavigationInEngine holder;
    EngravingOptionsNavigation* nav = holder.get();

    const char* qml = R"(
        import QtQuick 2.15
        Flickable {
            id: outer
            width: 400
            height: 300
            contentWidth: 400
            contentHeight: 1500

            Flickable {
                id: inner
                objectName: "inner"
                x: 40
                y: 900
                width: 300
                height: 120
                contentWidth: 900
                contentHeight: 120

                Item {
                    id: cell
                    objectName: "cell"
                    x: 650
                    y: 40
                    width: 100
                    height: 36

                    Item {
                        id: control
                        objectName: "control"
                        x: 4
                        y: 4
                        width: 72
                        height: 28
                    }
                }
            }
        }
    )";

    QQmlComponent component(holder.engine());
    component.setData(qml, QUrl());
    ASSERT_FALSE(component.isError()) << component.errorString().toStdString();

    std::unique_ptr<QObject> rootObj(component.create());
    ASSERT_NE(rootObj, nullptr);

    auto* outer = qobject_cast<QQuickItem*>(rootObj.get());
    auto* inner = outer->findChild<QQuickItem*>(QStringLiteral("inner"));
    ASSERT_NE(outer, nullptr);
    ASSERT_NE(inner, nullptr);

    auto* cell = outer->findChild<QQuickItem*>(QStringLiteral("cell"));
    auto* control = outer->findChild<QQuickItem*>(QStringLiteral("control"));
    ASSERT_NE(cell, nullptr);
    ASSERT_NE(control, nullptr);

    nav->ensureVisible(control, cell, true);

    //! NOTE Inner horizontal flickable (width 300): cell x=650..750 -> contentX = 750 - (300 - 16) = 466
    EXPECT_NEAR(inner->property("contentX").toDouble(), 466.0, 0.5);
    //! NOTE Outer vertical flickable (height 300): cell y=940..976 -> contentY = 976 - (300 - 20) = 696
    EXPECT_NEAR(outer->property("contentY").toDouble(), 696.0, 0.5);
}

TEST_F(Notation_EngravingOptionsNavigationTests, ensureVisibleRespectsRevealContainerFlagOnVisibleControl)
{
    NavigationInEngine holder;
    EngravingOptionsNavigation* nav = holder.get();

    const char* qml = R"(
        import QtQuick 2.15
        Flickable {
            width: 400
            height: 300
            contentWidth: 400
            contentHeight: 1200
            contentY: 200

            Item {
                id: optionRow
                objectName: "optionRow"
                x: 20
                y: 170
                width: 360
                height: 180

                Item {
                    id: control
                    objectName: "control"
                    x: 0
                    y: 120
                    width: 120
                    height: 30
                }
            }
        }
    )";

    QQmlComponent component(holder.engine());
    component.setData(qml, QUrl());
    ASSERT_FALSE(component.isError()) << component.errorString().toStdString();

    std::unique_ptr<QObject> rootObj(component.create());
    ASSERT_NE(rootObj, nullptr);

    auto* flick = qobject_cast<QQuickItem*>(rootObj.get());
    auto* optionRow = flick->findChild<QQuickItem*>(QStringLiteral("optionRow"));
    auto* control = flick->findChild<QQuickItem*>(QStringLiteral("control"));
    ASSERT_NE(flick, nullptr);
    ASSERT_NE(optionRow, nullptr);
    ASSERT_NE(control, nullptr);

    //! NOTE Mouse click (revealContainerWhenVisible=false): control is at viewport y=90..120 (visible) -> no scroll
    nav->ensureVisible(control, optionRow, false);
    EXPECT_NEAR(flick->property("contentY").toDouble(), 200.0, 0.5);

    //! NOTE Keyboard nav (revealContainerWhenVisible=true): optionRow top is at y=-30 -> scrolls up to y=150
    nav->ensureVisible(control, optionRow, true);
    EXPECT_NEAR(flick->property("contentY").toDouble(), 150.0, 0.5);
}
