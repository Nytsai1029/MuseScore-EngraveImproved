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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.FontDesign 1.0

Rectangle {
    id: root

    //! NOTE: if false, the view shows the glyph only: its toolbar and its tool palette are not shown,
    //! and the owner of the view provides the tools (see `glyphCanvas` and `model`)
    property bool chromeVisible: true

    readonly property alias glyphCanvas: canvas
    readonly property alias model: pageModel

    color: ui.theme.backgroundPrimaryColor

    FontDesignPageModel {
        id: pageModel
    }

    Component.onCompleted: {
        pageModel.init()
        canvas.init()
    }

    Column {
        anchors.fill: parent

        visible: pageModel.hasProject

        Rectangle {
            id: toolBarStrip

            width: parent.width
            height: root.chromeVisible ? 40 : 0
            visible: root.chromeVisible

            color: ui.theme.backgroundSecondaryColor

            //! 紧凑图标工具栏：文件组 | 撤销组 | 视图 —— 标题 —— 面板开关
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8

                spacing: 4

                FlatButton {
                    icon: IconCode.SAVE
                    transparent: true
                    enabled: pageModel.hasProject
                    toolTipTitle: qsTrc("fontdesign", "Save font")
                    toolTipShortcut: "Ctrl+S"

                    onClicked: {
                        pageModel.save()
                    }
                }

                FlatButton {
                    icon: IconCode.SHARE_FILE
                    transparent: true
                    enabled: pageModel.hasProject
                    toolTipTitle: qsTrc("fontdesign", "Export font as…")

                    onClicked: {
                        pageModel.exportFontAs()
                    }
                }

                FlatButton {
                    icon: IconCode.IMPORT
                    transparent: true
                    enabled: pageModel.hasProject
                    toolTipTitle: qsTrc("fontdesign", "Install to MuseScore music fonts folder")

                    onClicked: {
                        pageModel.installToMuseScore()
                    }
                }

                SeparatorLine {
                    Layout.preferredHeight: 22
                    orientation: Qt.Vertical
                }

                FlatButton {
                    icon: IconCode.UNDO
                    transparent: true
                    enabled: pageModel.canUndo
                    toolTipTitle: qsTrc("fontdesign", "Undo")
                    toolTipShortcut: "Ctrl+Z"

                    onClicked: {
                        pageModel.undo()
                    }
                }

                FlatButton {
                    icon: IconCode.REDO
                    transparent: true
                    enabled: pageModel.canRedo
                    toolTipTitle: qsTrc("fontdesign", "Redo")
                    toolTipShortcut: "Ctrl+Shift+Z"

                    onClicked: {
                        pageModel.redo()
                    }
                }

                SeparatorLine {
                    Layout.preferredHeight: 22
                    orientation: Qt.Vertical
                }

                FlatButton {
                    icon: IconCode.FIT_SELECTION
                    transparent: true
                    enabled: pageModel.hasProject
                    toolTipTitle: qsTrc("fontdesign", "Fit glyph to view")

                    onClicked: {
                        canvas.resetView()
                    }
                }

                FlatButton {
                    icon: IconCode.IMAGE_MOUNTAINS
                    transparent: !canvas.hasReferenceImage
                    accentButton: canvas.hasReferenceImage
                    enabled: pageModel.hasProject
                    toolTipTitle: qsTrc("fontdesign", "Import reference image (Alt+drag to move, Alt+wheel to scale)")

                    onClicked: {
                        canvas.importReferenceImage()
                    }
                }

                FlatButton {
                    icon: IconCode.PLUS
                    transparent: true
                    enabled: pageModel.hasProject
                    toolTipTitle: qsTrc("fontdesign", "Add glyph from another font…")

                    onClicked: {
                        pageModel.openAddGlyphDialog()
                    }
                }

                FlatButton {
                    icon: IconCode.TICK_RIGHT_ANGLE
                    transparent: true
                    enabled: pageModel.hasProject
                    toolTipTitle: qsTrc("fontdesign", "Check font (SMuFL validation)")

                    onClicked: {
                        pageModel.openLintDialog()
                    }
                }

                StyledTextLabel {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 32
                    Layout.leftMargin: 8
                    Layout.rightMargin: 8

                    text: pageModel.projectTitle + (pageModel.isDirty ? " •" : "")
                    font: ui.theme.bodyBoldFont
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideMiddle
                }

                //! 侧面板开关（面板被关闭/浮动关闭后由此恢复）
                FlatButton {
                    icon: IconCode.GRID
                    transparent: !pageModel.glyphsPanelOpen
                    accentButton: pageModel.glyphsPanelOpen
                    toolTipTitle: qsTrc("fontdesign", "Show/hide the glyph browser panel")

                    onClicked: {
                        pageModel.toggleGlyphsPanel()
                    }
                }

                FlatButton {
                    icon: IconCode.LIST
                    transparent: !pageModel.inspectorPanelOpen
                    accentButton: pageModel.inspectorPanelOpen
                    toolTipTitle: qsTrc("fontdesign", "Show/hide the properties panel")

                    onClicked: {
                        pageModel.toggleInspectorPanel()
                    }
                }
            }

            SeparatorLine {
                anchors.bottom: parent.bottom
            }
        }

        Item {
            width: parent.width
            height: parent.height - toolBarStrip.height

            GlyphCanvas {
                id: canvas

                anchors.fill: parent

                backgroundColor: ui.theme.backgroundPrimaryColor
                gridColor: ui.theme.strokeColor
                outlineColor: ui.theme.fontPrimaryColor
                accentColor: ui.theme.accentColor

                focus: true
            }

            // 左侧工具面板（选择 / 节点 / 直线钢笔 / 曲线钢笔 / 矩形 / 椭圆）
            Column {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: 8

                visible: root.chromeVisible

                spacing: 4

                Repeater {
                    model: [
                        { "tool": 0, "kind": "select", "tip": qsTrc("fontdesign", "Select / move (V)") },
                        { "tool": 1, "kind": "node", "tip": qsTrc("fontdesign", "Edit nodes (A)") },
                        { "tool": 2, "kind": "pen", "tip": qsTrc("fontdesign", "Pen — straight (P)") },
                        { "tool": 5, "kind": "curvepen", "tip": qsTrc("fontdesign", "Pen — curve (C)") },
                        { "tool": 3, "kind": "rect", "tip": qsTrc("fontdesign", "Rectangle (R)") },
                        { "tool": 4, "kind": "ellipse", "tip": qsTrc("fontdesign", "Ellipse (E)") }
                    ]

                    delegate: FlatButton {
                        width: 34
                        height: 34

                        transparent: canvas.tool !== modelData.tool
                        toolTipTitle: modelData.tip
                        accentButton: canvas.tool === modelData.tool

                        onClicked: {
                            canvas.tool = modelData.tool
                            canvas.forceActiveFocus()
                        }

                        FontToolIcon {
                            anchors.centerIn: parent
                            kind: modelData.kind
                        }
                    }
                }

                SeparatorLine {
                    width: 34
                }

                // 捕捉开关（整数字体单位 / 谱线 / 原点与 advance 线）
                FlatButton {
                    width: 34
                    height: 34

                    transparent: !canvas.snapEnabled
                    accentButton: canvas.snapEnabled
                    toolTipTitle: qsTrc("fontdesign", "Snap to units, staff lines and advance")

                    FontPaletteIcon {
                        anchors.centerIn: parent
                        kind: "snap"
                    }

                    onClicked: {
                        canvas.snapEnabled = !canvas.snapEnabled
                        canvas.forceActiveFocus()
                    }
                }

                SeparatorLine {
                    width: 34
                }

                // 布尔运算（选中轮廓作为第二操作数）
                FlatButton {
                    width: 34
                    height: 34

                    transparent: true
                    toolTipTitle: qsTrc("fontdesign", "Union — merge selected contours, or all when nothing is selected")

                    FontPaletteIcon {
                        anchors.centerIn: parent
                        kind: "union"
                    }

                    onClicked: {
                        canvas.booleanUnion()
                        canvas.forceActiveFocus()
                    }
                }

                FlatButton {
                    width: 34
                    height: 34

                    transparent: true
                    toolTipTitle: qsTrc("fontdesign", "Subtract — cut the selected contours out of the rest")

                    FontPaletteIcon {
                        anchors.centerIn: parent
                        kind: "subtract"
                    }

                    onClicked: {
                        canvas.booleanSubtract()
                        canvas.forceActiveFocus()
                    }
                }

                FlatButton {
                    width: 34
                    height: 34

                    transparent: true
                    toolTipTitle: qsTrc("fontdesign", "Intersect — keep the overlap of the selected contours and the rest")

                    FontPaletteIcon {
                        anchors.centerIn: parent
                        kind: "intersect"
                    }

                    onClicked: {
                        canvas.booleanIntersect()
                        canvas.forceActiveFocus()
                    }
                }

                SeparatorLine {
                    width: 34
                }

                // 翻转（选中轮廓；无选择 = 全部）
                FlatButton {
                    width: 34
                    height: 34

                    transparent: true
                    toolTipTitle: qsTrc("fontdesign", "Flip horizontally")

                    FontPaletteIcon {
                        anchors.centerIn: parent
                        kind: "fliph"
                    }

                    onClicked: {
                        canvas.flipHorizontally()
                        canvas.forceActiveFocus()
                    }
                }

                FlatButton {
                    width: 34
                    height: 34

                    transparent: true
                    toolTipTitle: qsTrc("fontdesign", "Flip vertically")

                    FontPaletteIcon {
                        anchors.centerIn: parent
                        kind: "flipv"
                    }

                    onClicked: {
                        canvas.flipVertically()
                        canvas.forceActiveFocus()
                    }
                }

                FlatButton {
                    width: 34
                    height: 34

                    transparent: true
                    toolTipTitle: qsTrc("fontdesign", "Correct path directions — make nested contours punch holes")

                    FontPaletteIcon {
                        anchors.centerIn: parent
                        kind: "direction"
                    }

                    onClicked: {
                        canvas.correctPathDirections()
                        canvas.forceActiveFocus()
                    }
                }
            }

            // 参考图控制条（透明度 / 复位 / 移除）
            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 8

                visible: canvas.hasReferenceImage

                width: refControls.implicitWidth + 20
                height: 40
                radius: 4
                color: ui.theme.backgroundSecondaryColor
                border.color: ui.theme.strokeColor
                border.width: 1

                RowLayout {
                    id: refControls

                    anchors.centerIn: parent

                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("fontdesign", "Reference")
                        opacity: 0.7
                    }

                    StyledSlider {
                        implicitWidth: 96

                        from: 0.05
                        to: 1.0
                        value: canvas.referenceImageOpacity

                        onMoved: {
                            canvas.referenceImageOpacity = value
                        }
                    }

                    FlatButton {
                        icon: IconCode.FIT_PROJECT
                        transparent: true
                        toolTipTitle: qsTrc("fontdesign", "Reset image to staff height")

                        onClicked: {
                            canvas.resetReferenceImagePlacement()
                        }
                    }

                    FlatButton {
                        icon: IconCode.CLOSE_X_ROUNDED
                        transparent: true
                        toolTipTitle: qsTrc("fontdesign", "Remove reference image")

                        onClicked: {
                            canvas.clearReferenceImage()
                        }
                    }
                }
            }

            // 开放路径状态提示（钢笔在建轮廓）
            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 12

                visible: canvas.penActive

                width: penHintLabel.implicitWidth + 24
                height: penHintLabel.implicitHeight + 12
                radius: 4
                color: ui.theme.backgroundSecondaryColor
                border.color: ui.theme.strokeColor
                border.width: 1

                StyledTextLabel {
                    id: penHintLabel

                    anchors.centerIn: parent

                    text: qsTrc("fontdesign", "Open path — click the start point or press Enter to close · Esc discards · Backspace removes the last point")
                }
            }
        }
    }

    Column {
        anchors.centerIn: parent
        width: Math.min(parent.width - 80, 480)

        visible: !pageModel.hasProject

        spacing: 24

        StyledIconLabel {
            anchors.horizontalCenter: parent.horizontalCenter

            iconCode: IconCode.BRUSH
            font.pixelSize: 64
        }

        StyledTextLabel {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width

            text: qsTrc("fontdesign", "No font project is open")
            font: ui.theme.headerBoldFont
            horizontalAlignment: Text.AlignHCenter
        }

        StyledTextLabel {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width

            text: qsTrc("fontdesign", "Open a SMuFL font (OTF/TTF) to browse and edit its glyphs and metadata.")
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 12

            FlatButton {
                text: qsTrc("fontdesign", "New font…")
                accentButton: true

                onClicked: {
                    pageModel.newFont()
                }
            }

            FlatButton {
                text: qsTrc("fontdesign", "Open font…")

                onClicked: {
                    pageModel.openFont()
                }
            }

            FlatButton {
                text: qsTrc("fontdesign", "Go to font design home")

                onClicked: {
                    pageModel.goToProjectsSection()
                }
            }
        }
    }
}
