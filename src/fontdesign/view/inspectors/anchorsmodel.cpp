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
#include "anchorsmodel.h"

#include "translation.h"

using namespace mu::fontdesign;
using namespace muse;

AnchorsModel::AnchorsModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

QVariant AnchorsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size()) {
        return QVariant();
    }

    const Item& item = m_items.at(index.row());
    switch (role) {
    case NameRole: return QString::fromStdString(anchorNameById(item.id));
    case XRole: return item.pos.x();
    case YRole: return item.pos.y();
    case DescriptionRole:
        return muse::qtrc("fontdesign", anchorDescriptionKey(item.id));
    }

    return QVariant();
}

int AnchorsModel::rowCount(const QModelIndex&) const
{
    return m_items.size();
}

QHash<int, QByteArray> AnchorsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { NameRole, "name" },
        { XRole, "anchorX" },
        { YRole, "anchorY" },
        { DescriptionRole, "description" },
    };

    return roles;
}

void AnchorsModel::init()
{
    fontDesignService()->currentProjectChanged().onNotify(this, [this]() {
        attachToProject();
        reload();
    });

    attachToProject();
    reload();
}

void AnchorsModel::setX(int row, double x)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_items.size()) {
        return;
    }

    const Item& item = m_items.at(row);
    if (qFuzzyCompare(item.pos.x(), x)) {
        return;
    }

    proj->undoStack().push(std::make_unique<SetAnchorCommand>(proj.get(), proj->currentGlyph(), item.id,
                                                              PointF(x, item.pos.y())));
}

void AnchorsModel::setY(int row, double y)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_items.size()) {
        return;
    }

    const Item& item = m_items.at(row);
    if (qFuzzyCompare(item.pos.y(), y)) {
        return;
    }

    proj->undoStack().push(std::make_unique<SetAnchorCommand>(proj.get(), proj->currentGlyph(), item.id,
                                                              PointF(item.pos.x(), y)));
}

void AnchorsModel::removeAnchor(int row)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_items.size()) {
        return;
    }

    proj->undoStack().push(std::make_unique<RemoveAnchorCommand>(proj.get(), proj->currentGlyph(), m_items.at(row).id));
}

void AnchorsModel::addAnchor(const QString& name)
{
    FontDesignProjectPtr proj = project();
    if (!proj || !currentGlyphItem()) {
        return;
    }

    const auto& ids = anchorIdsByName();
    auto it = ids.find(name.toStdString());
    if (it == ids.end()) {
        return;
    }

    proj->undoStack().push(std::make_unique<SetAnchorCommand>(proj.get(), proj->currentGlyph(), it->second,
                                                              PointF(0.0, 0.0)));
}

QStringList AnchorsModel::availableAnchorNames() const
{
    QStringList result;

    const GlyphItem* glyph = currentGlyphItem();
    if (!glyph) {
        return result;
    }

    for (const auto& pair : anchorIdsByName()) {
        if (glyph->anchors.find(pair.second) == glyph->anchors.end()) {
            result << QString::fromStdString(pair.first);
        }
    }

    return result;
}

bool AnchorsModel::hasGlyph() const
{
    return currentGlyphItem() != nullptr;
}

void AnchorsModel::attachToProject()
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
        proj->currentGlyphChanged().onNotify(this, [this]() {
            reload();
        });
    }
}

void AnchorsModel::reload()
{
    beginResetModel();
    m_items.clear();

    if (const GlyphItem* glyph = currentGlyphItem()) {
        for (const auto& pair : glyph->anchors) {
            Item item;
            item.id = pair.first;
            item.pos = pair.second;
            m_items << item;
        }
    }

    endResetModel();
    emit anchorsChanged();
}

FontDesignProjectPtr AnchorsModel::project() const
{
    return fontDesignService()->currentProject();
}

const GlyphItem* AnchorsModel::currentGlyphItem() const
{
    FontDesignProjectPtr proj = project();
    return proj ? proj->glyph(proj->currentGlyph()) : nullptr;
}
