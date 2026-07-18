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
#include "glyphbrowsermodel.h"

#include "translation.h"

#include "internal/fontdesigntypes.h"
#include "internal/smufldatabase.h"

using namespace mu::fontdesign;

static QString hexOfCode(char32_t code)
{
    return QString("U+%1").arg(QString::number(code, 16).toUpper().rightJustified(4, '0'));
}

GlyphBrowserModel::GlyphBrowserModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

QVariant GlyphBrowserModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size()) {
        return QVariant();
    }

    const Item& item = m_items.at(index.row());
    switch (role) {
    case CodepointRole: return static_cast<int>(item.codepoint);
    case NameRole: return item.name;
    case HexRole: return hexOfCode(item.codepoint);
    case HasOutlineRole: return item.hasOutline;
    }

    return QVariant();
}

int GlyphBrowserModel::rowCount(const QModelIndex&) const
{
    return m_items.size();
}

QHash<int, QByteArray> GlyphBrowserModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { CodepointRole, "codepoint" },
        { NameRole, "name" },
        { HexRole, "hex" },
        { HasOutlineRole, "hasOutline" },
    };

    return roles;
}

void GlyphBrowserModel::load()
{
    const SmuflDatabase& db = fontDesignService()->smuflDatabase();

    m_rangeNames.clear();
    m_rangeNames << muse::qtrc("fontdesign", "All symbols");
    for (const SmuflDatabase::Range& range : db.ranges()) {
        m_rangeNames << QString::fromStdString(range.description);
    }
    m_rangeNames << muse::qtrc("fontdesign", "Optional glyphs (U+F400+)");

    emit rangeNamesChanged();

    fontDesignService()->currentProjectChanged().onNotify(this, [this]() {
        rebuild();
        attachToProject();
        emit currentCodepointChanged();
    });

    // 面板可能在项目打开之后才创建，初始化时也要主动挂接当前项目
    attachToProject();
    rebuild();
}

void GlyphBrowserModel::attachToProject()
{
    FontDesignProjectPtr project = fontDesignService()->currentProject();
    if (project.get() == m_attachedProject) {
        return;
    }

    m_attachedProject = project.get();

    if (project) {
        project->currentGlyphChanged().onNotify(this, [this]() {
            emit currentCodepointChanged();
        });
    }
}

void GlyphBrowserModel::selectGlyph(int row)
{
    if (row < 0 || row >= m_items.size()) {
        return;
    }

    if (FontDesignProjectPtr project = fontDesignService()->currentProject()) {
        project->setCurrentGlyph(m_items.at(row).codepoint);
    }
}

QStringList GlyphBrowserModel::rangeNames() const
{
    return m_rangeNames;
}

int GlyphBrowserModel::currentRangeIndex() const
{
    return m_currentRangeIndex;
}

void GlyphBrowserModel::setCurrentRangeIndex(int index)
{
    if (m_currentRangeIndex == index) {
        return;
    }

    m_currentRangeIndex = index;
    emit currentRangeIndexChanged();

    rebuild();
}

QString GlyphBrowserModel::searchText() const
{
    return m_searchText;
}

void GlyphBrowserModel::setSearchText(const QString& text)
{
    if (m_searchText == text) {
        return;
    }

    m_searchText = text;
    emit searchTextChanged();

    rebuild();
}

int GlyphBrowserModel::currentCodepoint() const
{
    FontDesignProjectPtr project = fontDesignService()->currentProject();
    return project ? static_cast<int>(project->currentGlyph()) : 0;
}

QString GlyphBrowserModel::currentGlyphInfo() const
{
    FontDesignProjectPtr project = fontDesignService()->currentProject();
    if (!project) {
        return QString();
    }

    char32_t code = project->currentGlyph();
    const GlyphItem* glyph = project->glyph(code);
    QString name = glyph && !glyph->smuflName.empty() ? QString::fromStdString(glyph->smuflName) : QString();

    return name.isEmpty() ? hexOfCode(code) : QString("%1  %2").arg(name, hexOfCode(code));
}

void GlyphBrowserModel::rebuild()
{
    beginResetModel();
    m_items.clear();

    FontDesignProjectPtr project = fontDesignService()->currentProject();
    if (!project) {
        endResetModel();
        return;
    }

    const SmuflDatabase& db = fontDesignService()->smuflDatabase();
    const QString search = m_searchText.trimmed().toLower();

    auto matchesSearch = [&search](const QString& name, char32_t code) {
        if (search.isEmpty()) {
            return true;
        }
        if (name.toLower().contains(search)) {
            return true;
        }
        return hexOfCode(code).toLower().contains(search);
    };

    auto appendItem = [this, &matchesSearch](char32_t code, const QString& name, bool hasOutline) {
        if (matchesSearch(name, code)) {
            Item item;
            item.codepoint = code;
            item.name = name;
            item.hasOutline = hasOutline;
            m_items << item;
        }
    };

    const int rangeCount = static_cast<int>(db.ranges().size());

    if (m_currentRangeIndex == 0) {
        // 全部：字体中实际存在的字形，按码位排序
        for (const auto& pair : project->glyphs()) {
            const GlyphItem& glyph = pair.second;
            appendItem(glyph.codepoint, QString::fromStdString(glyph.smuflName), !glyph.outline.isEmpty());
        }
    } else if (m_currentRangeIndex >= 1 && m_currentRangeIndex <= rangeCount) {
        // 单个 SMuFL range：按规范的字形顺序，含缺失字形（空位）
        const SmuflDatabase::Range& range = db.ranges().at(m_currentRangeIndex - 1);
        for (const std::string& glyphName : range.glyphNames) {
            const SmuflDatabase::GlyphInfo* info = db.infoByName(glyphName);
            if (!info) {
                continue;
            }

            const GlyphItem* glyph = project->glyph(info->codepoint);
            appendItem(info->codepoint, QString::fromStdString(glyphName), glyph && !glyph->outline.isEmpty());
        }
    } else {
        // 可选字形：U+F400 起
        for (const auto& pair : project->glyphs()) {
            if (pair.first < SMUFL_OPTIONAL_START) {
                continue;
            }
            const GlyphItem& glyph = pair.second;
            appendItem(glyph.codepoint, QString::fromStdString(glyph.smuflName), !glyph.outline.isEmpty());
        }
    }

    endResetModel();
}
