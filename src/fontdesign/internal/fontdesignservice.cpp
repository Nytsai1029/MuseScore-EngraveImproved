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
#include "fontdesignservice.h"

#include <QFile>
#include <QFileInfo>

#include "io/fontexporter.h"
#include "io/metadatawriter.h"
#include "io/projectfiles.h"

#include "log.h"

using namespace mu::fontdesign;
using namespace muse;

Ret FontDesignService::openProject(const io::path_t& fontPath)
{
    if (!m_smuflDb.isInited()) {
        m_smuflDb.init();
    }

    io::path_t metadataPath = ProjectFiles::findMetadataFor(fontPath);

    FontDesignProjectPtr project = std::make_shared<FontDesignProject>();
    Ret ret = project->load(fontPath, metadataPath, m_smuflDb);
    if (!ret) {
        return ret;
    }

    m_project = project;
    configuration()->setLastOpenedFontPath(fontPath);
    configuration()->prependRecentFontPath(fontPath);
    m_currentProjectChanged.notify();

    return make_ok();
}

Ret FontDesignService::newProject(const NewFontParams& params)
{
    if (!m_smuflDb.isInited()) {
        m_smuflDb.init();
    }

    FontDesignProjectPtr project = std::make_shared<FontDesignProject>();
    Ret ret = project->createNew(params, m_smuflDb);
    if (!ret) {
        return ret;
    }

    m_project = project;
    configuration()->setLastOpenedFontPath(project->fontPath());
    m_currentProjectChanged.notify();

    return make_ok();
}

Ret FontDesignService::saveTargets(SaveTargets& targets) const
{
    targets = SaveTargets();

    if (!m_project) {
        return make_ret(Ret::Code::UnknownError, std::string("no project to save"));
    }

    return ProjectFiles::saveTargets(*m_project, targets);
}

Ret FontDesignService::saveProject(SaveReport& report)
{
    report = SaveReport();

    if (!m_project) {
        return make_ret(Ret::Code::UnknownError, std::string("no project to save"));
    }

    //! 文件名跟随字体名：保存即写 <fontName>.otf + <fontName>.json；
    //! 名称变化等效重命名（旧文件移入废纸篓，可恢复）
    SaveTargets targets;
    Ret targetsRet = ProjectFiles::saveTargets(*m_project, targets);
    if (!targetsRet) {
        return targetsRet;
    }

    const io::path_t oldFontPath = m_project->fontPath();
    const io::path_t oldMetaPath = m_project->metadataPath();
    const bool hadFilesOnDisk = !m_project->neverSaved();

    //! 先在内存里生成并校验字体：这一步失败时磁盘上什么都不动
    FontExporter::Report exportReport;
    std::vector<uint8_t> fontBytes;
    Ret buildRet = FontExporter::buildFontBytes(*m_project, fontBytes, &exportReport);
    if (!buildRet) {
        return buildRet;
    }

    //! 外来字体：保存是按轮廓重建整个字体（布局特性、hinting、未编码字形等都不保留），
    //! 首次覆盖前把原字体与原元数据各留一份 .bak；备份不成则不覆盖
    if (hadFilesOnDisk && m_project->backupBeforeOverwrite()) {
        for (const io::path_t& original : { oldFontPath, oldMetaPath }) {
            io::path_t backupPath;
            Ret backupRet = ProjectFiles::backupOriginal(original, backupPath);
            if (!backupRet) {
                return backupRet;
            }
            if (!backupPath.empty()) {
                report.backups.push_back(backupPath);
            }
        }
    }

    Ret fontRet = FontExporter::writeFontBytes(fontBytes, targets.fontPath);
    if (!fontRet) {
        return fontRet;
    }

    Ret metaRet = MetadataWriter::write(*m_project, targets.metadataPath);
    if (!metaRet) {
        return metaRet;
    }

    //! 重命名迁移：旧文件移入废纸篓（大小写不敏感文件系统上同一文件时跳过；失败不阻塞保存）
    auto trashOldFile = [](const io::path_t& oldPath, const io::path_t& newPath) {
        if (oldPath.empty() || oldPath == newPath) {
            return;
        }
        if (!QFileInfo::exists(oldPath.toQString()) || ProjectFiles::isSameFile(oldPath, newPath)) {
            return;
        }
        QFile::moveToTrash(oldPath.toQString());
    };
    if (hadFilesOnDisk) {
        trashOldFile(oldFontPath, targets.fontPath);
        trashOldFile(oldMetaPath, targets.metadataPath);
    }

    m_project->setFontPath(targets.fontPath);
    m_project->setMetadataPath(targets.metadataPath);
    configuration()->setLastOpenedFontPath(targets.fontPath);
    configuration()->prependRecentFontPath(targets.fontPath);

    m_project->undoStack().markClean();
    m_project->setNeverSaved(false);
    m_project->setBackupBeforeOverwrite(false);    // 磁盘上现在是本模块写出的字体
    report.warnings = exportReport.warnings;

    return make_ok();
}

void FontDesignService::closeProject()
{
    if (!m_project) {
        return;
    }

    m_project.reset();
    m_currentProjectChanged.notify();
}

FontDesignProjectPtr FontDesignService::currentProject() const
{
    return m_project;
}

bool FontDesignService::hasCurrentProject() const
{
    return m_project != nullptr;
}

muse::async::Notification FontDesignService::currentProjectChanged() const
{
    return m_currentProjectChanged;
}

const SmuflDatabase& FontDesignService::smuflDatabase() const
{
    if (!m_smuflDb.isInited()) {
        m_smuflDb.init();
    }

    return m_smuflDb;
}

void FontDesignService::setActiveEditSurface(IFontDesignEditSurface* surface)
{
    m_editSurface = surface;
}

IFontDesignEditSurface* FontDesignService::activeEditSurface() const
{
    return m_editSurface;
}
