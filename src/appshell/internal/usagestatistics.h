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
#include <QElapsedTimer>
#include <QTimer>

#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "global/iglobalconfiguration.h"
#include "global/io/ifilesystem.h"
#include "modularity/ioc.h"
#include "multiinstances/imultiinstancesprovider.h"

#include "iappshellconfiguration.h"
#include "iusagestatistics.h"
#include "usagestatisticsdata.h"

namespace mu::appshell {
class UsageStatistics : public QObject, public IUsageStatistics, public muse::Injectable, public muse::async::Asyncable
{
    muse::Inject<muse::IGlobalConfiguration> globalConfiguration = { this };
    muse::Inject<muse::io::IFileSystem> fileSystem = { this };
    muse::Inject<muse::mi::IMultiInstancesProvider> multiInstancesProvider = { this };
    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<IAppShellConfiguration> appShellConfiguration = { this };

public:
    explicit UsageStatistics(const muse::modularity::ContextPtr& iocCtx);

    void init();
    void deinit();

    UsageStatisticsSnapshot snapshot() const override;
    muse::async::Notification statisticsChanged() const override;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    enum class DiskDataStatus {
        Missing,
        Valid,
        Corrupt,
        ReadError
    };

    struct DiskDataResult {
        DiskDataStatus status = DiskDataStatus::ReadError;
        UsageStatisticsData data;
    };

    qint64 nowMilliseconds() const;
    bool shouldAccumulate() const;
    void setAccumulationActive(bool active);
    void updateActiveState();

    void bindCurrentProject(const project::INotationProjectPtr& project);
    void refreshCurrentProjectIdentity();
    QString scoreKey(const project::INotationProjectPtr& project) const;
    QString pathScoreKey(const muse::io::path_t& path) const;

    muse::io::path_t statisticsFilePath() const;
    DiskDataResult readDiskData() const;
    bool writeDiskData(const UsageStatisticsData& data) const;
    bool backupCorruptData() const;

    void reloadFromDisk();
    void flush();
    void resetStatistics();
    bool applyExternalResetIfNeeded(const UsageStatisticsData& diskData, qint64 now);

    QElapsedTimer m_clock;
    QTimer m_checkpointTimer;
    UsageStatisticsAccumulator m_accumulator;
    UsageStatisticsData m_persistedData;
    QVector<UsageStatisticsMigration> m_pendingMigrations;

    project::INotationProjectPtr m_currentProject;
    QString m_currentScoreKey;
    QString m_currentScoreName;
    QString m_runtimeScoreKey;

    QString m_pendingResetGeneration;
    bool m_resetPending = false;
    bool m_hasPersistedData = false;
    bool m_initialized = false;

    muse::async::Notification m_statisticsChanged;
};
}
