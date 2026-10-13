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
import QtQuick 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

//! NOTE: a small rounded button which is on or off (a state of the selection, something which is shown)
FlatButton {
    id: root

    property bool checked: false

    width: implicitWidth
    height: 24

    implicitWidth: label.implicitWidth + 20

    transparent: true
    isClickOnKeyNavTriggered: true

    accessible.role: MUAccessible.CheckBox
    accessible.checked: root.checked

    backgroundItem: Rectangle {
        radius: height / 2

        color: {
            if (root.checked) {
                return AtStyle.soft
            }

            if (root.mouseArea.pressed) {
                return AtStyle.pressed
            }

            return root.mouseArea.containsMouse ? AtStyle.hover : AtStyle.button
        }

        NavigationFocusBorder {
            navigationCtrl: root.navigation
        }
    }

    contentItem: Item {
        implicitWidth: label.implicitWidth
        implicitHeight: label.implicitHeight
    }

    StyledTextLabel {
        id: label

        anchors.centerIn: parent

        text: root.text
        font: AtStyle.font(12, Font.Medium)
        color: root.checked ? AtStyle.ink : AtStyle.text2
    }
}
