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

//! NOTE: the line icons of the Atelier skin (16 × 16, 1.5 px strokes), which the icon font does not have
Canvas {
    id: root

    property string name: ""
    property color color: AtStyle.text2

    readonly property bool isEmpty: !Boolean(prv.icons[name])

    width: 16
    height: 16

    onNameChanged: requestPaint()
    onColorChanged: requestPaint()
    onVisibleChanged: requestPaint()

    QtObject {
        id: prv

        readonly property string frame: "M4.25 2.75h7.5a2.5 2.5 0 0 1 2.5 2.5v5.5a2.5 2.5 0 0 1-2.5 2.5h-7.5a2.5 2.5 0 0 1-2.5-2.5v-5.5a2.5 2.5 0 0 1 2.5-2.5z"

        //! NOTE: each part is { d: path, fill: filled instead of stroked, w: stroke width }
        readonly property var icons: ({
            "sidebar-left": [{ d: frame }, { d: "M6 3v10" }],
            "sidebar-right": [{ d: frame }, { d: "M10 3v10" }],
            "home": [{ d: "M2.5 7.2 8 2.6l5.5 4.6V13a.9.9 0 0 1-.9.9H9.8v-3.6H6.2v3.6H3.4a.9.9 0 0 1-.9-.9z" }],
            "dots": [{ d: "M2.1 8a1.3 1.3 0 1 0 2.6 0a1.3 1.3 0 1 0-2.6 0M6.7 8a1.3 1.3 0 1 0 2.6 0a1.3 1.3 0 1 0-2.6 0M11.3 8a1.3 1.3 0 1 0 2.6 0a1.3 1.3 0 1 0-2.6 0", fill: true }],
            "mixer": [{ d: "M4 2.6v10.8M8 2.6v10.8M12 2.6v10.8" }, { d: "M2.7 9.4h2.6M6.7 5.4h2.6M10.7 10.8h2.6", w: 2.6 }],
            "rewind": [{ d: "M4 3.6v8.8" }, { d: "M12.4 3.9v8.2L6.3 8z", fill: true, stroke: true }],
            "play": [{ d: "M5.2 3.4v9.2L12.6 8z", fill: true, stroke: true }],
            "pause": [{ d: "M5.6 3.6v8.8M10.4 3.6v8.8", w: 2.2 }],
            "loop": [{ d: "M3 7.5v-.7A2.8 2.8 0 0 1 5.8 4H12M10.2 2.2 12 4l-1.8 1.8M13 8.5v.7a2.8 2.8 0 0 1-2.8 2.8H4M5.8 13.8 4 12l1.8-1.8" }],
            "metronome": [{ d: "M5.7 2.8h4.6l2.5 10.4H3.2zM8 10.2l3.2-6.3M4 10.2h8" }],
            "undo": [{ d: "M5.5 3.2 2.8 5.9l2.7 2.7M3 5.9h6.2a3.6 3.6 0 0 1 0 7.2H6.5" }],
            "redo": [{ d: "M10.5 3.2l2.7 2.7-2.7 2.7M13 5.9H6.8a3.6 3.6 0 0 0 0 7.2h2.7" }],
            "chevron-down": [{ d: "M5 6.6 8 9.6l3-3" }],
            "chevron-right": [{ d: "M6.6 5 9.6 8l-3 3" }],
            "search": [{ d: "M2.8 7a4.2 4.2 0 1 0 8.4 0a4.2 4.2 0 1 0-8.4 0M10.2 10.2 13.5 13.5" }],
            "plus": [{ d: "M8 3.2v9.6M3.2 8h9.6" }],
            "minus": [{ d: "M3.5 8h9" }],
            "close": [{ d: "M4.2 4.2l7.6 7.6M11.8 4.2l-7.6 7.6" }],
            "pencil": [{ d: "M10.9 2.6l2.5 2.5-7.8 7.8-3.1.6.6-3.1z" }, { d: "M9.6 3.9l2.5 2.5" }],
            "flip": [{ d: "M5 13V3.5M2.8 5.7 5 3.5l2.2 2.2M11 3v9.5M8.8 10.3 11 12.5l2.2-2.2" }],
            "arrow-left": [{ d: "M13 8H3.5M7 4.2 3.2 8 7 11.8" }],
            "arrow-up": [{ d: "M8 13V3.5M4.2 7 8 3.2 11.8 7" }],
            "arrow-down": [{ d: "M8 3v9.5M4.2 9 8 12.8 11.8 9" }],
            "arrow-right": [{ d: "M3 8h9.5M9 4.2 12.8 8 9 11.8" }],
            "reset": [{ d: "M4.3 4.6A4.8 4.8 0 1 1 3.2 8" }, { d: "M4.6 1.9v2.9H1.7" }],
            "stretch-less": [{ d: "M1.8 8h4.4M4.2 6l2 2-2 2M14.2 8H9.8M11.8 6l-2 2 2 2" }],
            "stretch-more": [{ d: "M6.6 8H2.2M4.2 6l-2 2 2 2M9.4 8h4.4M11.8 6l2 2-2 2" }],
            "print": [{ d: "M4.5 6V2.5h7V6M4.5 11.5H3a1 1 0 0 1-1-1V7a1 1 0 0 1 1-1h10a1 1 0 0 1 1 1v3.5a1 1 0 0 1-1 1h-1.5M4.5 9.5h7v4h-7z" }],
            "export": [{ d: "M8 2.5V10M5 5.5l3-3 3 3M3 9.5v3a1 1 0 0 0 1 1h8a1 1 0 0 0 1-1v-3" }],
            "cloud-up": [{ d: "M4.6 12.4a3 3 0 0 1-.4-5.9 4 4 0 0 1 7.7 1 2.5 2.5 0 0 1-.4 4.9M8 13.6V8.4M6 10.2l2-2 2 2" }],
            "audio": [{ d: "M3 6.5v3M5.5 4.5v7M8 2.8v10.4M10.5 5v6M13 6.8v2.4" }],
            "window": [{ d: "M3.8 5h5.4a1.3 1.3 0 0 1 1.3 1.3v5.4a1.3 1.3 0 0 1-1.3 1.3H3.8a1.3 1.3 0 0 1-1.3-1.3V6.3A1.3 1.3 0 0 1 3.8 5z" },
                       { d: "M5.6 5V3.8a1.3 1.3 0 0 1 1.3-1.3h5.3a1.3 1.3 0 0 1 1.3 1.3v5.3a1.3 1.3 0 0 1-1.3 1.3h-1.7" }],
            "concert-pitch": [{ d: "M5.5 2.2v4.3a2.5 2.5 0 0 0 5 0V2.2M8 9v4.8" }],
            "restart": [{ d: "M12.8 8a4.8 4.8 0 1 1-1.4-3.4" }, { d: "M11.6 1.9v2.9H8.7" }],
            "gear": [{ d: "M8 5.6a2.4 2.4 0 1 0 0 4.8a2.4 2.4 0 1 0 0-4.8M8 1.8v1.6M8 12.6v1.6M1.8 8h1.6M12.6 8h1.6M3.6 3.6l1.1 1.1M11.3 11.3l1.1 1.1M3.6 12.4l1.1-1.1M11.3 4.7l1.1-1.1" }]
        })
    }

    onPaint: {
        var ctx = getContext("2d")
        ctx.reset()

        var parts = prv.icons[root.name]
        if (!parts) {
            return
        }

        ctx.scale(root.width / 16, root.height / 16)

        ctx.strokeStyle = root.color
        ctx.fillStyle = root.color
        ctx.lineCap = "round"
        ctx.lineJoin = "round"

        for (var i = 0; i < parts.length; ++i) {
            var part = parts[i]

            ctx.lineWidth = part.w ? part.w : 1.5
            ctx.beginPath()
            ctx.path = part.d

            if (part.fill) {
                ctx.fill()
            }

            if (!part.fill || part.stroke) {
                ctx.stroke()
            }
        }
    }
}
