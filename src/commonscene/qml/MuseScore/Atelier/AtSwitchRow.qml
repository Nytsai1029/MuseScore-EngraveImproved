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

//! NOTE: a line of a pane: what is switched on the left, the switch on the right
Item {
    id: root

    property string text: ""
    property alias checked: toggle.checked
    property alias navigation: toggle.navigation

    property font font: AtStyle.font(13)
    property int minimumHeight: 30

    signal toggled()

    implicitHeight: Math.max(minimumHeight, label.implicitHeight + 8)
    height: implicitHeight

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    StyledTextLabel {
        id: label

        anchors.left: parent.left
        anchors.right: toggle.left
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter

        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap
        maximumLineCount: 2

        text: root.text
        font: root.font
    }

    MouseArea {
        anchors.fill: label
        enabled: root.enabled

        onClicked: {
            root.toggled()
        }
    }

    AtSwitch {
        id: toggle

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        //! NOTE: the opacity of the disabled state is on the whole line
        opacity: 1.0

        navigation.name: root.text
        navigation.accessible.name: root.text

        onToggled: {
            root.toggled()
        }
    }
}
