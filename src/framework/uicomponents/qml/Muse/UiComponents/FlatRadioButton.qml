/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
import QtQuick.Controls 2.15

import Muse.Ui 1.0

RadioDelegate {
    id: root

    //! NOTE Don't use the `icon` property.
    //!      It's a property of the ancestor of RadioDelegate
    //!      and has the wrong type (QQuickIcon).
    //!      It can't be overridden either, as it is marked `FINAL`.
    property int iconCode: IconCode.NONE
    property int iconFontSize: ui.theme.iconsFont.pixelSize

    property string toolTipTitle: ""
    property string toolTipDescription: ""
    property string toolTipShortcut: ""

    property alias radius: backgroundRect.radius

    property bool transparent: false
    property color normalColor: transparent ? "transparent" : ui.theme.buttonColor
    property color hoverHitColor: ui.theme.buttonColor
    property color checkedColor: ui.theme.accentColor

    property alias navigation: navCtrl
    property alias navigationFocusBorder: navigationFocusBorder

    //! NOTE: if the theme asks for it, the buttons of a horizontal list are joined into one strip:
    //! they fill the spacing between them (except for a thin divider), and only the ends of the strip are rounded
    readonly property bool isJoined: ui.theme.joinedButtonGroups && !transparent && Boolean(ListView.view)
                                     && ListView.view.orientation === ListView.Horizontal && ListView.view.count > 1
    readonly property bool isFirstInStrip: !isJoined || root.x < 1
    readonly property bool isLastInStrip: !isJoined || root.x + root.width > ListView.view.contentWidth - 1

    ButtonGroup.group: ListView.view && ListView.view instanceof RadioButtonGroup ? ListView.view.radioButtonGroup : null

    implicitHeight: {
        if (ListView.view && ListView.view.orientation === ListView.Horizontal) {
            return ListView.view.height
        } else {
            return ui.theme.defaultButtonSize
        }
    }
    implicitWidth: {
        if (ListView.view) {
            if (ListView.view.orientation === ListView.Horizontal) {
                return (ListView.view.width - (ListView.view.spacing * (ListView.view.count - 1))) / ListView.view.count
            } else {
                return ListView.view.width
            }
        } else {
            return ui.theme.defaultButtonSize
        }
    }

    hoverEnabled: root.enabled

    onClicked: {
        navigation.requestActiveByInteraction()
    }

    onPressedChanged: {
        ui.tooltip.hide(root, true)
    }

    onHoveredChanged: {
        if (!Boolean(root.toolTipTitle)) {
            return
        }

        if (hovered) {
            ui.tooltip.show(root, root.toolTipTitle, root.toolTipDescription, root.toolTipShortcut)
        } else {
            ui.tooltip.hide(root)
        }
    }

    NavigationControl {
        id: navCtrl
        name: root.objectName != "" ? root.objectName : "FlatRadioButton"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.RadioButton
        accessible.name: root.text
        accessible.checked: root.checked

        onTriggered: root.toggled()
    }

    background: Item {
        id: backgroundItem

        readonly property real dividerWidth: 1
        readonly property real spacingToFill: root.isJoined ? Math.max(root.ListView.view.spacing - dividerWidth, 0)
                                                            : 0

        //! NOTE: used by the navigation focus border
        readonly property real radius: backgroundRect.radius

        anchors.fill: parent
        anchors.leftMargin: root.isFirstInStrip ? 0 : -Math.floor(spacingToFill / 2)
        anchors.rightMargin: root.isLastInStrip ? 0 : -Math.ceil(spacingToFill / 2)

        Item {
            anchors.fill: parent

            //! NOTE: clips the rounded corners away from the sides at which the strip continues
            clip: root.isJoined

            Rectangle {
                id: backgroundRect

                anchors.fill: parent
                anchors.leftMargin: root.isFirstInStrip ? 0 : -radius
                anchors.rightMargin: root.isLastInStrip ? 0 : -radius

                color: root.checked ? root.checkedColor : root.normalColor
                opacity: root.isJoined && root.checked ? 1.0 : ui.theme.buttonOpacityNormal

                border.width: ui.theme.borderWidth
                border.color: ui.theme.strokeColor
                radius: root.isJoined ? 3 : 2

                states: [
                    State {
                        name: "HOVERED"
                        when: root.hovered && !root.pressed

                        PropertyChanges {
                            target: backgroundRect
                            color: root.checked ? root.checkedColor : root.hoverHitColor
                            opacity: root.isJoined && root.checked ? 0.9 : ui.theme.buttonOpacityHover
                        }
                    },

                    State {
                        name: "PRESSED"
                        when: root.pressed

                        PropertyChanges {
                            target: backgroundRect
                            color: root.checked ? root.checkedColor : root.hoverHitColor
                            opacity: root.isJoined && root.checked ? 0.8 : ui.theme.buttonOpacityHit
                        }
                    }
                ]
            }
        }

        NavigationFocusBorder {
            id: navigationFocusBorder
            navigationCtrl: navCtrl
            opacity: backgroundRect.opacity
        }
    }

    contentItem: Loader {
        id: contentLoader

        sourceComponent: {
            if (root.iconCode && root.iconCode !== IconCode.NONE) {
                return iconComponent
            }

            if (root.text) {
                return textComponent
            }

            return null
        }

        Component {
            id: iconComponent

            StyledIconLabel {
                iconCode: root.iconCode
                font.pixelSize: root.iconFontSize
                color: root.isJoined && root.checked ? "#FFFFFF" : ui.theme.fontPrimaryColor
            }
        }

        Component {
            id: textComponent

            StyledTextLabel {
                text: root.text
                maximumLineCount: 1
                color: root.isJoined && root.checked ? "#FFFFFF" : ui.theme.fontPrimaryColor
            }
        }
    }

    indicator: null
}
