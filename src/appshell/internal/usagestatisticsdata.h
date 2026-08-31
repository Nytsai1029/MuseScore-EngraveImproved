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

#pragma once

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVector>

namespace mu::appshell {
struct UsageStatisticsData {
    QString generation;
    qint64 totalActiveMilliseconds = 0;
    QHash<QString, qint64> scoreActiveMilliseconds;
    QHash<QString, QString> scoreAliases;
};

struct UsageStatisticsMigration {
    QString fromKey;
    QString toKey;
};

enum class UsageStatisticsDataStatus {
    Valid,
    Corrupt
};

struct UsageStatisticsDataResult {
    UsageStatisticsDataStatus status = UsageStatisticsDataStatus::Corrupt;
    UsageStatisticsData data;
};

struct UsageStatisticsDurationParts {
    qint64 hours = 0;
    int minutes = 0;
};

qint64 usageStatisticsSaturatedAdd(qint64 left, qint64 right);
bool isPersistentUsageStatisticsKey(const QString& key);
UsageStatisticsDurationParts usageStatisticsDurationParts(qint64 milliseconds);

UsageStatisticsDataResult decodeUsageStatisticsData(const QByteArray& bytes);
QByteArray encodeUsageStatisticsData(const UsageStatisticsData& data);
void migrateUsageStatisticsScore(UsageStatisticsData& data, const QString& fromKey, const QString& toKey);
void applyUsageStatisticsMigrations(UsageStatisticsData& data, const QVector<UsageStatisticsMigration>& migrations);
void mergeUsageStatisticsDeltas(UsageStatisticsData& data, qint64 totalDelta,
                                const QHash<QString, qint64>& scoreDeltas);
QString resolveUsageStatisticsScoreKey(const UsageStatisticsData& data, const QString& key);
qint64 usageStatisticsScoreMilliseconds(const UsageStatisticsData& data, const QString& key);

class UsageStatisticsAccumulator
{
public:
    void setActive(bool active, qint64 nowMilliseconds);
    void setCurrentScoreKey(const QString& key, qint64 nowMilliseconds);
    void migrateScoreKey(const QString& fromKey, const QString& toKey, qint64 nowMilliseconds);
    void checkpoint(qint64 nowMilliseconds);
    void reset(qint64 nowMilliseconds);

    qint64 pendingTotalMilliseconds(qint64 nowMilliseconds) const;
    qint64 pendingScoreMilliseconds(const QString& key, qint64 nowMilliseconds) const;
    const QHash<QString, qint64>& pendingScoreMilliseconds() const;

    void clearPersistedDeltas();
    void discardScoreKey(const QString& key);

    bool active() const;
    const QString& currentScoreKey() const;

private:
    qint64 liveSegmentMilliseconds(qint64 nowMilliseconds) const;

    bool m_active = false;
    QString m_currentScoreKey;
    qint64 m_segmentStartMilliseconds = 0;
    qint64 m_pendingTotalMilliseconds = 0;
    QHash<QString, qint64> m_pendingScoreMilliseconds;
};
}
