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

import MuseScore.NotationScene 1.0

import "../PublishPage"

//! NOTE: the score as it will be printed and exported
Item {
    id: root

    required property AtelierShellView shell

    readonly property string pageUri: "musescore://publish"

    readonly property string leftPaneName: ""
    readonly property string leftPaneTitle: ""
    readonly property string rightPaneName: ""
    readonly property string rightPaneTitle: ""

    property NavigationSection publishToolBarKeyNavSec: NavigationSection {
        name: "PublishToolBarSection"
        order: 2
    }

    Component.onCompleted: {
        root.shell.declarePanel(root.pageUri, "playbackToolBar", true)
        root.shell.declarePanel(root.pageUri, "notationStatusBar", true)
    }

    Rectangle {
        id: strip

        anchors.top: parent.top
        width: parent.width
        height: 44

        color: AtStyle.island

        PublishToolBar {
            anchors.centerIn: parent

            navigationPanel.section: root.publishToolBarKeyNavSec
            navigationPanel.order: 1
        }

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: AtStyle.stroke
        }
    }

    Loader {
        anchors.top: strip.bottom
        anchors.bottom: statusBar.top
        width: parent.width

        //! NOTE: there is one view of the score at a time
        active: root.visible

        sourceComponent: NotationView {
            name: "PublishNotationView"
            publishMode: true
            tabPanelAllowed: false
        }
    }

    AtelierStatusBar {
        id: statusBar

        anchors.bottom: parent.bottom
        width: parent.width
        height: visible ? AtStyle.statusBarHeight : 0

        visible: root.shell.isPanelOpen(root.pageUri, "notationStatusBar", root.shell.panelsRevision)
    }
}
