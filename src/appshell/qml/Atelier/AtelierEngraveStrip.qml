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
import MuseScore.AppShell 1.0
import MuseScore.NotationScene 1.0
import MuseScore.Atelier 1.0

//! NOTE: the tool strip of the Engrave mode: moving what is selected (the arrow keys do the same),
//! flipping and resetting it, the stretch of the measures, the placement and what is shown on the score
Rectangle {
    id: root

    property NavigationSection navigationSection: null

    property bool keyHintsVisible: true

    implicitHeight: strip.height + 9
    color: AtStyle.island

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "EngraveBar"
        section: root.navigationSection
        order: 1
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("appshell", "Engrave tools")
    }

    EngraveModeModel {
        id: engraveModel

        Component.onCompleted: {
            load()
        }
    }

    AtelierActionsModel {
        id: actions

        actionCodes: [
            "notation-move-left", "pitch-up", "pitch-down", "notation-move-right",
            "flip", "reset", "stretch-", "stretch+", "toggle-autoplace",
            "show-invisible", "show-unprintable", "show-frames", "show-pageborders", "show-alignment-guides"
        ]

        Component.onCompleted: {
            load()
        }
    }

    //! NOTE: the actions by their codes
    readonly property var items: {
        var result = {}
        var list = actions.items

        for (var i = 0; i < list.length; ++i) {
            result[list[i].code] = list[i]
        }

        return result
    }

    component ActionButton: AtStripButton {
        property string code: ""
        readonly property var item: root.items[code]

        enabled: Boolean(item) && item.enabled

        toolTipTitle: Boolean(item) ? AtStyle.plainTitle(item.title) : ""
        keyHint: Boolean(item) ? AtStyle.keyHint(item.shortcuts) : ""
        keyHintsVisible: root.keyHintsVisible

        navigation.panel: root.navigationPanel
        navigation.name: code

        onClicked: {
            actions.trigger(code)
        }
    }

    component ShowChip: Item {
        id: chipCell

        property string code: ""
        property alias text: chip.text
        property alias navigation: chip.navigation

        readonly property var item: root.items[code]

        width: chip.width
        height: 32

        AtChip {
            id: chip

            anchors.verticalCenter: parent.verticalCenter

            checked: Boolean(chipCell.item) && chipCell.item.checked
            enabled: Boolean(chipCell.item) && chipCell.item.enabled

            toolTipTitle: Boolean(chipCell.item) ? AtStyle.plainTitle(chipCell.item.title) : ""

            navigation.panel: root.navigationPanel
            navigation.name: chipCell.code

            onClicked: {
                actions.trigger(chipCell.code)
            }
        }
    }

    Row {
        id: strip

        anchors.horizontalCenter: parent.horizontalCenter
        y: 5

        spacing: 1

        // ----- nudge -----

        AtStripButton {
            buttonWidth: 28
            shape: "arrow-left"
            enabled: engraveModel.hasSelection

            toolTipTitle: qsTrc("appshell", "Nudge left")
            keyHint: "←"
            keyHintsVisible: root.keyHintsVisible

            navigation.panel: root.navigationPanel
            navigation.name: "NudgeLeft"
            navigation.order: 1

            onClicked: {
                actions.trigger("notation-move-left")
            }
        }

        AtStripButton {
            buttonWidth: 28
            shape: "arrow-up"
            enabled: engraveModel.hasSelection

            toolTipTitle: qsTrc("appshell", "Nudge up")
            keyHint: "↑"
            keyHintsVisible: root.keyHintsVisible

            navigation.panel: root.navigationPanel
            navigation.name: "NudgeUp"
            navigation.order: 2

            onClicked: {
                actions.trigger("pitch-up")
            }
        }

        AtStripButton {
            buttonWidth: 28
            shape: "arrow-down"
            enabled: engraveModel.hasSelection

            toolTipTitle: qsTrc("appshell", "Nudge down")
            keyHint: "↓"
            keyHintsVisible: root.keyHintsVisible

            navigation.panel: root.navigationPanel
            navigation.name: "NudgeDown"
            navigation.order: 3

            onClicked: {
                actions.trigger("pitch-down")
            }
        }

        AtStripButton {
            buttonWidth: 28
            shape: "arrow-right"
            enabled: engraveModel.hasSelection

            toolTipTitle: qsTrc("appshell", "Nudge right")
            keyHint: "→"
            keyHintsVisible: root.keyHintsVisible

            navigation.panel: root.navigationPanel
            navigation.name: "NudgeRight"
            navigation.order: 4

            onClicked: {
                actions.trigger("notation-move-right")
            }
        }

        // ----- the step of the nudge -----

        Item {
            width: 8
            height: 1
        }

        Repeater {
            //! NOTE: the keys are the modifiers which give this step whatever step is chosen
            model: [
                { step: 0.01, title: "0.01", key: "" },
                { step: 0.1, title: "0.1", key: "" },
                { step: 1.0, title: "1", key: Qt.platform.os === "osx" ? "⌘" : "Ctrl" }
            ]

            delegate: Item {
                id: stepCell

                readonly property bool isCurrent: Math.abs(engraveModel.nudgeStep - modelData.step) < 0.0001

                width: stepButton.width
                height: 32 + (root.keyHintsVisible ? 12 : 0)

                AtToolButton {
                    id: stepButton

                    y: 4
                    width: Math.max(30, implicitWidth)
                    height: 24
                    radius: 6

                    text: modelData.title
                    textFont: AtStyle.font(12, Font.DemiBold)
                    contentColor: AtStyle.text

                    checked: stepCell.isCurrent
                    checkedStyle: "accent"

                    //: %1 is a distance in staff spaces, e.g. 0.1
                    toolTipTitle: qsTrc("appshell", "Step %1 sp").arg(modelData.title)

                    navigation.panel: root.navigationPanel
                    navigation.name: "NudgeStep" + modelData.title
                    navigation.order: 5 + index
                    navigation.accessible.name: toolTipTitle

                    onClicked: {
                        engraveModel.nudgeStep = modelData.step
                    }
                }

                StyledTextLabel {
                    y: 33
                    width: parent.width
                    height: 11

                    visible: root.keyHintsVisible

                    text: modelData.key
                    font: AtStyle.font(9, Font.Medium)
                    color: AtStyle.text3
                }
            }
        }

        Item {
            width: unitLabel.implicitWidth + 4
            height: 32

            StyledTextLabel {
                id: unitLabel

                x: 4
                anchors.verticalCenter: parent.verticalCenter

                text: qsTrc("global", "sp")
                font: AtStyle.font(11.5)
                color: AtStyle.text3
            }
        }

        AtDivider {
            y: 5
            lineHeight: 22
            sideMargin: 9
        }

        // ----- the selection -----

        ActionButton {
            code: "flip"
            shape: "flip"
            navigation.order: 10
        }

        ActionButton {
            code: "reset"
            shape: "reset"
            navigation.order: 11
        }

        AtDivider {
            y: 5
            lineHeight: 22
            sideMargin: 9
        }

        // ----- the stretch of the measures -----

        ActionButton {
            code: "stretch-"
            shape: "stretch-less"
            navigation.order: 12
        }

        ActionButton {
            code: "stretch+"
            shape: "stretch-more"
            navigation.order: 13
        }

        AtDivider {
            y: 5
            lineHeight: 22
            sideMargin: 9
        }

        // ----- placement -----

        Row {
            height: 32
            spacing: 4

            AtChip {
                anchors.verticalCenter: parent.verticalCenter

                text: qsTrc("inspector", "Auto-place")
                checked: engraveModel.isAutoplaceOn
                enabled: engraveModel.hasSelection

                toolTipTitle: Boolean(root.items["toggle-autoplace"]) ? AtStyle.plainTitle(root.items["toggle-autoplace"].title)
                                                                      : ""

                navigation.panel: root.navigationPanel
                navigation.name: "AutoPlace"
                navigation.order: 14

                onClicked: {
                    engraveModel.toggleAutoplace()
                }
            }

            AtChip {
                anchors.verticalCenter: parent.verticalCenter

                text: qsTrc("inspector", "Snap to grid")
                checked: engraveModel.isSnappedToGrid

                navigation.panel: root.navigationPanel
                navigation.name: "SnapToGrid"
                navigation.order: 15

                onClicked: {
                    engraveModel.isSnappedToGrid = !engraveModel.isSnappedToGrid
                }
            }
        }

        AtDivider {
            y: 5
            lineHeight: 22
            sideMargin: 9
        }

        // ----- what is shown -----

        Row {
            height: 32
            spacing: 4

            StyledTextLabel {
                anchors.verticalCenter: parent.verticalCenter
                rightPadding: 4

                text: qsTrc("inspector", "Show")
                font: AtStyle.font(11.5)
                color: AtStyle.text3
            }

            ShowChip {
                code: "show-invisible"
                text: qsTrc("inspector", "Invisible")
                navigation.order: 16
            }

            ShowChip {
                code: "show-unprintable"
                text: qsTrc("inspector", "Formatting")
                navigation.order: 17
            }

            ShowChip {
                code: "show-frames"
                text: qsTrc("inspector", "Frames")
                navigation.order: 18
            }

            ShowChip {
                code: "show-pageborders"
                text: qsTrc("appshell", "Margins")
                navigation.order: 19
            }

            ShowChip {
                code: "show-alignment-guides"
                text: qsTrc("inspector", "Guides")
                navigation.order: 20
            }

            //! NOTE: not an action of the score: a working aid which is shown in this mode only
            AtChip {
                anchors.verticalCenter: parent.verticalCenter

                text: qsTrc("appshell", "Ledger guides")
                checked: engraveModel.isLedgerGuidesVisible

                toolTipTitle: qsTrc("appshell",
                                    "Show guide lines at the ledger line positions above and below each staff")

                navigation.panel: root.navigationPanel
                navigation.name: "LedgerGuides"
                navigation.order: 21

                onClicked: {
                    engraveModel.isLedgerGuidesVisible = !engraveModel.isLedgerGuidesVisible
                }
            }
        }
    }
}
