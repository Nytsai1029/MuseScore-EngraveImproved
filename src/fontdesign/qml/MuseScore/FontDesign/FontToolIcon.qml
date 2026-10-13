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

// 手绘矢量工具图标（随主题色重绘）
Canvas {
    id: root

    //! "select", "node", "pen", "curvepen", "rect", "ellipse"
    property string kind: ""

    width: 20
    height: 20

    property color strokeColor: ui.theme.fontPrimaryColor

    onStrokeColorChanged: requestPaint()
    onKindChanged: requestPaint()
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

        switch (root.kind) {
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
