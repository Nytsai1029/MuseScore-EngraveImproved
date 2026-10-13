/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

GridLayout {
    id: root

    property string text: ""

    //! NOTE: in the Atelier skin, the name of the option is before its control in one line (and not above it)
    readonly property bool rowStyle: ui.theme.skin === "atelier"

    width: rowStyle ? parent.width : Math.min(implicitWidth, parent.width)

    columns: rowStyle ? 2 : 1
    rowSpacing: 8
    columnSpacing: 12

    StyledTextLabel {
        id: label
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignLeft

        //! NOTE: a name in a line does not end with a colon
        text: root.rowStyle ? root.text.replace(/\s*[:：]\s*$/, "") : root.text
    }
}
