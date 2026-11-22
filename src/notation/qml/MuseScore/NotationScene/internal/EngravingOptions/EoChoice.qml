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

//! NOTE: one of several values
EoOption {
    kind: "choice"

    //! NOTE: a list of { value, title }. For "tiles" a choice can also have `overrides`: the style values which
    //! its picture sets besides the one of the option
    property var choices: []

    //! NOTE: how the choices are shown:
    //! "segmented": side by side on a track (two to four short titles),
    //! "dropdown": in a drop-down list,
    //! "list": one under the other with radio buttons (long titles),
    //! "tiles": as pictures of what each choice does to `snippet`
    property string presentation: "segmented"

    //! NOTE: for "tiles": the snippet of music (see EngravingPreview), the zoom of the pictures,
    //! the size of a tile, and the width of the systems of the snippet in staff spaces (0: as wide as the music)
    property string snippet: ""
    property real tileZoom: 1.0
    property int tileWidth: 148
    property int tileHeight: 92
    property real systemWidth: 0
}
