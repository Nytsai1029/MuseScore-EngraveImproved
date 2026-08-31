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

#include <QObject>
#include <QTimer>

#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "iusagestatistics.h"

namespace mu::appshell {
class UsageStatisticsModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString totalUsageTime READ totalUsageTime NOTIFY statisticsChanged)
    Q_PROPERTY(QString currentScoreUsageTime READ currentScoreUsageTime NOTIFY statisticsChanged)
    Q_PROPERTY(QString currentScoreName READ currentScoreName NOTIFY statisticsChanged)
    Q_PROPERTY(bool hasCurrentScore READ hasCurrentScore NOTIFY statisticsChanged)

    muse::Inject<IUsageStatistics> usageStatistics = { this };

public:
    explicit UsageStatisticsModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    QString totalUsageTime() const;
    QString currentScoreUsageTime() const;
    QString currentScoreName() const;
    bool hasCurrentScore() const;

signals:
    void statisticsChanged();

private:
    static QString formatDuration(qint64 milliseconds);
    void refresh();

    QTimer m_refreshTimer;
    QString m_totalUsageTime;
    QString m_currentScoreUsageTime;
    QString m_currentScoreName;
    bool m_hasCurrentScore = false;
    bool m_loaded = false;
};
}
