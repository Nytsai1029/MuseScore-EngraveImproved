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
#include "newfontmodel.h"

#include <QDir>
#include <QRegularExpression>
#include <QStandardPaths>

#include "io/fileinfo.h"
#include "translation.h"

using namespace mu::fontdesign;
using namespace muse;

NewFontModel::NewFontModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

QString NewFontModel::defaultFolder() const
{
    io::path_t last = configuration()->lastOpenedFontPath();
    if (!last.empty()) {
        return io::FileInfo(last).dirPath().toQString();
    }
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
}

QString NewFontModel::create(const QString& name, const QString& version, int upem,
                             const QString& copyright, const QString& folder)
{
    const QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty()) {
        return qtrc("fontdesign", "Font name is required");
    }
    //! 名称同时用作文件名与安装目录名：禁掉路径分隔与非法文件名字符
    static const QRegularExpression invalidChars(QStringLiteral("[\\\\/:*?\"<>|]"));
    if (trimmedName.contains(invalidChars)) {
        return qtrc("fontdesign", "Font name contains invalid characters");
    }

    const QString trimmedFolder = folder.trimmed();
    if (trimmedFolder.isEmpty()) {
        return qtrc("fontdesign", "Choose a folder to save the font in");
    }
    if (!QDir(trimmedFolder).exists()) {
        return qtrc("fontdesign", "The chosen folder does not exist");
    }

    bool versionOk = false;
    double versionValue = version.trimmed().toDouble(&versionOk);
    if (!versionOk || versionValue <= 0) {
        versionValue = 1.0;
    }

    NewFontParams params;
    params.fontName = trimmedName.toStdString();
    params.fontVersion = versionValue;
    params.upem = static_cast<double>(upem);
    params.copyright = copyright.trimmed().toStdString();
    params.folder = io::path_t(trimmedFolder);

    Ret ret = fontDesignService()->newProject(params);
    if (!ret) {
        return QString::fromStdString(ret.text());
    }

    return QString();
}
