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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0
import MuseScore.Atelier 1.0

//! NOTE: a picture of a snippet of music on a piece of paper (see EngravingPreview).
//! The paper is white whatever the theme is: it is what the page of the score looks like
Rectangle {
    id: root

    property alias snippet: preview.snippet
    property alias overrides: preview.overrides
    property alias systemWidth: preview.systemWidth
    property alias zoom: preview.zoom

    //! NOTE: the room around the picture
    property int padding: 14

    readonly property alias contentSize: preview.contentSize

    implicitWidth: preview.contentSize.width + 2 * padding
    implicitHeight: preview.contentSize.height + 2 * padding

    radius: 8
    color: "#FFFFFF"
    border.width: 1
    border.color: AtStyle.stroke

    EngravingPreview {
        id: preview

        anchors.fill: parent
        anchors.margins: root.padding
    }
}
