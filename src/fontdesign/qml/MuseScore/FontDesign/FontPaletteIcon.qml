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

//! 工具面板功能键的手绘矢量图标（捕捉/布尔/翻转）
Canvas {
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
