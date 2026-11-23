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

//! NOTE: a picture of music on a piece of paper (see EngravingPreview).
//! The paper is white whatever the theme is: it is what the page of the score looks like.
//! Music which is not valid is not drawn: what is wrong with it is written instead, so that the author
//! of a page sees it at once
Rectangle {
    id: root

    property alias music: preview.music
    property alias overrides: preview.overrides
    property alias systemWidth: preview.systemWidth
    property alias zoom: preview.zoom

    property alias activeSid: preview.activeSid
    property alias probeValue: preview.probeValue
    property alias isDraggable: preview.isDraggable
    property alias showsHandles: preview.showsHandles
    property alias showsControlPoints: preview.showsControlPoints
    property alias dragMinimum: preview.dragMinimum
    property alias dragMaximum: preview.dragMaximum
    property alias dragStep: preview.dragStep

    readonly property alias naturalSize: preview.naturalSize
    readonly property alias hasEffect: preview.hasEffect
    readonly property alias hasHandle: preview.hasHandle
    readonly property alias isDragging: preview.isDragging
    readonly property alias dragValue: preview.dragValue

    //! NOTE: the handle was let go inside of the picture
    signal valueDropped(var value)

    //! NOTE: the room around the picture
    property int padding: 14

    readonly property alias contentSize: preview.contentSize

    implicitWidth: preview.contentSize.width + 2 * padding
    implicitHeight: preview.error ? errorLabel.implicitHeight + 2 * padding
                                  : preview.contentSize.height + 2 * padding

    radius: 8
    color: "#FFFFFF"
    border.width: 1
    border.color: AtStyle.stroke

    //! NOTE: a picture wider than the paper is scrolled horizontally inside it, not scaled down
    Flickable {
        id: flick

        anchors.fill: parent

        boundsBehavior: Flickable.StopAtBounds
        interactive: contentWidth > width || contentHeight > height
        clip: true

        contentWidth: preview.width + 2 * root.padding
        contentHeight: preview.height + 2 * root.padding

        //! NOTE: where the change is is what is shown when the picture does not fit
        function scrollToChange() {
            const box = preview.changeBox
            const room = contentWidth - width
            if (box.width <= 0 || room <= 0) {
                return
            }

            const centre = root.padding + box.x + box.width / 2
            contentX = Math.max(0, Math.min(room, centre - width / 2))
        }

        EngravingPreview {
            id: preview

            x: root.padding
            y: root.padding
            width: Math.max(flick.width - 2 * root.padding, contentSize.width)
            height: Math.max(flick.height - 2 * root.padding, contentSize.height)

            accentColor: AtStyle.accent

            onChangeBoxChanged: {
                flick.scrollToChange()
            }

            onValueDropped: function(value) {
                root.valueDropped(value)
            }
        }
    }

    StyledTextLabel {
        id: errorLabel

        anchors.fill: parent
        anchors.margins: root.padding

        visible: Boolean(preview.error)
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap

        text: preview.error
        font: AtStyle.font(11)
        color: "#D93A3A"
    }
}
