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

#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "../../ifontdesignservice.h"
#include "../../internal/project/projectcommands.h"

namespace mu::fontdesign {
//! 当前字形的 SMuFL 锚点列表（sp，y 向上），与画布标记双向联动
class AnchorsModel : public QAbstractListModel, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QStringList availableAnchorNames READ availableAnchorNames NOTIFY anchorsChanged)
    Q_PROPERTY(bool hasGlyph READ hasGlyph NOTIFY anchorsChanged)

    muse::Inject<IFontDesignService> fontDesignService = { this };

public:
    explicit AnchorsModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void init();
    Q_INVOKABLE void setX(int row, double x);
    Q_INVOKABLE void setY(int row, double y);
    Q_INVOKABLE void removeAnchor(int row);
    Q_INVOKABLE void addAnchor(const QString& name);

    QStringList availableAnchorNames() const;
    bool hasGlyph() const;

signals:
    void anchorsChanged();

private:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        XRole,
        YRole,
        DescriptionRole
    };

    struct Item {
        AnchorId id = AnchorId::stemUpSE;
        muse::PointF pos;
    };

    void attachToProject();
    void reload();

    FontDesignProjectPtr project() const;
    const GlyphItem* currentGlyphItem() const;

    const FontDesignProject* m_attachedProject = nullptr;
    QList<Item> m_items;
};
}
