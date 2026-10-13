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
import MuseScore.Project 1.0

import MuseScore.Atelier 1.0

//! NOTE: the one toolbar of the window: where you are on the left, the modes in the middle,
//! playback, history and the properties pane on the right
Rectangle {
    id: root

    required property AtelierShellView shell
    required property AtelierContextModel context

    //! NOTE: the current page. It can give the toolbar its own title and content
    //! (toolBarTitle, toolBarSubtitle, toolBarCenter, toolBarRight)
    property var pageItem: null

    readonly property bool hasPageTitle: Boolean(root.pageItem) && root.pageItem.toolBarTitle !== undefined
    readonly property Component pageCenter: Boolean(root.pageItem) && Boolean(root.pageItem.toolBarCenter)
                                            ? root.pageItem.toolBarCenter : null
    readonly property Component pageRight: Boolean(root.pageItem) && Boolean(root.pageItem.toolBarRight)
                                           ? root.pageItem.toolBarRight : null

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

    //! NOTE: without an open score, the modes open the score which was opened last
    RecentScoresModel {
        id: recentScoresModel

        Component.onCompleted: load()
    }

    Instantiator {
        id: recentScores

        model: recentScoresModel

        delegate: QtObject {
            readonly property var score: model.score
        }
    }

    readonly property var lastScore: {
        for (var i = 0; i < recentScores.count; ++i) {
            var item = recentScores.objectAt(i)
            if (Boolean(item) && Boolean(item.score) && Boolean(item.score.path)) {
                return item.score
            }
        }

        return null
    }

    readonly property bool canOpenModes: root.context.hasProject || Boolean(root.lastScore)

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

                    if (root.hasPageTitle) {
                        return root.pageItem.toolBarTitle
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

            StyledTextLabel {
                anchors.verticalCenter: parent.verticalCenter

                visible: !isEmpty
                text: root.hasPageTitle && Boolean(root.pageItem.toolBarSubtitle) ? root.pageItem.toolBarSubtitle : ""
                font: AtStyle.font(12)
                color: AtStyle.text2
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
            { title: qsTrc("appshell", "Input"), value: "musescore://notation", enabled: root.canOpenModes },
            { title: qsTrc("appshell", "Engrave"), value: "musescore://engrave", enabled: root.canOpenModes },
            { title: qsTrc("appshell", "Preview"), value: "musescore://publish", enabled: root.canOpenModes }
        ]

        onActivated: function(value) {
            if (root.context.hasProject) {
                root.pageRequested(value)
            } else if (Boolean(root.lastScore)) {
                root.shell.openScoreInPage(root.lastScore.path, root.lastScore.name, value)
            }
        }
    }

    Loader {
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: root.leadingInset / 2

        active: Boolean(root.pageCenter)
        sourceComponent: root.pageCenter
    }

    Row {
        id: pageZone

        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter

        visible: Boolean(root.pageRight)
        spacing: 0

        Loader {
            anchors.verticalCenter: parent.verticalCenter

            active: Boolean(root.pageRight)
            sourceComponent: root.pageRight
        }

        AtDivider {
            anchors.verticalCenter: parent.verticalCenter
            visible: pageRightPaneButton.visible
        }

        AtToolButton {
            id: pageRightPaneButton

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

    // ---------- right: what you do everywhere ----------

    Row {
        id: homeZone

        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter

        visible: root.isHome
        spacing: 8

        AtOutlineButton {
            text: qsTrc("project", "Open other…")
            textFont: AtStyle.font(13)

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

            //! NOTE: the action of the mixer is for the pages where the score is edited; the drawer is on each page
            onClicked: {
                root.shell.togglePanel(root.uri, "mixerPanel")
            }
        }

        //! NOTE: nothing is edited in the Preview mode
        AtDivider {
            anchors.verticalCenter: parent.verticalCenter
            visible: undoRedo.visible
        }

        AtelierUndoRedo {
            id: undoRedo

            anchors.verticalCenter: parent.verticalCenter

            visible: root.uri !== "musescore://publish"

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
