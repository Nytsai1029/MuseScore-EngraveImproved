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
import MuseScore.AppShell 1.0
import MuseScore.Atelier 1.0

//! NOTE: the inner edge of a side pane. Dragging it changes the width of the pane, a double click gives the pane
//! the width of its own back. The width is kept with the layout of the window (see AtelierShellView)
Item {
    id: root

    required property AtelierShellView shell

    //! NOTE: the pane. The resizer is beside it, not in it: it is also over the edge of what is next to the pane
    required property Item pane

    //! NOTE: the edge of the pane which is dragged:
    //! the right one of a pane on the left, the left one of a pane on the right
    property int edge: Qt.RightEdge

    //! NOTE: the page and the name under which the width is kept
    property string pageUri: ""
    property string name: ""

    property int defaultWidth: 280
    property int minimumWidth: 200
    property int maximumWidth: 520

    //! NOTE: the width which the pane has to take
    readonly property int paneWidth: {
        var width = prv.dragWidth > 0 ? prv.dragWidth : (prv.savedWidth > 0 ? prv.savedWidth : root.defaultWidth)
        return Math.max(root.minimumWidth, Math.min(width, root.maximumWidth))
    }

    readonly property bool isResizing: mouseArea.pressed

    //! NOTE: mostly beside the pane, so that what is at the edge in the pane (a scroll bar) can still be reached
    x: root.edge === Qt.RightEdge ? root.pane.x + root.pane.width - 2 : root.pane.x - 5
    y: root.pane.y
    width: 7
    height: root.pane.height

    visible: root.pane.visible

    QtObject {
        id: prv

        readonly property int savedWidth: root.shell.paneWidth(root.pageUri, root.name, root.shell.paneWidthsRevision)

        //! NOTE: the width which is being given (0: the edge is not being dragged)
        property int dragWidth: 0
        property int pressWidth: 0
        property real pressX: 0

        function sceneX(x) {
            return mouseArea.mapToItem(null, x, 0).x
        }
    }

    //! NOTE: the line of the edge, while it is dragged
    Rectangle {
        x: root.edge === Qt.RightEdge ? 1 : 5
        width: 1
        height: parent.height

        color: ui.theme.accentColor
        visible: mouseArea.pressed
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        acceptedButtons: Qt.LeftButton
        cursorShape: Qt.SplitHCursor
        preventStealing: true

        onPressed: function(mouse) {
            prv.pressX = prv.sceneX(mouse.x)
            prv.pressWidth = root.paneWidth
            prv.dragWidth = root.paneWidth
        }

        onPositionChanged: function(mouse) {
            if (!mouseArea.pressed) {
                return
            }

            var delta = prv.sceneX(mouse.x) - prv.pressX
            prv.dragWidth = Math.max(1, prv.pressWidth + (root.edge === Qt.RightEdge ? delta : -delta))
        }

        onReleased: {
            var width = root.paneWidth
            if (width !== prv.pressWidth) {
                root.shell.setPaneWidth(root.pageUri, root.name, width)
            }

            prv.dragWidth = 0
        }

        onCanceled: {
            prv.dragWidth = 0
        }

        onDoubleClicked: {
            root.shell.setPaneWidth(root.pageUri, root.name, 0)
        }
    }
}
