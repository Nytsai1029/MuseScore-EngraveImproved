/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "autobot/iautobot.h"

#include "../cmdoptions.h"

namespace mu::app {
//! NOTE Runs the test case (a script of the autobot) and quits the app when the script is finished,
//! when one of its steps has failed (with the code of the step), or when the script was aborted
void launchTestCase(const std::shared_ptr<muse::autobot::IAutobot>& autobot, const CmdOptions::Autobot& task);
}
