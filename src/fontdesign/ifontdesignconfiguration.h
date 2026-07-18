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

#include "io/path.h"

namespace mu::fontdesign {
class IFontDesignConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IFontDesignConfiguration)

public:
    virtual ~IFontDesignConfiguration() = default;

    virtual muse::io::path_t lastOpenedFontPath() const = 0;
    virtual void setLastOpenedFontPath(const muse::io::path_t& path) = 0;

    //! 最近打开的字体（去重，新到旧，上限 10；打开/新建成功时前插）
    virtual muse::io::paths_t recentFontPaths() const = 0;
    virtual void prependRecentFontPath(const muse::io::path_t& path) = 0;
    virtual void removeRecentFontPath(const muse::io::path_t& path) = 0;
};
}
