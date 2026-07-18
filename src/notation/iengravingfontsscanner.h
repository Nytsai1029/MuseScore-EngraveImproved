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

namespace mu::notation {
//! 重新扫描全局 + 用户音乐字体目录（安装自定义 SMuFL 字体后即时生效）
class IEngravingFontsScanner : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IEngravingFontsScanner)

public:
    virtual ~IEngravingFontsScanner() = default;

    virtual void rescanFonts() = 0;
};
}
