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

//! NOTE: a picture of a snippet of music with the options as they are (see EngravingPreview).
//! The title is written under the picture
EoOption {
    kind: "preview"

    property string snippet: ""

    //! NOTE: style values which the picture sets whatever the options are
    property var overrides: ({})

    property real zoom: 1.0

    //! NOTE: the width of the systems of the snippet in staff spaces (0: as wide as the music)
    property real systemWidth: 0
}
