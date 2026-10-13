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

//! NOTE: a button of a tool strip, with the key that triggers it written below
Item {
    id: root

    property alias button: btn

    property alias icon: btn.icon
    property alias text: btn.text
    property alias checked: btn.checked
    property alias checkedStyle: btn.checkedStyle
    property alias hasMenuArrow: btn.hasMenuArrow
    property alias shape: btn.shape
    property alias iconFont: btn.iconFont
    property alias textFont: btn.textFont
    property alias contentColor: btn.contentColor
    property alias toolTipTitle: btn.toolTipTitle
    property alias toolTipDescription: btn.toolTipDescription
    property alias toolTipShortcut: btn.toolTipShortcut
    property alias navigation: btn.navigation

    property int buttonWidth: 30
    property int buttonHeight: 32

    property string keyHint: ""
    property bool keyHintsVisible: true

    signal clicked(var mouse)

    implicitWidth: Math.max(buttonWidth, btn.implicitWidth)
    implicitHeight: buttonHeight + (keyHintsVisible ? 12 : 0)

    width: implicitWidth
    height: implicitHeight

    opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled

    AtToolButton {
        id: btn

        anchors.horizontalCenter: parent.horizontalCenter

        width: Boolean(text) || hasMenuArrow ? implicitWidth : root.buttonWidth
        height: root.buttonHeight

        contentColor: AtStyle.text
        iconFont: ui.theme.toolbarIconsFont

        //! NOTE: the opacity of the disabled state is on the whole item
        opacity: 1.0

        onClicked: function(mouse) {
            root.clicked(mouse)
        }
    }

    StyledTextLabel {
        anchors.top: btn.bottom
        anchors.topMargin: 1
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width + 6
        height: 11

        visible: root.keyHintsVisible

        text: root.keyHint
        font: AtStyle.font(9, Font.Medium)
        color: AtStyle.text3
    }
}
