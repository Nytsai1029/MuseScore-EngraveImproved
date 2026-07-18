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
#include "fontdesignuiactions.h"

#include "context/uicontext.h"
#include "context/shortcutcontext.h"
#include "types/translatablestring.h"

using namespace mu::fontdesign;
using namespace muse;
using namespace muse::ui;
using namespace muse::actions;

//! 全部绑定 UiCtxFontDesignOpened + CTX_FONTDESIGN_OPENED：
//! 只有字体设计页面激活时才启用/触发，快捷键与乐谱页的同键绑定互不干扰。
const UiActionList FontDesignUiActions::m_actions = {
    UiAction("fontdesign-undo",
             mu::context::UiCtxFontDesignOpened,
             mu::context::CTX_FONTDESIGN_OPENED,
             TranslatableString("action", "Undo"),
             TranslatableString("action", "Undo last font-design edit")
             ),
    UiAction("fontdesign-redo",
             mu::context::UiCtxFontDesignOpened,
             mu::context::CTX_FONTDESIGN_OPENED,
             TranslatableString("action", "Redo"),
             TranslatableString("action", "Redo last font-design edit")
             ),
    UiAction("fontdesign-save",
             mu::context::UiCtxFontDesignOpened,
             mu::context::CTX_FONTDESIGN_OPENED,
             TranslatableString("action", "Save font"),
             TranslatableString("action", "Save font and metadata")
             ),
    UiAction("fontdesign-close",
             mu::context::UiCtxFontDesignOpened,
             mu::context::CTX_FONTDESIGN_OPENED,
             TranslatableString("action", "Close font"),
             TranslatableString("action", "Close the current font project")
             ),
    UiAction("fontdesign-copy-glyph",
             mu::context::UiCtxFontDesignOpened,
             mu::context::CTX_FONTDESIGN_OPENED,
             TranslatableString("action", "Copy glyph outline"),
             TranslatableString("action", "Copy the current glyph outline")
             ),
    UiAction("fontdesign-paste-glyph",
             mu::context::UiCtxFontDesignOpened,
             mu::context::CTX_FONTDESIGN_OPENED,
             TranslatableString("action", "Paste glyph outline"),
             TranslatableString("action", "Paste the copied outline into the current glyph")
             ),
    UiAction("fontdesign-select-all",
             mu::context::UiCtxFontDesignOpened,
             mu::context::CTX_FONTDESIGN_OPENED,
             TranslatableString("action", "Select all points"),
             TranslatableString("action", "Select all points of the current glyph outline")
             ),
};

FontDesignUiActions::FontDesignUiActions(std::shared_ptr<FontDesignActionController> controller)
    : m_controller(controller)
{
}

const UiActionList& FontDesignUiActions::actionsList() const
{
    return m_actions;
}

bool FontDesignUiActions::actionEnabled(const UiAction& act) const
{
    return m_controller->canReceiveAction(act.code);
}

bool FontDesignUiActions::actionChecked(const UiAction&) const
{
    return false;
}

muse::async::Channel<ActionCodeList> FontDesignUiActions::actionEnabledChanged() const
{
    return m_controller->actionEnabledChanged();
}

muse::async::Channel<ActionCodeList> FontDesignUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}
