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
import QtQml.Models 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.AppShell 1.0
import MuseScore.NotationScene 1.0
import MuseScore.Playback 1.0

import MuseScore.Atelier 1.0

//! NOTE: the one toolbar of the window: where you are on the left, the modes in the middle,
//! playback, history and the properties pane on the right
Rectangle {
    id: root

    required property AtelierShellView shell
    required property AtelierContextModel context

    //! NOTE: the names of the panes of the current page, which the buttons at the ends of the toolbar toggle
    property string leftPaneName: ""
    property string leftPaneTitle: ""
    property string rightPaneName: ""
    property string rightPaneTitle: ""

    //! NOTE: room for the buttons of the window when the title bar is merged with the toolbar
    property int leadingInset: 0

    readonly property string uri: shell.currentPageUri
    readonly property bool isHome: uri === "musescore://home"
    readonly property bool isScorePage: uri === "musescore://notation" || uri === "musescore://engrave"
                                        || uri === "musescore://publish"

    property NavigationSection navigationSection: NavigationSection {
        name: "TopTool"
        order: 1
    }

    signal pageRequested(string uri)

    height: AtStyle.toolBarHeight
    color: AtStyle.island

    function paneToggleTitle(paneName, paneTitle) {
        var isOpen = root.shell.isPanelOpen(root.uri, paneName, root.shell.panelsRevision)
        return (isOpen ? qsTrc("appshell", "Hide %1") : qsTrc("appshell", "Show %1")).arg(paneTitle)
    }

    NavigationPanel {
        id: navPanel
        name: "MainToolBar"
        section: root.navigationSection
        order: 1
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("appshell", "Main toolbar")
    }

    AtelierActionsModel {
        id: paneActions
        actionCodes: ["toggle-mixer", "file-new", "file-open", "parts"]
        Component.onCompleted: load()
    }

    NotationSwitchListModel {
        id: notationsModel

        property int currentIndex: 0

        onCurrentNotationIndexChanged: function(index) {
            notationsModel.currentIndex = index
        }

        Component.onCompleted: load()
    }

    Instantiator {
        id: notations

        model: notationsModel

        delegate: QtObject {
            readonly property string title: model.title
        }
    }

    // ---------- left: where you are ----------

    Row {
        id: leftZone

        anchors.left: parent.left
        anchors.leftMargin: 20 + root.leadingInset
        anchors.verticalCenter: parent.verticalCenter

        spacing: 12

        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2

            AtToolButton {
                shape: "sidebar-left"
                visible: Boolean(root.leftPaneName)
                toolTipTitle: root.paneToggleTitle(root.leftPaneName, root.leftPaneTitle)

                navigation.name: "ToggleLeftPane"
                navigation.panel: navPanel
                navigation.order: 1

                onClicked: {
                    root.shell.togglePanel(root.uri, root.leftPaneName)
                }
            }

            AtToolButton {
                shape: "home"
                checked: root.isHome
                checkedStyle: "soft"
                toolTipTitle: qsTrc("appshell", "Home")

                navigation.name: "Home"
                navigation.panel: navPanel
                navigation.order: 2

                onClicked: {
                    root.pageRequested("musescore://home")
                }
            }
        }

        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            StyledTextLabel {
                id: titleLabel

                anchors.verticalCenter: parent.verticalCenter
                width: Math.min(implicitWidth, Math.max(60, modeSwitcher.x - leftZone.x - 220))

                horizontalAlignment: Text.AlignLeft
                font: AtStyle.font(14, Font.DemiBold)

                text: {
                    if (root.isHome) {
                        return qsTrc("appshell", "Home")
                    }

                    if (root.uri === "musescore://fontdesign") {
                        return qsTrc("appshell", "Font design")
                    }

                    if (root.uri === "musescore://devtools") {
                        return "DevTools"
                    }

                    return root.context.projectTitle
                }
            }

            AtToolButton {
                id: partsButton

                anchors.verticalCenter: parent.verticalCenter
                height: 24

                visible: root.isScorePage && root.context.hasProject

                text: Boolean(root.context.partName) ? root.context.partName : qsTrc("engraving", "Full score")
                textFont: AtStyle.font(12)
                hasMenuArrow: true

                toolTipTitle: qsTrc("notation", "Parts")

                navigation.name: "Parts"
                navigation.panel: navPanel
                navigation.order: 3

                onClicked: {
                    var items = []
                    for (var i = 0; i < notations.count; ++i) {
                        items.push({ id: "notation_" + i, title: notations.objectAt(i).title,
                                     checkable: true, checked: i === notationsModel.currentIndex })
                    }

                    items.push({ id: "", title: "" })
                    items.push({ id: "parts", title: qsTrc("appshell", "Manage parts…") })

                    partsMenu.toggleOpened(items)
                }

                StyledMenuLoader {
                    id: partsMenu

                    onHandleMenuItem: function(itemId) {
                        if (itemId === "parts") {
                            paneActions.trigger("parts")
                        } else if (itemId.indexOf("notation_") === 0) {
                            notationsModel.setCurrentNotation(parseInt(itemId.substring(9)))
                        }
                    }
                }
            }
        }
    }

    // ---------- middle: the modes ----------

    AtSegmented {
        id: modeSwitcher

        anchors.centerIn: parent
        anchors.horizontalCenterOffset: root.leadingInset / 2

        visible: root.uri !== "musescore://fontdesign" && root.uri !== "musescore://devtools"

        navigationPanel: navPanel
        navigationOrderStart: 10

        currentValue: root.uri

        model: [
            { title: qsTrc("appshell", "Input"), value: "musescore://notation", enabled: root.context.hasProject },
            { title: qsTrc("appshell", "Engrave"), value: "musescore://engrave", enabled: root.context.hasProject },
            { title: qsTrc("appshell", "Preview"), value: "musescore://publish", enabled: root.context.hasProject }
        ]

        onActivated: function(value) {
            root.pageRequested(value)
        }
    }

    // ---------- right: what you do everywhere ----------

    Row {
        id: homeZone

        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter

        visible: root.isHome
        spacing: 8

        AtToolButton {
            text: qsTrc("project", "Open other…")
            textFont: AtStyle.font(13, Font.Medium)
            contentColor: AtStyle.text

            navigation.name: "OpenOther"
            navigation.panel: navPanel
            navigation.order: 20

            onClicked: {
                paneActions.trigger("file-open")
            }
        }

        FlatButton {
            height: 28
            minWidth: 0
            margins: 16

            text: qsTrc("project", "New")
            accentButton: true

            navigation.name: "NewScore"
            navigation.panel: navPanel
            navigation.order: 21

            onClicked: {
                paneActions.trigger("file-new")
            }
        }
    }

    Row {
        id: scoreZone

        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter

        visible: root.isScorePage
        spacing: 0

        AtelierPlaybackControls {
            anchors.verticalCenter: parent.verticalCenter

            //! NOTE: View > Toolbars > Playback controls
            visible: root.shell.isPanelOpen(root.uri, "playbackToolBar", root.shell.panelsRevision)

            navigationPanel: navPanel
            navigationOrderStart: 30
        }

        AtToolButton {
            anchors.verticalCenter: parent.verticalCenter

            shape: "mixer"
            checked: root.shell.isPanelOpen(root.uri, "mixerPanel", root.shell.panelsRevision)
            checkedStyle: "soft"

            toolTipTitle: qsTrc("appshell", "Mixer")
            toolTipShortcut: "F10"

            navigation.name: "Mixer"
            navigation.panel: navPanel
            navigation.order: 50

            onClicked: {
                paneActions.trigger("toggle-mixer")
            }
        }

        AtDivider {
            anchors.verticalCenter: parent.verticalCenter
        }

        AtelierUndoRedo {
            anchors.verticalCenter: parent.verticalCenter

            navigationPanel: navPanel
            navigationOrderStart: 60
        }

        AtDivider {
            anchors.verticalCenter: parent.verticalCenter
            visible: rightPaneButton.visible
        }

        AtToolButton {
            id: rightPaneButton

            anchors.verticalCenter: parent.verticalCenter

            shape: "sidebar-right"
            visible: Boolean(root.rightPaneName)
            toolTipTitle: root.paneToggleTitle(root.rightPaneName, root.rightPaneTitle)

            navigation.name: "ToggleRightPane"
            navigation.panel: navPanel
            navigation.order: 70

            onClicked: {
                root.shell.togglePanel(root.uri, root.rightPaneName)
            }
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: AtStyle.stroke
    }
}
