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
#include "optionalglyphsmodel.h"

#include "../../internal/puaallocator.h"
#include "translation.h"

using namespace mu::fontdesign;
using namespace muse;

OptionalGlyphsModel::OptionalGlyphsModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

QVariant OptionalGlyphsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_visible.size()) {
        return QVariant();
    }

    const Item& item = m_allItems.at(m_visible.at(index.row()));
    switch (role) {
    case NameRole: return item.name;
    case CodepointRole: return QString::fromStdString(PuaAllocator::toHex(item.codepoint));
    case ClassesRole: return item.classes;
    case DescriptionRole: return item.description;
    }

    return QVariant();
}

int OptionalGlyphsModel::rowCount(const QModelIndex&) const
{
    return m_visible.size();
}

QHash<int, QByteArray> OptionalGlyphsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { NameRole, "name" },
        { CodepointRole, "codepoint" },
        { ClassesRole, "classes" },
        { DescriptionRole, "description" },
    };
    return roles;
}

void OptionalGlyphsModel::init()
{
    fontDesignService()->currentProjectChanged().onNotify(this, [this]() {
        attachToProject();
        reload();
    });

    attachToProject();
    reload();
}

void OptionalGlyphsModel::addOptional(const QString& name)
{
    FontDesignProjectPtr proj = project();
    if (!proj || name.isEmpty()) {
        return;
    }

    if (proj->metadata().optionalGlyphs.count(name.toStdString())) {
        emit errorOccurred(QString::fromStdString(trc("fontdesign", "Optional glyph already exists")));
        return;
    }

    char32_t code = PuaAllocator::nextFreePua(*proj);
    if (code == 0) {
        emit errorOccurred(QString::fromStdString(trc("fontdesign", "No free PUA codepoint available")));
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    OptionalGlyphInfo info;
    info.codepoint = code;
    tables.optionalGlyphs[name.toStdString()] = std::move(info);
    commit(tables, "add optional glyph");
}

void OptionalGlyphsModel::removeRowAt(int row)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size()) {
        return;
    }

    const Item& item = m_allItems.at(m_visible.at(row));
    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    tables.optionalGlyphs.erase(item.name.toStdString());
    commit(tables, "remove optional glyph");
}

void OptionalGlyphsModel::setName(int row, const QString& name)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size() || name.isEmpty()) {
        return;
    }

    const Item& item = m_allItems.at(m_visible.at(row));
    if (item.name == name) {
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    auto it = tables.optionalGlyphs.find(item.name.toStdString());
    if (it == tables.optionalGlyphs.end()) {
        return;
    }

    if (tables.optionalGlyphs.count(name.toStdString())) {
        emit errorOccurred(QString::fromStdString(trc("fontdesign", "Optional glyph already exists")));
        return;
    }

    OptionalGlyphInfo info = it->second;
    tables.optionalGlyphs.erase(it);
    tables.optionalGlyphs[name.toStdString()] = std::move(info);
    commit(tables, "rename optional glyph");
}

void OptionalGlyphsModel::setCodepoint(int row, const QString& hex)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size()) {
        return;
    }

    char32_t code = PuaAllocator::fromHex(hex.toStdString());
    if (code == 0) {
        emit errorOccurred(QString::fromStdString(trc("fontdesign", "Invalid codepoint")));
        return;
    }

    const Item& item = m_allItems.at(m_visible.at(row));
    if (item.codepoint == code) {
        return;
    }

    if (PuaAllocator::isUsed(*proj, code) && code != item.codepoint) {
        emit errorOccurred(QString::fromStdString(trc("fontdesign", "Codepoint already in use")));
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    auto it = tables.optionalGlyphs.find(item.name.toStdString());
    if (it == tables.optionalGlyphs.end()) {
        return;
    }

    it->second.codepoint = code;
    commit(tables, "set optional glyph codepoint");
}

void OptionalGlyphsModel::setClasses(int row, const QString& csv)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size()) {
        return;
    }

    const Item& item = m_allItems.at(m_visible.at(row));
    if (item.classes == csv) {
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    auto it = tables.optionalGlyphs.find(item.name.toStdString());
    if (it == tables.optionalGlyphs.end()) {
        return;
    }

    it->second.classes = csvToList(csv);
    commit(tables, "set optional glyph classes");
}

void OptionalGlyphsModel::setDescription(int row, const QString& text)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size()) {
        return;
    }

    const Item& item = m_allItems.at(m_visible.at(row));
    if (item.description == text) {
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    auto it = tables.optionalGlyphs.find(item.name.toStdString());
    if (it == tables.optionalGlyphs.end()) {
        return;
    }

    it->second.description = text.toStdString();
    commit(tables, "set optional glyph description");
}

void OptionalGlyphsModel::assignNextPua(int row)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size()) {
        return;
    }

    char32_t code = PuaAllocator::nextFreePua(*proj);
    if (code == 0) {
        emit errorOccurred(QString::fromStdString(trc("fontdesign", "No free PUA codepoint available")));
        return;
    }

    setCodepoint(row, QString::fromStdString(PuaAllocator::toHex(code)));
}

bool OptionalGlyphsModel::hasProject() const
{
    return project() != nullptr;
}

QString OptionalGlyphsModel::filterText() const
{
    return m_filterText;
}

void OptionalGlyphsModel::setFilterText(const QString& text)
{
    if (m_filterText == text) {
        return;
    }

    m_filterText = text;
    emit filterTextChanged();
    reload();
}

void OptionalGlyphsModel::attachToProject()
{
    FontDesignProjectPtr proj = project();
    if (proj.get() == m_attachedProject) {
        return;
    }

    m_attachedProject = proj.get();
    if (proj) {
        proj->changed().onNotify(this, [this]() {
            reload();
        });
    }
}

void OptionalGlyphsModel::reload()
{
    QList<Item> all;
    if (FontDesignProjectPtr proj = project()) {
        for (const auto& pair : proj->metadata().optionalGlyphs) {
            Item item;
            item.name = QString::fromStdString(pair.first);
            item.codepoint = pair.second.codepoint;
            item.classes = listToCsv(pair.second.classes);
            item.description = QString::fromStdString(pair.second.description);
            all << item;
        }
    }

    QList<int> visible;
    for (int i = 0; i < all.size(); ++i) {
        if (matchesFilter(all.at(i))) {
            visible << i;
        }
    }

    if (all.size() == m_allItems.size() && visible == m_visible) {
        bool same = true;
        for (int i = 0; i < all.size(); ++i) {
            const Item& a = all.at(i);
            const Item& b = m_allItems.at(i);
            if (a.name != b.name || a.codepoint != b.codepoint || a.classes != b.classes
                || a.description != b.description) {
                same = false;
                break;
            }
        }
        if (same) {
            return;
        }
    }

    beginResetModel();
    m_allItems = std::move(all);
    m_visible = std::move(visible);
    endResetModel();
    emit rowsChanged();
}

void OptionalGlyphsModel::commit(const SetMetadataTablesCommand::Tables& tables, const std::string& name)
{
    if (FontDesignProjectPtr proj = project()) {
        proj->undoStack().push(std::make_unique<SetMetadataTablesCommand>(proj.get(), tables, name));
    }
}

FontDesignProjectPtr OptionalGlyphsModel::project() const
{
    return fontDesignService()->currentProject();
}

bool OptionalGlyphsModel::matchesFilter(const Item& item) const
{
    if (m_filterText.isEmpty()) {
        return true;
    }

    return item.name.contains(m_filterText, Qt::CaseInsensitive)
           || item.classes.contains(m_filterText, Qt::CaseInsensitive)
           || item.description.contains(m_filterText, Qt::CaseInsensitive)
           || QString::fromStdString(PuaAllocator::toHex(item.codepoint)).contains(m_filterText, Qt::CaseInsensitive);
}

QString OptionalGlyphsModel::listToCsv(const std::vector<std::string>& values)
{
    QStringList list;
    for (const std::string& v : values) {
        list << QString::fromStdString(v);
    }
    return list.join(", ");
}

std::vector<std::string> OptionalGlyphsModel::csvToList(const QString& csv)
{
    std::vector<std::string> result;
    const QStringList parts = csv.split(',', Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        const QString trimmed = part.trimmed();
        if (!trimmed.isEmpty()) {
            result.push_back(trimmed.toStdString());
        }
    }
    return result;
}
