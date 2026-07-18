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

#include <string>
#include <vector>

#include "modularity/imoduleinterface.h"

#include "async/notification.h"
#include "io/path.h"
#include "types/ret.h"

#include "internal/project/fontdesignproject.h"

namespace mu::fontdesign {
class SmuflDatabase;
class IFontDesignEditSurface;
class IFontDesignService : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IFontDesignService)

public:
    virtual ~IFontDesignService() = default;

    //! 打开字体文件；同目录的 SMuFL 元数据 JSON 自动配对
    virtual muse::Ret openProject(const muse::io::path_t& fontPath) = 0;

    //! 从零新建空白字体项目（文件在首次保存时写出）
    virtual muse::Ret newProject(const NewFontParams& params) = 0;

    //! 原地重导出字体 + 元数据并 markClean；导出告警回填到 warnings。
    //! 不弹 UI（由调用方决定如何提示）。
    virtual muse::Ret saveProject(std::vector<std::string>& warnings) = 0;

    //! 关闭当前项目（不保存）
    virtual void closeProject() = 0;

    virtual FontDesignProjectPtr currentProject() const = 0;
    virtual bool hasCurrentProject() const = 0;
    virtual muse::async::Notification currentProjectChanged() const = 0;

    virtual const SmuflDatabase& smuflDatabase() const = 0;

    //! 当前活动的画布编辑面（无则 nullptr）；由 GlyphCanvas 注册/注销
    virtual void setActiveEditSurface(IFontDesignEditSurface* surface) = 0;
    virtual IFontDesignEditSurface* activeEditSurface() const = 0;
};
}
