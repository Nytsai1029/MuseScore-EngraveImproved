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
#include "alternatesmodel.h"

#include <algorithm>

#include "../../internal/puaallocator.h"
#include "translation.h"

using namespace mu::fontdesign;
using namespace muse;

AlternatesModel::AlternatesModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

QVariant AlternatesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_visible.size()) {
        return QVariant();
    }

    const Item& item = m_allItems.at(m_visible.at(index.row()));
    switch (role) {
    case BaseNameRole: return item.baseName;
    case AltNameRole: return item.altName;
    case CodepointRole: return QString::fromStdString(PuaAllocator::toHex(item.codepoint));
    }

    return QVariant();
}

int AlternatesModel::rowCount(const QModelIndex&) const
{
    return m_visible.size();
}

QHash<int, QByteArray> AlternatesModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { BaseNameRole, "baseName" },
        { AltNameRole, "altName" },
        { CodepointRole, "codepoint" },
    };
    return roles;
}

void AlternatesModel::init()
{
    fontDesignService()->currentProjectChanged().onNotify(this, [this]() {
        attachToProject();
        reload();
    });

    attachToProject();
    reload();
}

void AlternatesModel::addAlternate(const QString& baseName, const QString& altName)
{
    FontDesignProjectPtr proj = project();
    if (!proj || baseName.isEmpty() || altName.isEmpty()) {
        return;
    }

    char32_t code = PuaAllocator::nextFreePua(*proj);
    if (code == 0) {
        emit errorOccurred(QString::fromStdString(trc("fontdesign", "No free PUA codepoint available")));
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    AlternateInfo info;
    info.name = altName.toStdString();
    info.codepoint = code;
    tables.alternates[baseName.toStdString()].push_back(std::move(info));
    commit(tables, "add alternate");
}

void AlternatesModel::removeRowAt(int row)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size()) {
        return;
    }

    const Item& item = m_allItems.at(m_visible.at(row));
    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    auto it = tables.alternates.find(item.baseName.toStdString());
    if (it == tables.alternates.end()) {
        return;
    }

    auto& list = it->second;
    list.erase(std::remove_if(list.begin(), list.end(), [&](const AlternateInfo& alt) {
        return alt.name == item.altName.toStdString() && alt.codepoint == item.codepoint;
    }), list.end());

    if (list.empty()) {
        tables.alternates.erase(it);
    }

    commit(tables, "remove alternate");
}

void AlternatesModel::setBaseName(int row, const QString& name)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size() || name.isEmpty()) {
        return;
    }

    const Item& item = m_allItems.at(m_visible.at(row));
    if (item.baseName == name) {
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    auto oldIt = tables.alternates.find(item.baseName.toStdString());
    if (oldIt == tables.alternates.end()) {
        return;
    }

    auto& oldList = oldIt->second;
    auto altIt = std::find_if(oldList.begin(), oldList.end(), [&](const AlternateInfo& alt) {
        return alt.name == item.altName.toStdString() && alt.codepoint == item.codepoint;
    });
    if (altIt == oldList.end()) {
        return;
    }

    AlternateInfo moved = *altIt;
    oldList.erase(altIt);
    if (oldList.empty()) {
        tables.alternates.erase(oldIt);
    }
    tables.alternates[name.toStdString()].push_back(std::move(moved));
    commit(tables, "set alternate base");
}

void AlternatesModel::setAltName(int row, const QString& name)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size() || name.isEmpty()) {
        return;
    }

    const Item& item = m_allItems.at(m_visible.at(row));
    if (item.altName == name) {
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    auto it = tables.alternates.find(item.baseName.toStdString());
    if (it == tables.alternates.end()) {
        return;
    }

    for (AlternateInfo& alt : it->second) {
        if (alt.name == item.altName.toStdString() && alt.codepoint == item.codepoint) {
            alt.name = name.toStdString();
            commit(tables, "set alternate name");
            return;
        }
    }
}

void AlternatesModel::setCodepoint(int row, const QString& hex)
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
    auto it = tables.alternates.find(item.baseName.toStdString());
    if (it == tables.alternates.end()) {
        return;
    }

    for (AlternateInfo& alt : it->second) {
        if (alt.name == item.altName.toStdString() && alt.codepoint == item.codepoint) {
            alt.codepoint = code;
            commit(tables, "set alternate codepoint");
            return;
        }
    }
}

void AlternatesModel::assignNextPua(int row)
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

bool AlternatesModel::hasProject() const
{
    return project() != nullptr;
}

QString AlternatesModel::filterText() const
{
    return m_filterText;
}

void AlternatesModel::setFilterText(const QString& text)
{
    if (m_filterText == text) {
        return;
    }

    m_filterText = text;
    emit filterTextChanged();
    reload();
}

void AlternatesModel::attachToProject()
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

void AlternatesModel::reload()
{
    QList<Item> all;
    if (FontDesignProjectPtr proj = project()) {
        for (const auto& pair : proj->metadata().alternates) {
            for (const AlternateInfo& alt : pair.second) {
                Item item;
                item.baseName = QString::fromStdString(pair.first);
                item.altName = QString::fromStdString(alt.name);
                item.codepoint = alt.codepoint;
                all << item;
            }
        }
    }

    QList<int> visible;
    for (int i = 0; i < all.size(); ++i) {
        if (matchesFilter(all.at(i))) {
            visible << i;
        }
    }

    // 锚点/advance 等无关修改时跳过重置，避免大表卡顿
    if (all.size() == m_allItems.size() && visible == m_visible) {
        bool same = true;
        for (int i = 0; i < all.size(); ++i) {
            const Item& a = all.at(i);
            const Item& b = m_allItems.at(i);
            if (a.baseName != b.baseName || a.altName != b.altName || a.codepoint != b.codepoint) {
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

void AlternatesModel::commit(const SetMetadataTablesCommand::Tables& tables, const std::string& name)
{
    if (FontDesignProjectPtr proj = project()) {
        proj->undoStack().push(std::make_unique<SetMetadataTablesCommand>(proj.get(), tables, name));
    }
}

FontDesignProjectPtr AlternatesModel::project() const
{
    return fontDesignService()->currentProject();
}

bool AlternatesModel::matchesFilter(const Item& item) const
{
    if (m_filterText.isEmpty()) {
        return true;
    }

    return item.baseName.contains(m_filterText, Qt::CaseInsensitive)
           || item.altName.contains(m_filterText, Qt::CaseInsensitive)
           || QString::fromStdString(PuaAllocator::toHex(item.codepoint)).contains(m_filterText, Qt::CaseInsensitive);
}
