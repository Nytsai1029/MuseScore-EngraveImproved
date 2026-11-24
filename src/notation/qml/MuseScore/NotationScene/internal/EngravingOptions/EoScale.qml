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

//! NOTE: a scale in two directions: the width and the height of a sign as multiples of its normal size, side by side
EoOption {
    kind: "scale"

    property real min: 0.5
    property real max: 10
    property real step: 0.25
    property int decimals: 2

    //! NOTE: the style value (a switch) which keeps the width and the height the same: while it is on, a new width is
    //! also the height and the other way round. The control shows it as a lock between the two numbers, so it is
    //! one of the values of the option
    property string lockSid: ""

    extraSids: Boolean(lockSid) ? [lockSid] : []
}
