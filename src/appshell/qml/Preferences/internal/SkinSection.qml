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
import QtQuick.Controls 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.GraphicalEffects 1.0

BaseSection {
    id: root

    title: qsTrc("appshell/preferences", "Interface skin")
    navigation.direction: NavigationPanel.Horizontal

    property alias skins: skinsRepeater.model
    property string currentSkinCode: ""

    //! NOTE: some skins are applied only when the app starts
    property bool restartRequired: false
    property string restartText: ""

    signal skinChangeRequested(string newSkinCode)
    signal restartRequested()

    readonly property color softAccentColor: Utils.colorWithAlpha(ui.theme.accentColor, ui.theme.accentOpacityNormal)

    StyledTextLabel {
        width: parent.width

        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap
        opacity: 0.75

        text: qsTrc("appshell/preferences", "How the window is arranged. Themes and accent color apply to every skin.")
    }

    Flow {
        id: cardsFlow

        //! NOTE: the cards share the width, but they are not wider than a column of the other sections
        readonly property real cardWidth: {
            var count = Math.max(skinsRepeater.count, 1)
            return Math.min(root.columnWidth, Math.floor((width - (count - 1) * spacing) / count))
        }

        width: parent.width
        spacing: root.columnSpacing

        Repeater {
            id: skinsRepeater

            delegate: FocusScope {
                id: card

                readonly property string code: modelData["code"]
                readonly property bool checked: root.currentSkinCode === code

                width: cardsFlow.cardWidth
                height: cardContent.height + 17

                NavigationControl {
                    id: navCtrl

                    name: "SkinButton_" + card.code
                    panel: root.navigation
                    column: model.index
                    enabled: card.enabled && card.visible

                    accessible.role: MUAccessible.RadioButton
                    accessible.name: modelData["title"]
                    accessible.description: modelData["description"]
                    accessible.checked: card.checked

                    onTriggered: {
                        root.skinChangeRequested(card.code)
                    }
                }

                Rectangle {
                    id: cardBackground

                    anchors.fill: parent

                    radius: 10
                    color: card.checked ? root.softAccentColor
                                        : (cardMouseArea.containsMouse ? Utils.colorWithAlpha(ui.theme.buttonColor, ui.theme.buttonOpacityNormal)
                                                                       : "transparent")
                    border.width: card.checked ? 1.5 : 1
                    border.color: card.checked ? ui.theme.accentColor
                                               : (ui.theme.controlBorderColor.valid ? ui.theme.controlBorderColor
                                                                                   : ui.theme.strokeColor)

                    NavigationFocusBorder { navigationCtrl: navCtrl }
                }

                Column {
                    id: cardContent

                    x: 8
                    y: 8
                    width: parent.width - 16
                    spacing: 0

                    //! NOTE: a small diagram of how the skin arranges the window
                    Rectangle {
                        id: diagram

                        readonly property color panelColor: ui.theme.backgroundPrimaryColor
                        readonly property color canvasColor: ui.theme.isDark ? "#4A4B52" : "#D5D6DC"
                        readonly property color paperColor: ui.theme.isDark ? "#F4F4F6" : "#FFFFFF"

                        width: parent.width
                        height: 104

                        radius: 6
                        color: canvasColor
                        clip: true

                        layer.enabled: ui.isEffectsAllowed
                        layer.effect: RoundedCornersEffect {
                            radius: diagram.radius
                        }

                        // The classic arrangement: toolbars on top, the panels on the left
                        Item {
                            anchors.fill: parent
                            visible: card.code !== "dorico" && card.code !== "atelier"

                            Rectangle { width: parent.width; height: 11; color: diagram.panelColor }
                            Rectangle { y: 12; width: parent.width; height: 9; color: diagram.panelColor }
                            Rectangle { y: 22; width: 44; height: parent.height - 22; color: diagram.panelColor }
                            Rectangle { x: 76; y: 30; width: 62; height: parent.height - 20; color: diagram.paperColor }
                        }

                        // Dorico: the tools on the left, the palettes on the right, the properties below
                        Item {
                            anchors.fill: parent
                            visible: card.code === "dorico"

                            Rectangle { width: parent.width; height: 11; color: diagram.panelColor }
                            Rectangle { y: 12; width: 12; height: parent.height - 12; color: diagram.panelColor }
                            Rectangle { x: parent.width - 10; y: 12; width: 10; height: parent.height - 12; color: diagram.panelColor }
                            Rectangle { x: 13; y: parent.height - 26; width: parent.width - 24; height: 26; color: diagram.panelColor }
                            Rectangle { x: 62; y: 20; width: 62; height: 66; color: diagram.paperColor }
                        }

                        // Atelier: the mode switcher, the tool strip, the panes on both sides, the status bar
                        Item {
                            anchors.fill: parent
                            visible: card.code === "atelier"

                            Rectangle { width: parent.width; height: 11; color: diagram.panelColor }
                            Rectangle { x: (parent.width - 42) / 2; y: 3; width: 42; height: 5; radius: 3; color: diagram.canvasColor }
                            Rectangle { x: (parent.width - 14) / 2; y: 3; width: 14; height: 5; radius: 3; color: ui.theme.accentColor }
                            Rectangle { y: 12; width: parent.width; height: 9; color: diagram.panelColor }
                            Rectangle { y: 22; width: 38; height: parent.height - 28; color: diagram.panelColor }
                            Rectangle { x: parent.width - 34; y: 22; width: 34; height: parent.height - 28; color: diagram.panelColor }
                            Rectangle { y: parent.height - 5; width: parent.width; height: 5; color: diagram.panelColor }
                            Rectangle { x: 62; y: 28; width: 56; height: parent.height - 34; color: diagram.paperColor }
                        }
                    }

                    Item { width: 1; height: 8 }

                    StyledTextLabel {
                        x: 2
                        width: parent.width - 4

                        horizontalAlignment: Text.AlignLeft
                        font: ui.theme.bodyBoldFont
                        text: modelData["title"]
                    }

                    Item { width: 1; height: 1 }

                    StyledTextLabel {
                        x: 2
                        width: parent.width - 4

                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        opacity: 0.75

                        text: modelData["description"]
                    }
                }

                MouseArea {
                    id: cardMouseArea

                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: {
                        navCtrl.requestActiveByInteraction()
                        root.skinChangeRequested(card.code)
                    }
                }
            }
        }
    }

    Rectangle {
        width: parent.width
        height: Math.max(restartLabel.height, restartButton.height) + 18

        visible: root.restartRequired

        radius: 9
        color: root.softAccentColor

        StyledIconLabel {
            id: restartIcon

            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.verticalCenter: parent.verticalCenter

            iconCode: IconCode.UPDATE
            color: ui.theme.linkColor
        }

        StyledTextLabel {
            id: restartLabel

            anchors.left: restartIcon.right
            anchors.leftMargin: 9
            anchors.right: restartButton.left
            anchors.rightMargin: 14
            anchors.verticalCenter: parent.verticalCenter

            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
            maximumLineCount: 2

            textFormat: Text.StyledText
            text: "<b>" + qsTrc("appshell/preferences", "Restart required") + "</b> · " + root.restartText
        }

        FlatButton {
            id: restartButton

            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter

            height: 26
            minWidth: 0
            margins: 12

            text: qsTrc("appshell/preferences", "Restart now")
            accentButton: true

            navigation.name: "RestartNowButton"
            navigation.panel: root.navigation
            navigation.column: skinsRepeater.count

            onClicked: {
                root.restartRequested()
            }
        }
    }
}
