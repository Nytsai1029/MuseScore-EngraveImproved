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

#include <limits>
#include <memory>

#include <QQmlComponent>
#include <QQmlEngine>
#include <QScopedPointer>

#include <gtest/gtest.h>

#include "appshell/internal/usagestatisticsdata.h"
#include "appshell/view/usagestatisticsmodel.h"

using namespace mu::appshell;

namespace {
const QString SCORE_A = QStringLiteral("eid:score-a");
const QString SCORE_B = QStringLiteral("eid:score-b");
const QString PATH_SCORE = QStringLiteral("path:legacy-score");

class UsageStatisticsStub : public IUsageStatistics
{
public:
    UsageStatisticsSnapshot snapshot() const override
    {
        UsageStatisticsSnapshot result;
        result.totalActiveMilliseconds = 90 * 60 * 1000;
        return result;
    }

    muse::async::Notification statisticsChanged() const override
    {
        return m_statisticsChanged;
    }

private:
    muse::async::Notification m_statisticsChanged;
};

TEST(UsageStatisticsAccumulatorTests, CountsOnlyWhileActive)
{
    UsageStatisticsAccumulator accumulator;
    accumulator.setCurrentScoreKey(SCORE_A, 0);

    accumulator.setActive(true, 1000);
    EXPECT_EQ(accumulator.pendingTotalMilliseconds(61000), 60000);
    EXPECT_EQ(accumulator.pendingScoreMilliseconds(SCORE_A, 61000), 60000);

    accumulator.setActive(false, 121000);
    EXPECT_EQ(accumulator.pendingTotalMilliseconds(500000), 120000);
    EXPECT_EQ(accumulator.pendingScoreMilliseconds(SCORE_A, 500000), 120000);

    accumulator.setActive(true, 600000);
    accumulator.setActive(false, 630000);
    EXPECT_EQ(accumulator.pendingTotalMilliseconds(630000), 150000);
}

TEST(UsageStatisticsAccumulatorTests, TracksNoScoreAndScoreSwitches)
{
    UsageStatisticsAccumulator accumulator;
    accumulator.setActive(true, 0);

    accumulator.setCurrentScoreKey(SCORE_A, 10000);
    accumulator.setCurrentScoreKey(SCORE_B, 40000);
    accumulator.setCurrentScoreKey(QString(), 100000);
    accumulator.checkpoint(130000);

    EXPECT_EQ(accumulator.pendingTotalMilliseconds(130000), 130000);
    EXPECT_EQ(accumulator.pendingScoreMilliseconds(SCORE_A, 130000), 30000);
    EXPECT_EQ(accumulator.pendingScoreMilliseconds(SCORE_B, 130000), 60000);
}

TEST(UsageStatisticsAccumulatorTests, ResetRestartsCurrentActiveSegment)
{
    UsageStatisticsAccumulator accumulator;
    accumulator.setCurrentScoreKey(SCORE_A, 0);
    accumulator.setActive(true, 0);
    accumulator.checkpoint(60000);

    accumulator.reset(60000);

    EXPECT_EQ(accumulator.pendingTotalMilliseconds(119999), 59999);
    EXPECT_EQ(accumulator.pendingScoreMilliseconds(SCORE_A, 119999), 59999);
}

TEST(UsageStatisticsAccumulatorTests, MigratesPendingPathTimeToEid)
{
    UsageStatisticsAccumulator accumulator;
    accumulator.setCurrentScoreKey(PATH_SCORE, 0);
    accumulator.setActive(true, 0);
    accumulator.migrateScoreKey(PATH_SCORE, SCORE_A, 60000);

    EXPECT_EQ(accumulator.currentScoreKey(), SCORE_A);
    EXPECT_EQ(accumulator.pendingScoreMilliseconds(PATH_SCORE, 90000), 0);
    EXPECT_EQ(accumulator.pendingScoreMilliseconds(SCORE_A, 90000), 90000);
}

TEST(UsageStatisticsAccumulatorTests, RetainsPendingDeltasUntilClearedAfterWrite)
{
    UsageStatisticsAccumulator accumulator;
    accumulator.setCurrentScoreKey(SCORE_A, 0);
    accumulator.setActive(true, 0);
    accumulator.checkpoint(60000);

    EXPECT_EQ(accumulator.pendingTotalMilliseconds(60000), 60000);
    EXPECT_EQ(accumulator.pendingScoreMilliseconds(SCORE_A, 60000), 60000);

    accumulator.clearPersistedDeltas();

    EXPECT_EQ(accumulator.pendingTotalMilliseconds(60000), 0);
    EXPECT_EQ(accumulator.pendingScoreMilliseconds(SCORE_A, 60000), 0);
}

TEST(UsageStatisticsDataTests, RoundTripsVersionedJsonAndLargeIntegers)
{
    UsageStatisticsData data;
    data.generation = QStringLiteral("generation-one");
    data.totalActiveMilliseconds = std::numeric_limits<qint64>::max();
    data.scoreActiveMilliseconds.insert(SCORE_A, std::numeric_limits<qint64>::max() - 1);

    const UsageStatisticsDataResult result = decodeUsageStatisticsData(encodeUsageStatisticsData(data));

    ASSERT_EQ(result.status, UsageStatisticsDataStatus::Valid);
    EXPECT_EQ(result.data.generation, data.generation);
    EXPECT_EQ(result.data.totalActiveMilliseconds, data.totalActiveMilliseconds);
    EXPECT_EQ(result.data.scoreActiveMilliseconds.value(SCORE_A), data.scoreActiveMilliseconds.value(SCORE_A));
}

TEST(UsageStatisticsDataTests, TreatsInvalidValuesAsZeroAndIgnoresUnknownFields)
{
    const QByteArray json = R"json({
        "version": 1,
        "generation": "generation-one",
        "totalActiveMs": -10,
        "scores": {
            "eid:negative": -20,
            "eid:invalid": "not-a-number",
            "eid:fractional": 1.5,
            "runtime:not-persistent": 3000
        },
        "futureField": true
    })json";

    const UsageStatisticsDataResult result = decodeUsageStatisticsData(json);

    ASSERT_EQ(result.status, UsageStatisticsDataStatus::Valid);
    EXPECT_EQ(result.data.totalActiveMilliseconds, 0);
    EXPECT_TRUE(result.data.scoreActiveMilliseconds.isEmpty());
}

TEST(UsageStatisticsDataTests, RejectsMissingAndCorruptDocuments)
{
    EXPECT_EQ(decodeUsageStatisticsData(QByteArray()).status, UsageStatisticsDataStatus::Corrupt);
    EXPECT_EQ(decodeUsageStatisticsData(QByteArrayLiteral("not json")).status, UsageStatisticsDataStatus::Corrupt);
    EXPECT_EQ(decodeUsageStatisticsData(QByteArrayLiteral("{\"version\":2}")).status,
              UsageStatisticsDataStatus::Corrupt);
}

TEST(UsageStatisticsDataTests, MergesIndependentInstanceDeltasByAddition)
{
    UsageStatisticsData data;
    data.generation = QStringLiteral("generation-one");

    mergeUsageStatisticsDeltas(data, 60000, { { SCORE_A, 45000 } });
    mergeUsageStatisticsDeltas(data, 90000, { { SCORE_A, 15000 }, { SCORE_B, 75000 } });

    EXPECT_EQ(data.totalActiveMilliseconds, 150000);
    EXPECT_EQ(data.scoreActiveMilliseconds.value(SCORE_A), 60000);
    EXPECT_EQ(data.scoreActiveMilliseconds.value(SCORE_B), 75000);
}

TEST(UsageStatisticsDataTests, CopiesWithSameEidShareTimeAndDifferentEidsDoNot)
{
    UsageStatisticsData data;
    mergeUsageStatisticsDeltas(data, 0, { { SCORE_A, 60000 } });

    const QString copiedScoreKey = SCORE_A;
    EXPECT_EQ(data.scoreActiveMilliseconds.value(copiedScoreKey), 60000);
    EXPECT_EQ(data.scoreActiveMilliseconds.value(SCORE_B), 0);
}

TEST(UsageStatisticsDataTests, MigratesPathFallbackIntoExistingEidTotal)
{
    UsageStatisticsData data;
    data.scoreActiveMilliseconds.insert(PATH_SCORE, 60000);
    data.scoreActiveMilliseconds.insert(SCORE_A, 30000);

    applyUsageStatisticsMigrations(data, { { PATH_SCORE, SCORE_A } });

    EXPECT_FALSE(data.scoreActiveMilliseconds.contains(PATH_SCORE));
    EXPECT_EQ(data.scoreActiveMilliseconds.value(SCORE_A), 90000);
}

TEST(UsageStatisticsDataTests, RoutesLatePathDeltasThroughPersistedAlias)
{
    UsageStatisticsData data;
    data.scoreActiveMilliseconds.insert(PATH_SCORE, 60000);
    migrateUsageStatisticsScore(data, PATH_SCORE, SCORE_A);

    mergeUsageStatisticsDeltas(data, 0, { { PATH_SCORE, 30000 } });

    EXPECT_EQ(resolveUsageStatisticsScoreKey(data, PATH_SCORE), SCORE_A);
    EXPECT_EQ(usageStatisticsScoreMilliseconds(data, PATH_SCORE), 90000);
    EXPECT_EQ(data.scoreActiveMilliseconds.value(SCORE_A), 90000);

    const UsageStatisticsDataResult roundTrip = decodeUsageStatisticsData(encodeUsageStatisticsData(data));
    ASSERT_EQ(roundTrip.status, UsageStatisticsDataStatus::Valid);
    EXPECT_EQ(resolveUsageStatisticsScoreKey(roundTrip.data, PATH_SCORE), SCORE_A);
    EXPECT_EQ(usageStatisticsScoreMilliseconds(roundTrip.data, PATH_SCORE), 90000);
}

TEST(UsageStatisticsDataTests, SaturatesInsteadOfOverflowing)
{
    UsageStatisticsData data;
    data.totalActiveMilliseconds = std::numeric_limits<qint64>::max() - 5;
    data.scoreActiveMilliseconds.insert(SCORE_A, std::numeric_limits<qint64>::max() - 10);

    mergeUsageStatisticsDeltas(data, 10, { { SCORE_A, 20 } });

    EXPECT_EQ(data.totalActiveMilliseconds, std::numeric_limits<qint64>::max());
    EXPECT_EQ(data.scoreActiveMilliseconds.value(SCORE_A), std::numeric_limits<qint64>::max());
}

TEST(UsageStatisticsDurationTests, FloorsAtHourAndMinuteBoundaries)
{
    EXPECT_EQ(usageStatisticsDurationParts(59999).hours, 0);
    EXPECT_EQ(usageStatisticsDurationParts(59999).minutes, 0);
    EXPECT_EQ(usageStatisticsDurationParts(60000).minutes, 1);
    EXPECT_EQ(usageStatisticsDurationParts(3599999).minutes, 59);
    EXPECT_EQ(usageStatisticsDurationParts(3600000).hours, 1);
    EXPECT_EQ(usageStatisticsDurationParts(3660000).minutes, 1);
    EXPECT_EQ(usageStatisticsDurationParts(-1).hours, 0);
}

TEST(UsageStatisticsModelTests, ConstructorDefersIocAccessUntilLoad)
{
    UsageStatisticsModel model;

    EXPECT_TRUE(model.totalUsageTime().isEmpty());
    EXPECT_TRUE(model.currentScoreUsageTime().isEmpty());
    EXPECT_FALSE(model.hasCurrentScore());
}

TEST(UsageStatisticsModelTests, LoadsAfterQmlEngineContextIsAttached)
{
    const std::string moduleName = "appshell_tests";
    const std::shared_ptr<UsageStatisticsStub> usageStatistics = std::make_shared<UsageStatisticsStub>();
    muse::modularity::globalIoc()->registerExport<IUsageStatistics>(moduleName, usageStatistics);

    qmlRegisterType<UsageStatisticsModel>("MuseScore.AppShell.Tests", 1, 0, "UsageStatisticsModel");

    QQmlEngine engine;
    muse::QmlIoCContext qmlIocContext(&engine);
    qmlIocContext.ctx = muse::modularity::globalCtx();
    engine.setProperty("ioc_context", QVariant::fromValue(&qmlIocContext));

    QQmlComponent component(&engine);
    component.setData(R"qml(
        import QtQml 2.15
        import MuseScore.AppShell.Tests 1.0

        UsageStatisticsModel {
            Component.onCompleted: load()
        }
    )qml", QUrl());

    QScopedPointer<QObject> object(component.create());
    EXPECT_TRUE(object) << component.errorString().toStdString();

    UsageStatisticsModel* model = qobject_cast<UsageStatisticsModel*>(object.get());
    EXPECT_TRUE(model);
    if (model) {
        EXPECT_FALSE(model->totalUsageTime().isEmpty());
    }

    muse::modularity::globalIoc()->unregister<IUsageStatistics>(moduleName);
}
}
