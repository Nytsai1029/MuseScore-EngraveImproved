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

#include <QAbstractListModel>

#include "abstractfontdesignmodel.h"

namespace mu::fontdesign {
//! engravingDefaults 表单：SMuFL 规范键全集 + 字体自带的额外键
class EngravingDefaultsModel : public QAbstractListModel, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    muse::Inject<IFontDesignService> fontDesignService = { this };

public:
    explicit EngravingDefaultsModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void init();
    Q_INVOKABLE void setValue(int row, double value);

private:
    enum Roles {
        KeyRole = Qt::UserRole + 1,
        ValueRole,
        IsSetRole
    };

    struct Item {
        QString key;
        double value = 0.0;
        bool isSet = false;
    };

    void attachToProject();
    void reload();

    FontDesignProjectPtr project() const;

    const FontDesignProject* m_attachedProject = nullptr;
    QList<Item> m_items;
};
}
