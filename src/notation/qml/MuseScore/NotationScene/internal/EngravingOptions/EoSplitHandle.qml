/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

import Muse.UiComponents 1.0
import MuseScore.Atelier 1.0

//! NOTE: the hairline between two columns of the dialog which is dragged to give one of them a new
//! width: a line of 1 px in a hit area of 9 px, with the split cursor; the accent color while it is
//! touched, and its default width on a double click
Item {
    id: root

    //! NOTE: the width of the column which the handle moves, where the drag starts
    required property real value
    required property real minimum
    required property real maximum
    required property real defaultValue

    //! NOTE: the column is on the right of the handle (the preview pane): dragging left makes it wider
    property bool inverted: false

    //! NOTE: the width the column takes where the pointer is
    signal moved(real width)

    //! NOTE: the drag is done: the pointer is let go, or the default width is restored —
    //! the one moment to keep the width, not at every pointer move
    signal dropped()

    width: 9

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        width: 1
        height: parent.height

        color: dragArea.containsMouse || dragArea.pressed ? AtStyle.accent : AtStyle.stroke
    }

    MouseArea {
        id: dragArea

        anchors.fill: parent

        hoverEnabled: true
        cursorShape: Qt.SplitHCursor

        //! NOTE: where the pointer took the handle, so that the width follows the pointer's position
        //! in the parent and does not drift
        property real grabOffset: 0

        function widthAt(pos) {
            return root.inverted ? root.parent.width - pos.x : pos.x
        }

        onPressed: function(mouse) {
            grabOffset = widthAt(mapToItem(root.parent, mouse.x, 0)) - root.value
        }

        onPositionChanged: function(mouse) {
            var width = widthAt(mapToItem(root.parent, mouse.x, 0)) - grabOffset
            root.moved(Math.max(root.minimum, Math.min(root.maximum, width)))
        }

        onReleased: {
            root.dropped()
        }

        onDoubleClicked: function(mouse) {
            root.moved(root.defaultValue)
            root.dropped()
        }
    }
}
