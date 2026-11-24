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

import "editors"
import "EoFilter.js" as EoFilter

//! NOTE: every value of the style by its name (the tag which it has in a style file), with a control which is chosen
//! by the kind of the value. The pages of the dialog describe the options which have a meaning for an engraver;
//! this list is for what they leave out, so that nothing of the style is out of reach
Item {
    id: root

    //! NOTE: the EngravingOptionsModel
    property var optionsModel: null

    property NavigationPanel navigationPanel: null

    property bool modifiedOnly: false

    readonly property var allNames: Boolean(optionsModel) ? optionsModel.names() : []

    readonly property var shownNames: {
        var words = EoFilter.words(filterField.searchText)

        if (words.length === 0 && !root.modifiedOnly) {
            return root.allNames
        }

        if (root.modifiedOnly) {
            root.optionsModel.revision
        }

        return root.allNames.filter(function(name) {
            if (!EoFilter.hasAllWords(name.toLowerCase(), words)) {
                return false
            }

            return !root.modifiedOnly || root.optionsModel.modifiedCount([name]) > 0
        })
    }

    readonly property int sideMargin: 28

    Item {
        id: filterRow

        anchors.left: parent.left
        anchors.leftMargin: root.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: root.sideMargin
        anchors.top: parent.top
        anchors.topMargin: 14

        height: 30

        SearchField {
            id: filterField

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            width: 280

            hint: qsTrc("notation/engravingoptions", "Filter by name")

            navigation.panel: root.navigationPanel
            navigation.row: 1
            navigation.column: 1
            navigation.name: "FilterAllValues"
        }

        AtChip {
            anchors.left: filterField.right
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter

            text: qsTrc("notation/engravingoptions", "Changed only")
            checked: root.modifiedOnly

            navigation.panel: root.navigationPanel
            navigation.row: 1
            navigation.column: 2
            navigation.name: "ChangedOnlyAllValues"

            onClicked: {
                root.modifiedOnly = !root.modifiedOnly
            }
        }

        StyledTextLabel {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            //: %1 is the number of style values which are listed, %2 the number of all of them
            text: qsTrc("notation/engravingoptions", "%1 of %2").arg(root.shownNames.length).arg(root.allNames.length)
            font: AtStyle.font(12)
            color: AtStyle.text3
        }
    }

    StyledTextLabel {
        id: explanation

        anchors.left: filterRow.left
        anchors.right: filterRow.right
        anchors.top: filterRow.bottom
        anchors.topMargin: 10

        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap

        text: qsTrc("notation/engravingoptions",
                    "Every value of the style, by the name it has in a style file. "
                    + "Distances are in staff spaces unless the name says otherwise.")
        font: AtStyle.font(12)
        color: AtStyle.text3
    }

    Rectangle {
        id: topLine

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: explanation.bottom
        anchors.topMargin: 12

        height: 1
        color: AtStyle.stroke
    }

    StyledListView {
        id: listView

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: topLine.bottom
        anchors.bottom: parent.bottom

        topMargin: 6
        bottomMargin: 16

        model: root.shownNames

        delegate: Item {
            id: valueLine

            readonly property string name: modelData
            readonly property var styleItem: root.optionsModel.item(name)
            readonly property string kind: root.optionsModel.valueKind(name)
            readonly property int navigationRow: 10 + index

            width: ListView.view.width
            height: 34

            Rectangle {
                anchors.fill: parent
                anchors.leftMargin: root.sideMargin - 8
                anchors.rightMargin: root.sideMargin - 8

                radius: 7
                color: lineMouseArea.containsMouse ? AtStyle.cell : "transparent"
            }

            MouseArea {
                id: lineMouseArea

                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.NoButton
            }

            StyledTextLabel {
                anchors.left: parent.left
                anchors.leftMargin: root.sideMargin
                anchors.right: controls.left
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter

                horizontalAlignment: Text.AlignLeft

                text: valueLine.name
                font: AtStyle.font(12.5, valueLine.styleItem.isDefault ? Font.Normal : Font.DemiBold)
                color: valueLine.styleItem.isDefault ? AtStyle.text : AtStyle.ink
            }

            Row {
                id: controls

                anchors.right: parent.right
                anchors.rightMargin: root.sideMargin
                anchors.verticalCenter: parent.verticalCenter

                spacing: 6

                Loader {
                    anchors.verticalCenter: parent.verticalCenter

                    sourceComponent: {
                        switch (valueLine.kind) {
                        case "spatium":
                        case "real":
                        case "int": return numberComp
                        case "bool": return boolComp
                        case "string": return stringComp
                        case "point": return pointComp
                        case "size": return sizeComp
                        case "align": return alignComp
                        case "color": return colorComp
                        }
                        return otherComp
                    }
                }

                Item {
                    anchors.verticalCenter: parent.verticalCenter

                    width: 26
                    height: 26

                    AtToolButton {
                        anchors.fill: parent

                        shape: "reset"
                        visible: !valueLine.styleItem.isDefault

                        toolTipTitle: qsTrc("notation/engravingoptions", "Reset to default")

                        navigation.panel: root.navigationPanel
                        navigation.row: valueLine.navigationRow
                        navigation.column: 100
                        navigation.name: "Reset_" + valueLine.name
                        navigation.accessible.name: toolTipTitle + ": " + valueLine.name

                        onClicked: {
                            root.optionsModel.resetValues([valueLine.name])
                        }
                    }
                }
            }

            Component {
                id: numberComp

                IncrementalPropertyControl {
                    width: 120

                    currentValue: valueLine.styleItem.value

                    minValue: -9999
                    maxValue: 9999
                    step: valueLine.kind === "int" ? 1 : 0.05
                    decimals: valueLine.kind === "int" ? 0 : 3

                    measureUnitsSymbol: valueLine.kind === "spatium" ? qsTrc("global", "sp") : ""

                    navigation.panel: root.navigationPanel
                    navigation.row: valueLine.navigationRow
                    navigation.column: 1
                    navigation.name: "Value_" + valueLine.name
                    navigation.accessible.name: valueLine.name

                    onValueEdited: function(newValue) {
                        valueLine.styleItem.value = newValue
                    }
                }
            }

            Component {
                id: boolComp

                AtSwitch {
                    checked: Boolean(valueLine.styleItem.value)

                    navigation.panel: root.navigationPanel
                    navigation.row: valueLine.navigationRow
                    navigation.column: 1
                    navigation.name: "Value_" + valueLine.name
                    navigation.accessible.name: valueLine.name

                    onToggled: {
                        valueLine.styleItem.value = !valueLine.styleItem.value
                    }
                }
            }

            Component {
                id: stringComp

                EoTextField {
                    width: 280

                    currentText: valueLine.styleItem.value

                    navigation.panel: root.navigationPanel
                    navigation.row: valueLine.navigationRow
                    navigation.column: 1
                    navigation.name: "Value_" + valueLine.name
                    navigation.accessible.name: valueLine.name

                    onTextEditingFinished: function(newTextValue) {
                        valueLine.styleItem.value = newTextValue
                    }
                }
            }

            Component {
                id: pointComp

                Row {
                    spacing: 6

                    Repeater {
                        model: ["x", "y"]

                        delegate: IncrementalPropertyControl {
                            readonly property string axis: modelData

                            width: 104

                            currentValue: axis === "x" ? valueLine.styleItem.value.x : valueLine.styleItem.value.y

                            minValue: -9999
                            maxValue: 9999
                            step: 0.05
                            decimals: 3

                            navigation.panel: root.navigationPanel
                            navigation.row: valueLine.navigationRow
                            navigation.column: 1 + index
                            navigation.name: "Value_" + valueLine.name + "_" + axis
                            navigation.accessible.name: valueLine.name + " " + axis.toUpperCase()

                            onValueEdited: function(newValue) {
                                var value = valueLine.styleItem.value
                                valueLine.styleItem.value = axis === "x" ? Qt.point(newValue, value.y)
                                                                         : Qt.point(value.x, newValue)
                            }
                        }
                    }
                }
            }

            Component {
                id: sizeComp

                Row {
                    spacing: 6

                    Repeater {
                        model: ["width", "height"]

                        delegate: IncrementalPropertyControl {
                            readonly property string dimension: modelData

                            width: 104

                            prefixIcon: dimension === "width" ? IconCode.HORIZONTAL : IconCode.VERTICAL
                            currentValue: dimension === "width" ? valueLine.styleItem.value.width
                                                                : valueLine.styleItem.value.height

                            minValue: 0
                            maxValue: 9999
                            step: 0.05
                            decimals: 3

                            navigation.panel: root.navigationPanel
                            navigation.row: valueLine.navigationRow
                            navigation.column: 1 + index
                            navigation.name: "Value_" + valueLine.name + "_" + dimension
                            navigation.accessible.name: valueLine.name + " " + dimension

                            onValueEdited: function(newValue) {
                                var value = valueLine.styleItem.value
                                valueLine.styleItem.value = dimension === "width" ? Qt.size(newValue, value.height)
                                                                                  : Qt.size(value.width, newValue)
                            }
                        }
                    }
                }
            }

            Component {
                id: alignComp

                //! NOTE: an alignment is the text the score file has for it, horizontal then vertical
                //! ("left,baseline"); each of the two lists changes its part of it
                Row {
                    spacing: 6

                    Repeater {
                        model: [
                            [
                                { title: qsTrc("notation/engravingoptions", "Left"), value: "left" },
                                { title: qsTrc("notation/engravingoptions", "Center"), value: "center" },
                                { title: qsTrc("notation/engravingoptions", "Right"), value: "right" }
                            ],
                            [
                                { title: qsTrc("notation/engravingoptions", "Top"), value: "top" },
                                { title: qsTrc("notation/engravingoptions", "Middle"), value: "center" },
                                { title: qsTrc("notation/engravingoptions", "Bottom"), value: "bottom" },
                                { title: qsTrc("notation/engravingoptions", "Baseline"), value: "baseline" }
                            ]
                        ]

                        delegate: StyledDropdown {
                            readonly property int part: index

                            width: 104

                            model: modelData
                            textRole: "title"
                            valueRole: "value"

                            currentIndex: indexOfValue(String(valueLine.styleItem.value).split(",")[part])

                            navigation.panel: root.navigationPanel
                            navigation.row: valueLine.navigationRow
                            navigation.column: 1 + part
                            navigation.name: "Value_" + valueLine.name + "_" + part
                            navigation.accessible.name: valueLine.name

                            onActivated: function(index, value) {
                                var parts = String(valueLine.styleItem.value).split(",")
                                parts[part] = value
                                valueLine.styleItem.value = parts.join(",")
                            }
                        }
                    }
                }
            }

            Component {
                id: colorComp

                ColorPicker {
                    width: 120

                    color: valueLine.styleItem.value
                    allowAlpha: true

                    navigation.panel: root.navigationPanel
                    navigation.row: valueLine.navigationRow
                    navigation.column: 1
                    navigation.name: "Value_" + valueLine.name
                    navigation.accessible.name: valueLine.name

                    onNewColorSelected: function(newColor) {
                        valueLine.styleItem.value = newColor
                    }
                }
            }

            Component {
                id: otherComp

                //! NOTE: a value which is not a number, a text, a point, a size, an alignment or a color is edited on
                //! its page
                StyledTextLabel {
                    text: qsTrc("notation/engravingoptions", "Edited on its page")
                    font: AtStyle.font(12)
                    color: AtStyle.text3
                }
            }
        }
    }

    StyledTextLabel {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: topLine.bottom
        anchors.topMargin: 56

        visible: root.shownNames.length === 0

        text: qsTrc("notation/engravingoptions", "No style value matches")
        font: AtStyle.font(13)
        color: AtStyle.text3
    }
}
