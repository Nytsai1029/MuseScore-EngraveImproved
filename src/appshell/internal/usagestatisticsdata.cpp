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

#include "usagestatisticsdata.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QSet>

using namespace mu::appshell;

namespace {
constexpr int USAGE_STATISTICS_VERSION = 1;
constexpr qint64 MILLISECONDS_PER_MINUTE = 60 * 1000;
constexpr qint64 MINUTES_PER_HOUR = 60;

qint64 nonNegativeInteger(const QJsonValue& value)
{
    qint64 result = 0;

    if (value.isDouble()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
        const qint64 integer = value.toInteger(-1);
        if (integer >= 0 && value.toDouble(-1.0) == static_cast<double>(integer)) {
            result = integer;
        }
#else
        const double number = value.toDouble();
        if (number >= 0.0 && std::floor(number) == number
            && number < static_cast<double>(std::numeric_limits<qint64>::max())) {
            result = static_cast<qint64>(number);
        }
#endif
    } else if (value.isString()) {
        bool ok = false;
        result = value.toString().toLongLong(&ok);
        if (!ok) {
            result = 0;
        }
    }

    return result < 0 ? 0 : result;
}

QJsonValue integerJsonValue(qint64 value)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
    return QJsonValue(value);
#else
    return QJsonValue(QString::number(value));
#endif
}
}

qint64 mu::appshell::usageStatisticsSaturatedAdd(qint64 left, qint64 right)
{
    left = std::max<qint64>(left, 0);
    right = std::max<qint64>(right, 0);

    if (right > std::numeric_limits<qint64>::max() - left) {
        return std::numeric_limits<qint64>::max();
    }

    return left + right;
}

bool mu::appshell::isPersistentUsageStatisticsKey(const QString& key)
{
    return key.startsWith(QStringLiteral("eid:")) || key.startsWith(QStringLiteral("path:"));
}

UsageStatisticsDurationParts mu::appshell::usageStatisticsDurationParts(qint64 milliseconds)
{
    const qint64 totalMinutes = std::max<qint64>(milliseconds, 0) / MILLISECONDS_PER_MINUTE;
    return { totalMinutes / MINUTES_PER_HOUR, static_cast<int>(totalMinutes % MINUTES_PER_HOUR) };
}

UsageStatisticsDataResult mu::appshell::decodeUsageStatisticsData(const QByteArray& bytes)
{
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        return {};
    }

    const QJsonObject object = document.object();
    if (object.value(QStringLiteral("version")).toInt(-1) != USAGE_STATISTICS_VERSION) {
        return {};
    }

    UsageStatisticsData data;
    data.generation = object.value(QStringLiteral("generation")).toString();
    data.totalActiveMilliseconds = nonNegativeInteger(object.value(QStringLiteral("totalActiveMs")));

    const QJsonValue aliasesValue = object.value(QStringLiteral("aliases"));
    if (aliasesValue.isObject()) {
        const QJsonObject aliases = aliasesValue.toObject();
        for (auto it = aliases.constBegin(); it != aliases.constEnd(); ++it) {
            const QString toKey = it.value().toString();
            if (isPersistentUsageStatisticsKey(it.key()) && isPersistentUsageStatisticsKey(toKey)
                && it.key() != toKey) {
                data.scoreAliases.insert(it.key(), toKey);
            }
        }
    }

    const QJsonValue scoresValue = object.value(QStringLiteral("scores"));
    if (scoresValue.isObject()) {
        const QJsonObject scores = scoresValue.toObject();
        for (auto it = scores.constBegin(); it != scores.constEnd(); ++it) {
            if (it.key().isEmpty() || !isPersistentUsageStatisticsKey(it.key())) {
                continue;
            }

            const qint64 milliseconds = nonNegativeInteger(it.value());
            if (milliseconds > 0) {
                data.scoreActiveMilliseconds.insert(it.key(), milliseconds);
            }
        }
    }

    const QHash<QString, qint64> unnormalizedScores = data.scoreActiveMilliseconds;
    data.scoreActiveMilliseconds.clear();
    for (auto it = unnormalizedScores.constBegin(); it != unnormalizedScores.constEnd(); ++it) {
        const QString resolvedKey = resolveUsageStatisticsScoreKey(data, it.key());
        data.scoreActiveMilliseconds[resolvedKey]
            = usageStatisticsSaturatedAdd(data.scoreActiveMilliseconds.value(resolvedKey), it.value());
    }

    return { UsageStatisticsDataStatus::Valid, data };
}

QByteArray mu::appshell::encodeUsageStatisticsData(const UsageStatisticsData& data)
{
    QJsonObject scores;
    for (auto it = data.scoreActiveMilliseconds.constBegin(); it != data.scoreActiveMilliseconds.constEnd(); ++it) {
        if (!isPersistentUsageStatisticsKey(it.key())) {
            continue;
        }

        const qint64 milliseconds = std::max<qint64>(it.value(), 0);
        if (milliseconds > 0) {
            scores.insert(it.key(), integerJsonValue(milliseconds));
        }
    }

    QJsonObject aliases;
    for (auto it = data.scoreAliases.constBegin(); it != data.scoreAliases.constEnd(); ++it) {
        const QString resolvedKey = resolveUsageStatisticsScoreKey(data, it.value());
        if (isPersistentUsageStatisticsKey(it.key()) && isPersistentUsageStatisticsKey(resolvedKey)
            && it.key() != resolvedKey) {
            aliases.insert(it.key(), resolvedKey);
        }
    }

    QJsonObject object;
    object.insert(QStringLiteral("version"), USAGE_STATISTICS_VERSION);
    object.insert(QStringLiteral("generation"), data.generation);
    object.insert(QStringLiteral("totalActiveMs"), integerJsonValue(std::max<qint64>(data.totalActiveMilliseconds, 0)));
    object.insert(QStringLiteral("scores"), scores);
    object.insert(QStringLiteral("aliases"), aliases);

    return QJsonDocument(object).toJson(QJsonDocument::Indented);
}

void mu::appshell::migrateUsageStatisticsScore(UsageStatisticsData& data, const QString& fromKey, const QString& toKey)
{
    if (fromKey.isEmpty() || toKey.isEmpty() || fromKey == toKey) {
        return;
    }

    if (!isPersistentUsageStatisticsKey(fromKey) || !isPersistentUsageStatisticsKey(toKey)) {
        return;
    }

    const QString resolvedFromKey = resolveUsageStatisticsScoreKey(data, fromKey);
    const QString resolvedToKey = resolveUsageStatisticsScoreKey(data, toKey);
    if (resolvedFromKey == resolvedToKey) {
        data.scoreAliases.insert(fromKey, resolvedToKey);
        return;
    }

    const qint64 milliseconds = data.scoreActiveMilliseconds.take(resolvedFromKey);
    data.scoreActiveMilliseconds[resolvedToKey]
        = usageStatisticsSaturatedAdd(data.scoreActiveMilliseconds.value(resolvedToKey), milliseconds);
    data.scoreAliases.insert(fromKey, resolvedToKey);
    if (resolvedFromKey != fromKey) {
        data.scoreAliases.insert(resolvedFromKey, resolvedToKey);
    }
}

void mu::appshell::applyUsageStatisticsMigrations(UsageStatisticsData& data,
                                                  const QVector<UsageStatisticsMigration>& migrations)
{
    for (const UsageStatisticsMigration& migration : migrations) {
        migrateUsageStatisticsScore(data, migration.fromKey, migration.toKey);
    }
}

void mu::appshell::mergeUsageStatisticsDeltas(UsageStatisticsData& data, qint64 totalDelta,
                                              const QHash<QString, qint64>& scoreDeltas)
{
    data.totalActiveMilliseconds = usageStatisticsSaturatedAdd(data.totalActiveMilliseconds, totalDelta);

    for (auto it = scoreDeltas.constBegin(); it != scoreDeltas.constEnd(); ++it) {
        if (!isPersistentUsageStatisticsKey(it.key())) {
            continue;
        }

        const QString resolvedKey = resolveUsageStatisticsScoreKey(data, it.key());
        data.scoreActiveMilliseconds[resolvedKey]
            = usageStatisticsSaturatedAdd(data.scoreActiveMilliseconds.value(resolvedKey), it.value());
    }
}

QString mu::appshell::resolveUsageStatisticsScoreKey(const UsageStatisticsData& data, const QString& key)
{
    QString resolvedKey = key;
    QSet<QString> visitedKeys;

    while (data.scoreAliases.contains(resolvedKey) && !visitedKeys.contains(resolvedKey)) {
        visitedKeys.insert(resolvedKey);
        resolvedKey = data.scoreAliases.value(resolvedKey);
    }

    if (visitedKeys.contains(resolvedKey)) {
        return key;
    }

    return resolvedKey;
}

qint64 mu::appshell::usageStatisticsScoreMilliseconds(const UsageStatisticsData& data, const QString& key)
{
    return data.scoreActiveMilliseconds.value(resolveUsageStatisticsScoreKey(data, key));
}

void UsageStatisticsAccumulator::setActive(bool active, qint64 nowMilliseconds)
{
    if (m_active == active) {
        return;
    }

    checkpoint(nowMilliseconds);
    m_active = active;
    m_segmentStartMilliseconds = nowMilliseconds;
}

void UsageStatisticsAccumulator::setCurrentScoreKey(const QString& key, qint64 nowMilliseconds)
{
    if (m_currentScoreKey == key) {
        return;
    }

    checkpoint(nowMilliseconds);
    m_currentScoreKey = key;
}

void UsageStatisticsAccumulator::migrateScoreKey(const QString& fromKey, const QString& toKey, qint64 nowMilliseconds)
{
    if (fromKey.isEmpty() || toKey.isEmpty() || fromKey == toKey) {
        return;
    }

    checkpoint(nowMilliseconds);

    const qint64 milliseconds = m_pendingScoreMilliseconds.take(fromKey);
    if (milliseconds > 0) {
        m_pendingScoreMilliseconds[toKey]
            = usageStatisticsSaturatedAdd(m_pendingScoreMilliseconds.value(toKey), milliseconds);
    }

    if (m_currentScoreKey == fromKey) {
        m_currentScoreKey = toKey;
    }
}

void UsageStatisticsAccumulator::checkpoint(qint64 nowMilliseconds)
{
    const qint64 elapsed = liveSegmentMilliseconds(nowMilliseconds);
    if (elapsed <= 0) {
        if (m_active && nowMilliseconds > m_segmentStartMilliseconds) {
            m_segmentStartMilliseconds = nowMilliseconds;
        }
        return;
    }

    m_pendingTotalMilliseconds = usageStatisticsSaturatedAdd(m_pendingTotalMilliseconds, elapsed);
    if (!m_currentScoreKey.isEmpty()) {
        m_pendingScoreMilliseconds[m_currentScoreKey]
            = usageStatisticsSaturatedAdd(m_pendingScoreMilliseconds.value(m_currentScoreKey), elapsed);
    }

    m_segmentStartMilliseconds = nowMilliseconds;
}

void UsageStatisticsAccumulator::reset(qint64 nowMilliseconds)
{
    m_pendingTotalMilliseconds = 0;
    m_pendingScoreMilliseconds.clear();
    m_segmentStartMilliseconds = nowMilliseconds;
}

qint64 UsageStatisticsAccumulator::pendingTotalMilliseconds(qint64 nowMilliseconds) const
{
    return usageStatisticsSaturatedAdd(m_pendingTotalMilliseconds, liveSegmentMilliseconds(nowMilliseconds));
}

qint64 UsageStatisticsAccumulator::pendingScoreMilliseconds(const QString& key, qint64 nowMilliseconds) const
{
    qint64 milliseconds = m_pendingScoreMilliseconds.value(key);
    if (m_active && m_currentScoreKey == key) {
        milliseconds = usageStatisticsSaturatedAdd(milliseconds, liveSegmentMilliseconds(nowMilliseconds));
    }

    return milliseconds;
}

const QHash<QString, qint64>& UsageStatisticsAccumulator::pendingScoreMilliseconds() const
{
    return m_pendingScoreMilliseconds;
}

void UsageStatisticsAccumulator::clearPersistedDeltas()
{
    m_pendingTotalMilliseconds = 0;

    for (auto it = m_pendingScoreMilliseconds.begin(); it != m_pendingScoreMilliseconds.end();) {
        if (isPersistentUsageStatisticsKey(it.key())) {
            it = m_pendingScoreMilliseconds.erase(it);
        } else {
            ++it;
        }
    }
}

void UsageStatisticsAccumulator::discardScoreKey(const QString& key)
{
    m_pendingScoreMilliseconds.remove(key);
}

bool UsageStatisticsAccumulator::active() const
{
    return m_active;
}

const QString& UsageStatisticsAccumulator::currentScoreKey() const
{
    return m_currentScoreKey;
}

qint64 UsageStatisticsAccumulator::liveSegmentMilliseconds(qint64 nowMilliseconds) const
{
    if (!m_active || nowMilliseconds <= m_segmentStartMilliseconds) {
        return 0;
    }

    return nowMilliseconds - m_segmentStartMilliseconds;
}
