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

//! NOTE: the title of a pane, with an optional line below it and an optional button on the right
Item {
    id: root

    property string title: ""
    property string subtitle: ""

    property bool separatorVisible: false

    property int leftPadding: 16
    property int rightPadding: 10

    default property alias trailingContent: trailing.data

    implicitHeight: column.y + column.height + (subtitleLabel.visible ? 12 : 8)

    Column {
        id: column

        x: root.leftPadding
        y: subtitleLabel.visible ? 13 : 12
        width: parent.width - root.leftPadding - root.rightPadding - trailing.width - (trailing.width > 0 ? 8 : 0)

        spacing: 3

        StyledTextLabel {
            width: parent.width
            horizontalAlignment: Text.AlignLeft
            text: root.title
            font: AtStyle.font(15, Font.DemiBold)
        }

        StyledTextLabel {
            id: subtitleLabel

            width: parent.width
            horizontalAlignment: Text.AlignLeft
            visible: Boolean(root.subtitle)
            text: root.subtitle
            font: AtStyle.font(12)
            color: AtStyle.text2
            wrapMode: Text.WordWrap
            maximumLineCount: 2
        }
    }

    Row {
        id: trailing

        anchors.right: parent.right
        anchors.rightMargin: root.rightPadding
        y: 9

        spacing: 2
    }

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        visible: root.separatorVisible
        color: AtStyle.stroke
    }
}
