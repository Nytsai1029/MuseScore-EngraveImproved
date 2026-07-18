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

#include <memory>

#include "ui/iuiactionsmodule.h"
#include "async/asyncable.h"

#include "fontdesignactioncontroller.h"

namespace mu::fontdesign {
class FontDesignUiActions : public muse::ui::IUiActionsModule, public muse::async::Asyncable
{
public:
    explicit FontDesignUiActions(std::shared_ptr<FontDesignActionController> controller);

    const muse::ui::UiActionList& actionsList() const override;

    bool actionEnabled(const muse::ui::UiAction& act) const override;
    muse::async::Channel<muse::actions::ActionCodeList> actionEnabledChanged() const override;

    bool actionChecked(const muse::ui::UiAction& act) const override;
    muse::async::Channel<muse::actions::ActionCodeList> actionCheckedChanged() const override;

private:
    const static muse::ui::UiActionList m_actions;
    std::shared_ptr<FontDesignActionController> m_controller;
    muse::async::Channel<muse::actions::ActionCodeList> m_actionCheckedChanged;
};
}
