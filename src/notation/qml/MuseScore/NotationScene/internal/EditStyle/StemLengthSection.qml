/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

StyleDialogPage {
    id: root

    spacing: 12

    readonly property color numberColor: "#E38C22"

    readonly property var positionRows: [
        { number: 1, label: qsTrc("notation/editstyle/notes", "Top line and above:"),
            item: notesPageModel.stemCustomLengthFirstLine },
        { number: 2, label: qsTrc("notation/editstyle/notes", "4th space:"),
            item: notesPageModel.stemCustomLengthFirstSpace },
        { number: 3, label: qsTrc("notation/editstyle/notes", "4th line:"),
            item: notesPageModel.stemCustomLengthSecondLine },
        { number: 4, label: qsTrc("notation/editstyle/notes", "3rd space:"),
            item: notesPageModel.stemCustomLengthSecondSpace },
        { number: 5, label: qsTrc("notation/editstyle/notes", "Middle line:"),
            item: notesPageModel.stemCustomLengthThirdLine },
        { number: 6, label: qsTrc("notation/editstyle/notes", "2nd space:"),
            item: notesPageModel.stemCustomLengthThirdSpace },
        { number: 7, label: qsTrc("notation/editstyle/notes", "2nd line:"),
            item: notesPageModel.stemCustomLengthFourthLine },
        { number: 8, label: qsTrc("notation/editstyle/notes", "1st space:"),
            item: notesPageModel.stemCustomLengthFourthSpace },
        { number: 9, label: qsTrc("notation/editstyle/notes", "Bottom line:"),
            item: notesPageModel.stemCustomLengthFifthLine }
    ]

    NotesPageModel {
        id: notesPageModel
    }

    CheckBox {
        width: parent.width

        text: qsTrc("notation/editstyle/notes", "Apply default stem shortening rules")
        checked: notesPageModel.useDefaultStemShorteningRules.value

        onClicked: {
            notesPageModel.useDefaultStemShorteningRules.value = !checked
        }
    }

    StyledGroupBox {
        id: customLengthGroup

        width: parent.width
        height: customLengthContent.implicitHeight + topPadding + bottomPadding + implicitLabelHeight + spacing

        title: qsTrc("notation/editstyle/notes", "Custom stem length by notehead position")
        enabled: !notesPageModel.useDefaultStemShorteningRules.value
        opacity: enabled ? 1.0 : 0.45

        // The group box does not lay out its content: without the explicit width the column would
        // size to its own implicit width and push the controls out of the page
        ColumnLayout {
            id: customLengthContent

            width: parent.width
            spacing: 8

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: diagram.height

                StyledImage {
                    id: diagram

                    anchors.horizontalCenter: parent.horizontalCenter
                    forceWidth: 320
                    verticalPadding: 4
                    source: "stem_length_positions.svg"
                }
            }

            Repeater {
                model: root.positionRows

                delegate: RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    StyledTextLabel {
                        Layout.preferredWidth: 14
                        horizontalAlignment: Text.AlignRight
                        color: root.numberColor
                        text: modelData.number
                    }

                    StyledTextLabel {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        text: modelData.label
                    }

                    IncrementalPropertyControl {
                        Layout.preferredWidth: 80

                        currentValue: modelData.item.value

                        minValue: 0.5
                        maxValue: 12
                        step: 0.25
                        decimals: 2
                        measureUnitsSymbol: qsTrc("global", "sp")

                        onValueEdited: function(newValue) {
                            modelData.item.value = newValue
                        }
                    }

                    StyleResetButton {
                        styleItem: modelData.item
                    }
                }
            }

            StyledTextLabel {
                Layout.fillWidth: true
                // keep the unwrapped text out of the column's implicit width, and report the
                // height it needs once wrapped
                Layout.preferredWidth: 1
                Layout.preferredHeight: implicitHeight
                Layout.topMargin: 4
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.WordWrap
                font.italic: true
                text: qsTrc("notation/editstyle/notes", "Positions are shown for stems pointing up on a five-line staff; stems pointing down mirror them. On other staves they are counted from the staff line nearest the stem tip. Noteheads beyond the opposite staff line keep the default rules, and stems still lengthen for beams and flags that would otherwise collide.")
            }
        }
    }
}
