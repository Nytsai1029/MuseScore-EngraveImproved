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
#include <gmock/gmock.h>

#include <map>
#include <memory>

#include <QQmlContext>
#include <QQmlEngine>

#include "modularity/ioc.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/masterscore.h"
#include "engraving/style/defaultstyle.h"

#include "notation/view/engravingoptions/engravingoptionsmodel.h"
#include "notation/view/styledialog/styleitem.h"

#include "mocks/globalcontextmock.h"
#include "mocks/notationelementsmock.h"
#include "mocks/notationmock.h"
#include "mocks/notationstylemock.h"
#include "mocks/notationundostackmock.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;

using ::testing::_;
using ::testing::ElementsAre;
using ::testing::NiceMock;
using ::testing::Return;

namespace {
//! NOTE A model, as QML makes it: an object of an engine, which is where it gets the injected services from
class ModelInEngine
{
public:
    ModelInEngine()
        : m_context(&m_engine), m_model(new EngravingOptionsModel())
    {
        QQmlEngine::setContextForObject(m_model, &m_context);
        QQmlEngine::setObjectOwnership(m_model, QQmlEngine::CppOwnership);
    }

    ~ModelInEngine() { delete m_model; }

    EngravingOptionsModel* get() const { return m_model; }

    void destroy()
    {
        delete m_model;
        m_model = nullptr;
    }

private:
    QQmlEngine m_engine;
    QQmlContext m_context;
    EngravingOptionsModel* m_model = nullptr;
};
}

class Notation_EngravingOptionsModelTests : public ::testing::Test, public async::Asyncable
{
protected:
    void SetUp() override
    {
        m_globalContext = std::make_shared<NiceMock<context::GlobalContextMock> >();
        m_notation = std::make_shared<NiceMock<NotationMock> >();
        m_style = std::make_shared<NiceMock<NotationStyleMock> >();
        m_undoStack = std::make_shared<NiceMock<NotationUndoStackMock> >();

        modularity::globalIoc()->registerExport<context::IGlobalContext>("utests", m_globalContext);

        ON_CALL(*m_globalContext, currentNotation()).WillByDefault(Return(m_notation));
        ON_CALL(*m_notation, style()).WillByDefault(Return(m_style));
        ON_CALL(*m_notation, undoStack()).WillByDefault(Return(m_undoStack));

        ON_CALL(*m_style, styleChanged()).WillByDefault(Return(m_styleChanged));
        ON_CALL(*m_style, defaultStyleValue(_)).WillByDefault([](const StyleId& id) {
            return DefaultStyle::defaultStyle().styleV(id);
        });
        ON_CALL(*m_style, styleValue(_)).WillByDefault([this](const StyleId& id) {
            auto found = m_values.find(id);
            return found != m_values.end() ? found->second : DefaultStyle::defaultStyle().styleV(id);
        });
        ON_CALL(*m_style, setStyleValue(_, _)).WillByDefault([this](const StyleId& id, const PropertyValue& value) {
            m_written.emplace_back(id, value);
        });
    }

    void TearDown() override
    {
        m_styleChanged.resetOnNotify(this);
        modularity::globalIoc()->unregister<context::IGlobalContext>("utests");
    }

    std::shared_ptr<NiceMock<context::GlobalContextMock> > m_globalContext;
    std::shared_ptr<NiceMock<NotationMock> > m_notation;
    std::shared_ptr<NiceMock<NotationStyleMock> > m_style;
    std::shared_ptr<NiceMock<NotationUndoStackMock> > m_undoStack;

    async::Notification m_styleChanged;
    std::map<StyleId, PropertyValue> m_values;
    std::vector<std::pair<StyleId, PropertyValue> > m_written;
};

/**
 * @TEST_CASE sessionIsOneUndoStep
 * @details A session prepares the changes once and commits them once when it is accepted, and is over then
 */
TEST_F(Notation_EngravingOptionsModelTests, sessionIsOneUndoStep)
{
    // [GIVEN] A loaded model
    ModelInEngine holder;
    EngravingOptionsModel* model = holder.get();

    int notified = 0;
    m_styleChanged.onNotify(this, [&notified]() { ++notified; });

    EXPECT_CALL(*m_undoStack, prepareChanges(_)).Times(1);
    EXPECT_CALL(*m_undoStack, commitChanges()).Times(1);
    EXPECT_CALL(*m_undoStack, rollbackChanges()).Times(0);

    model->load();
    ASSERT_TRUE(model->isLoaded());

    // [WHEN] Accepted
    model->accept();

    // [THEN] Over, and the style was told
    EXPECT_FALSE(model->isLoaded());
    EXPECT_EQ(notified, 1);

    // [THEN] Nothing happens the second time, or when the other end is called
    model->accept();
    model->reject();
    EXPECT_EQ(notified, 1);

    holder.destroy();
}

/**
 * @TEST_CASE rejectAndDestructionRollBack
 * @details What is not accepted is taken back
 */
TEST_F(Notation_EngravingOptionsModelTests, rejectAndDestructionRollBack)
{
    {
        ModelInEngine holder;

        EXPECT_CALL(*m_undoStack, prepareChanges(_)).Times(1);
        EXPECT_CALL(*m_undoStack, rollbackChanges()).Times(1);
        EXPECT_CALL(*m_undoStack, commitChanges()).Times(0);

        holder.get()->load();
        holder.get()->reject();
        EXPECT_FALSE(holder.get()->isLoaded());

        holder.get()->reject();
    }

    ::testing::Mock::VerifyAndClearExpectations(m_undoStack.get());

    {
        ModelInEngine holder;

        EXPECT_CALL(*m_undoStack, prepareChanges(_)).Times(1);
        EXPECT_CALL(*m_undoStack, rollbackChanges()).Times(1);
        EXPECT_CALL(*m_undoStack, commitChanges()).Times(0);

        holder.get()->load();
        ASSERT_TRUE(holder.get()->isLoaded());

        holder.destroy();
    }

    ::testing::Mock::VerifyAndClearExpectations(m_undoStack.get());
}

/**
 * @TEST_CASE itemReadsAndWritesByName
 * @details A style value is reached by its name: a distance is a plain number, a write goes to the style
 */
TEST_F(Notation_EngravingOptionsModelTests, itemReadsAndWritesByName)
{
    ModelInEngine holder;
    EngravingOptionsModel* model = holder.get();
    model->load();
    ASSERT_TRUE(model->isLoaded());

    // [THEN] The value of a distance is a number of staff spaces
    StyleItem* item = model->item("minNoteDistance");
    ASSERT_TRUE(item);
    EXPECT_EQ(item->value().typeId(), QMetaType::Double);
    EXPECT_DOUBLE_EQ(item->value().toDouble(), DefaultStyle::defaultStyle().styleS(Sid::minNoteDistance).val());
    EXPECT_DOUBLE_EQ(item->defaultValue().toDouble(), DefaultStyle::defaultStyle().styleS(Sid::minNoteDistance).val());

    // [THEN] The same item the second time
    EXPECT_EQ(model->item("minNoteDistance"), item);

    // [WHEN] The value is set
    EXPECT_CALL(*m_style, setStyleValue(Sid::minNoteDistance, _)).Times(1);
    item->modifyValue(0.75);

    // [THEN] The style got a distance of that many staff spaces
    ASSERT_EQ(m_written.size(), 1u);
    EXPECT_EQ(m_written.front().first, Sid::minNoteDistance);
    EXPECT_EQ(m_written.front().second.value<Spatium>().val(), 0.75);

    // [THEN] A name which is not a style value gives nothing
    EXPECT_EQ(model->item("noSuchStyleValue"), nullptr);
    EXPECT_FALSE(model->contains("noSuchStyleValue"));
    EXPECT_TRUE(model->contains("minNoteDistance"));

    holder.destroy();
}

/**
 * @TEST_CASE valueKinds
 * @details What a control is chosen by
 */
TEST_F(Notation_EngravingOptionsModelTests, valueKinds)
{
    ModelInEngine holder;
    EngravingOptionsModel* model = holder.get();

    // [THEN] Nothing is known before the session
    EXPECT_EQ(model->valueKind("minNoteDistance"), QString());

    model->load();
    ASSERT_TRUE(model->isLoaded());

    EXPECT_EQ(model->valueKind("minNoteDistance"), QString("spatium"));
    EXPECT_EQ(model->valueKind("measureSpacing"), QString("real"));
    EXPECT_EQ(model->valueKind("lyricsMaxDashCount"), QString("int"));
    EXPECT_EQ(model->valueKind("showHeader"), QString("bool"));
    EXPECT_EQ(model->valueKind("musicalSymbolFont"), QString("string"));
    EXPECT_EQ(model->valueKind("lyricsPosAbove"), QString("point"));
    EXPECT_EQ(model->valueKind("hairpinColor"), QString("color"));
    EXPECT_EQ(model->valueKind("noSuchStyleValue"), QString());

    EXPECT_TRUE(model->isInStaffSpaces("minNoteDistance"));
    EXPECT_FALSE(model->isInStaffSpaces("measureSpacing"));

    holder.destroy();
}

/**
 * @TEST_CASE musicFontTakesDynamicsFontAlong
 * @details A new font of the musical symbols is the font of the dynamics, unless the dynamics have their own
 */
TEST_F(Notation_EngravingOptionsModelTests, musicFontTakesDynamicsFontAlong)
{
    ModelInEngine holder;
    EngravingOptionsModel* model = holder.get();
    model->load();
    ASSERT_TRUE(model->isLoaded());

    // [GIVEN] The values which the font recommends are not asked for
    model->setLoadsFontDefaults(false);

    StyleItem* item = model->item("musicalSymbolFont");
    ASSERT_TRUE(item);

    // [WHEN] The dynamics follow the font
    m_values[Sid::dynamicsOverrideFont] = false;
    item->modifyValue(QString("Bravura"));

    // [THEN] The font of the dynamics changes with it, and nothing else is written
    ASSERT_EQ(m_written.size(), 2u);
    EXPECT_EQ(m_written.at(0).first, Sid::dynamicsFont);
    EXPECT_EQ(m_written.at(0).second.value<String>(), String(u"Bravura"));
    EXPECT_EQ(m_written.at(1).first, Sid::musicalSymbolFont);
    EXPECT_EQ(m_written.at(1).second.value<String>(), String(u"Bravura"));

    // [WHEN] The dynamics have a font of their own
    m_written.clear();
    m_values[Sid::dynamicsOverrideFont] = true;
    item->modifyValue(QString("Petaluma"));

    // [THEN] Only the font of the symbols is written
    ASSERT_EQ(m_written.size(), 1u);
    EXPECT_EQ(m_written.front().first, Sid::musicalSymbolFont);
    EXPECT_EQ(m_written.front().second.value<String>(), String(u"Petaluma"));

    holder.destroy();
}

/**
 * @TEST_CASE countsAndResetsByName
 * @details The values which differ from their defaults are counted, names of no value are left out
 */
TEST_F(Notation_EngravingOptionsModelTests, countsAndResetsByName)
{
    ModelInEngine holder;
    EngravingOptionsModel* model = holder.get();

    // [THEN] Nothing counts before the session
    EXPECT_EQ(model->modifiedCount({ "minNoteDistance" }), 0);

    model->load();
    ASSERT_TRUE(model->isLoaded());

    const QStringList names { "minNoteDistance", "noSuchStyleValue", "showHeader", "measureSpacing" };
    EXPECT_EQ(model->modifiedCount(names), 0);

    // [WHEN] Two of them are changed
    m_values[Sid::minNoteDistance] = Spatium(1.25);
    m_values[Sid::showHeader] = !DefaultStyle::defaultStyle().styleB(Sid::showHeader);

    // [THEN] Both are counted
    EXPECT_EQ(model->modifiedCount(names), 2);
    EXPECT_EQ(model->modifiedCount({ "noSuchStyleValue" }), 0);

    // [THEN] Exactly the known ones are reset
    EXPECT_CALL(*m_style, resetStyleValues(ElementsAre(Sid::minNoteDistance, Sid::showHeader, Sid::measureSpacing))).Times(1);
    model->resetValues(names);

    holder.destroy();
}

/**
 * @TEST_CASE metaTagsListTheScores
 * @details The tags of the score are listed, the master's first for an excerpt, "-" for an empty value
 */
TEST_F(Notation_EngravingOptionsModelTests, metaTagsListTheScores)
{
    ModelInEngine holder;
    EngravingOptionsModel* model = holder.get();

    // [THEN] No notation: nothing
    EXPECT_TRUE(model->metaTags().isEmpty());

    auto elements = std::make_shared<NiceMock<NotationElementsMock> >();
    ON_CALL(*m_notation, elements()).WillByDefault(Return(elements));

    // [GIVEN] A master score with tags, one of them empty
    std::unique_ptr<engraving::MasterScore> master(engraving::compat::ScoreAccess::createMasterScore(nullptr));
    ASSERT_TRUE(master);
    master->setMetaTags({ { u"workTitle", u"The Piece" }, { u"source", u"" } });
    ON_CALL(*elements, msScore()).WillByDefault(Return(master.get()));

    // [THEN] Its tags only, "-" for the empty one (the map keeps them by name)
    QVariantList tags = model->metaTags();
    ASSERT_EQ(tags.size(), 2);
    EXPECT_EQ(tags.at(0).toMap().value("tag").toString(), QString("source"));
    EXPECT_EQ(tags.at(0).toMap().value("value").toString(), QString("-"));
    EXPECT_EQ(tags.at(1).toMap().value("tag").toString(), QString("workTitle"));
    EXPECT_EQ(tags.at(1).toMap().value("value").toString(), QString("The Piece"));

    // [GIVEN] An excerpt with a tag of its own
    std::unique_ptr<engraving::Score> excerpt(master->createScore());
    excerpt->setMetaTags({ { u"arranger", u"Me" } });
    ON_CALL(*elements, msScore()).WillByDefault(Return(excerpt.get()));

    // [THEN] The tags of the master first, then its own
    tags = model->metaTags();
    ASSERT_EQ(tags.size(), 3);
    EXPECT_EQ(tags.at(0).toMap().value("tag").toString(), QString("source"));
    EXPECT_EQ(tags.at(1).toMap().value("tag").toString(), QString("workTitle"));
    EXPECT_EQ(tags.at(2).toMap().value("tag").toString(), QString("arranger"));
    EXPECT_EQ(tags.at(2).toMap().value("value").toString(), QString("Me"));

    holder.destroy();
}

/**
 * @TEST_CASE styleChangeRefreshesItems
 * @details What changes the style outside the dialog (the undo of a step, a reset) shows in the items
 */
TEST_F(Notation_EngravingOptionsModelTests, styleChangeRefreshesItems)
{
    ModelInEngine holder;
    EngravingOptionsModel* model = holder.get();
    model->load();
    ASSERT_TRUE(model->isLoaded());

    StyleItem* distance = model->item("minNoteDistance");
    StyleItem* header = model->item("showHeader");
    ASSERT_TRUE(distance);
    ASSERT_TRUE(header);

    const int revision = model->revision();
    EXPECT_TRUE(distance->isDefault());

    // [WHEN] The style changes
    m_values[Sid::minNoteDistance] = Spatium(1.5);
    m_values[Sid::showHeader] = false;
    m_styleChanged.notify();

    // [THEN] The items which exist have the new values
    EXPECT_DOUBLE_EQ(distance->value().toDouble(), 1.5);
    EXPECT_FALSE(distance->isDefault());
    EXPECT_EQ(header->value().toBool(), false);
    EXPECT_GT(model->revision(), revision);

    EXPECT_EQ(model->item("minNoteDistance"), distance);

    holder.destroy();
}
