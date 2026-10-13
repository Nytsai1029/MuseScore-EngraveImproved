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

//! NOTE: a command in a list of a pane: its name on the left, its key on the right
FlatButton {
    id: root

    //! NOTE: e.g. "⌘R"
    property string keyHint: ""

    height: Math.max(28, label.implicitHeight + 6)

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

        NavigationFocusBorder {
            navigationCtrl: root.navigation
        }
    }

    contentItem: Item {
        implicitWidth: 0
        implicitHeight: 0
    }

    StyledTextLabel {
        id: label

        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.right: hintLabel.visible ? hintLabel.left : parent.right
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter

        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap
        maximumLineCount: 2

        text: root.text
        font: AtStyle.font(12.5)
    }

    StyledTextLabel {
        id: hintLabel

        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter

        visible: Boolean(root.keyHint)

        text: root.keyHint
        font: AtStyle.font(11)
        color: AtStyle.text3
    }
}
