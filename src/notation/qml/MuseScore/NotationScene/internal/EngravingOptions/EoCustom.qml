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
import QtQuick 2.15

//! NOTE: an option with a control of its own. The component is made with these properties, if it has them:
//! `option` (this object), `optionsModel` (the EngravingOptionsModel) and `navigationPanel`.
//! `sid` and `extraSids` name the style values which the control changes, so that they are reset and counted
EoOption {
    kind: "custom"

    property Component component: null

    //! NOTE: the control takes the whole width of the page instead of the place of a control beside the title
    property bool fullWidth: true
}
