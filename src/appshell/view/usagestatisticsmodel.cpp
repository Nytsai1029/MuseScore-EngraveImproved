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

#include "usagestatisticsmodel.h"

#include "internal/usagestatisticsdata.h"

#include "translation.h"

using namespace mu::appshell;

UsageStatisticsModel::UsageStatisticsModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
    m_refreshTimer.setInterval(1000);
    QObject::connect(&m_refreshTimer, &QTimer::timeout, this, [this]() {
        refresh();
    });
}

void UsageStatisticsModel::load()
{
    if (m_loaded) {
        return;
    }

    m_loaded = true;
    usageStatistics()->statisticsChanged().onNotify(this, [this]() {
        refresh();
    });

    m_refreshTimer.start();
    refresh();
}

QString UsageStatisticsModel::totalUsageTime() const
{
    return m_totalUsageTime;
}

QString UsageStatisticsModel::currentScoreUsageTime() const
{
    return m_currentScoreUsageTime;
}

QString UsageStatisticsModel::currentScoreName() const
{
    return m_currentScoreName;
}

bool UsageStatisticsModel::hasCurrentScore() const
{
    return m_hasCurrentScore;
}

QString UsageStatisticsModel::formatDuration(qint64 milliseconds)
{
    const UsageStatisticsDurationParts parts = usageStatisticsDurationParts(milliseconds);
    return muse::qtrc("appshell/statistics", "%1 hours %2 minutes")
           .arg(parts.hours)
           .arg(parts.minutes);
}

void UsageStatisticsModel::refresh()
{
    const UsageStatisticsSnapshot snapshot = usageStatistics()->snapshot();
    const QString totalUsageTime = formatDuration(snapshot.totalActiveMilliseconds);
    const QString currentScoreUsageTime = formatDuration(snapshot.currentScoreActiveMilliseconds);

    if (m_totalUsageTime == totalUsageTime
        && m_currentScoreUsageTime == currentScoreUsageTime
        && m_currentScoreName == snapshot.currentScoreName
        && m_hasCurrentScore == snapshot.hasCurrentScore) {
        return;
    }

    m_totalUsageTime = totalUsageTime;
    m_currentScoreUsageTime = currentScoreUsageTime;
    m_currentScoreName = snapshot.currentScoreName;
    m_hasCurrentScore = snapshot.hasCurrentScore;
    emit statisticsChanged();
}
