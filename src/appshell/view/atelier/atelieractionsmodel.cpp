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
#include "atelieractionsmodel.h"

using namespace mu::appshell;
using namespace muse::uicomponents;

AtelierActionsModel::AtelierActionsModel(QObject* parent)
    : AbstractToolBarModel(parent)
{
}

QStringList AtelierActionsModel::actionCodes() const
{
    return m_actionCodes;
}

void AtelierActionsModel::setActionCodes(const QStringList& codes)
{
    if (m_actionCodes == codes) {
        return;
    }

    m_actionCodes = codes;
    emit actionCodesChanged();

    if (m_loaded) {
        rebuild();
    }
}

void AtelierActionsModel::load()
{
    rebuild();

    if (!m_loaded) {
        m_loaded = true;
        AbstractToolBarModel::load();
    }
}

void AtelierActionsModel::rebuild()
{
    ToolBarItemList items;

    for (const QString& code : m_actionCodes) {
        items << makeItem(code.toStdString());
    }

    setItems(items);
}

void AtelierActionsModel::trigger(const QString& actionCode)
{
    dispatch(actionCode.toStdString());
}
