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
import MuseScore.Palette 1.0
import MuseScore.Inspector 1.0
import MuseScore.InstrumentsScene 1.0
import MuseScore.Playback 1.0

//! NOTE: the page of the score, with fixed zones: the tools of the mode in the strip, what you add on the left,
//! the score in the middle, the properties on the right, and a drawer (mixer, keyboard, …) below the score
Item {
    id: root

    required property AtelierShellView shell

    //! NOTE: "input" (the notes and what is added to them) or "engrave" (where everything sits).
    //! The score stays where it is, the tools around it change
    property string mode: "input"
    readonly property bool isEngrave: root.mode === "engrave"

    readonly property string inputUri: "musescore://notation"
    readonly property string engraveUri: "musescore://engrave"
    readonly property string pageUri: root.isEngrave ? root.engraveUri : root.inputUri

    //! NOTE: the tools of the Engrave mode are created when the mode is opened for the first time
    property bool engraveNeeded: false

    function updateMode() {
        if (root.shell.currentPageUri === root.engraveUri) {
            root.engraveNeeded = true
            root.mode = "engrave"
        } else if (root.shell.currentPageUri === root.inputUri) {
            root.mode = "input"
        }
    }

    Connections {
        target: root.shell

        function onCurrentPageUriChanged() {
            root.updateMode()
        }
    }

    //! NOTE: the paint view of the score (for the popups of the properties)
    property var notationView: null

    //! NOTE: the left pane has the name of the palettes in both modes, so the same action and key toggle it
    readonly property string leftPaneName: "palettesPanel"
    readonly property string leftPaneTitle: root.isEngrave ? qsTrc("appshell", "Layout") : qsTrc("appshell", "Library")
    readonly property string rightPaneName: "inspectorPanel"
    readonly property string rightPaneTitle: qsTrc("appshell", "Properties")

    function isOpen(name) {
        return root.shell.isPanelOpen(root.pageUri, name, root.shell.panelsRevision)
    }

    NotationPageModel {
        id: pageModel
    }

    property NavigationSection noteInputKeyNavSec: NavigationSection {
        name: "NoteInputSection"
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
        //! NOTE: the names are the ones which the actions of the app toggle (see ApplicationUiActions)
        var panels = {
            "playbackToolBar": true,
            "noteInputBar": true,
            "palettesPanel": true,
            "inspectorPanel": true,
            "instrumentsPanel": false,
            "selectionFiltersPanel": false,
            "undoHistoryPanel": false,
            "mixerPanel": false,
            "pianoKeyboardPanel": false,
            "timelinePanel": false,
            "drumsetPanel": false,
            "percussionPanel": false,
            "notationStatusBar": true
        }

        for (var name in panels) {
            root.shell.declarePanel(root.inputUri, name, panels[name])
            root.shell.declarePanel(root.engraveUri, name, panels[name])
        }

        root.updateMode()

        Qt.callLater(pageModel.init)
    }

    // ---------- the strip and the status bar take the whole width ----------

    Item {
        id: strip

        anchors.top: parent.top
        width: parent.width
        height: !visible ? 0 : (root.isEngrave ? engraveStripLoader.height : inputStrip.height)

        //! NOTE: View > Toolbars > Note input shows and hides the strip of the mode
        visible: root.isOpen("noteInputBar")

        AtelierNoteInputStrip {
            id: inputStrip

            width: parent.width
            height: implicitHeight

            visible: !root.isEngrave

            navigationSection: root.noteInputKeyNavSec
        }

        Loader {
            id: engraveStripLoader

            width: parent.width
            height: Boolean(item) ? item.implicitHeight : 0

            active: root.engraveNeeded
            visible: root.isEngrave

            sourceComponent: AtelierEngraveStrip {
                navigationSection: root.noteInputKeyNavSec
            }
        }
    }

    AtelierStatusBar {
        id: statusBar

        anchors.bottom: parent.bottom
        width: parent.width
        height: visible ? AtStyle.statusBarHeight : 0

        visible: root.isOpen("notationStatusBar")
    }

    // ---------- left: what you add ----------

    Item {
        id: leftPane

        anchors.left: parent.left
        anchors.top: strip.bottom
        anchors.bottom: statusBar.top

        width: visible ? AtStyle.libraryWidth : 0
        visible: root.isOpen("palettesPanel")

        PaletteLibrary {
            anchors.fill: parent
            anchors.rightMargin: 1

            visible: !root.isEngrave

            navigationSection: root.keynavLeftPanelSec
            navigationOrderStart: 1
        }

        Loader {
            anchors.fill: parent
            anchors.rightMargin: 1

            active: root.engraveNeeded
            visible: root.isEngrave

            sourceComponent: AtelierLayoutPane {
                navigationSection: root.keynavLeftPanelSec
                navigationOrderStart: 1
            }
        }

        Rectangle {
            anchors.right: parent.right
            width: 1
            height: parent.height
            color: AtStyle.stroke
        }
    }

    // ---------- right: what the selection is ----------

    Item {
        id: rightPane

        anchors.right: parent.right
        anchors.top: strip.bottom
        anchors.bottom: statusBar.top

        width: visible ? AtStyle.propertiesWidth : 0
        visible: rightStack.hasOpenPanels

        Rectangle {
            width: 1
            height: parent.height
            color: AtStyle.stroke
        }

        AtelierPaneStack {
            id: rightStack

            anchors.fill: parent
            anchors.leftMargin: 1

            shell: root.shell
            pageUri: root.pageUri

            entries: [
                { name: "inspectorPanel", title: qsTrc("appshell", "Properties"), component: propertiesComp },
                { name: "instrumentsPanel", title: qsTrc("instruments", "Instruments"), component: instrumentsComp },
                { name: "selectionFiltersPanel", title: qsTrc("appshell", "Selection filter"), component: selectionFilterComp },
                { name: "undoHistoryPanel", title: qsTrc("notation", "History"), component: historyComp }
            ]
        }
    }

    Component {
        id: propertiesComp

        Item {
            //! NOTE: the first part of the status line is what is selected, the rest describes it
            readonly property var infoParts: Boolean(statusBar.selectionInfo) ? statusBar.selectionInfo.split("; ") : []

            AtPaneHeader {
                id: propertiesHeader

                width: parent.width

                title: parent.infoParts.length > 0 ? parent.infoParts[0] : qsTrc("appshell", "Properties")
                subtitle: parent.infoParts.length > 1 ? parent.infoParts.slice(1).join(" · ") : ""
                separatorVisible: true
            }

            InspectorForm {
                anchors.top: propertiesHeader.bottom
                anchors.bottom: propertiesFooter.top
                width: parent.width

                clip: true

                mode: root.mode

                navigationSection: root.keynavRightPanelSec
                navigationOrderStart: 1
                notationView: root.notationView
            }

            //! NOTE: where the other properties of the selection are
            Item {
                id: propertiesFooter

                anchors.bottom: parent.bottom
                width: parent.width
                height: Math.max(otherModeButton.height, otherModeNote.implicitHeight) + 22

                Rectangle {
                    width: parent.width
                    height: 1
                    color: AtStyle.stroke
                }

                StyledTextLabel {
                    id: otherModeNote

                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.right: otherModeButton.left
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter

                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.WordWrap
                    lineHeight: 1.15

                    text: root.isEngrave ? qsTrc("appshell", "Notes, dynamics and text are entered in Input.")
                                         : qsTrc("appshell", "Stem, beam, ledger lines and offsets are in Engrave.")
                    font: AtStyle.font(11.5)
                    color: AtStyle.text2
                }

                AtToolButton {
                    id: otherModeButton

                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter

                    height: 26
                    radius: 7

                    text: root.isEngrave ? qsTrc("appshell", "Input") : qsTrc("appshell", "Engrave")
                    textFont: AtStyle.font(12, Font.DemiBold)
                    contentColor: AtStyle.ink

                    //! NOTE: it looks like a button without being hovered
                    checked: !mouseArea.containsMouse && !mouseArea.pressed
                    checkedStyle: "raised"

                    navigation.name: "OtherMode"
                    navigation.panel: propertiesFooterNavPanel

                    onClicked: {
                        root.shell.openPage(root.isEngrave ? root.inputUri : root.engraveUri)
                    }
                }

                NavigationPanel {
                    id: propertiesFooterNavPanel
                    name: "PropertiesFooter"
                    section: root.keynavRightPanelSec
                    order: 1000
                    enabled: propertiesFooter.enabled && propertiesFooter.visible
                }
            }
        }
    }

    Component {
        id: instrumentsComp

        Item {
            AtPaneHeader {
                id: instrumentsHeader
                width: parent.width
                title: qsTrc("instruments", "Instruments")
                visible: rightStack.openEntries.length === 1
                height: visible ? implicitHeight : 0
            }

            LayoutPanel {
                anchors.top: instrumentsHeader.bottom
                anchors.bottom: parent.bottom
                width: parent.width

                navigationSection: root.keynavRightPanelSec
                navigationOrderStart: 1
            }
        }
    }

    Component {
        id: selectionFilterComp

        Item {
            AtPaneHeader {
                id: selectionFilterHeader
                width: parent.width
                title: qsTrc("appshell", "Selection filter")
                visible: rightStack.openEntries.length === 1
                height: visible ? implicitHeight : 0
            }

            SelectionFilterPanel {
                anchors.top: selectionFilterHeader.bottom
                anchors.bottom: parent.bottom
                width: parent.width

                navigationSection: root.keynavRightPanelSec
                navigationOrderStart: 1
            }
        }
    }

    Component {
        id: historyComp

        Item {
            AtPaneHeader {
                id: historyHeader
                width: parent.width
                title: qsTrc("notation", "History")
                visible: rightStack.openEntries.length === 1
                height: visible ? implicitHeight : 0
            }

            UndoHistoryPanel {
                anchors.top: historyHeader.bottom
                anchors.bottom: parent.bottom
                width: parent.width

                navigationSection: root.keynavRightPanelSec
                navigationOrderStart: 1
            }
        }
    }

    // ---------- middle: the score, and the drawer below it ----------

    Item {
        id: center

        anchors.left: leftPane.right
        anchors.right: rightPane.left
        anchors.top: strip.bottom
        anchors.bottom: statusBar.top

        Loader {
            id: notationViewLoader

            anchors.top: parent.top
            anchors.bottom: drawer.top
            width: parent.width

            //! NOTE: there is one view of the score at a time
            active: root.visible

            sourceComponent: NotationView {
                id: notationView

                name: "MainNotationView"

                isNavigatorVisible: pageModel.isNavigatorVisible
                isBraillePanelVisible: pageModel.isBraillePanelVisible
                isMainView: true

                //! NOTE: the open parts are chosen in the toolbar
                tabPanelAllowed: false

                Component.onCompleted: {
                    root.notationView = notationView.paintView
                    root.shell.setDefaultNavigationControl(notationView.defaultNavigationControl)
                }

                Component.onDestruction: {
                    root.notationView = null
                    root.shell.setDefaultNavigationControl(null)
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
                property real percussionHeight: 200

                entries: [
                    { name: "mixerPanel", title: qsTrc("appshell", "Mixer"), component: mixerComp, size: drawerStack.mixerHeight },
                    { name: "pianoKeyboardPanel", title: qsTrc("appshell", "Piano keyboard"), component: pianoKeyboardComp, size: 200 },
                    { name: "timelinePanel", title: qsTrc("appshell", "Timeline"), component: timelineComp, size: 200 },
                    { name: "percussionPanel", title: qsTrc("appshell", "Percussion"), component: percussionComp, size: drawerStack.percussionHeight },
                    { name: "drumsetPanel", title: qsTrc("appshell", "Drumset tools"), component: drumsetComp, size: 64 }
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

    Component {
        id: pianoKeyboardComp

        PianoKeyboardPanel {
            navigationSection: root.keynavBottomPanelSec
            contentNavigationPanelOrderStart: 1
        }
    }

    Component {
        id: timelineComp

        Timeline {
            navigationSection: root.keynavBottomPanelSec
            contentNavigationPanelOrderStart: 1
        }
    }

    Component {
        id: percussionComp

        PercussionPanel {
            navigationSection: root.keynavBottomPanelSec
            contentNavigationPanelOrderStart: 1

            onResizeRequested: function(newWidth, newHeight) {
                drawerStack.percussionHeight = newHeight
            }

            Component.onCompleted: {
                Qt.callLater(resizePanelToContentHeight)
            }
        }
    }

    Component {
        id: drumsetComp

        DrumsetPanel {
            navigationSection: root.keynavBottomPanelSec
            contentNavigationPanelOrderStart: 1
        }
    }
}
