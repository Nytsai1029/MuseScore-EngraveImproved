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
#include "installedfontsmodel.h"

#include <QDir>
#include <QFile>

#include "translation.h"

using namespace mu::fontdesign;
using namespace muse;

InstalledFontsModel::InstalledFontsModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

int InstalledFontsModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_items.size());
}

QVariant InstalledFontsModel::data(const QModelIndex& index, int role) const
{
    if (!isValidRow(index.row())) {
        return QVariant();
    }

    const Item& item = m_items[index.row()];
    switch (role) {
    case NameRole: return item.name;
    case FontFileRole: return item.fontFile.toQString();
    case HasMetadataRole: return item.hasMetadata;
    default: return QVariant();
    }
}

QHash<int, QByteArray> InstalledFontsModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { FontFileRole, "fontFile" },
        { HasMetadataRole, "hasMetadata" },
    };
}

void InstalledFontsModel::load()
{
    beginResetModel();
    m_items.clear();

    //! 与 engravingfontscontroller 的扫描约定一致：
    //! userMusicFontsPath 下每个子目录 = 一个字体，目录内同名 .otf/.ttf + 任意 *.json
    QDir root(notationConfiguration()->userMusicFontsPath().toQString());
    const QStringList dirs = root.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    for (const QString& dirName : dirs) {
        QDir fontDir(root.filePath(dirName));

        Item item;
        item.name = dirName;
        item.dir = io::path_t(fontDir.absolutePath());

        const QStringList files = fontDir.entryList(QDir::Files);
        QString anyFont;
        for (const QString& fileName : files) {
            const QString lower = fileName.toLower();
            if (lower.endsWith(".otf") || lower.endsWith(".ttf")) {
                if (anyFont.isEmpty()) {
                    anyFont = fileName;
                }
                QFileInfo info(fileName);
                if (info.baseName().compare(dirName, Qt::CaseInsensitive) == 0) {
                    anyFont = fileName;
                    break;
                }
            }
        }
        for (const QString& fileName : files) {
            if (fileName.toLower().endsWith(".json")) {
                item.hasMetadata = true;
                break;
            }
        }

        if (anyFont.isEmpty()) {
            continue;
        }
        item.fontFile = io::path_t(fontDir.filePath(anyFont));
        m_items.push_back(item);
    }

    endResetModel();
    emit countChanged();
}

QString InstalledFontsModel::fontFileAt(int row) const
{
    if (!isValidRow(row)) {
        return QString();
    }
    return m_items[row].fontFile.toQString();
}

void InstalledFontsModel::openInNewWindow(int row)
{
    if (!isValidRow(row)) {
        return;
    }
    multiInstancesProvider()->openNewAppInstance({ "--fontdesign", m_items[row].fontFile.toQString() });
}

void InstalledFontsModel::revealInFileBrowser(int row)
{
    if (!isValidRow(row)) {
        return;
    }
    interactive()->revealInFileBrowser(m_items[row].fontFile);
}

void InstalledFontsModel::uninstall(int row)
{
    if (!isValidRow(row)) {
        return;
    }

    const Item item = m_items[row];

    IInteractive::Result res = interactive()->questionSync(
        trc("fontdesign", "Uninstall font"),
        qtrc("fontdesign", "Remove “%1” from the MuseScore music fonts folder? The folder will be moved to the trash.")
        .arg(item.name).toStdString(),
        { interactive()->buttonData(IInteractive::Button::Yes),
          interactive()->buttonData(IInteractive::Button::Cancel) });

    if (res.standardButton() != IInteractive::Button::Yes) {
        return;
    }

    //! 移到废纸篓（可恢复），不做硬删除
    if (!QFile::moveToTrash(item.dir.toQString())) {
        interactive()->error(trc("fontdesign", "Unable to uninstall font"),
                             trc("fontdesign", "The font folder could not be moved to the trash."));
        return;
    }

    fontsScanner()->rescanFonts();
    load();
}
