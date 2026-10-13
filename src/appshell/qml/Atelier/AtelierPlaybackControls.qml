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
import MuseScore.Playback 1.0

import MuseScore.Atelier 1.0

//! NOTE: the playback buttons, the position (measure.beat) and the tempo
Row {
    id: root

    property NavigationPanel navigationPanel: null
    property int navigationOrderStart: 0

    spacing: 1

    PlaybackToolBarModel {
        id: playbackModel
    }

    Component.onCompleted: {
        playbackModel.load()
    }

    //! NOTE: the line icons of the actions which have one; the others keep the icon of the icon font
    function shapeFor(item) {
        switch (item.code) {
        case "rewind": return "rewind"
        case "loop": return "loop"
        case "metronome": return "metronome"
        case "play": return item.icon === IconCode.PAUSE ? "pause" : "play"
        }

        return ""
    }

    Repeater {
        model: playbackModel

        delegate: AtToolButton {
            id: btn

            readonly property var item: model.itemRole
            readonly property bool hasMenu: Boolean(item) && item.subitems.length !== 0

            anchors.verticalCenter: parent.verticalCenter

            enabled: playbackModel.isPlayAllowed

            shape: Boolean(item) ? root.shapeFor(item) : ""
            icon: Boolean(item) ? item.icon : IconCode.NONE
            iconFont: ui.theme.toolbarIconsFont

            contentColor: Boolean(item) && item.code === "play" ? AtStyle.text : AtStyle.text2
            checked: (Boolean(item) && item.checked && item.code !== "play") || menuLoader.isMenuOpened
            checkedStyle: "soft"

            toolTipTitle: Boolean(item) ? item.title : ""
            toolTipDescription: Boolean(item) ? item.description : ""
            toolTipShortcut: Boolean(item) ? item.shortcuts : ""

            navigation.name: Boolean(item) ? item.id : ""
            navigation.panel: root.navigationPanel
            navigation.order: root.navigationOrderStart + model.index

            onClicked: {
                if (btn.hasMenu) {
                    menuLoader.toggleOpened(item.subitems)
                    return
                }

                Qt.callLater(playbackModel.handleMenuItem, item.id)
            }

            StyledMenuLoader {
                id: menuLoader

                onHandleMenuItem: function(itemId) {
                    playbackModel.handleMenuItem(itemId)
                }
            }
        }
    }

    Item {
        width: 8
        height: 1
    }

    StyledTextLabel {
        anchors.verticalCenter: parent.verticalCenter

        //: The playback position: measure number, then beat number
        text: playbackModel.measureNumber + "." + playbackModel.beatNumber
        font: AtStyle.font(12)
        color: AtStyle.text2
    }

    Item {
        width: 8
        height: 1
    }

    Item {
        anchors.verticalCenter: parent.verticalCenter

        width: tempoNote.width + 3 + tempoValue.width
        height: tempoValue.height

        //! NOTE: the note stands on the baseline of the number
        StyledTextLabel {
            id: tempoNote

            anchors.baseline: tempoValue.baseline

            text: Boolean(playbackModel.tempo) ? playbackModel.tempo.noteSymbol : ""
            font.family: ui.theme.musicalFont.family
            font.pixelSize: 16
            color: AtStyle.text2
        }

        StyledTextLabel {
            id: tempoValue

            x: tempoNote.width + 3

            text: "= " + (Boolean(playbackModel.tempo) ? Math.round(playbackModel.tempo.value * playbackModel.tempoMultiplier) : "")
            font: AtStyle.font(12)
            color: AtStyle.text2
        }
    }

    Item {
        width: 6
        height: 1
    }
}
