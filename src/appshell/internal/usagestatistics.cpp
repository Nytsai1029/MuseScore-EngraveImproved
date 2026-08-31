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

#include "usagestatistics.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include <QGuiApplication>
#include <QSaveFile>
#include <QUuid>
#include <QWindow>

#include "engraving/dom/masterscore.h"
#include "multiinstances/resourcelockguard.h"

#include "log.h"

using namespace mu::appshell;

namespace {
constexpr int CHECKPOINT_INTERVAL_MILLISECONDS = 60 * 1000;
const std::string USAGE_STATISTICS_RESOURCE_NAME = "USAGE_STATISTICS";

QString newGeneration()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}
}

UsageStatistics::UsageStatistics(const muse::modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx)
{
}

void UsageStatistics::init()
{
    if (m_initialized) {
        return;
    }

    m_initialized = true;
    m_clock.start();

    {
        muse::mi::ReadResourceLockGuard guard(multiInstancesProvider.get(), USAGE_STATISTICS_RESOURCE_NAME);
        const DiskDataResult result = readDiskData();
        if (result.status == DiskDataStatus::Valid) {
            m_persistedData = result.data;
            m_hasPersistedData = true;
        } else if (result.status == DiskDataStatus::Corrupt) {
            LOGE() << "Usage statistics file is corrupt; it will be backed up at the next checkpoint";
        } else if (result.status == DiskDataStatus::ReadError) {
            LOGE() << "Failed to read usage statistics";
        }
    }

    bindCurrentProject(globalContext()->currentProject());

    globalContext()->currentProjectChanged().onNotify(this, [this]() {
        bindCurrentProject(globalContext()->currentProject());
        flush();
    });

    appShellConfiguration()->factorySettingsReverted().onNotify(this, [this]() {
        resetStatistics();
    });

    multiInstancesProvider()->resourceChanged().onReceive(this, [this](const std::string& resourceName) {
        if (resourceName == USAGE_STATISTICS_RESOURCE_NAME) {
            reloadFromDisk();
        }
    });

    QObject::connect(qApp, &QGuiApplication::applicationStateChanged, this, [this]() {
        updateActiveState();
    });
    qApp->installEventFilter(this);

    m_checkpointTimer.setInterval(CHECKPOINT_INTERVAL_MILLISECONDS);
    QObject::connect(&m_checkpointTimer, &QTimer::timeout, this, [this]() {
        flush();
    });
    m_checkpointTimer.start();

    updateActiveState();
}

void UsageStatistics::deinit()
{
    if (!m_initialized) {
        return;
    }

    m_checkpointTimer.stop();
    m_accumulator.setActive(false, nowMilliseconds());
    flush();

    qApp->removeEventFilter(this);
    QObject::disconnect(qApp, nullptr, this, nullptr);
    m_initialized = false;
}

UsageStatisticsSnapshot UsageStatistics::snapshot() const
{
    UsageStatisticsData effectiveData = m_persistedData;
    applyUsageStatisticsMigrations(effectiveData, m_pendingMigrations);

    const qint64 now = nowMilliseconds();

    UsageStatisticsSnapshot result;
    result.totalActiveMilliseconds = usageStatisticsSaturatedAdd(
        effectiveData.totalActiveMilliseconds, m_accumulator.pendingTotalMilliseconds(now));
    result.currentScoreActiveMilliseconds = usageStatisticsSaturatedAdd(
        usageStatisticsScoreMilliseconds(effectiveData, m_currentScoreKey),
        m_accumulator.pendingScoreMilliseconds(m_currentScoreKey, now));
    result.currentScoreName = m_currentScoreName;
    result.hasCurrentScore = static_cast<bool>(m_currentProject);
    return result;
}

muse::async::Notification UsageStatistics::statisticsChanged() const
{
    return m_statisticsChanged;
}

bool UsageStatistics::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::ApplicationDeactivate) {
        setAccumulationActive(false);
    } else if (event->type() == QEvent::ApplicationActivate
               || event->type() == QEvent::ApplicationStateChange) {
        QTimer::singleShot(0, this, [this]() {
            updateActiveState();
        });
    }

    if (watched && watched->isWindowType()) {
        switch (event->type()) {
        case QEvent::Show:
        case QEvent::Hide:
        case QEvent::Close:
        case QEvent::WindowStateChange:
            QTimer::singleShot(0, this, [this]() {
                updateActiveState();
            });
            break;
        default:
            break;
        }
    }

    return QObject::eventFilter(watched, event);
}

qint64 UsageStatistics::nowMilliseconds() const
{
    return m_clock.isValid() ? m_clock.elapsed() : 0;
}

bool UsageStatistics::shouldAccumulate() const
{
    if (QGuiApplication::applicationState() != Qt::ApplicationActive) {
        return false;
    }

    for (const QWindow* window : QGuiApplication::topLevelWindows()) {
        if (window->isVisible() && window->visibility() != QWindow::Minimized) {
            return true;
        }
    }

    return false;
}

void UsageStatistics::updateActiveState()
{
    setAccumulationActive(shouldAccumulate());
}

void UsageStatistics::setAccumulationActive(bool active)
{
    if (m_accumulator.active() == active) {
        return;
    }

    m_accumulator.setActive(active, nowMilliseconds());
    m_statisticsChanged.notify();

    if (!active) {
        flush();
    }
}

void UsageStatistics::bindCurrentProject(const project::INotationProjectPtr& project)
{
    const qint64 now = nowMilliseconds();
    const QString oldKey = m_currentScoreKey;

    m_accumulator.setCurrentScoreKey(QString(), now);
    if (!oldKey.isEmpty() && !isPersistentUsageStatisticsKey(oldKey)) {
        m_accumulator.discardScoreKey(oldKey);
    }

    m_currentProject = project;
    m_currentScoreKey.clear();
    m_currentScoreName.clear();
    m_runtimeScoreKey.clear();

    if (m_currentProject) {
        m_currentScoreName = m_currentProject->displayName();
        m_currentScoreKey = scoreKey(m_currentProject);
        if (m_currentScoreKey.isEmpty()) {
            m_runtimeScoreKey = QStringLiteral("runtime:") + newGeneration();
            m_currentScoreKey = m_runtimeScoreKey;
        }

        m_accumulator.setCurrentScoreKey(m_currentScoreKey, now);

        const std::weak_ptr<project::INotationProject> weakProject(m_currentProject);
        m_currentProject->pathChanged().onNotify(this, [this, weakProject]() {
            if (weakProject.lock() == m_currentProject) {
                refreshCurrentProjectIdentity();
            }
        });
        m_currentProject->displayNameChanged().onNotify(this, [this, weakProject]() {
            if (weakProject.lock() == m_currentProject) {
                m_currentScoreName = m_currentProject->displayName();
                m_statisticsChanged.notify();
            }
        });
        m_currentProject->saveComplited().onReceive(
            this, [this, weakProject](const muse::io::path_t&, project::SaveMode) {
                if (weakProject.lock() == m_currentProject) {
                    refreshCurrentProjectIdentity();
                    flush();
                }
            });
    }

    m_statisticsChanged.notify();
}

void UsageStatistics::refreshCurrentProjectIdentity()
{
    if (!m_currentProject) {
        return;
    }

    QString newKey = scoreKey(m_currentProject);
    if (newKey.isEmpty()) {
        newKey = m_runtimeScoreKey;
    }

    if (newKey == m_currentScoreKey || newKey.isEmpty()) {
        return;
    }

    const qint64 now = nowMilliseconds();
    const QString oldKey = m_currentScoreKey;
    m_accumulator.migrateScoreKey(oldKey, newKey, now);
    if (isPersistentUsageStatisticsKey(oldKey) && isPersistentUsageStatisticsKey(newKey)) {
        m_pendingMigrations.push_back({ oldKey, newKey });
    }

    m_currentScoreKey = newKey;
    m_runtimeScoreKey.clear();
    m_statisticsChanged.notify();
}

QString UsageStatistics::scoreKey(const project::INotationProjectPtr& project) const
{
    if (!project) {
        return {};
    }

    if (project->isNewlyCreated() && project->path().empty()) {
        return {};
    }

    const notation::IMasterNotationPtr masterNotation = project->masterNotation();
    const engraving::MasterScore* score = masterNotation ? masterNotation->masterScore() : nullptr;
    if (score) {
        const engraving::EID eid = score->eid();
        if (eid.isValid()) {
            return QStringLiteral("eid:") + QString::fromStdString(eid.toStdString());
        }
    }

    return pathScoreKey(project->path());
}

QString UsageStatistics::pathScoreKey(const muse::io::path_t& path) const
{
    if (path.empty()) {
        return {};
    }

    muse::io::path_t normalizedPath = fileSystem()->canonicalFilePath(path);
    if (normalizedPath.empty()) {
        normalizedPath = fileSystem()->absoluteFilePath(path);
    }

    const QString cleanPath = QDir::cleanPath(normalizedPath.toQString());
    if (cleanPath.isEmpty()) {
        return {};
    }

    const QByteArray hash = QCryptographicHash::hash(cleanPath.toUtf8(), QCryptographicHash::Sha256).toHex();
    return QStringLiteral("path:") + QString::fromLatin1(hash);
}

muse::io::path_t UsageStatistics::statisticsFilePath() const
{
    return globalConfiguration()->userAppDataPath() + "/usage-statistics.json";
}

UsageStatistics::DiskDataResult UsageStatistics::readDiskData() const
{
    const muse::io::path_t path = statisticsFilePath();
    if (!fileSystem()->exists(path)) {
        return { DiskDataStatus::Missing, {} };
    }

    const muse::RetVal<muse::ByteArray> bytes = fileSystem()->readFile(path);
    if (!bytes.ret) {
        LOGE() << "Failed to read usage statistics from " << path << ": " << bytes.ret.toString();
        return { DiskDataStatus::ReadError, {} };
    }

    const UsageStatisticsDataResult result = decodeUsageStatisticsData(bytes.val.toQByteArray());
    if (result.status == UsageStatisticsDataStatus::Corrupt) {
        return { DiskDataStatus::Corrupt, {} };
    }

    return { DiskDataStatus::Valid, result.data };
}

bool UsageStatistics::writeDiskData(const UsageStatisticsData& data) const
{
    const muse::Ret makePathResult = fileSystem()->makePath(globalConfiguration()->userAppDataPath());
    if (!makePathResult) {
        LOGE() << "Failed to create usage statistics directory: " << makePathResult.toString();
        return false;
    }

    QSaveFile file(statisticsFilePath().toQString());
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly)) {
        LOGE() << "Failed to open usage statistics for writing: " << file.errorString();
        return false;
    }

    const QByteArray bytes = encodeUsageStatisticsData(data);
    if (file.write(bytes) != bytes.size()) {
        LOGE() << "Failed to write usage statistics: " << file.errorString();
        file.cancelWriting();
        return false;
    }

    if (!file.commit()) {
        LOGE() << "Failed to commit usage statistics: " << file.errorString();
        return false;
    }

    return true;
}

bool UsageStatistics::backupCorruptData() const
{
    const muse::io::path_t source = statisticsFilePath();
    const QString timestamp = QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"));
    const muse::io::path_t backup = globalConfiguration()->userAppDataPath()
                                    + (QStringLiteral("/usage-statistics.corrupt-") + timestamp
                                       + QStringLiteral("-") + newGeneration() + QStringLiteral(".json"));
    const muse::Ret result = fileSystem()->move(source, backup, false);
    if (!result) {
        LOGE() << "Failed to preserve corrupt usage statistics: " << result.toString();
        return false;
    }

    LOGW() << "Preserved corrupt usage statistics at " << backup;
    return true;
}

void UsageStatistics::reloadFromDisk()
{
    if (!m_initialized || m_resetPending) {
        return;
    }

    muse::mi::ReadResourceLockGuard guard(multiInstancesProvider.get(), USAGE_STATISTICS_RESOURCE_NAME);
    const DiskDataResult result = readDiskData();
    if (result.status != DiskDataStatus::Valid) {
        return;
    }

    const qint64 now = nowMilliseconds();
    applyExternalResetIfNeeded(result.data, now);
    m_persistedData = result.data;
    m_hasPersistedData = true;
    m_statisticsChanged.notify();
}

void UsageStatistics::flush()
{
    if (!m_initialized) {
        return;
    }

    const qint64 now = nowMilliseconds();
    m_accumulator.checkpoint(now);

    muse::mi::WriteResourceLockGuard guard(multiInstancesProvider.get(), USAGE_STATISTICS_RESOURCE_NAME);

    UsageStatisticsData data;
    if (m_resetPending) {
        data.generation = m_pendingResetGeneration;
    } else {
        const DiskDataResult result = readDiskData();
        if (result.status == DiskDataStatus::ReadError) {
            return;
        }

        if (result.status == DiskDataStatus::Corrupt) {
            if (!backupCorruptData()) {
                return;
            }
            if (m_hasPersistedData) {
                data = m_persistedData;
            }
        } else if (result.status == DiskDataStatus::Valid) {
            if (applyExternalResetIfNeeded(result.data, now)) {
                m_persistedData = result.data;
                m_hasPersistedData = true;
            }
            data = result.data;
        } else if (m_hasPersistedData) {
            data = m_persistedData;
        }
    }

    if (data.generation.isEmpty()) {
        data.generation = newGeneration();
    }

    applyUsageStatisticsMigrations(data, m_pendingMigrations);
    mergeUsageStatisticsDeltas(data, m_accumulator.pendingTotalMilliseconds(now),
                               m_accumulator.pendingScoreMilliseconds());

    if (!writeDiskData(data)) {
        return;
    }

    m_persistedData = data;
    m_hasPersistedData = true;
    m_pendingMigrations.clear();
    m_accumulator.clearPersistedDeltas();
    m_resetPending = false;
    m_pendingResetGeneration.clear();
    m_statisticsChanged.notify();
}

void UsageStatistics::resetStatistics()
{
    if (!m_initialized) {
        return;
    }

    const qint64 now = nowMilliseconds();
    m_accumulator.reset(now);
    m_pendingMigrations.clear();

    m_pendingResetGeneration = newGeneration();
    m_resetPending = true;
    m_persistedData = UsageStatisticsData();
    m_persistedData.generation = m_pendingResetGeneration;
    m_hasPersistedData = true;

    m_statisticsChanged.notify();
    flush();
}

bool UsageStatistics::applyExternalResetIfNeeded(const UsageStatisticsData& diskData, qint64 now)
{
    if (m_persistedData.generation.isEmpty() || diskData.generation.isEmpty()
        || m_persistedData.generation == diskData.generation) {
        return false;
    }

    m_accumulator.reset(now);
    m_pendingMigrations.clear();
    m_resetPending = false;
    m_pendingResetGeneration.clear();
    return true;
}
