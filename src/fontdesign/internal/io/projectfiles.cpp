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
#include "projectfiles.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "io/fileinfo.h"
#include "serialization/json.h"

#include "../project/fontdesignproject.h"

#include "log.h"

using namespace mu::fontdesign;
using namespace muse;

static const QString BACKUP_SUFFIX(".bak");

std::string ProjectFiles::normalizedName(const std::string& name)
{
    QString result;
    for (const QChar ch : QString::fromStdString(name)) {
        if (ch == u' ' || ch == u'-' || ch == u'_') {
            continue;
        }
        result.append(ch.toLower());
    }
    return result.toStdString();
}

//! JSON 顶层的 fontName；读不了或不是对象时为空
static std::string fontNameOfMetadata(const QString& jsonPath)
{
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return std::string();
    }

    std::string err;
    JsonDocument doc = JsonDocument::fromJson(ByteArray::fromQByteArray(file.readAll()), &err);
    if (!err.empty() || !doc.isObject()) {
        return std::string();
    }

    return doc.rootObject().value("fontName").toStdString();
}

io::path_t ProjectFiles::findMetadataFor(const io::path_t& fontPath)
{
    const QFileInfo fontInfo(fontPath.toQString());
    const QDir dir = fontInfo.dir();
    const std::string fontKey = normalizedName(fontInfo.completeBaseName().toStdString());
    if (fontKey.empty()) {
        return io::path_t();
    }

    const QStringList jsonFiles = dir.entryList({ "*.json" }, QDir::Files, QDir::Name);

    // 1. 文件名对得上：<字体名>.json 优先于 <字体名>_metadata.json
    QString suffixMatch;
    QStringList others;
    for (const QString& fileName : jsonFiles) {
        const std::string key = normalizedName(QFileInfo(fileName).completeBaseName().toStdString());
        if (key == fontKey) {
            return io::path_t(dir.filePath(fileName));
        }
        if (key == fontKey + "metadata") {
            if (suffixMatch.isEmpty()) {
                suffixMatch = fileName;
            }
        } else {
            others << fileName;
        }
    }
    if (!suffixMatch.isEmpty()) {
        return io::path_t(dir.filePath(suffixMatch));
    }

    // 2. 文件名对不上（metadata.json、目录里唯一的 JSON…）：只有 JSON 自己声明的 fontName
    //    就是这个字体时才认领
    for (const QString& fileName : others) {
        const QString path = dir.filePath(fileName);
        if (normalizedName(fontNameOfMetadata(path)) == fontKey) {
            return io::path_t(path);
        }
    }

    if (!others.isEmpty()) {
        LOGI() << "no metadata paired with " << fontPath << " (" << others.size()
               << " unrelated JSON file(s) in the folder)";
    }

    return io::path_t();
}

bool ProjectFiles::isSameFile(const io::path_t& a, const io::path_t& b)
{
    if (a.empty() || b.empty()) {
        return false;
    }

    const QString canonicalA = QFileInfo(a.toQString()).canonicalFilePath();   // 文件不存在时为空
    return !canonicalA.isEmpty() && canonicalA == QFileInfo(b.toQString()).canonicalFilePath();
}

Ret ProjectFiles::saveTargets(const FontDesignProject& project, SaveTargets& out)
{
    out = SaveTargets();

    const io::path_t currentFontPath = project.fontPath();
    if (currentFontPath.empty()) {
        return make_ret(Ret::Code::UnknownError, std::string("no font file path associated with this project"));
    }

    io::FileInfo fontInfo(currentFontPath);
    std::string fontName = project.metadata().fontName;
    if (fontName.empty()) {
        fontName = fontInfo.baseName().toStdString();
    }
    if (fontName.find('/') != std::string::npos || fontName.find('\\') != std::string::npos
        || fontName.find(':') != std::string::npos) {
        return make_ret(Ret::Code::UnknownError, std::string("font name contains invalid path characters"));
    }

    const io::path_t dir = fontInfo.dirPath();
    out.fontPath = dir + "/" + fontName + ".otf";
    out.metadataPath = dir + "/" + fontName + ".json";

    //! 新建而未保存过的项目在磁盘上还没有自己的文件：目标位置上已有的都是别人的
    const bool hasOwnFiles = !project.neverSaved();

    if (QFile::exists(out.fontPath.toQString())
        && !(hasOwnFiles && isSameFile(out.fontPath, currentFontPath))) {
        out.foreignFiles.push_back(out.fontPath);
    }
    if (QFile::exists(out.metadataPath.toQString())
        && !(hasOwnFiles && isSameFile(out.metadataPath, project.metadataPath()))) {
        out.foreignFiles.push_back(out.metadataPath);
    }

    return make_ok();
}

Ret ProjectFiles::backupOriginal(const io::path_t& path, io::path_t& backupPath)
{
    backupPath = io::path_t();

    const QString source = path.toQString();
    if (path.empty() || !QFile::exists(source)) {
        return make_ok();
    }

    const QString backup = source + BACKUP_SUFFIX;
    if (QFile::exists(backup)) {
        return make_ok();
    }

    if (!QFile::copy(source, backup)) {
        return make_ret(Ret::Code::UnknownError,
                        std::string("cannot back up the original file: ") + path.toStdString());
    }

    backupPath = io::path_t(backup);
    LOGI() << "original kept as: " << backupPath;
    return make_ok();
}
