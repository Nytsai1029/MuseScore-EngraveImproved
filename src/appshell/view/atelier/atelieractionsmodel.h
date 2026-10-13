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

#include "uicomponents/view/abstracttoolbarmodel.h"

namespace mu::appshell {
//! NOTE The buttons of the Atelier skin are laid out in QML; this model gives them the actions
//! (title, icon, shortcut, enabled and checked state) for a list of action codes
class AtelierActionsModel : public muse::uicomponents::AbstractToolBarModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList actionCodes READ actionCodes WRITE setActionCodes NOTIFY actionCodesChanged)

public:
    explicit AtelierActionsModel(QObject* parent = nullptr);

    QStringList actionCodes() const;
    void setActionCodes(const QStringList& codes);

    Q_INVOKABLE void load() override;

    //! NOTE Dispatches the action, even if it is not in the list
    Q_INVOKABLE void trigger(const QString& actionCode);

signals:
    void actionCodesChanged();

private:
    void rebuild();

    QStringList m_actionCodes;
    bool m_loaded = false;
};
}
