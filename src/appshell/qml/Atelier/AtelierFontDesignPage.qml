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

import MuseScore.FontDesign 1.0

//! NOTE: the font design window: the glyphs on the left, the glyph which is edited in the middle,
//! its properties on the right. The tools are in the toolbar of the window and in the strip below it
Item {
    id: root

    required property AtelierShellView shell

    readonly property string pageUri: "musescore://fontdesign"

    readonly property string leftPaneName: "fontDesignGlyphBrowserPanel"
    readonly property string leftPaneTitle: qsTrc("appshell", "Glyphs")
    readonly property string rightPaneName: "fontDesignInspectorPanel"
    readonly property string rightPaneTitle: qsTrc("appshell", "Properties")

    readonly property var fontModel: central.model
    readonly property var glyphCanvas: central.glyphCanvas
    readonly property bool hasFont: Boolean(root.fontModel) && root.fontModel.hasProject

    //! NOTE: what the toolbar of the window shows for this page
    readonly property string toolBarTitle: root.hasFont ? root.fontModel.projectTitle : qsTrc("appshell", "Font design")
    readonly property bool toolBarModified: root.hasFont && root.fontModel.isDirty
    readonly property string toolBarSubtitle: root.hasFont ? qsTrc("appshell", "Font design") : ""
    readonly property Component toolBarCenter: root.hasFont ? toolsComp : null
    readonly property Component toolBarRight: root.hasFont ? fileActionsComp : null

    function isOpen(name) {
        return root.shell.isPanelOpen(root.pageUri, name, root.shell.panelsRevision)
    }

    property NavigationSection toolsNavSec: NavigationSection {
        name: "FontDesignTools"
        enabled: root.visible
        order: 2
    }

    Component.onCompleted: {
        root.shell.declarePanel(root.pageUri, root.leftPaneName, true)
        root.shell.declarePanel(root.pageUri, root.rightPaneName, true)
    }

    // ---------- the tools (shown in the middle of the toolbar) ----------

    Component {
        id: toolsComp

        Rectangle {
            width: toolsRow.width + 4
            height: 30

            radius: 9
            color: AtStyle.button

            Row {
                id: toolsRow

                x: 2
                y: 2
                spacing: 2

                Repeater {
                    //! NOTE: the values of GlyphCanvas.Tool
                    model: [
                        { tool: 0, kind: "select", title: qsTrc("fontdesign", "Select / move (V)") },
                        { tool: 1, kind: "node", title: qsTrc("fontdesign", "Edit nodes (A)") },
                        { tool: 2, kind: "pen", title: qsTrc("fontdesign", "Pen — straight (P)") },
                        { tool: 5, kind: "curvepen", title: qsTrc("fontdesign", "Pen — curve (C)") },
                        { tool: 3, kind: "rect", title: qsTrc("fontdesign", "Rectangle (R)") },
                        { tool: 4, kind: "ellipse", title: qsTrc("fontdesign", "Ellipse (E)") }
                    ]

                    delegate: AtToolButton {
                        readonly property bool isCurrent: Boolean(root.glyphCanvas) && root.glyphCanvas.tool === modelData.tool

                        width: 34
                        height: 26

                        checked: isCurrent
                        checkedStyle: "raised"

                        toolTipTitle: modelData.title

                        navigation.name: "Tool_" + modelData.kind
                        navigation.accessible.name: modelData.title

                        onClicked: {
                            root.glyphCanvas.tool = modelData.tool
                            root.glyphCanvas.forceActiveFocus()
                        }

                        FontToolIcon {
                            anchors.centerIn: parent
                            kind: modelData.kind
                            strokeColor: parent.isCurrent ? AtStyle.text : AtStyle.text2
                        }
                    }
                }
            }
        }
    }

    // ---------- the history and the file (shown at the right of the toolbar) ----------

    Component {
        id: fileActionsComp

        Row {
            spacing: 0

            AtToolButton {
                anchors.verticalCenter: parent.verticalCenter

                shape: "undo"
                enabled: root.fontModel.canUndo

                toolTipTitle: qsTrc("fontdesign", "Undo")
                toolTipShortcut: "Ctrl+Z"

                onClicked: {
                    root.fontModel.undo()
                }
            }

            AtToolButton {
                anchors.verticalCenter: parent.verticalCenter

                shape: "redo"
                enabled: root.fontModel.canRedo

                toolTipTitle: qsTrc("fontdesign", "Redo")
                toolTipShortcut: "Ctrl+Shift+Z"

                onClicked: {
                    root.fontModel.redo()
                }
            }

            AtDivider {
                anchors.verticalCenter: parent.verticalCenter
            }

            AtToolButton {
                anchors.verticalCenter: parent.verticalCenter

                text: qsTrc("fontdesign", "Check font")
                textFont: AtStyle.font(13)

                toolTipTitle: qsTrc("fontdesign", "Check font (SMuFL validation)")

                onClicked: {
                    root.fontModel.openLintDialog()
                }
            }

            AtToolButton {
                anchors.verticalCenter: parent.verticalCenter

                text: qsTrc("fontdesign", "Save")
                textFont: AtStyle.font(13)

                toolTipTitle: qsTrc("fontdesign", "Save font")
                toolTipShortcut: "Ctrl+S"

                onClicked: {
                    root.fontModel.save()
                }
            }

            AtToolButton {
                anchors.verticalCenter: parent.verticalCenter

                text: qsTrc("fontdesign", "Export…")
                textFont: AtStyle.font(13)

                toolTipTitle: qsTrc("fontdesign", "Export font as…")

                onClicked: {
                    root.fontModel.exportFontAs()
                }
            }

            Item {
                width: 6
                height: 1
            }

            FlatButton {
                anchors.verticalCenter: parent.verticalCenter

                height: 28
                minWidth: 0
                margins: 14

                text: qsTrc("fontdesign", "Install")
                accentButton: true

                toolTipTitle: qsTrc("fontdesign", "Install to MuseScore music fonts folder")

                onClicked: {
                    root.fontModel.installToMuseScore()
                }
            }
        }
    }

    // ---------- the strip: what is done to the contours ----------

    Rectangle {
        id: strip

        anchors.top: parent.top
        width: parent.width
        height: visible ? stripRow.height + 9 : 0

        visible: root.hasFont
        color: AtStyle.island

        NavigationPanel {
            id: stripNavPanel
            name: "FontDesignStrip"
            section: root.toolsNavSec
            order: 1
            enabled: strip.visible
            accessible.name: qsTrc("appshell", "Font design tools")
        }

        component ShapeButton: AtToolButton {
            property string kind: ""

            width: 30
            height: 32

            navigation.panel: stripNavPanel
            navigation.name: kind
            navigation.accessible.name: toolTipTitle

            FontPaletteIcon {
                anchors.centerIn: parent
                kind: parent.kind
                strokeColor: AtStyle.text
            }
        }

        Row {
            id: stripRow

            anchors.horizontalCenter: parent.horizontalCenter
            y: 5

            spacing: 1

            Item {
                width: snapChip.width
                height: 32

                AtChip {
                    id: snapChip

                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTrc("fontdesign", "Snap")
                    checked: Boolean(root.glyphCanvas) && root.glyphCanvas.snapEnabled

                    toolTipTitle: qsTrc("fontdesign", "Snap to units, staff lines and advance")

                    navigation.panel: stripNavPanel
                    navigation.name: "Snap"
                    navigation.order: 1

                    onClicked: {
                        root.glyphCanvas.snapEnabled = !root.glyphCanvas.snapEnabled
                        root.glyphCanvas.forceActiveFocus()
                    }
                }
            }

            AtDivider {
                y: 5
                lineHeight: 22
                sideMargin: 8
            }

            ShapeButton {
                kind: "union"
                toolTipTitle: qsTrc("fontdesign", "Union — merge selected contours, or all when nothing is selected")
                navigation.order: 2

                onClicked: {
                    root.glyphCanvas.booleanUnion()
                    root.glyphCanvas.forceActiveFocus()
                }
            }

            ShapeButton {
                kind: "subtract"
                toolTipTitle: qsTrc("fontdesign", "Subtract — cut the selected contours out of the rest")
                navigation.order: 3

                onClicked: {
                    root.glyphCanvas.booleanSubtract()
                    root.glyphCanvas.forceActiveFocus()
                }
            }

            ShapeButton {
                kind: "intersect"
                toolTipTitle: qsTrc("fontdesign", "Intersect — keep the overlap of the selected contours and the rest")
                navigation.order: 4

                onClicked: {
                    root.glyphCanvas.booleanIntersect()
                    root.glyphCanvas.forceActiveFocus()
                }
            }

            AtDivider {
                y: 5
                lineHeight: 22
                sideMargin: 8
            }

            ShapeButton {
                kind: "fliph"
                toolTipTitle: qsTrc("fontdesign", "Flip horizontally")
                navigation.order: 5

                onClicked: {
                    root.glyphCanvas.flipHorizontally()
                    root.glyphCanvas.forceActiveFocus()
                }
            }

            ShapeButton {
                kind: "flipv"
                toolTipTitle: qsTrc("fontdesign", "Flip vertically")
                navigation.order: 6

                onClicked: {
                    root.glyphCanvas.flipVertically()
                    root.glyphCanvas.forceActiveFocus()
                }
            }

            ShapeButton {
                kind: "direction"
                toolTipTitle: qsTrc("fontdesign", "Correct path directions — make nested contours punch holes")
                navigation.order: 7

                onClicked: {
                    root.glyphCanvas.correctPathDirections()
                    root.glyphCanvas.forceActiveFocus()
                }
            }

            AtDivider {
                y: 5
                lineHeight: 22
                sideMargin: 8
            }

            AtToolButton {
                width: 30
                height: 32

                icon: IconCode.IMAGE_MOUNTAINS
                iconFont: ui.theme.toolbarIconsFont
                contentColor: AtStyle.text

                checked: Boolean(root.glyphCanvas) && root.glyphCanvas.hasReferenceImage
                checkedStyle: "accent"

                toolTipTitle: qsTrc("fontdesign", "Import reference image (Alt+drag to move, Alt+wheel to scale)")

                navigation.panel: stripNavPanel
                navigation.name: "ImportReferenceImage"
                navigation.order: 8

                onClicked: {
                    root.glyphCanvas.importReferenceImage()
                }
            }

            AtToolButton {
                width: 30
                height: 32

                icon: IconCode.FIT_SELECTION
                iconFont: ui.theme.toolbarIconsFont
                contentColor: AtStyle.text

                toolTipTitle: qsTrc("fontdesign", "Fit glyph to view")

                navigation.panel: stripNavPanel
                navigation.name: "FitGlyph"
                navigation.order: 9

                onClicked: {
                    root.glyphCanvas.resetView()
                }
            }

            AtToolButton {
                width: 30
                height: 32

                shape: "plus"
                contentColor: AtStyle.text

                toolTipTitle: qsTrc("fontdesign", "Add glyph from another font…")

                navigation.panel: stripNavPanel
                navigation.name: "AddGlyph"
                navigation.order: 10

                onClicked: {
                    root.fontModel.openAddGlyphDialog()
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

    // ---------- the footer: the zoom of the glyph ----------

    Rectangle {
        id: footer

        anchors.bottom: parent.bottom
        width: parent.width
        height: visible ? AtStyle.statusBarHeight : 0

        visible: root.hasFont
        color: AtStyle.island

        Rectangle {
            width: parent.width
            height: 1
            color: AtStyle.stroke
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter

            spacing: 2

            AtToolButton {
                anchors.verticalCenter: parent.verticalCenter
                width: 22
                height: 22
                radius: 6

                shape: "minus"
                toolTipTitle: qsTrc("notation", "Zoom out")

                onClicked: {
                    root.glyphCanvas.zoomOut()
                }
            }

            AtToolButton {
                anchors.verticalCenter: parent.verticalCenter
                height: 22
                radius: 6

                //: A button which fits the glyph to the view
                text: qsTrc("appshell", "Fit")
                textFont: AtStyle.font(11.5)

                toolTipTitle: qsTrc("fontdesign", "Fit glyph to view")

                onClicked: {
                    root.glyphCanvas.resetView()
                }
            }

            AtToolButton {
                anchors.verticalCenter: parent.verticalCenter
                width: 22
                height: 22
                radius: 6

                shape: "plus"
                toolTipTitle: qsTrc("notation", "Zoom in")

                onClicked: {
                    root.glyphCanvas.zoomIn()
                }
            }
        }
    }

    // ---------- left: the glyphs ----------

    Item {
        id: leftPane

        anchors.left: parent.left
        anchors.top: strip.bottom
        anchors.bottom: footer.top

        width: visible ? leftResizer.paneWidth : 0
        visible: root.hasFont && root.isOpen(root.leftPaneName)

        AtPaneHeader {
            id: glyphsHeader

            width: parent.width - 1

            title: qsTrc("appshell", "Glyphs")
            leftPadding: 14
        }

        GlyphBrowserPanel {
            anchors.top: glyphsHeader.bottom
            anchors.bottom: parent.bottom
            width: parent.width - 1

            color: AtStyle.island
        }

        Rectangle {
            anchors.right: parent.right
            width: 1
            height: parent.height
            color: AtStyle.stroke
        }
    }

    // ---------- right: the properties ----------

    Item {
        id: rightPane

        anchors.right: parent.right
        anchors.top: strip.bottom
        anchors.bottom: footer.top

        width: visible ? rightResizer.paneWidth : 0
        visible: root.hasFont && root.isOpen(root.rightPaneName)

        Rectangle {
            width: 1
            height: parent.height
            color: AtStyle.stroke
        }

        AtPaneHeader {
            id: propertiesHeader

            x: 1
            width: parent.width - 1

            title: qsTrc("appshell", "Properties")
            separatorVisible: true
        }

        FontDesignInspectorPanel {
            anchors.top: propertiesHeader.bottom
            anchors.bottom: parent.bottom
            x: 1
            width: parent.width - 1
        }
    }

    // ---------- middle: the glyph ----------

    FontDesignCentral {
        id: central

        anchors.left: leftPane.right
        anchors.right: rightPane.left
        anchors.top: strip.bottom
        anchors.bottom: footer.top

        chromeVisible: false
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
        minimumWidth: 220
        maximumWidth: Math.max(minimumWidth, Math.min(520, (root.width - 320) / 2))
    }

    AtelierPaneResizer {
        id: rightResizer

        shell: root.shell
        pane: rightPane
        edge: Qt.LeftEdge

        pageUri: root.pageUri
        name: "rightPane"

        defaultWidth: 320
        minimumWidth: 280
        maximumWidth: Math.max(minimumWidth, Math.min(520, (root.width - 320) / 2))
    }
}
