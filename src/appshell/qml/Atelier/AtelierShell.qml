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
import QtQuick.Window 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.AppShell 1.0
import MuseScore.Atelier 1.0
import MuseScore.Project 1.0

//! NOTE: the content of the window in the Atelier skin: one toolbar, and below it the page of the current mode.
//! See AtelierShellView about what it keeps for the rest of the app
AtelierShellView {
    id: root

    objectName: "WindowContent"

    readonly property string pageKind: {
        switch (root.currentPageUri) {
        case "musescore://home": return "home"
        case "musescore://notation": return "score"
        case "musescore://engrave": return "score"
        case "musescore://publish": return "preview"
        case "musescore://fontdesign": return "fontdesign"
        case "musescore://devtools": return "devtools"
        }

        return ""
    }

    //! NOTE: a page is created when it is opened for the first time, and then it is kept
    property bool homeNeeded: false
    property bool scoreNeeded: false
    property bool previewNeeded: false
    property bool fontDesignNeeded: false
    property bool devToolsNeeded: false

    onPageKindChanged: {
        switch (root.pageKind) {
        case "home": root.homeNeeded = true; break
        case "score": root.scoreNeeded = true; break
        case "preview": root.previewNeeded = true; break
        case "fontdesign": root.fontDesignNeeded = true; break
        case "devtools": root.devToolsNeeded = true; break
        }
    }

    readonly property var currentPageItem: {
        switch (root.pageKind) {
        case "home": return homeLoader.item
        case "score": return scoreLoader.item
        case "preview": return previewLoader.item
        case "fontdesign": return fontDesignLoader.item
        case "devtools": return devToolsLoader.item
        }

        return null
    }

    onPageLoaded: {
        interactiveProvider.onPageOpened()

        if (Boolean(root.Window.window)) {
            root.Window.window.opacity = 1.0
        }
    }

    function openPage(uri) {
        api.launcher.open(uri)
    }

    //! NOTE: opens the score in a mode (a page of the score). The request is kept here,
    //! because the page which makes it (the home page) is gone before the score is open
    function openScoreInPage(scorePath, displayName, pageUri) {
        scoresOpener.openScoreInPage(scorePath, displayName, pageUri)
    }

    ScoresPageModel {
        id: scoresOpener
    }

    InteractiveProvider {
        id: interactiveProvider
        topParent: root

        onRequestedDockPage: function(uri, params) {
            root.loadPage(uri, params)
        }
    }

    AtelierContextModel {
        id: contextModel

        Component.onCompleted: {
            load()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: AtStyle.island
    }

    AtelierToolBar {
        id: toolBar

        anchors.top: parent.top
        width: parent.width

        shell: root
        context: contextModel

        pageItem: root.currentPageItem

        leftPaneName: Boolean(root.currentPageItem) ? root.currentPageItem.leftPaneName : ""
        leftPaneTitle: Boolean(root.currentPageItem) ? root.currentPageItem.leftPaneTitle : ""
        rightPaneName: Boolean(root.currentPageItem) ? root.currentPageItem.rightPaneName : ""
        rightPaneTitle: Boolean(root.currentPageItem) ? root.currentPageItem.rightPaneTitle : ""

        onPageRequested: function(uri) {
            root.openPage(uri)
        }
    }

    Item {
        id: body

        anchors.top: toolBar.bottom
        anchors.bottom: parent.bottom
        width: parent.width

        Loader {
            id: homeLoader

            anchors.fill: parent
            active: root.homeNeeded
            visible: root.pageKind === "home"

            sourceComponent: AtelierHomePage {
                shell: root
                context: contextModel

                onPageRequested: function(uri) {
                    root.openPage(uri)
                }
            }
        }

        Loader {
            id: scoreLoader

            anchors.fill: parent
            active: root.scoreNeeded
            visible: root.pageKind === "score"

            sourceComponent: AtelierScorePage {
                shell: root
            }
        }

        Loader {
            id: previewLoader

            anchors.fill: parent
            active: root.previewNeeded
            visible: root.pageKind === "preview"

            sourceComponent: AtelierPreviewPage {
                shell: root
                context: contextModel
            }
        }

        Loader {
            id: fontDesignLoader

            anchors.fill: parent
            active: root.fontDesignNeeded
            visible: root.pageKind === "fontdesign"

            sourceComponent: AtelierFontDesignPage {
                shell: root
            }
        }

        Loader {
            id: devToolsLoader

            anchors.fill: parent
            active: root.devToolsNeeded
            visible: root.pageKind === "devtools"

            sourceComponent: AtelierDevToolsPage {
                shell: root
            }
        }
    }
}
