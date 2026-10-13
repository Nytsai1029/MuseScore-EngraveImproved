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

#include "global/iinteractive.h"

namespace mu::notation {
//! NOTE The Engrave mode is a page of the Atelier skin, which shows the score like the notation page does.
//! While it is open, notes are not entered or changed, and the arrow keys nudge what is selected
//! by the step which is chosen there
inline bool isEngraveModeOpened(const std::shared_ptr<muse::IInteractive>& interactive)
{
    static const muse::Uri ENGRAVE_PAGE_URI("musescore://engrave");
    return interactive && interactive->isOpened(ENGRAVE_PAGE_URI).val;
}

//! NOTE The step of the nudge with the Ctrl (Cmd) key, in sp
inline constexpr double ENGRAVE_QUICK_NUDGE_STEP = 1.0;
}
