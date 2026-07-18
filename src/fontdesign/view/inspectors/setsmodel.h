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
//! sets 扁平列表：一行 = set 内一个字形；set 元数据随行展示
class SetsModel : public QAbstractListModel, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool hasProject READ hasProject NOTIFY rowsChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)

    muse::Inject<IFontDesignService> fontDesignService = { this };

public:
    explicit SetsModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void init();
    Q_INVOKABLE void addSet(const QString& setId);
    Q_INVOKABLE void addGlyphToSet(const QString& setId, const QString& glyphName);
    Q_INVOKABLE void removeRowAt(int row);
    Q_INVOKABLE void setSetDescription(int row, const QString& text);
    Q_INVOKABLE void setSetType(int row, const QString& type);
    Q_INVOKABLE void setGlyphName(int row, const QString& name);
    Q_INVOKABLE void setAlternateFor(int row, const QString& name);
    Q_INVOKABLE void setCodepoint(int row, const QString& hex);
    Q_INVOKABLE void setGlyphDescription(int row, const QString& text);
    Q_INVOKABLE void assignNextPua(int row);

    bool hasProject() const;
    QString filterText() const;
    void setFilterText(const QString& text);

signals:
    void rowsChanged();
    void filterTextChanged();
    void errorOccurred(const QString& message);

private:
    enum Roles {
        SetIdRole = Qt::UserRole + 1,
        SetDescriptionRole,
        SetTypeRole,
        GlyphNameRole,
        AlternateForRole,
        CodepointRole,
        GlyphDescriptionRole
    };

    struct Item {
        QString setId;
        QString setDescription;
        QString setType;
        QString glyphName;
        QString alternateFor;
        char32_t codepoint = 0;
        QString glyphDescription;
        int glyphIndex = -1; // -1 = set shell without glyphs
    };

    void attachToProject();
    void reload();
    void commit(const SetMetadataTablesCommand::Tables& tables, const std::string& name);
    FontDesignProjectPtr project() const;
    bool matchesFilter(const Item& item) const;
    SetGlyphInfo* findGlyph(SetMetadataTablesCommand::Tables& tables, const Item& item);

    const FontDesignProject* m_attachedProject = nullptr;
    QList<Item> m_allItems;
    QList<int> m_visible;
    QString m_filterText;
};
}
