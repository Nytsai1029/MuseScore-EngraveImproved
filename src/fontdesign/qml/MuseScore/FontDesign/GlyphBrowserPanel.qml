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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.FontDesign 1.0

Rectangle {
    id: root

    color: ui.theme.backgroundPrimaryColor

    GlyphBrowserModel {
        id: browserModel
    }

    Component.onCompleted: {
        browserModel.load()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12

        spacing: 12

        StyledDropdown {
            Layout.fillWidth: true

            model: browserModel.rangeNames
            currentIndex: browserModel.currentRangeIndex

            onActivated: function(index, value) {
                browserModel.currentRangeIndex = index
            }
        }

        SearchField {
            Layout.fillWidth: true

            hint: qsTrc("fontdesign", "Name or U+XXXX")

            onSearchTextChanged: {
                browserModel.searchText = searchText
            }
        }

        StyledGridView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true

            cellWidth: 56
            cellHeight: 56

            model: browserModel

            delegate: Item {
                width: 56
                height: 56

                Rectangle {
                    id: cell

                    anchors.fill: parent
                    anchors.margins: 2

                    readonly property bool isCurrent: model.codepoint === browserModel.currentCodepoint

                    radius: 4
                    color: cellMouseArea.containsMouse ? ui.theme.buttonColor : "transparent"
                    border.width: cell.isCurrent ? 2 : 0
                    border.color: ui.theme.accentColor

                    GlyphCellView {
                        anchors.fill: parent
                        anchors.margins: 2

                        codepoint: model.codepoint
                        color: ui.theme.fontPrimaryColor
                    }

                    StyledTextLabel {
                        anchors.centerIn: parent

                        visible: !model.hasOutline

                        text: model.hex
                        font.pixelSize: 9
                        opacity: 0.4
                    }

                    MouseArea {
                        id: cellMouseArea

                        anchors.fill: parent
                        hoverEnabled: true

                        onClicked: {
                            browserModel.selectGlyph(model.index)
                        }
                    }
                }
            }
        }

        SeparatorLine {}

        StyledTextLabel {
            Layout.fillWidth: true

            text: browserModel.currentGlyphInfo
            horizontalAlignment: Text.AlignLeft
        }
    }
}
