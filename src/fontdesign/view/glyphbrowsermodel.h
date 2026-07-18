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

#include "../ifontdesignservice.h"

namespace mu::fontdesign {
class GlyphBrowserModel : public QAbstractListModel, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QStringList rangeNames READ rangeNames NOTIFY rangeNamesChanged)
    Q_PROPERTY(int currentRangeIndex READ currentRangeIndex WRITE setCurrentRangeIndex NOTIFY currentRangeIndexChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(int currentCodepoint READ currentCodepoint NOTIFY currentCodepointChanged)
    Q_PROPERTY(QString currentGlyphInfo READ currentGlyphInfo NOTIFY currentCodepointChanged)

    muse::Inject<IFontDesignService> fontDesignService = { this };

public:
    explicit GlyphBrowserModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void selectGlyph(int row);

    QStringList rangeNames() const;

    int currentRangeIndex() const;
    void setCurrentRangeIndex(int index);

    QString searchText() const;
    void setSearchText(const QString& text);

    int currentCodepoint() const;
    QString currentGlyphInfo() const;

signals:
    void rangeNamesChanged();
    void currentRangeIndexChanged();
    void searchTextChanged();
    void currentCodepointChanged();

private:
    enum Roles {
        CodepointRole = Qt::UserRole + 1,
        NameRole,
        HexRole,
        HasOutlineRole
    };

    struct Item {
        char32_t codepoint = 0;
        QString name;
        bool hasOutline = false;
    };

    void attachToProject();
    void rebuild();

    const FontDesignProject* m_attachedProject = nullptr;   // 仅作身份比较

    QStringList m_rangeNames;
    int m_currentRangeIndex = 0;
    QString m_searchText;
    QList<Item> m_items;
};
}
