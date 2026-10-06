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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0

FocusScope {
    id: root

    property bool checked: false
    property alias pressed: clickableArea.containsPress
    property alias hovered: clickableArea.containsMouse
    property bool isIndeterminate: false

    //! NOTE: shows a switch instead of the box
    property bool showAsSwitch: false

    property alias text: label.text
    property alias font: label.font
    property alias backgroundColor: box.color
    property alias backgroundOpacity: box.opacity

    property alias navigation: navCtrl

    signal clicked

    implicitHeight: contentRow.implicitHeight
    implicitWidth: contentRow.implicitWidth

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }
    }

    QtObject {
        id: prv

        readonly property bool isOn: root.checked || root.isIndeterminate

        //! NOTE: the switch and the filled box (if the theme asks for it) show their state by the accent color
        readonly property bool isFilled: root.showAsSwitch || ui.theme.filledCheckBoxes
        readonly property color borderColor: ui.theme.controlBorderColor.valid ? ui.theme.controlBorderColor
                                                                               : ui.theme.strokeColor
    }

    NavigationControl {
        id: navCtrl

        name: root.objectName != "" ? root.objectName : "CheckBox"
        enabled: root.enabled && root.visible
        accessible.role: MUAccessible.CheckBox
        accessible.name: root.text
        accessible.checked: root.checked

        onActiveChanged: {
            if (!root.activeFocus) {
                root.forceActiveFocus()
            }
        }

        onTriggered: root.clicked()
    }

    RowLayout {
        id: contentRow
        spacing: 6

        Rectangle {
            id: box

            height: root.showAsSwitch ? 14 : (prv.isFilled ? 16 : 20)
            width: root.showAsSwitch ? 26 : height

            opacity: prv.isFilled ? 1.0 : ui.theme.buttonOpacityNormal

            border.width: prv.isFilled ? (prv.isOn ? 0 : 1) : ui.theme.borderWidth
            border.color: prv.isFilled ? prv.borderColor : ui.theme.strokeColor
            color: {
                if (!prv.isFilled) {
                    return ui.theme.buttonColor
                }

                if (prv.isOn) {
                    return ui.theme.accentColor
                }

                return root.showAsSwitch ? ui.theme.backgroundSecondaryColor : ui.theme.textFieldColor
            }

            radius: root.showAsSwitch ? height / 2 : 2

            NavigationFocusBorder { navigationCtrl: navCtrl }

            StyledIconLabel {
                anchors.fill: parent
                iconCode: root.isIndeterminate ? IconCode.MINUS : IconCode.TICK_RIGHT_ANGLE
                font.pixelSize: prv.isFilled ? 12 : ui.theme.iconsFont.pixelSize
                color: prv.isFilled ? "#FFFFFF" : ui.theme.fontPrimaryColor
                visible: prv.isOn && !root.showAsSwitch
            }

            Rectangle {
                id: switchHandle

                readonly property real margin: 2

                anchors.verticalCenter: parent.verticalCenter
                x: root.isIndeterminate ? (parent.width - width) / 2
                                        : root.checked ? parent.width - width - margin : margin

                width: parent.height - 2 * margin
                height: width
                radius: width / 2

                color: prv.isOn ? "#FFFFFF" : ui.theme.fontPrimaryColor
                opacity: prv.isOn ? 1.0 : 0.35
                visible: root.showAsSwitch
            }
        }

        StyledTextLabel {
            id: label
            visible: !isEmpty

            readonly property real availableWidth: root.width - contentRow.spacing - box.width

            Layout.preferredWidth: availableWidth > 0 ? Math.min(availableWidth, label.implicitWidth) : label.implicitWidth
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
            maximumLineCount: 2
        }
    }

    MouseArea {
        id: clickableArea

        anchors.fill: contentRow
        anchors.margins: -4

        hoverEnabled: !label.hoveredLink
        z: label.z - 1 // enable clicking on links in label text

        onClicked: {
            navigation.requestActiveByInteraction()

            root.clicked()
        }
    }

    states: [
        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !clickableArea.pressed

            PropertyChanges {
                target: box
                opacity: prv.isFilled ? 0.8 : ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "PRESSED"
            when: clickableArea.containsMouse && clickableArea.pressed

            PropertyChanges {
                target: box
                opacity: prv.isFilled ? 0.6 : ui.theme.buttonOpacityHit
            }
        }
    ]
}
