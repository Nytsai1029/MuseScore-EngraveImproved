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
import MuseScore.NotationScene 1.0
import MuseScore.Atelier 1.0

//! NOTE: the tool strip of the Input mode. It has the buttons of the note input toolbar (the same ones, as the user
//! has customized them), in groups, with the key of each button written below it.
//! The note input methods are one button which shows the method in use
Rectangle {
    id: root

    property NavigationSection navigationSection: null

    property bool keyHintsVisible: true

    implicitHeight: strip.height + 9
    color: AtStyle.island

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "NoteInputBar"
        section: root.navigationSection
        order: 1
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("notation", "Note input toolbar")
    }

    NoteInputBarModel {
        id: noteInputModel
    }

    Component.onCompleted: {
        noteInputModel.load()
    }

    QtObject {
        id: prv

        //! NOTE: the method which the button toggles while note input is off
        property string lastMethodId: ""

        function isMethod(item) {
            return Boolean(item) && String(item.code).indexOf("note-input") === 0
        }
    }

    Row {
        id: strip

        anchors.horizontalCenter: parent.horizontalCenter
        y: 5

        spacing: 1

        enabled: noteInputModel.isInputAllowed

        // ----- the note input method -----

        Item {
            id: methodButton

            readonly property var methodItems: {
                var result = []
                for (var i = 0; i < methodsRepeater.count; ++i) {
                    var obj = methodsRepeater.itemAt(i)
                    if (Boolean(obj) && prv.isMethod(obj.item)) {
                        result.push(obj.item)
                    }
                }
                return result
            }

            readonly property var checkedItem: {
                for (var i = 0; i < methodItems.length; ++i) {
                    if (methodItems[i].checked) {
                        return methodItems[i]
                    }
                }
                return null
            }

            readonly property var currentItem: {
                if (Boolean(checkedItem)) {
                    return checkedItem
                }

                for (var i = 0; i < methodItems.length; ++i) {
                    if (methodItems[i].id === prv.lastMethodId) {
                        return methodItems[i]
                    }
                }

                return methodItems.length > 0 ? methodItems[0] : null
            }

            onCheckedItemChanged: {
                if (Boolean(checkedItem)) {
                    prv.lastMethodId = checkedItem.id
                }
            }

            visible: methodItems.length > 0

            width: visible ? pill.width + 17 : 0
            height: 32 + (root.keyHintsVisible ? 12 : 0)

            //! NOTE: only to read the items of the model
            Repeater {
                id: methodsRepeater

                model: noteInputModel

                delegate: Item {
                    readonly property var item: model.itemRole
                    readonly property string section: model.section
                    visible: false
                }
            }

            Rectangle {
                id: pill

                readonly property bool isOn: Boolean(methodButton.checkedItem)
                readonly property color contentColor: isOn ? "#FFFFFF" : AtStyle.text

                width: pillRow.width + 19
                height: 32

                radius: 8
                color: isOn ? AtStyle.accent
                            : (pillMouseArea.pressed ? AtStyle.pressed
                                                     : (pillMouseArea.containsMouse ? AtStyle.hover : AtStyle.button))

                NavigationControl {
                    id: pillNavCtrl

                    name: "NoteInputMethod"
                    panel: root.navigationPanel
                    order: 0
                    enabled: strip.enabled && root.visible

                    accessible.role: MUAccessible.Button
                    accessible.name: Boolean(methodButton.currentItem) ? methodButton.currentItem.title : ""

                    onTriggered: {
                        pill.toggle()
                    }
                }

                NavigationFocusBorder {
                    navigationCtrl: pillNavCtrl
                }

                function toggle() {
                    if (Boolean(methodButton.currentItem)) {
                        Qt.callLater(noteInputModel.handleMenuItem, methodButton.currentItem.id)
                    }
                }

                //! NOTE: not in the handler of the click itself, otherwise the same click closes the menu
                function openMethodsMenu() {
                    Qt.callLater(methodsMenu.toggleOpened, methodButton.methodItems)
                }

                Row {
                    id: pillRow

                    x: 10
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 6

                    AtIcon {
                        anchors.verticalCenter: parent.verticalCenter
                        name: "pencil"
                        color: pill.contentColor
                    }

                    StyledTextLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        text: Boolean(methodButton.currentItem) ? methodButton.currentItem.title : ""
                        font: AtStyle.font(12.5, Font.DemiBold)
                        color: pill.contentColor
                    }

                    AtIcon {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 12
                        height: 12
                        name: "chevron-down"
                        color: pill.contentColor
                        visible: methodButton.methodItems.length > 1
                    }
                }

                MouseArea {
                    id: pillMouseArea

                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: function(mouse) {
                        pillNavCtrl.requestActiveByInteraction()

                        //! NOTE: the arrow at the end opens the list of the methods
                        if (methodButton.methodItems.length > 1 && mouse.x > pill.width - 26) {
                            pill.openMethodsMenu()
                            return
                        }

                        pill.toggle()
                    }

                    onPressAndHold: {
                        if (methodButton.methodItems.length > 1) {
                            pill.openMethodsMenu()
                        }
                    }

                    onPressed: {
                        ui.tooltip.hide(pill, true)
                    }

                    onContainsMouseChanged: {
                        if (!Boolean(methodButton.currentItem)) {
                            return
                        }

                        if (containsMouse) {
                            ui.tooltip.show(pill, methodButton.currentItem.title, methodButton.currentItem.description,
                                            methodButton.currentItem.shortcuts)
                        } else {
                            ui.tooltip.hide(pill)
                        }
                    }
                }

                StyledMenuLoader {
                    id: methodsMenu

                    onHandleMenuItem: function(itemId) {
                        prv.lastMethodId = itemId
                        noteInputModel.handleMenuItem(itemId)
                    }
                }
            }

            StyledTextLabel {
                x: 0
                y: 33
                width: pill.width
                height: 11

                visible: root.keyHintsVisible
                text: Boolean(methodButton.currentItem) ? AtStyle.keyHint(methodButton.currentItem.shortcuts) : ""
                font: AtStyle.font(9, Font.Medium)
                color: AtStyle.text3
            }

            AtDivider {
                x: pill.width + 2
                y: 5
                lineHeight: 22
                sideMargin: 7
            }
        }

        // ----- the tools, in the groups of the toolbar -----

        Repeater {
            model: noteInputModel

            delegate: Row {
                id: cell

                readonly property var item: model.itemRole
                readonly property bool isMethod: prv.isMethod(item)
                readonly property bool startsGroup: model.index > 0 && !isMethod && model.section !== cell.previousSection
                readonly property string previousSection: {
                    //! NOTE: the methods are not shown here, so the group after them does not start with a divider
                    for (var i = model.index - 1; i >= 0; --i) {
                        var prev = methodsRepeater.itemAt(i)
                        if (Boolean(prev) && !prv.isMethod(prev.item)) {
                            return prev.section
                        }
                    }
                    return model.section
                }

                visible: !isMethod
                spacing: 0

                AtDivider {
                    y: 5
                    lineHeight: 22
                    sideMargin: 7
                    visible: cell.startsGroup
                }

                AtStripButton {
                    id: btn

                    readonly property bool hasMenu: Boolean(cell.item) && cell.item.subitems.length !== 0

                    keyHintsVisible: root.keyHintsVisible
                    keyHint: Boolean(cell.item) ? AtStyle.keyHint(cell.item.shortcuts) : ""

                    icon: Boolean(cell.item) ? cell.item.icon : IconCode.NONE
                    hasMenuArrow: btn.hasMenu && Boolean(cell.item) && cell.item.code === "add"

                    checked: (Boolean(cell.item) && cell.item.checked) || menuLoader.isMenuOpened

                    toolTipTitle: Boolean(cell.item) ? cell.item.title : ""
                    toolTipDescription: Boolean(cell.item) ? cell.item.description : ""
                    toolTipShortcut: Boolean(cell.item) ? cell.item.shortcuts : ""

                    navigation.panel: root.navigationPanel
                    navigation.name: Boolean(cell.item) ? cell.item.id : ""
                    navigation.order: model.index + 1

                    onClicked: {
                        if (btn.hasMenu) {
                            menuLoader.toggleOpened(cell.item.subitems)
                        } else {
                            Qt.callLater(noteInputModel.handleMenuItem, cell.item.id)
                        }
                    }

                    StyledMenuLoader {
                        id: menuLoader

                        onHandleMenuItem: function(itemId) {
                            noteInputModel.handleMenuItem(itemId)
                        }
                    }
                }
            }
        }
    }

    //! NOTE: the buttons of the strip are chosen in the same popup as in the other skins
    MouseArea {
        anchors.fill: parent
        z: -1
        acceptedButtons: Qt.RightButton

        onClicked: function(mouse) {
            stripMenu.show(Qt.point(mouse.x, mouse.y))
        }

        ContextMenuLoader {
            id: stripMenu

            items: [
                { id: "customize", title: qsTrc("notation", "Customize toolbar") }
            ]

            onHandleMenuItem: function(itemId) {
                if (itemId === "customize") {
                    customizePopup.toggleOpened()
                }
            }
        }
    }

    Item {
        id: customizeAnchor
        anchors.right: parent.right
        anchors.rightMargin: 40
        anchors.bottom: parent.bottom
        width: 1
        height: 1

        NoteInputBarCustomisePopup {
            id: customizePopup
            anchorItem: ui.rootItem
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: AtStyle.stroke
    }
}
