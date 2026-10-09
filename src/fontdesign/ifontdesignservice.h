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

#include "internal/io/projectfiles.h"
#include "internal/project/fontdesignproject.h"

namespace mu::fontdesign {
class SmuflDatabase;
class IFontDesignEditSurface;

//! saveProject 的结果说明（不含成败，成败看 Ret）
struct SaveReport {
    std::vector<std::string> warnings;          // 导出回读校验的告警
    std::vector<muse::io::path_t> backups;      // 本次保存新建的原件备份（*.bak）
};

class IFontDesignService : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IFontDesignService)

public:
    virtual ~IFontDesignService() = default;

    //! 打开字体文件；同目录的 SMuFL 元数据 JSON 自动配对
    virtual muse::Ret openProject(const muse::io::path_t& fontPath) = 0;

    //! 从零新建空白字体项目（文件在首次保存时写出）
    virtual muse::Ret newProject(const NewFontParams& params) = 0;

    //! 保存会写到哪些文件、会不会盖掉不属于本项目的文件。
    //! 界面层在保存前据此向用户确认（见 IFontDesignProjectScenario）
    virtual muse::Ret saveTargets(SaveTargets& targets) const = 0;

    //! 重导出字体 + 元数据并 markClean；告警与备份路径回填到 report。
    //! 外来字体首次被覆盖前，原字体与原元数据各留一份 .bak。
    //! 不弹 UI、不做覆盖确认：界面上的保存一律走 IFontDesignProjectScenario::saveCurrentProject
    virtual muse::Ret saveProject(SaveReport& report) = 0;

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
