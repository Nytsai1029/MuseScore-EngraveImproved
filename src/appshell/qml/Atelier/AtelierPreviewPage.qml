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
import MuseScore.Playback 1.0
import MuseScore.Project 1.0

import "../PublishPage"

//! NOTE: the score as it will be printed and exported: its pages on the left, the whole page in the middle
//! (with its own zoom, so the place in the other modes is kept), the export on the right
Item {
    id: root

    required property AtelierShellView shell
    required property AtelierContextModel context

    readonly property string pageUri: "musescore://publish"

    //! NOTE: the panes have the names of the panes of the other modes, so the same actions and keys toggle them
    readonly property string leftPaneName: "palettesPanel"
    readonly property string leftPaneTitle: qsTrc("appshell", "Pages")
    readonly property string rightPaneName: "inspectorPanel"
    readonly property string rightPaneTitle: qsTrc("project/export", "Export")

    function isOpen(name) {
        return root.shell.isPanelOpen(root.pageUri, name, root.shell.panelsRevision)
    }

    property NavigationSection publishToolBarKeyNavSec: NavigationSection {
        name: "PublishToolBarSection"
        order: 2
    }

    property NavigationSection keynavLeftPanelSec: NavigationSection {
        name: "NavigationLeftPanel"
        enabled: root.visible
        order: 4
    }

    property NavigationSection keynavRightPanelSec: NavigationSection {
        name: "NavigationRightPanel"
        enabled: root.visible
        order: 6
    }

    property NavigationSection keynavBottomPanelSec: NavigationSection {
        name: "NavigationBottomPanel"
        enabled: root.visible
        order: 7
    }

    Component.onCompleted: {
        var panels = {
            "playbackToolBar": true,
            "noteInputBar": true,
            "palettesPanel": true,
            "inspectorPanel": true,
            "mixerPanel": false,
            "notationStatusBar": true
        }

        for (var name in panels) {
            root.shell.declarePanel(root.pageUri, name, panels[name])
        }
    }

    PreviewPagesModel {
        id: pagesModel

        Component.onCompleted: {
            load()
        }
    }

    AtelierActionsModel {
        id: actions

        actionCodes: ["print"]

        Component.onCompleted: {
            load()
        }
    }

    // ---------- the preview as a window of its own ----------

    property var previewWindow: null

    function openSeparateWindow() {
        if (Boolean(root.previewWindow)) {
            root.previewWindow.show()
            root.previewWindow.raise()
            root.previewWindow.requestActivate()
            return
        }

        root.previewWindow = previewWindowComp.createObject(root)
        if (!Boolean(root.previewWindow)) {
            return
        }

        root.previewWindow.closed.connect(function() {
            root.previewWindow = null
        })

        root.previewWindow.show()
        root.previewWindow.requestActivate()
    }

    Component {
        id: previewWindowComp

        PublishPreviewWindow {}
    }

    // ---------- the strip and the status bar take the whole width ----------

    AtelierPreviewStrip {
        id: strip

        anchors.top: parent.top
        width: parent.width
        height: visible ? implicitHeight : 0

        visible: root.isOpen("noteInputBar")

        navigationSection: root.publishToolBarKeyNavSec

        onExportRequested: {
            //! NOTE: the settings of the export are in the pane, so it is shown
            root.shell.setPanelOpen(root.pageUri, root.rightPaneName, true)
            exportPanel.exportScores()
        }

        onSeparateWindowRequested: {
            root.openSeparateWindow()
        }
    }

    AtelierPreviewStatusBar {
        id: statusBar

        anchors.bottom: parent.bottom
        width: parent.width
        height: visible ? AtStyle.statusBarHeight : 0

        visible: root.isOpen("notationStatusBar")

        pagesModel: pagesModel
        view: pagesModel.view
    }

    // ---------- left: the pages ----------

    Item {
        id: leftPane

        anchors.left: parent.left
        anchors.top: strip.bottom
        anchors.bottom: statusBar.top

        width: visible ? leftResizer.paneWidth : 0
        visible: root.isOpen(root.leftPaneName)

        AtelierPagesPane {
            anchors.fill: parent
            anchors.rightMargin: 1

            pagesModel: pagesModel

            navigationSection: root.keynavLeftPanelSec
            navigationOrderStart: 1
        }

        Rectangle {
            anchors.right: parent.right
            width: 1
            height: parent.height
            color: AtStyle.stroke
        }
    }

    // ---------- right: the export ----------

    Item {
        id: rightPane

        anchors.right: parent.right
        anchors.top: strip.bottom
        anchors.bottom: statusBar.top

        width: visible ? rightResizer.paneWidth : 0
        visible: root.isOpen(root.rightPaneName)

        Rectangle {
            width: 1
            height: parent.height
            color: AtStyle.stroke
        }

        ExportPanel {
            id: exportPanel

            anchors.fill: parent
            anchors.leftMargin: 1

            //: The name of the score and the number of its pages, e.g. "Träumerei · 2 pages"
            subtitle: root.context.hasProject ? root.context.projectTitle + " · "
                                                + qsTrc("appshell", "%n page(s)", "", pagesModel.pageCount)
                                              : ""

            navigationSection: root.keynavRightPanelSec
            navigationOrderStart: 1

            onPrintRequested: {
                actions.trigger("print")
            }
        }
    }

    // ---------- middle: the preview, and the drawer below it ----------

    Item {
        id: center

        anchors.left: leftPane.right
        anchors.right: rightPane.left
        anchors.top: strip.bottom
        anchors.bottom: statusBar.top

        Loader {
            anchors.top: parent.top
            anchors.bottom: drawer.top
            width: parent.width

            //! NOTE: there is one view of the score at a time
            active: root.visible

            sourceComponent: NotationView {
                id: previewView

                name: "PublishNotationView"
                publishMode: true

                //! NOTE: the preview has its own zoom and place, and it starts with the whole page
                syncViewState: false
                startWithWholePage: true

                //! NOTE: the open parts are chosen in the toolbar
                tabPanelAllowed: false

                Component.onCompleted: {
                    pagesModel.view = previewView.paintView
                }

                Component.onDestruction: {
                    pagesModel.view = null
                }
            }
        }

        Item {
            id: drawer

            anchors.bottom: parent.bottom
            width: parent.width

            visible: drawerStack.hasOpenPanels
            height: visible ? Math.min(drawerStack.contentY + drawerStack.preferredSize, center.height * 0.6) : 0

            Rectangle {
                anchors.fill: parent
                color: AtStyle.island
            }

            Rectangle {
                width: parent.width
                height: 1
                color: AtStyle.stroke
                z: 1
            }

            AtelierPaneStack {
                id: drawerStack

                anchors.fill: parent
                anchors.topMargin: 1

                shell: root.shell
                pageUri: root.pageUri
                alwaysShowTabs: true

                property real mixerHeight: 368

                entries: [
                    { name: "mixerPanel", title: qsTrc("appshell", "Mixer"), component: mixerComp, size: drawerStack.mixerHeight }
                ]
            }
        }
    }

    Component {
        id: mixerComp

        MixerPanel {
            navigationSection: root.keynavBottomPanelSec
            contentNavigationPanelOrderStart: 1

            onResizeRequested: function(newWidth, newHeight) {
                drawerStack.mixerHeight = newHeight
            }

            Component.onCompleted: {
                Qt.callLater(resizePanelToContentHeight)
            }
        }
    }

    // ---------- the edges of the panes: dragged, they change the widths of the panes ----------

    AtelierPaneResizer {
        id: leftResizer

        shell: root.shell
        pane: leftPane
        edge: Qt.RightEdge

        pageUri: root.pageUri
        name: "leftPane"

        defaultWidth: AtStyle.libraryWidth
        minimumWidth: 200
        maximumWidth: Math.max(minimumWidth, Math.min(520, (root.width - 320) / 2))
    }

    AtelierPaneResizer {
        id: rightResizer

        shell: root.shell
        pane: rightPane
        edge: Qt.LeftEdge

        pageUri: root.pageUri
        name: "rightPane"

        defaultWidth: AtStyle.propertiesWidth
        minimumWidth: 260
        maximumWidth: Math.max(minimumWidth, Math.min(520, (root.width - 320) / 2))
    }
}
