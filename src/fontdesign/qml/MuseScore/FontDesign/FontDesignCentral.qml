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

    color: ui.theme.backgroundPrimaryColor

    //! 工具面板功能键的手绘矢量图标（捕捉/布尔/翻转）
    component PaletteIcon: Canvas {
        id: paletteIcon

        property string kind

        property color strokeColor: ui.theme.fontPrimaryColor

        width: 20
        height: 20

        onStrokeColorChanged: requestPaint()
        Component.onCompleted: requestPaint()

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.strokeStyle = strokeColor
            ctx.fillStyle = strokeColor
            ctx.lineWidth = 1.6
            ctx.lineCap = "round"
            ctx.lineJoin = "round"

            switch (paletteIcon.kind) {
            case "snap": {
                // 磁铁（U 形）
                ctx.lineWidth = 2.6
                ctx.beginPath()
                ctx.moveTo(6.5, 4)
                ctx.lineTo(6.5, 10.5)
                ctx.arc(10, 10.5, 3.5, Math.PI, 0, true)
                ctx.lineTo(13.5, 4)
                ctx.stroke()
                ctx.fillRect(5, 2.5, 3, 2.8)
                ctx.fillRect(12, 2.5, 3, 2.8)
                break
            }
            case "union": {
                ctx.beginPath()
                ctx.arc(7.6, 10, 4.6, 0, Math.PI * 2)
                ctx.arc(12.4, 10, 4.6, 0, Math.PI * 2)
                ctx.fill()
                break
            }
            case "subtract": {
                ctx.beginPath()
                ctx.arc(7.6, 10, 4.6, 0, Math.PI * 2)
                ctx.fill()
                ctx.globalCompositeOperation = "destination-out"
                ctx.beginPath()
                ctx.arc(12.9, 10, 4.6, 0, Math.PI * 2)
                ctx.fill()
                ctx.globalCompositeOperation = "source-over"
                ctx.globalAlpha = 0.5
                ctx.beginPath()
                ctx.arc(12.9, 10, 4.6, 0, Math.PI * 2)
                ctx.stroke()
                ctx.globalAlpha = 1.0
                break
            }
            case "intersect": {
                ctx.globalAlpha = 0.55
                ctx.beginPath()
                ctx.arc(7.6, 10, 4.6, 0, Math.PI * 2)
                ctx.stroke()
                ctx.beginPath()
                ctx.arc(12.4, 10, 4.6, 0, Math.PI * 2)
                ctx.stroke()
                ctx.globalAlpha = 1.0
                ctx.save()
                ctx.beginPath()
                ctx.arc(7.6, 10, 4.6, 0, Math.PI * 2)
                ctx.clip()
                ctx.beginPath()
                ctx.arc(12.4, 10, 4.6, 0, Math.PI * 2)
                ctx.fill()
                ctx.restore()
                break
            }
            case "fliph": {
                ctx.globalAlpha = 0.5
                ctx.beginPath()
                ctx.moveTo(10, 2.5)
                ctx.lineTo(10, 17.5)
                ctx.stroke()
                ctx.globalAlpha = 1.0
                ctx.beginPath()
                ctx.moveTo(3, 5.5)
                ctx.lineTo(8, 10)
                ctx.lineTo(3, 14.5)
                ctx.closePath()
                ctx.fill()
                ctx.beginPath()
                ctx.moveTo(17, 5.5)
                ctx.lineTo(12, 10)
                ctx.lineTo(17, 14.5)
                ctx.closePath()
                ctx.stroke()
                break
            }
            case "direction": {
                // 圆形箭头（路径方向）
                ctx.beginPath()
                ctx.arc(10, 10, 6, Math.PI * 0.15, Math.PI * 1.6)
                ctx.stroke()
                // 箭头
                var ax = 10 + 6 * Math.cos(Math.PI * 0.15)
                var ay = 10 + 6 * Math.sin(Math.PI * 0.15)
                ctx.beginPath()
                ctx.moveTo(ax + 2.6, ay - 2.2)
                ctx.lineTo(ax + 0.4, ay + 2.4)
                ctx.lineTo(ax - 3.2, ay - 1.2)
                ctx.closePath()
                ctx.fill()
                break
            }
            case "flipv": {
                ctx.globalAlpha = 0.5
                ctx.beginPath()
                ctx.moveTo(2.5, 10)
                ctx.lineTo(17.5, 10)
                ctx.stroke()
                ctx.globalAlpha = 1.0
                ctx.beginPath()
                ctx.moveTo(5.5, 3)
                ctx.lineTo(10, 8)
                ctx.lineTo(14.5, 3)
                ctx.closePath()
                ctx.fill()
                ctx.beginPath()
                ctx.moveTo(5.5, 17)
                ctx.lineTo(10, 12)
                ctx.lineTo(14.5, 17)
                ctx.closePath()
                ctx.stroke()
                break
            }
            }
        }
    }

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
            height: 40

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

                        // 手绘矢量工具图标（随主题色重绘）
                        Canvas {
                            id: iconCanvas

                            anchors.centerIn: parent
                            width: 20
                            height: 20

                            property color strokeColor: ui.theme.fontPrimaryColor

                            onStrokeColorChanged: requestPaint()
                            Component.onCompleted: requestPaint()

                            onPaint: {
                                var ctx = getContext("2d")
                                ctx.reset()
                                ctx.strokeStyle = strokeColor
                                ctx.fillStyle = strokeColor
                                ctx.lineWidth = 1.6
                                ctx.lineCap = "round"
                                ctx.lineJoin = "round"

                                function nodeSquare(x, y, r) {
                                    ctx.fillRect(x - r, y - r, 2 * r, 2 * r)
                                }
                                function nodeCircle(x, y, r) {
                                    ctx.beginPath()
                                    ctx.arc(x, y, r, 0, Math.PI * 2)
                                    ctx.fill()
                                }

                                switch (modelData.kind) {
                                case "select":
                                    // 光标箭头
                                    ctx.beginPath()
                                    ctx.moveTo(6, 2.5)
                                    ctx.lineTo(6, 15.5)
                                    ctx.lineTo(9.2, 12.6)
                                    ctx.lineTo(11.3, 17.5)
                                    ctx.lineTo(13.5, 16.4)
                                    ctx.lineTo(11.3, 11.8)
                                    ctx.lineTo(15.6, 11.4)
                                    ctx.closePath()
                                    ctx.fill()
                                    break
                                case "node":
                                    // 曲线 + 中央节点与切线柄
                                    ctx.beginPath()
                                    ctx.moveTo(2, 15.5)
                                    ctx.bezierCurveTo(6, 8, 14, 8, 18, 15.5)
                                    ctx.stroke()
                                    ctx.lineWidth = 1.1
                                    ctx.beginPath()
                                    ctx.moveTo(4, 9.6)
                                    ctx.lineTo(16, 9.6)
                                    ctx.stroke()
                                    nodeCircle(4, 9.6, 1.7)
                                    nodeCircle(16, 9.6, 1.7)
                                    nodeSquare(10, 9.6, 2.3)
                                    break
                                case "pen":
                                    // 折线 + 角点（直线段钢笔）
                                    ctx.beginPath()
                                    ctx.moveTo(3, 16)
                                    ctx.lineTo(8, 5.5)
                                    ctx.lineTo(13, 12.5)
                                    ctx.lineTo(17.5, 3.5)
                                    ctx.stroke()
                                    nodeSquare(3, 16, 2)
                                    nodeSquare(8, 5.5, 2)
                                    nodeSquare(13, 12.5, 2)
                                    nodeSquare(17.5, 3.5, 2)
                                    break
                                case "curvepen":
                                    // S 形曲线 + 平滑节点（曲线钢笔）
                                    ctx.beginPath()
                                    ctx.moveTo(3.5, 16.5)
                                    ctx.bezierCurveTo(11, 16.5, 6.5, 4.5, 16.5, 3.8)
                                    ctx.stroke()
                                    nodeCircle(3.5, 16.5, 2.1)
                                    nodeCircle(16.5, 3.8, 2.1)
                                    break
                                case "rect":
                                    ctx.strokeRect(3.2, 5, 13.6, 10)
                                    break
                                case "ellipse":
                                    var cx = 10, cy = 10, rx = 6.8, ry = 5
                                    var kx = 0.5523 * rx, ky = 0.5523 * ry
                                    ctx.beginPath()
                                    ctx.moveTo(cx + rx, cy)
                                    ctx.bezierCurveTo(cx + rx, cy + ky, cx + kx, cy + ry, cx, cy + ry)
                                    ctx.bezierCurveTo(cx - kx, cy + ry, cx - rx, cy + ky, cx - rx, cy)
                                    ctx.bezierCurveTo(cx - rx, cy - ky, cx - kx, cy - ry, cx, cy - ry)
                                    ctx.bezierCurveTo(cx + kx, cy - ry, cx + rx, cy - ky, cx + rx, cy)
                                    ctx.closePath()
                                    ctx.stroke()
                                    break
                                }
                            }
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

                    PaletteIcon {
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

                    PaletteIcon {
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

                    PaletteIcon {
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

                    PaletteIcon {
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

                    PaletteIcon {
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

                    PaletteIcon {
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

                    PaletteIcon {
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
