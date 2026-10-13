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

//! NOTE: an on/off switch. It does not change `checked` itself: the owner does it in `onToggled`
FocusScope {
    id: root

    property alias navigation: navCtrl

    property bool checked: false

    signal toggled()

    implicitWidth: 30
    implicitHeight: 18

    width: implicitWidth
    height: implicitHeight

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    NavigationControl {
        id: navCtrl
        name: root.objectName != "" ? root.objectName : "Switch"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.CheckBox
        accessible.checked: root.checked

        onTriggered: root.toggled()
    }

    Rectangle {
        anchors.fill: parent

        radius: height / 2
        color: root.checked ? AtStyle.accent : AtStyle.stroke2

        Behavior on color {
            ColorAnimation { duration: 150 }
        }

        NavigationFocusBorder {
            navigationCtrl: navCtrl
        }

        StyledRectangularShadow {
            anchors.fill: knob
            offset.y: 0.5
            blur: 3
            radius: knob.radius
        }

        Rectangle {
            id: knob

            x: root.checked ? parent.width - width - 2 : 2
            y: 2
            width: parent.height - 4
            height: width
            radius: width / 2

            color: "#FFFFFF"

            Behavior on x {
                NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        anchors.margins: -4

        enabled: root.enabled

        onClicked: {
            navCtrl.requestActiveByInteraction()
            root.toggled()
        }
    }
}
