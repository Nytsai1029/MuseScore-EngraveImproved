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

#include "io/path.h"
#include "types/ret.h"

namespace mu::fontdesign {
class FontDesignProject;

//! 保存会写到哪里，以及会不会盖掉别人的文件
struct SaveTargets {
    muse::io::path_t fontPath;
    muse::io::path_t metadataPath;
    //! 目标位置上已存在、但不是本项目自己的文件（另一个字体 / 另一份元数据）
    std::vector<muse::io::path_t> foreignFiles;
};

//! 字体项目的磁盘文件规则：元数据配对、保存目标、首次覆盖前的原件备份。
//! 纯函数（不依赖 IoC 与界面），供 FontDesignService 使用，可直接单测。
class ProjectFiles
{
public:
    //! 为字体文件找同目录的 SMuFL 元数据；没有可信的配对时返回空路径。
    //! 可信 = 文件名对得上（<字体名>.json / <字体名>_metadata.json），
    //! 或 JSON 自己的 fontName 就是这个字体的名字。
    //! 绝不因为「目录里只有这一个 JSON」就认领：保存时文件名跟随元数据的 fontName，
    //! 认错一次就会把同目录的另一个字体覆盖掉。
    static muse::io::path_t findMetadataFor(const muse::io::path_t& fontPath);

    //! 文件名跟随字体名：<目录>/<fontName>.otf + <fontName>.json
    static muse::Ret saveTargets(const FontDesignProject& project, SaveTargets& out);

    //! 把 path 复制为 path + ".bak"。已有备份时不动它（最早的那份才是原件）。
    //! backupPath：本次新建的备份；未新建则为空
    static muse::Ret backupOriginal(const muse::io::path_t& path, muse::io::path_t& backupPath);

    //! 两个路径是否指向同一个已存在的文件（大小写不敏感文件系统、符号链接都算）
    static bool isSameFile(const muse::io::path_t& a, const muse::io::path_t& b);

    //! 名字比较用：小写，去掉空格、'-'、'_'
    static std::string normalizedName(const std::string& name);
};
}
