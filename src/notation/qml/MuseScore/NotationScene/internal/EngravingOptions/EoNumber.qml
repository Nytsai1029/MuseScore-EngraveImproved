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

//! NOTE: a number: a distance, a thickness, a ratio, a count
EoOption {
    kind: "number"

    property real min: 0
    property real max: 99.99
    property real step: 0.05
    property int decimals: 2

    //! NOTE: what is written after the number. "auto": staff spaces if the value is a distance
    //! in staff spaces, else nothing
    property string unit: "auto"

    //! NOTE: the value is a ratio which is shown as a percentage (0.7 as 70 %)
    property bool percent: false

    //! NOTE: for a value which is a point (an offset) and of which the option is one coordinate: "x" or "y"
    property string axis: ""
}
