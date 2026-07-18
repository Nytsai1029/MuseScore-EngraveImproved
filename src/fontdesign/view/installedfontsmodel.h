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

#include <vector>

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "multiinstances/imultiinstancesprovider.h"

#include "notation/iengravingfontsscanner.h"
#include "notation/inotationconfiguration.h"

#include "../ifontdesignservice.h"

namespace mu::fontdesign {
//! Home 字体设计区块：管理已安装到 MuseScore 私有音乐字体目录的字体
//! （目录名 = 字体名，含 .otf/.ttf 与 metadata JSON，同 engravingfontscontroller 扫描约定）
class InstalledFontsModel : public QAbstractListModel, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

    muse::Inject<IFontDesignService> fontDesignService = { this };
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<muse::mi::IMultiInstancesProvider> multiInstancesProvider = { this };
    muse::Inject<mu::notation::INotationConfiguration> notationConfiguration = { this };
    muse::Inject<mu::notation::IEngravingFontsScanner> fontsScanner = { this };

public:
    explicit InstalledFontsModel(QObject* parent = nullptr);

    enum Roles {
        NameRole = Qt::UserRole + 1,
        FontFileRole,
        HasMetadataRole,
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return static_cast<int>(m_items.size()); }

    Q_INVOKABLE void load();
    //! 编辑器内打开走 ProjectsPageModel::openFontFile（替换/新窗口询问统一在那里）
    Q_INVOKABLE QString fontFileAt(int row) const;
    Q_INVOKABLE void openInNewWindow(int row);
    Q_INVOKABLE void revealInFileBrowser(int row);
    Q_INVOKABLE void uninstall(int row);

signals:
    void countChanged();

private:
    struct Item {
        QString name;
        muse::io::path_t dir;
        muse::io::path_t fontFile;
        bool hasMetadata = false;
    };

    bool isValidRow(int row) const { return row >= 0 && row < static_cast<int>(m_items.size()); }

    std::vector<Item> m_items;
};
}
