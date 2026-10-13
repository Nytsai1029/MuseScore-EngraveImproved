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

//! NOTE: a secondary button of a pane: a thin outline, no fill until it is hovered
FlatButton {
    id: root

    height: 28

    implicitWidth: label.implicitWidth + 22

    transparent: true
    isClickOnKeyNavTriggered: true

    backgroundItem: Rectangle {
        radius: 7

        color: {
            if (root.mouseArea.pressed) {
                return AtStyle.pressed
            }

            return root.mouseArea.containsMouse ? AtStyle.hover : "transparent"
        }

        border.width: 1
        border.color: AtStyle.stroke2

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
        width: Math.min(implicitWidth, parent.width - 12)

        text: root.text
        font: root.textFont
    }

    textFont: AtStyle.font(13, Font.Medium)
}
