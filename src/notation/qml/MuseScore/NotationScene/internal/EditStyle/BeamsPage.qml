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

    readonly property var slantOptionsModel: [
        { text: "0", value: 0 },
        { text: "1/4", value: 1 },
        { text: "1/2", value: 2 },
        { text: "3/4", value: 3 },
        { text: "1", value: 4 },
        { text: "1 1/4", value: 5 },
        { text: "1 1/2", value: 6 },
        { text: "1 3/4", value: 7 },
        { text: "2", value: 8 },
        { text: "2 1/4", value: 9 },
        { text: "2 1/2", value: 10 },
        { text: "2 3/4", value: 11 },
        { text: "3", value: 12 }
    ]

    component SlantRuleColumn: ColumnLayout {
        id: slantColumn

        property var rows: []

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop
        spacing: 8

        Repeater {
            model: slantColumn.rows

            delegate: RowLayout {
                Layout.fillWidth: true
                spacing: 8

                StyledTextLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignLeft
                    text: modelData.label
                }

                ComboBoxDropdown {
                    Layout.preferredWidth: 120
                    model: root.slantOptionsModel
                    styleItem: modelData.item
                }

                StyledTextLabel { text: qsTrc("notation", "spaces") }
            }
        }
    }

    BeamsPageModel {
        id: beamsPageModel
    }

    ItemWithTitle {
        title: qsTrc("notation", "Beam distance")

        RadioButtonGroup {
            width: 224
            height: 140
            spacing: 12

            model: [
                { iconCode: IconCode.USE_WIDE_BEAMS_REGULAR, text: qsTrc("notation", "Regular"), value: false },
                { iconCode: IconCode.USE_WIDE_BEAMS_WIDE, text: qsTrc("notation", "Wide"), value: true }
            ]

            delegate: FlatRadioButton {
                width: 106
                height: 70

                checked: modelData.value === beamsPageModel.useWideBeams.value

                Column {
                    anchors.centerIn: parent
                    spacing: 8

                    StyledIconLabel {
                        anchors.horizontalCenter: parent.horizontalCenter
                        iconCode: modelData.iconCode
                        font.pixelSize: 28
                    }

                    StyledTextLabel {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: modelData.text
                    }
                }

                onToggled: {
                    beamsPageModel.useWideBeams.value = modelData.value
                }
            }
        }
    }

    ItemWithTitle {
        title: qsTrc("notation", "Beam thickness")

        IncrementalPropertyControl {
            width: 106

            currentValue: beamsPageModel.beamWidth.value

            minValue: 0
            maxValue: 99
            step: 0.01
            decimals: 2
            measureUnitsSymbol: qsTrc("global", "sp")

            onValueEdited: function(newValue) {
                beamsPageModel.beamWidth.value = newValue
            }
        }
    }

    ItemWithTitle {
        title: qsTrc("notation", "Broken beam minimum length")

        IncrementalPropertyControl {
            width: 106

            currentValue: beamsPageModel.beamMinLen.value

            minValue: 0
            maxValue: 99
            step: 0.05
            decimals: 2
            measureUnitsSymbol: qsTrc("global", "sp")

            onValueEdited: function(newValue) {
                beamsPageModel.beamMinLen.value = newValue
            }
        }
    }

    CheckBox {
        width: parent.width
        text: qsTrc("notation", "Flatten all beams")
        checked: beamsPageModel.beamNoSlope.value
        onClicked: {
            beamsPageModel.beamNoSlope.value = !checked
        }
    }

    StyledGroupBox {
        id: beamStyleGroup
        width: parent.width
        height: Math.max(168, Math.max(beamStyleOptions.contentHeight, beamStylePreview.height) + topPadding + bottomPadding + implicitLabelHeight + spacing)

        title: qsTrc("notation", "Beam style")

        RowLayout {
            id: beamStyleContent
            width: parent.width
            implicitHeight: Math.max(beamStyleOptions.contentHeight, beamStylePreview.height)
            spacing: 12

            RadioButtonGroup {
                id: beamStyleOptions
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop

                spacing: 12
                orientation: ListView.Vertical

                model: [
                    { title: qsTrc("notation", "Draw inner stems through beams"), value: false },
                    { title: qsTrc("notation", "Draw inner stems to nearest beam (“French” style)"), value: true }
                ]

                delegate: RoundedRadioButton {
                    width: ListView.view.width
                    text: modelData.title
                    checked: modelData.value === beamsPageModel.frenchStyleBeams.value
                    onToggled: {
                        beamsPageModel.frenchStyleBeams.value = modelData.value
                    }
                }
            }

            StyledImage {
                id: beamStylePreview
                Layout.alignment: Qt.AlignTop
                forceWidth: 140
                forceHeight: 52
                verticalPadding: 12
                source: beamsPageModel.frenchStyleBeams.value ? "beam_style_french.svg" : "beam_style_regular.svg"
            }
        }
    }

    CheckBox {
        width: parent.width
        text: qsTrc("notation", "Apply default beam slant rules")
        checked: beamsPageModel.useDefaultBeamSlantRules.value
        onClicked: {
            beamsPageModel.useDefaultBeamSlantRules.value = !checked
        }
    }

    StyledGroupBox {
        id: customSlantGroup
        width: parent.width
        height: customSlantContent.implicitHeight + topPadding + bottomPadding + implicitLabelHeight + spacing
        title: qsTrc("notation", "Custom slant rules")
        enabled: !beamsPageModel.useDefaultBeamSlantRules.value
        opacity: enabled ? 1.0 : 0.45

        readonly property var twoNoteRowsLeft: [
            { label: qsTrc("notation", "2nd:"), item: beamsPageModel.beamCustomTwoNoteMaxSlantSecondInterval },
            { label: qsTrc("notation", "3rd:"), item: beamsPageModel.beamCustomTwoNoteMaxSlantThirdInterval },
            { label: qsTrc("notation", "4th:"), item: beamsPageModel.beamCustomTwoNoteMaxSlantFourthInterval },
            { label: qsTrc("notation", "5th:"), item: beamsPageModel.beamCustomTwoNoteMaxSlantFifthInterval },
            { label: qsTrc("notation", "6th:"), item: beamsPageModel.beamCustomTwoNoteMaxSlantSixthInterval }
        ]
        readonly property var twoNoteRowsRight: [
            { label: qsTrc("notation", "7th:"), item: beamsPageModel.beamCustomTwoNoteMaxSlantSeventhInterval },
            { label: qsTrc("notation", "8th:"), item: beamsPageModel.beamCustomTwoNoteMaxSlantOctaveInterval },
            { label: qsTrc("notation", "9th:"), item: beamsPageModel.beamCustomTwoNoteMaxSlantNinthInterval },
            { label: qsTrc("notation", "10th:"), item: beamsPageModel.beamCustomTwoNoteMaxSlantTenthInterval },
            { label: qsTrc("notation", "Greater than 10th:"), item: beamsPageModel.beamCustomTwoNoteMaxSlantGreaterThanTenthInterval }
        ]
        readonly property var multiNoteRowsLeft: [
            { label: qsTrc("notation", "2nd:"), item: beamsPageModel.beamCustomMaxSlantSecondInterval },
            { label: qsTrc("notation", "3rd:"), item: beamsPageModel.beamCustomMaxSlantThirdInterval },
            { label: qsTrc("notation", "4th:"), item: beamsPageModel.beamCustomMaxSlantFourthInterval },
            { label: qsTrc("notation", "5th:"), item: beamsPageModel.beamCustomMaxSlantFifthInterval },
            { label: qsTrc("notation", "6th:"), item: beamsPageModel.beamCustomMaxSlantSixthInterval }
        ]
        readonly property var multiNoteRowsRight: [
            { label: qsTrc("notation", "7th:"), item: beamsPageModel.beamCustomMaxSlantSeventhInterval },
            { label: qsTrc("notation", "8th:"), item: beamsPageModel.beamCustomMaxSlantOctave },
            { label: qsTrc("notation", "9th:"), item: beamsPageModel.beamCustomMaxSlantNinthInterval },
            { label: qsTrc("notation", "10th:"), item: beamsPageModel.beamCustomMaxSlantTenthInterval },
            { label: qsTrc("notation", "Greater than 10th:"), item: beamsPageModel.beamCustomMaxSlantGreaterThanTenthInterval }
        ]

        ColumnLayout {
            id: customSlantContent
            width: parent.width
            spacing: 12

            StyledTextLabel {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.WordWrap
                text: qsTrc("notation", "Slant for beams of only two notes, by interval:")
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 24

                SlantRuleColumn { rows: customSlantGroup.twoNoteRowsLeft }
                SlantRuleColumn { rows: customSlantGroup.twoNoteRowsRight }
            }

            StyledTextLabel {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.WordWrap
                text: qsTrc("notation", "Slant for beams of more than two notes, by interval:")
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 24

                SlantRuleColumn { rows: customSlantGroup.multiNoteRowsLeft }
                SlantRuleColumn { rows: customSlantGroup.multiNoteRowsRight }
            }

            StyledTextLabel {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignLeft
                font.italic: true
                text: qsTrc("notation", "These options determine the ideal slant of the beam in terms of the vertical distance between the tips of the stems of the first and last notes within the beam, based on the interval between the first and last notes within the beam, provided the contour of the other notes within the beam is such that the beam should be slanted.")
            }
        }
    }
}
