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

#include "modularity/imoduleinterface.h"

namespace mu::fontdesign {
//! 字体项目的「带界面」流程：保存、关闭前确认。
//! 页面按钮、快捷键动作、主页入口、应用退出都走这里，保证只有一套保存/确认逻辑
//! （IFontDesignService::saveProject 本身不弹任何界面）。
class IFontDesignProjectScenario : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IFontDesignProjectScenario)

public:
    virtual ~IFontDesignProjectScenario() = default;

    //! 保存当前项目（字体 + 元数据）。会盖掉不属于本项目的文件时先确认；
    //! 失败、告警、原件备份都在这里提示。返回是否已保存
    virtual bool saveCurrentProject() = 0;

    //! 当前项目有未保存修改时询问 保存 / 不保存 / 取消。
    //! 返回 true = 可以继续（没有修改、已保存、或用户放弃修改）；
    //! false = 用户取消，或选择保存但保存失败
    virtual bool confirmDiscardOrSave() = 0;
};
}
