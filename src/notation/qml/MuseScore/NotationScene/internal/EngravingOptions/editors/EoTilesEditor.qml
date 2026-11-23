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

import ".."

//! NOTE: the control of a choice option with "tiles" presentation: the choices as pictures
//! of what each does to the music of the option
Flow {
    id: root

    required property var option
    required property var optionsModel

    property var session: null

    property NavigationPanel navigationPanel: null
    property int navigationRow: 0
    property int navigationColumn: 1
    property bool compact: false

    //! NOTE: the music of the option and the style values its picture sets
    property var musicInfo: null
    property var musicStyle: ({})

    readonly property var styleItem: Boolean(option.sid) ? optionsModel.item(option.sid) : null

    spacing: 10

    Repeater {
        model: root.option.choices

        delegate: Item {
            id: tile

            readonly property bool isCurrent: modelData.value === root.styleItem.value

            //! NOTE: the picture shows the music with this choice made, whatever the option is now
            readonly property var tileOverrides: {
                var result = {}
                for (var styleKey in root.musicStyle) {
                    result[styleKey] = root.musicStyle[styleKey]
                }

                var more = modelData.overrides
                for (var key in more) {
                    result[key] = more[key]
                }

                result[root.option.sid] = modelData.value
                return result
            }

            width: root.option.tileWidth
            height: paper.height + 6 + caption.implicitHeight

            NavigationControl {
                id: tileNav

                name: "Value_" + root.option.sid + "_" + model.index
                panel: root.navigationPanel
                row: root.navigationRow
                column: root.navigationColumn + 9 + model.index
                enabled: root.enabled && root.visible

                accessible.role: MUAccessible.RadioButton
                accessible.name: modelData.title
                accessible.checked: tile.isCurrent

                onTriggered: {
                    root.styleItem.value = modelData.value
                }
            }

            EoPreviewCard {
                id: paper

                width: parent.width
                height: root.option.tileHeight

                padding: 8

                music: Boolean(root.musicInfo) ? root.musicInfo.music : ""
                overrides: tile.tileOverrides
                systemWidth: Boolean(root.musicInfo) ? root.musicInfo.systemWidth : 0

                //! NOTE: the tile fits its picture to the room inside the paper
                zoom: {
                    const size = paper.naturalSize
                    if (size.width <= 0.0 || size.height <= 0.0) {
                        return 1.0
                    }
                    return Math.min((paper.width - 2 * paper.padding) / size.width,
                                    (paper.height - 2 * paper.padding) / size.height)
                }

                border.width: tile.isCurrent ? 2 : 1
                border.color: tile.isCurrent ? AtStyle.accent
                                             : (tileMouseArea.containsMouse ? AtStyle.text3 : AtStyle.stroke)

                NavigationFocusBorder {
                    navigationCtrl: tileNav
                }
            }

            StyledTextLabel {
                id: caption

                anchors.top: paper.bottom
                anchors.topMargin: 6
                width: parent.width

                wrapMode: Text.WordWrap
                maximumLineCount: 2

                text: modelData.title
                font: AtStyle.font(12, tile.isCurrent ? Font.DemiBold : Font.Normal)
                color: tile.isCurrent ? AtStyle.text : AtStyle.text2
            }

            MouseArea {
                id: tileMouseArea

                anchors.fill: parent
                hoverEnabled: true

                onClicked: {
                    tileNav.requestActiveByInteraction()
                    root.styleItem.value = modelData.value
                }
            }
        }
    }
}
