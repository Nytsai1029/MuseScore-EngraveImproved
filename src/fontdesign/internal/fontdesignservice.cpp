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

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "io/fileinfo.h"

#include "io/fontexporter.h"
#include "io/metadatawriter.h"

#include "log.h"

using namespace mu::fontdesign;
using namespace muse;

Ret FontDesignService::openProject(const io::path_t& fontPath)
{
    if (!m_smuflDb.isInited()) {
        m_smuflDb.init();
    }

    io::path_t metadataPath = findMetadataFor(fontPath);

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

Ret FontDesignService::saveProject(std::vector<std::string>& warnings)
{
    warnings.clear();

    if (!m_project) {
        return make_ret(Ret::Code::UnknownError, std::string("no project to save"));
    }

    const io::path_t oldFontPath = m_project->fontPath();
    if (oldFontPath.empty()) {
        return make_ret(Ret::Code::UnknownError, std::string("no font file path associated with this project"));
    }
    const io::path_t oldMetaPath = m_project->metadataPath();

    //! 文件名跟随字体名：保存即写 <fontName>.otf + <fontName>.json；
    //! 名称变化等效重命名（旧文件移入废纸篓，可恢复）
    io::FileInfo fontInfo(oldFontPath);
    std::string fontName = m_project->metadata().fontName;
    if (fontName.empty()) {
        fontName = fontInfo.baseName().toStdString();
    }
    if (fontName.find('/') != std::string::npos || fontName.find('\\') != std::string::npos
        || fontName.find(':') != std::string::npos) {
        return make_ret(Ret::Code::UnknownError, std::string("font name contains invalid path characters"));
    }

    const io::path_t dir = fontInfo.dirPath();
    const io::path_t fontPath = dir + "/" + fontName + ".otf";
    const io::path_t metaPath = dir + "/" + fontName + ".json";

    FontExporter::Report report;
    Ret fontRet = FontExporter::exportFont(*m_project, fontPath, &report);
    if (!fontRet) {
        return fontRet;
    }

    Ret metaRet = MetadataWriter::write(*m_project, metaPath);
    if (!metaRet) {
        return metaRet;
    }

    //! 重命名迁移：旧文件移入废纸篓（大小写不敏感文件系统上同一文件时跳过；失败不阻塞保存）
    auto trashOldFile = [](const io::path_t& oldPath, const io::path_t& newPath) {
        if (oldPath.empty() || oldPath == newPath) {
            return;
        }
        QFileInfo oldInfo(oldPath.toQString());
        if (!oldInfo.exists()) {
            return;
        }
        if (oldInfo.canonicalFilePath() == QFileInfo(newPath.toQString()).canonicalFilePath()) {
            return;
        }
        QFile::moveToTrash(oldPath.toQString());
    };
    trashOldFile(oldFontPath, fontPath);
    trashOldFile(oldMetaPath, metaPath);

    m_project->setFontPath(fontPath);
    m_project->setMetadataPath(metaPath);
    configuration()->setLastOpenedFontPath(fontPath);
    configuration()->prependRecentFontPath(fontPath);

    m_project->undoStack().markClean();
    m_project->setNeverSaved(false);
    warnings = report.warnings;

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

io::path_t FontDesignService::findMetadataFor(const io::path_t& fontPath) const
{
    io::FileInfo fontInfo(fontPath);
    QString fontBase = fontInfo.baseName().toQString().toLower();
    QDir dir(fontInfo.dirPath().toQString());

    QStringList jsonFiles = dir.entryList({ "*.json" }, QDir::Files, QDir::Name);
    if (jsonFiles.empty()) {
        return io::path_t();
    }

    // 优先级：<字体名>.json / <字体名>_metadata.json / metadata.json → 唯一 json → 首个
    QString exactMatch, metadataSuffixMatch, plainMetadata;
    for (const QString& fileName : jsonFiles) {
        QString base = io::FileInfo(io::path_t(fileName)).baseName().toQString().toLower();
        if (base == fontBase) {
            exactMatch = fileName;
        } else if (base == fontBase + "_metadata") {
            metadataSuffixMatch = fileName;
        } else if (base == "metadata") {
            plainMetadata = fileName;
        }
    }

    QString chosen;
    if (!exactMatch.isEmpty()) {
        chosen = exactMatch;
    } else if (!metadataSuffixMatch.isEmpty()) {
        chosen = metadataSuffixMatch;
    } else if (!plainMetadata.isEmpty()) {
        chosen = plainMetadata;
    } else if (jsonFiles.size() == 1) {
        chosen = jsonFiles.first();
    } else {
        chosen = jsonFiles.first();
        LOGW() << "multiple metadata candidates, picked: " << chosen;
    }

    return io::path_t(dir.filePath(chosen));
}

void FontDesignService::setActiveEditSurface(IFontDesignEditSurface* surface)
{
    m_editSurface = surface;
}

IFontDesignEditSurface* FontDesignService::activeEditSurface() const
{
    return m_editSurface;
}
