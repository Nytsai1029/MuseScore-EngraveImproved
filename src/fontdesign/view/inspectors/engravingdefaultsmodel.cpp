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
#include "engravingdefaultsmodel.h"

using namespace mu::fontdesign;

//! SMuFL engravingDefaults 规范键全集（textFontFamily 在字体信息分组编辑）
//! https://w3c-cg.github.io/smufl/latest/specification/engravingdefaults.html
static const QStringList SPEC_KEYS = {
    "staffLineThickness",
    "stemThickness",
    "beamThickness",
    "beamSpacing",
    "legerLineThickness",
    "legerLineExtension",
    "slurEndpointThickness",
    "slurMidpointThickness",
    "tieEndpointThickness",
    "tieMidpointThickness",
    "thinBarlineThickness",
    "thickBarlineThickness",
    "dashedBarlineThickness",
    "dashedBarlineDashLength",
    "dashedBarlineGapLength",
    "barlineSeparation",
    "thinThickBarlineSeparation",
    "repeatBarlineDotSeparation",
    "bracketThickness",
    "subBracketThickness",
    "hairpinThickness",
    "octaveLineThickness",
    "pedalLineThickness",
    "repeatEndingLineThickness",
    "arrowShaftThickness",
    "lyricLineThickness",
    "textEnclosureThickness",
    "tupletBracketThickness",
    "hBarThickness",
};

EngravingDefaultsModel::EngravingDefaultsModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

QVariant EngravingDefaultsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size()) {
        return QVariant();
    }

    const Item& item = m_items.at(index.row());
    switch (role) {
    case KeyRole: return item.key;
    case ValueRole: return item.value;
    case IsSetRole: return item.isSet;
    }

    return QVariant();
}

int EngravingDefaultsModel::rowCount(const QModelIndex&) const
{
    return m_items.size();
}

QHash<int, QByteArray> EngravingDefaultsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { KeyRole, "key" },
        { ValueRole, "value" },
        { IsSetRole, "isSet" },
    };

    return roles;
}

void EngravingDefaultsModel::init()
{
    fontDesignService()->currentProjectChanged().onNotify(this, [this]() {
        attachToProject();
        reload();
    });

    attachToProject();
    reload();
}

void EngravingDefaultsModel::setValue(int row, double value)
{
    FontDesignProjectPtr proj = project();
    if (!proj || row < 0 || row >= m_items.size()) {
        return;
    }

    const Item& item = m_items.at(row);
    if (item.isSet && qFuzzyCompare(item.value, value)) {
        return;
    }

    proj->undoStack().push(std::make_unique<SetEngravingDefaultCommand>(proj.get(), item.key.toStdString(), value));
}

void EngravingDefaultsModel::attachToProject()
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

void EngravingDefaultsModel::reload()
{
    QList<Item> items;

    if (FontDesignProjectPtr proj = project()) {
        const auto& defaults = proj->metadata().engravingDefaults;

        for (const QString& key : SPEC_KEYS) {
            Item item;
            item.key = key;

            auto it = defaults.find(key.toStdString());
            if (it != defaults.end()) {
                item.value = it->second;
                item.isSet = true;
            }

            items << item;
        }

        for (const auto& pair : defaults) {
            QString key = QString::fromStdString(pair.first);
            if (!SPEC_KEYS.contains(key)) {
                Item item;
                item.key = key;
                item.value = pair.second;
                item.isSet = true;
                items << item;
            }
        }
    }

    if (items.size() == m_items.size()) {
        bool same = true;
        for (int i = 0; i < items.size(); ++i) {
            if (items.at(i).key != m_items.at(i).key
                || items.at(i).isSet != m_items.at(i).isSet
                || !qFuzzyCompare(items.at(i).value, m_items.at(i).value)) {
                same = false;
                break;
            }
        }
        if (same) {
            return;
        }
    }

    beginResetModel();
    m_items = std::move(items);
    endResetModel();
}

FontDesignProjectPtr EngravingDefaultsModel::project() const
{
    return fontDesignService()->currentProject();
}
