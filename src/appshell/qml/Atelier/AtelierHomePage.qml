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
import MuseScore.Atelier 1.0

import Muse.Cloud 1.0
import Muse.Learn 1.0
import MuseScore.Project 1.0
import MuseScore.MuseSounds 1.0
import MuseScore.FontDesign 1.0

import "../HomePage"

//! NOTE: the home page: its sections in the sidebar, the chosen section beside it
Item {
    id: root

    required property AtelierShellView shell
    required property AtelierContextModel context

    readonly property string pageUri: "musescore://home"

    readonly property string leftPaneName: "homeMenu"
    readonly property string leftPaneTitle: qsTrc("appshell", "Sidebar")
    readonly property string rightPaneName: ""
    readonly property string rightPaneTitle: ""

    property string section: "scores"
    property string subSection: ""

    signal pageRequested(string uri)

    function applyParams(params) {
        if (!Boolean(params) || !Boolean(params["section"])) {
            return
        }

        var name = params["section"]
        root.section = name === "plugins" ? "extensions" : name // backward compatibility

        root.subSection = Boolean(params["subSection"]) ? params["subSection"] : ""
    }

    Component.onCompleted: {
        root.shell.declarePanel(root.pageUri, "homeMenu", true)
        root.applyParams(root.shell.currentPageParams)
    }

    Connections {
        target: root.shell

        function onCurrentPageParamsChanged() {
            if (root.shell.currentPageUri === root.pageUri) {
                root.applyParams(root.shell.currentPageParams)
            }
        }
    }

    AtelierActionsModel {
        id: actions
        actionCodes: ["preference-dialog"]
        Component.onCompleted: load()
    }

    NavigationSection {
        id: navSec
        name: "HomeMenuSection"
        enabled: root.enabled && root.visible
        order: 2
    }

    NavigationPanel {
        id: navPanel
        name: "HomeMenuPanel"
        section: navSec
        order: 1
        direction: NavigationPanel.Vertical
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("appshell", "Home menu")
    }

    // ---------- the sidebar ----------

    Rectangle {
        id: sidebar

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        width: visible ? 232 : 0
        visible: root.shell.isPanelOpen(root.pageUri, "homeMenu", root.shell.panelsRevision)

        color: AtStyle.island2

        component SidebarItem: Item {
            id: sidebarItem

            property string name: ""
            property string title: ""
            property int icon: IconCode.NONE
            property string shape: ""
            property int navigationRow: 0

            readonly property bool isCurrent: Boolean(name) && root.section === name

            signal clicked()

            width: parent.width
            height: 34

            NavigationControl {
                id: itemNavCtrl

                name: "HomeMenu_" + (Boolean(sidebarItem.name) ? sidebarItem.name : sidebarItem.title)
                panel: navPanel
                row: sidebarItem.navigationRow
                enabled: sidebarItem.enabled && sidebarItem.visible

                accessible.role: MUAccessible.ListItem
                accessible.name: sidebarItem.title
                accessible.selected: sidebarItem.isCurrent

                onTriggered: {
                    sidebarItem.clicked()
                }
            }

            Rectangle {
                anchors.fill: parent

                radius: 8
                color: sidebarItem.isCurrent ? AtStyle.soft
                                             : (itemMouseArea.pressed ? AtStyle.pressed
                                                                      : (itemMouseArea.containsMouse ? AtStyle.hover : "transparent"))

                NavigationFocusBorder {
                    navigationCtrl: itemNavCtrl
                }
            }

            Row {
                x: 10
                anchors.verticalCenter: parent.verticalCenter
                spacing: 10

                Item {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 18
                    height: 18

                    AtIcon {
                        anchors.centerIn: parent
                        name: sidebarItem.shape
                        color: sidebarItem.isCurrent ? AtStyle.ink : AtStyle.text2
                        visible: !isEmpty
                    }

                    StyledIconLabel {
                        anchors.centerIn: parent
                        iconCode: sidebarItem.icon
                        color: sidebarItem.isCurrent ? AtStyle.ink : AtStyle.text2
                        visible: !Boolean(sidebarItem.shape)
                    }
                }

                StyledTextLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: sidebarItem.title
                    font: AtStyle.font(13, sidebarItem.isCurrent ? Font.DemiBold : Font.Normal)
                    color: sidebarItem.isCurrent ? AtStyle.ink : AtStyle.text
                }
            }

            MouseArea {
                id: itemMouseArea

                anchors.fill: parent
                hoverEnabled: true

                onClicked: {
                    itemNavCtrl.requestActiveByInteraction()
                    sidebarItem.clicked()
                }
            }
        }

        Column {
            anchors.top: parent.top
            anchors.topMargin: 12
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 11

            spacing: 2

            SidebarItem {
                name: "scores"
                title: qsTrc("appshell", "Scores")
                icon: IconCode.MUSIC_NOTES
                navigationRow: 1
                onClicked: root.section = name
            }

            SidebarItem {
                name: "fontdesign"
                title: qsTrc("appshell", "Font design")
                icon: IconCode.BRUSH
                navigationRow: 2
                onClicked: root.section = name
            }

            SidebarItem {
                name: "extensions"
                title: qsTrc("appshell", "Plugins")
                icon: IconCode.PLUGIN
                navigationRow: 3
                onClicked: root.section = name
            }

            SidebarItem {
                name: "musesounds"
                title: qsTrc("appshell", "MuseSounds")
                icon: IconCode.PLAY
                navigationRow: 4
                onClicked: root.section = name
            }

            SidebarItem {
                name: "learn"
                title: qsTrc("appshell", "Learn")
                icon: IconCode.MORTAR_BOARD
                navigationRow: 5
                onClicked: root.section = name
            }
        }

        Column {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 12
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 11

            spacing: 2

            SidebarItem {
                title: "DevTools"
                shape: "gear"
                visible: root.context.devModeEnabled
                navigationRow: 10
                onClicked: root.pageRequested("musescore://devtools")
            }

            SidebarItem {
                name: "account"
                title: qsTrc("appshell", "Account")
                icon: IconCode.ACCOUNT
                navigationRow: 11
                onClicked: root.section = name
            }

            SidebarItem {
                title: qsTrc("action", "Preferences…")
                icon: IconCode.SETTINGS_COG
                navigationRow: 12
                onClicked: actions.trigger("preference-dialog")
            }
        }

        Rectangle {
            anchors.right: parent.right
            width: 1
            height: parent.height
            color: AtStyle.stroke
        }
    }

    // ---------- the section ----------

    Loader {
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        active: root.visible

        sourceComponent: {
            switch (root.section) {
            case "scores": return scoresComp
            case "extensions": return extensionsComp
            case "musesounds": return museSoundsComp
            case "learn": return learnComp
            case "fontdesign": return fontDesignComp
            case "account": return accountComp
            }

            return scoresComp
        }
    }

    Component {
        id: scoresComp

        ScoresPage {}
    }

    Component {
        id: extensionsComp

        PluginsPage {
            section: root.subSection
        }
    }

    Component {
        id: museSoundsComp

        MuseSoundsPage {}
    }

    Component {
        id: learnComp

        LearnPage {
            section: root.subSection
        }
    }

    Component {
        id: fontDesignComp

        FontDesignHomeSection {}
    }

    Component {
        id: accountComp

        AccountPage {}
    }
}
