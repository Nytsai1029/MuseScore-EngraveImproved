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

//! NOTE: a quiet button: no fill until it is hovered, pressed or checked
FlatButton {
    id: root

    property bool checked: false

    //! NOTE: "accent": filled with the accent color (tools which are on),
    //! "soft": tinted (the chosen item of a list), "raised": a key on a track
    property string checkedStyle: "accent"

    property color contentColor: AtStyle.text2
    property real radius: 7

    //! NOTE: a small arrow which tells that the button opens a menu
    property bool hasMenuArrow: false

    //! NOTE: the name of a line icon (see AtIcon), which is shown instead of the icon of the icon font
    property string shape: ""

    width: implicitWidth
    height: 28

    implicitWidth: Boolean(root.text) || root.hasMenuArrow ? contentLoaderWidth + 2 * textSidePadding
                                                           : 30

    readonly property int textSidePadding: Boolean(root.text) ? 8 : 6
    property real contentLoaderWidth: 0

    transparent: true
    isClickOnKeyNavTriggered: true

    readonly property color effectiveContentColor: {
        if (!root.checked) {
            return root.contentColor
        }

        switch (root.checkedStyle) {
        case "accent": return "#FFFFFF"
        case "soft": return AtStyle.ink
        }

        return AtStyle.text
    }

    backgroundItem: Rectangle {
        radius: root.radius

        color: {
            if (root.checked) {
                switch (root.checkedStyle) {
                case "accent": return AtStyle.accent
                case "soft": return AtStyle.soft
                }

                return AtStyle.raised
            }

            if (root.mouseArea.pressed) {
                return AtStyle.pressed
            }

            return root.mouseArea.containsMouse ? AtStyle.hover : "transparent"
        }

        border.width: root.checked && root.checkedStyle === "raised" ? 1 : 0
        border.color: AtStyle.stroke

        NavigationFocusBorder {
            navigationCtrl: root.navigation
        }
    }

    contentItem: Row {
        id: contentRow

        spacing: 5

        onImplicitWidthChanged: root.contentLoaderWidth = implicitWidth
        Component.onCompleted: root.contentLoaderWidth = implicitWidth

        AtIcon {
            anchors.verticalCenter: parent.verticalCenter
            name: root.shape
            color: root.effectiveContentColor
            visible: !isEmpty
        }

        StyledIconLabel {
            anchors.verticalCenter: parent.verticalCenter
            iconCode: root.icon
            font: root.iconFont
            color: root.effectiveContentColor
            visible: !isEmpty && !Boolean(root.shape)
        }

        StyledTextLabel {
            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            font: root.textFont
            color: root.effectiveContentColor
            visible: !isEmpty
        }

        AtIcon {
            anchors.verticalCenter: parent.verticalCenter
            width: 12
            height: 12
            name: "chevron-down"
            color: root.effectiveContentColor
            visible: root.hasMenuArrow
        }
    }
}
