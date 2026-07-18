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
#include "setsmodel.h"

#include "../../internal/puaallocator.h"
#include "translation.h"

using namespace mu::fontdesign;
using namespace muse;

SetsModel::SetsModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

QVariant SetsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_visible.size()) {
        return QVariant();
    }

    const Item& item = m_allItems.at(m_visible.at(index.row()));
    switch (role) {
    case SetIdRole: return item.setId;
    case SetDescriptionRole: return item.setDescription;
    case SetTypeRole: return item.setType;
    case GlyphNameRole: return item.glyphName;
    case AlternateForRole: return item.alternateFor;
    case CodepointRole: return item.codepoint ? QString::fromStdString(PuaAllocator::toHex(item.codepoint)) : QString();
    case GlyphDescriptionRole: return item.glyphDescription;
    }

    return QVariant();
}

int SetsModel::rowCount(const QModelIndex&) const
{
    return m_visible.size();
}

QHash<int, QByteArray> SetsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { SetIdRole, "setId" },
        { SetDescriptionRole, "setDescription" },
        { SetTypeRole, "setType" },
        { GlyphNameRole, "glyphName" },
        { AlternateForRole, "alternateFor" },
        { CodepointRole, "codepoint" },
        { GlyphDescriptionRole, "glyphDescription" },
    };
    return roles;
}

void SetsModel::init()
{
    fontDesignService()->currentProjectChanged().onNotify(this, [this]() {
        attachToProject();
        reload();
    });

    attachToProject();
    reload();
}

void SetsModel::addSet(const QString& setId)
{
    FontDesignProjectPtr proj = project();
    if (!proj || setId.isEmpty()) {
        return;
    }

    if (proj->metadata().sets.count(setId.toStdString())) {
        emit errorOccurred(QString::fromStdString(trc("fontdesign", "Set already exists")));
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    tables.sets[setId.toStdString()] = SetInfo();
    commit(tables, "add set");
}

void SetsModel::addGlyphToSet(const QString& setId, const QString& glyphName)
{
    FontDesignProjectPtr proj = project();
    if (!proj || setId.isEmpty() || glyphName.isEmpty()) {
        return;
    }

    char32_t code = PuaAllocator::nextFreePua(*proj);
    if (code == 0) {
        emit errorOccurred(QString::fromStdString(trc("fontdesign", "No free PUA codepoint available")));
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    auto it = tables.sets.find(setId.toStdString());
    if (it == tables.sets.end()) {
        emit errorOccurred(QString::fromStdString(trc("fontdesign", "Set not found")));
        return;
    }

    SetGlyphInfo glyph;
    glyph.name = glyphName.toStdString();
    glyph.codepoint = code;
    it->second.glyphs.push_back(std::move(glyph));
    commit(tables, "add set glyph");
}

void SetsModel::removeRowAt(int row)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size()) {
        return;
    }

    const Item& item = m_allItems.at(m_visible.at(row));
    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    auto it = tables.sets.find(item.setId.toStdString());
    if (it == tables.sets.end()) {
        return;
    }

    if (item.glyphIndex < 0) {
        tables.sets.erase(it);
    } else if (item.glyphIndex < static_cast<int>(it->second.glyphs.size())) {
        it->second.glyphs.erase(it->second.glyphs.begin() + item.glyphIndex);
    }

    commit(tables, "remove set row");
}

void SetsModel::setSetDescription(int row, const QString& text)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size()) {
        return;
    }

    const Item& item = m_allItems.at(m_visible.at(row));
    if (item.setDescription == text) {
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    auto it = tables.sets.find(item.setId.toStdString());
    if (it == tables.sets.end()) {
        return;
    }

    it->second.description = text.toStdString();
    commit(tables, "set set description");
}

void SetsModel::setSetType(int row, const QString& type)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size()) {
        return;
    }

    const Item& item = m_allItems.at(m_visible.at(row));
    if (item.setType == type) {
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    auto it = tables.sets.find(item.setId.toStdString());
    if (it == tables.sets.end()) {
        return;
    }

    it->second.type = type.toStdString();
    commit(tables, "set set type");
}

void SetsModel::setGlyphName(int row, const QString& name)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size() || name.isEmpty()) {
        return;
    }

    const Item& item = m_allItems.at(m_visible.at(row));
    if (item.glyphName == name || item.glyphIndex < 0) {
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    if (SetGlyphInfo* glyph = findGlyph(tables, item)) {
        glyph->name = name.toStdString();
        commit(tables, "set set glyph name");
    }
}

void SetsModel::setAlternateFor(int row, const QString& name)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size()) {
        return;
    }

    const Item& item = m_allItems.at(m_visible.at(row));
    if (item.alternateFor == name || item.glyphIndex < 0) {
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    if (SetGlyphInfo* glyph = findGlyph(tables, item)) {
        glyph->alternateFor = name.toStdString();
        commit(tables, "set set alternateFor");
    }
}

void SetsModel::setCodepoint(int row, const QString& hex)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size()) {
        return;
    }

    const Item& item = m_allItems.at(m_visible.at(row));
    if (item.glyphIndex < 0) {
        return;
    }

    char32_t code = PuaAllocator::fromHex(hex.toStdString());
    if (code == 0) {
        emit errorOccurred(QString::fromStdString(trc("fontdesign", "Invalid codepoint")));
        return;
    }

    if (item.codepoint == code) {
        return;
    }

    if (PuaAllocator::isUsed(*proj, code) && code != item.codepoint) {
        emit errorOccurred(QString::fromStdString(trc("fontdesign", "Codepoint already in use")));
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    if (SetGlyphInfo* glyph = findGlyph(tables, item)) {
        glyph->codepoint = code;
        commit(tables, "set set glyph codepoint");
    }
}

void SetsModel::setGlyphDescription(int row, const QString& text)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_visible.size()) {
        return;
    }

    const Item& item = m_allItems.at(m_visible.at(row));
    if (item.glyphDescription == text || item.glyphIndex < 0) {
        return;
    }

    auto tables = SetMetadataTablesCommand::captureOf(*proj);
    if (SetGlyphInfo* glyph = findGlyph(tables, item)) {
        glyph->description = text.toStdString();
        commit(tables, "set set glyph description");
    }
}

void SetsModel::assignNextPua(int row)
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

bool SetsModel::hasProject() const
{
    return project() != nullptr;
}

QString SetsModel::filterText() const
{
    return m_filterText;
}

void SetsModel::setFilterText(const QString& text)
{
    if (m_filterText == text) {
        return;
    }

    m_filterText = text;
    emit filterTextChanged();
    reload();
}

void SetsModel::attachToProject()
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

void SetsModel::reload()
{
    QList<Item> all;
    if (FontDesignProjectPtr proj = project()) {
        for (const auto& pair : proj->metadata().sets) {
            if (pair.second.glyphs.empty()) {
                Item item;
                item.setId = QString::fromStdString(pair.first);
                item.setDescription = QString::fromStdString(pair.second.description);
                item.setType = QString::fromStdString(pair.second.type);
                item.glyphIndex = -1;
                all << item;
                continue;
            }

            for (size_t i = 0; i < pair.second.glyphs.size(); ++i) {
                const SetGlyphInfo& glyph = pair.second.glyphs.at(i);
                Item item;
                item.setId = QString::fromStdString(pair.first);
                item.setDescription = QString::fromStdString(pair.second.description);
                item.setType = QString::fromStdString(pair.second.type);
                item.glyphName = QString::fromStdString(glyph.name);
                item.alternateFor = QString::fromStdString(glyph.alternateFor);
                item.codepoint = glyph.codepoint;
                item.glyphDescription = QString::fromStdString(glyph.description);
                item.glyphIndex = static_cast<int>(i);
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

    if (all.size() == m_allItems.size() && visible == m_visible) {
        bool same = true;
        for (int i = 0; i < all.size(); ++i) {
            const Item& a = all.at(i);
            const Item& b = m_allItems.at(i);
            if (a.setId != b.setId || a.setDescription != b.setDescription || a.setType != b.setType
                || a.glyphName != b.glyphName || a.alternateFor != b.alternateFor
                || a.codepoint != b.codepoint || a.glyphDescription != b.glyphDescription
                || a.glyphIndex != b.glyphIndex) {
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

void SetsModel::commit(const SetMetadataTablesCommand::Tables& tables, const std::string& name)
{
    if (FontDesignProjectPtr proj = project()) {
        proj->undoStack().push(std::make_unique<SetMetadataTablesCommand>(proj.get(), tables, name));
    }
}

FontDesignProjectPtr SetsModel::project() const
{
    return fontDesignService()->currentProject();
}

bool SetsModel::matchesFilter(const Item& item) const
{
    if (m_filterText.isEmpty()) {
        return true;
    }

    return item.setId.contains(m_filterText, Qt::CaseInsensitive)
           || item.glyphName.contains(m_filterText, Qt::CaseInsensitive)
           || item.alternateFor.contains(m_filterText, Qt::CaseInsensitive)
           || item.setDescription.contains(m_filterText, Qt::CaseInsensitive);
}

SetGlyphInfo* SetsModel::findGlyph(SetMetadataTablesCommand::Tables& tables, const Item& item)
{
    auto it = tables.sets.find(item.setId.toStdString());
    if (it == tables.sets.end() || item.glyphIndex < 0
        || item.glyphIndex >= static_cast<int>(it->second.glyphs.size())) {
        return nullptr;
    }

    return &it->second.glyphs[static_cast<size_t>(item.glyphIndex)];
}
