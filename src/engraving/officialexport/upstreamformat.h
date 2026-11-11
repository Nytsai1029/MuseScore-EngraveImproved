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

#include <string_view>

#include "style/styledef.h"

namespace mu::engraving {
//! What official MuseScore Studio (the upstream this fork is based on) knows of the file format.
//! Used when a score is exported for it: see OfficialLayoutBaker.
class UpstreamFormat
{
public:
    //! Whether upstream has this style key. Keys the fork added are not written to an official file.
    //! Every fork-only key defaults to upstream's behaviour, so resetting one switches its rule off.
    static bool hasStyleKey(std::string_view name);
    static bool hasStyle(Sid sid);

    //! True where the fork changed the default of an upstream key. Such a value has to be written even
    //! when it is the fork's default, or upstream would read its own default instead.
    static bool defaultDiffers(Sid sid);
};
}
