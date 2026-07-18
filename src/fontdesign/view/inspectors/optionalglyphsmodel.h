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
class OptionalGlyphsModel : public QAbstractListModel, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool hasProject READ hasProject NOTIFY rowsChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)

    muse::Inject<IFontDesignService> fontDesignService = { this };

public:
    explicit OptionalGlyphsModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void init();
    Q_INVOKABLE void addOptional(const QString& name);
    Q_INVOKABLE void removeRowAt(int row);
    Q_INVOKABLE void setName(int row, const QString& name);
    Q_INVOKABLE void setCodepoint(int row, const QString& hex);
    Q_INVOKABLE void setClasses(int row, const QString& csv);
    Q_INVOKABLE void setDescription(int row, const QString& text);
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
        NameRole = Qt::UserRole + 1,
        CodepointRole,
        ClassesRole,
        DescriptionRole
    };

    struct Item {
        QString name;
        char32_t codepoint = 0;
        QString classes;
        QString description;
    };

    void attachToProject();
    void reload();
    void commit(const SetMetadataTablesCommand::Tables& tables, const std::string& name);
    FontDesignProjectPtr project() const;
    bool matchesFilter(const Item& item) const;
    static QString listToCsv(const std::vector<std::string>& values);
    static std::vector<std::string> csvToList(const QString& csv);

    const FontDesignProject* m_attachedProject = nullptr;
    QList<Item> m_allItems;
    QList<int> m_visible;
    QString m_filterText;
};
}
