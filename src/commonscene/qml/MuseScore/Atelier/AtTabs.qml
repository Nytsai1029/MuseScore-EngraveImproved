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

//! NOTE: the titles of the panels which share a zone. The model is a list of { "title", "name" }
Item {
    id: root

    property var model: []
    property string currentName: ""

    signal activated(string name)
    signal closeRequested(string name)

    implicitHeight: 34

    Row {
        x: 10
        anchors.verticalCenter: parent.verticalCenter
        spacing: 2

        Repeater {
            model: root.model

            delegate: AtToolButton {
                height: 24
                text: modelData.title
                textFont: AtStyle.font(12, checked ? Font.DemiBold : Font.Medium)
                checked: modelData.name === root.currentName
                checkedStyle: "soft"

                onClicked: {
                    root.activated(modelData.name)
                }
            }
        }
    }

    AtToolButton {
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter

        width: 24
        height: 24

        shape: "close"
        toolTipTitle: qsTrc("global", "Close")

        onClicked: {
            root.closeRequested(root.currentName)
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: AtStyle.stroke
    }
}
