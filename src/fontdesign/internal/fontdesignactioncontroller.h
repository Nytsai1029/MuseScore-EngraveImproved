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

#include "modularity/ioc.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "async/asyncable.h"
#include "iinteractive.h"

#include "../ifontdesignservice.h"
#include "project/glyphoutline.h"

namespace mu::fontdesign {
//! 处理 Font Design 页面专属动作（撤销/重做/保存/关闭/复制/粘贴/全选）。
//! 这些动作绑定 UiCtxFontDesignOpened + CTX_FONTDESIGN_OPENED，
//! 因此 Ctrl+Z/S/W/C/V/A 等在字体设计页面独立生效，不落到乐谱页的绑定上。
//! 复制/粘贴/全选经 IFontDesignEditSurface 桥接画布选择集：
//! 有选择时复制选中 contour，粘贴为追加；无画布时退化为整字形替换。
class FontDesignActionController : public muse::actions::Actionable, public muse::async::Asyncable
{
    INJECT(muse::actions::IActionsDispatcher, dispatcher)
    INJECT(muse::IInteractive, interactive)
    INJECT(IFontDesignService, fontDesignService)

public:
    FontDesignActionController() = default;

    void init();

    bool canReceiveAction(const muse::actions::ActionCode& code) const;
    muse::async::Channel<muse::actions::ActionCodeList> actionEnabledChanged() const;

private:
    void undo();
    void redo();
    void save();
    void close();
    void copyGlyph();
    void pasteGlyph();
    void selectAll();

    void notifyActionEnabledChanged();

    FontDesignProjectPtr project() const;

    GlyphOutline m_glyphClipboard;
    bool m_hasClipboard = false;
    muse::async::Channel<muse::actions::ActionCodeList> m_actionEnabledChanged;
};
}
