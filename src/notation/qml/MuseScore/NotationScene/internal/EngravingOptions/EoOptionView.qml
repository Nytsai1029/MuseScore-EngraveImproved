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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0
import MuseScore.Atelier 1.0

//! NOTE: an option of a page as the dialog shows it: the title on the left, the control which the kind
//! of the option asks for on the right (or under the title, if it needs the width), and a button
//! which sets the option back to its default while it differs from it
Item {
    id: root

    //! NOTE: an EoOption
    required property var option

    //! NOTE: the EngravingOptionsModel
    required property var optionsModel

    property NavigationPanel navigationPanel: null
    property int navigationRow: 0

    readonly property var styleItem: Boolean(option.sid) ? optionsModel.item(option.sid) : null

    readonly property var sids: {
        var result = Boolean(option.sid) ? [option.sid] : []
        return result.concat(option.extraSids)
    }

    readonly property bool isModified: {
        //! NOTE: the count is asked again when any value changes
        optionsModel.revision
        return sids.length > 0 && optionsModel.modifiedCount(sids) > 0
    }

    //! NOTE: the kinds of which the control takes the width of the page, under the title
    readonly property bool isWide: {
        switch (option.kind) {
        case "preview":
        case "note":
            return true
        case "choice":
            return option.presentation === "tiles" || option.presentation === "list"
        case "text":
            return option.multiline
        case "custom":
            return option.fullWidth
        }
        return false
    }

    readonly property bool hasTitleRow: option.kind !== "preview" && option.kind !== "note"
                                        && (Boolean(option.title) || Boolean(option.hint) || !isWide)

    //! NOTE: a title which is taken from a form ends with a colon, which a line of this list does not need
    readonly property string plainTitle: String(option.title).replace(/\s*[:：]\s*$/, "")

    implicitHeight: content.implicitHeight
    height: implicitHeight

    enabled: option.enabled
    opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled

    function resetToDefault() {
        optionsModel.resetValues(sids)
    }

    Column {
        id: content

        width: parent.width
        spacing: 8

        // ----- the title, and the control if it has room beside it -----

        Item {
            id: titleRow

            width: parent.width
            height: Math.max(30, titleColumn.implicitHeight + 8, trailing.height)

            visible: root.hasTitleRow

            Column {
                id: titleColumn

                anchors.left: parent.left
                anchors.right: trailing.left
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter

                spacing: 2

                StyledTextLabel {
                    width: parent.width
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.WordWrap

                    text: root.plainTitle
                    font: AtStyle.font(13)
                }

                StyledTextLabel {
                    width: parent.width
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.WordWrap

                    visible: Boolean(root.option.hint)
                    text: root.option.hint
                    font: AtStyle.font(11.5)
                    color: AtStyle.text3
                }
            }

            Row {
                id: trailing

                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter

                height: Math.max(narrowLoader.height, resetButton.height)
                spacing: 6

                Loader {
                    id: narrowLoader

                    anchors.verticalCenter: parent.verticalCenter

                    active: !root.isWide
                    sourceComponent: {
                        switch (root.option.kind) {
                        case "number": return numberComp
                        case "offset": return offsetComp
                        case "toggle": return toggleComp
                        case "text": return textComp
                        case "color": return colorComp
                        case "font": return fontComp
                        case "custom": return customComp
                        case "choice": return root.option.presentation === "dropdown" ? dropdownComp : segmentedComp
                        }
                        return null
                    }
                }

                //! NOTE: the room of the button is kept, so that the controls of the options are in a line
                Item {
                    anchors.verticalCenter: parent.verticalCenter

                    width: resetButton.width
                    height: resetButton.height

                    AtToolButton {
                        id: resetButton

                        width: 26
                        height: 26

                        shape: "reset"
                        visible: root.isModified

                        toolTipTitle: qsTrc("notation/engravingoptions", "Reset to default")

                        navigation.panel: root.navigationPanel
                        navigation.row: root.navigationRow
                        navigation.column: 100
                        navigation.name: "Reset_" + root.option.sid
                        navigation.accessible.name: toolTipTitle + ": " + root.plainTitle

                        onClicked: {
                            root.resetToDefault()
                        }
                    }
                }
            }
        }

        // ----- the control which needs the width -----

        Loader {
            id: wideLoader

            width: parent.width

            active: root.isWide
            visible: active

            sourceComponent: {
                switch (root.option.kind) {
                case "preview": return previewComp
                case "note": return noteComp
                case "text": return textAreaComp
                case "custom": return customComp
                case "choice": return root.option.presentation === "tiles" ? tilesComp : listComp
                }
                return null
            }
        }
    }

    // ----- the controls -----

    Component {
        id: numberComp

        IncrementalPropertyControl {
            width: 112

            readonly property var plainValue: {
                var value = root.styleItem.value
                if (root.option.axis === "x") {
                    return value.x
                }
                if (root.option.axis === "y") {
                    return value.y
                }
                return value
            }

            currentValue: root.option.percent ? Number((plainValue * 100).toFixed(6)) : plainValue

            minValue: root.option.min
            maxValue: root.option.max
            step: root.option.step
            decimals: root.option.decimals

            measureUnitsSymbol: {
                if (root.option.percent) {
                    return "%"
                }
                if (root.option.unit !== "auto") {
                    return root.option.unit
                }
                return root.optionsModel.isInStaffSpaces(root.option.sid) ? qsTrc("global", "sp") : ""
            }

            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow
            navigation.column: 1
            navigation.name: "Value_" + root.option.sid
            navigation.accessible.name: root.plainTitle

            onValueEdited: function(newValue) {
                var number = root.option.percent ? newValue / 100 : newValue

                if (root.option.axis === "x") {
                    root.styleItem.value = Qt.point(number, root.styleItem.value.y)
                } else if (root.option.axis === "y") {
                    root.styleItem.value = Qt.point(root.styleItem.value.x, number)
                } else {
                    root.styleItem.value = number
                }
            }
        }
    }

    Component {
        id: offsetComp

        Row {
            spacing: 10

            Repeater {
                model: ["x", "y"]

                delegate: Row {
                    readonly property string axis: modelData

                    spacing: 5

                    StyledTextLabel {
                        anchors.verticalCenter: parent.verticalCenter

                        text: axis.toUpperCase()
                        font: AtStyle.font(11.5, Font.Medium)
                        color: AtStyle.text3
                    }

                    IncrementalPropertyControl {
                        width: 96

                        currentValue: axis === "x" ? root.styleItem.value.x : root.styleItem.value.y

                        minValue: root.option.min
                        maxValue: root.option.max
                        step: root.option.step
                        decimals: root.option.decimals

                        measureUnitsSymbol: root.option.unit === "auto" ? qsTrc("global", "sp") : root.option.unit

                        navigation.panel: root.navigationPanel
                        navigation.row: root.navigationRow
                        navigation.column: 10 + index
                        navigation.name: "Value_" + root.option.sid + "_" + axis
                        navigation.accessible.name: root.plainTitle + " " + axis.toUpperCase()

                        onValueEdited: function(newValue) {
                            var value = root.styleItem.value
                            root.styleItem.value = axis === "x" ? Qt.point(newValue, value.y) : Qt.point(value.x, newValue)
                        }
                    }
                }
            }
        }
    }

    Component {
        id: colorComp

        ColorPicker {
            width: 112

            color: root.styleItem.value
            allowAlpha: true

            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow
            navigation.column: 1
            navigation.name: "Value_" + root.option.sid
            navigation.accessible.name: root.plainTitle

            onNewColorSelected: function(newColor) {
                root.styleItem.value = newColor
            }
        }
    }

    Component {
        id: fontComp

        StyledDropdown {
            width: 220

            //! NOTE: the families of the system, and the family of the value if the system does not have it
            model: {
                var families = Qt.fontFamilies()
                var current = String(root.styleItem.value)
                var result = []

                if (Boolean(current) && families.indexOf(current) < 0) {
                    result.push({ text: current, value: current })
                }

                for (var i = 0; i < families.length; ++i) {
                    result.push({ text: families[i], value: families[i] })
                }

                return result
            }

            currentIndex: indexOfValue(String(root.styleItem.value))

            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow
            navigation.column: 1
            navigation.name: "Value_" + root.option.sid
            navigation.accessible.name: root.plainTitle

            onActivated: function(index, value) {
                root.styleItem.value = value
            }
        }
    }

    Component {
        id: toggleComp

        AtSwitch {
            checked: Boolean(root.styleItem.value) !== root.option.inverted

            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow
            navigation.column: 1
            navigation.name: "Value_" + root.option.sid
            navigation.accessible.name: root.plainTitle

            onToggled: {
                root.styleItem.value = !root.styleItem.value
            }
        }
    }

    Component {
        id: segmentedComp

        AtSegmented {
            model: root.option.choices
            currentValue: root.styleItem.value

            segmentWidth: {
                var longest = 0
                for (var i = 0; i < root.option.choices.length; ++i) {
                    longest = Math.max(longest, String(root.option.choices[i].title).length)
                }
                //! NOTE: about the width of the longest title, between the widths of a short word and of a phrase
                return Math.max(64, Math.min(168, 26 + longest * 11))
            }

            navigationPanel: root.navigationPanel
            navigationOrderStart: root.navigationRow * 100

            onActivated: function(value) {
                root.styleItem.value = value
            }
        }
    }

    Component {
        id: dropdownComp

        StyledDropdown {
            width: 220

            model: root.option.choices
            textRole: "title"
            valueRole: "value"

            currentIndex: indexOfValue(root.styleItem.value)

            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow
            navigation.column: 1
            navigation.name: "Value_" + root.option.sid
            navigation.accessible.name: root.plainTitle

            onActivated: function(index, value) {
                root.styleItem.value = value
            }
        }
    }

    Component {
        id: listComp

        Column {
            spacing: 6

            Repeater {
                model: root.option.choices

                delegate: RoundedRadioButton {
                    width: parent.width

                    text: modelData.title
                    checked: modelData.value === root.styleItem.value

                    navigation.panel: root.navigationPanel
                    navigation.row: root.navigationRow
                    navigation.column: 10 + model.index
                    navigation.name: "Value_" + root.option.sid + "_" + model.index

                    onToggled: {
                        root.styleItem.value = modelData.value
                    }
                }
            }
        }
    }

    Component {
        id: tilesComp

        Flow {
            spacing: 10

            Repeater {
                model: root.option.choices

                delegate: Item {
                    id: tile

                    readonly property bool isCurrent: modelData.value === root.styleItem.value

                    //! NOTE: the picture shows the snippet with this choice made, whatever the option is now
                    readonly property var tileOverrides: {
                        var result = {}
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
                        column: 10 + model.index
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

                        snippet: root.option.snippet
                        overrides: tile.tileOverrides
                        systemWidth: root.option.systemWidth
                        zoom: root.option.tileZoom

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
    }

    Component {
        id: textComp

        TextInputField {
            width: 260

            currentText: root.styleItem.value
            hint: root.option.placeholder

            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow
            navigation.column: 1
            navigation.name: "Value_" + root.option.sid
            navigation.accessible.name: root.plainTitle

            onTextEditingFinished: function(newTextValue) {
                root.styleItem.value = newTextValue
            }
        }
    }

    Component {
        id: textAreaComp

        TextInputArea {
            height: 64

            currentText: root.styleItem.value
            hint: root.option.placeholder

            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow
            navigation.column: 1
            navigation.name: "Value_" + root.option.sid
            navigation.accessible.name: root.plainTitle

            onTextEditingFinished: function(newTextValue) {
                root.styleItem.value = newTextValue
            }
        }
    }

    Component {
        id: previewComp

        Item {
            id: previewItem

            property real zoom: root.option.zoom

            //! NOTE: the paper is as wide as the picture, but never wider than the page of the dialog
            implicitHeight: previewPaper.height + (previewFooter.visible ? previewFooter.height + 6 : 0)

            EoPreviewCard {
                id: previewPaper

                width: Math.min(parent.width, implicitWidth)
                height: implicitHeight

                snippet: root.option.snippet
                overrides: root.option.overrides
                systemWidth: root.option.systemWidth
                zoom: previewItem.zoom
            }

            Item {
                id: previewFooter

                anchors.top: previewPaper.bottom
                anchors.topMargin: 6
                width: parent.width
                height: 24

                StyledTextLabel {
                    anchors.left: parent.left
                    anchors.right: zoomRow.left
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter

                    horizontalAlignment: Text.AlignLeft

                    text: root.option.title
                    font: AtStyle.font(11.5)
                    color: AtStyle.text3
                }

                Row {
                    id: zoomRow

                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter

                    spacing: 2

                    AtToolButton {
                        width: 24
                        height: 24

                        shape: "minus"
                        enabled: previewItem.zoom > 0.5

                        toolTipTitle: qsTrc("notation", "Zoom out")

                        navigation.panel: root.navigationPanel
                        navigation.row: root.navigationRow
                        navigation.column: 1
                        navigation.name: "ZoomOut_" + root.option.snippet

                        onClicked: {
                            previewItem.zoom = Math.max(0.5, previewItem.zoom - 0.25)
                        }
                    }

                    StyledTextLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 44

                        text: Math.round(previewItem.zoom * 100) + " %"
                        font: AtStyle.font(11.5)
                        color: AtStyle.text2
                    }

                    AtToolButton {
                        width: 24
                        height: 24

                        shape: "plus"
                        enabled: previewItem.zoom < 3.0

                        toolTipTitle: qsTrc("notation", "Zoom in")

                        navigation.panel: root.navigationPanel
                        navigation.row: root.navigationRow
                        navigation.column: 2
                        navigation.name: "ZoomIn_" + root.option.snippet

                        onClicked: {
                            previewItem.zoom = Math.min(3.0, previewItem.zoom + 0.25)
                        }
                    }
                }
            }
        }
    }

    Component {
        id: noteComp

        StyledTextLabel {
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap

            text: root.option.title
            font: AtStyle.font(12)
            color: AtStyle.text2
        }
    }

    Component {
        id: customComp

        Loader {
            sourceComponent: root.option.component

            onLoaded: {
                if (item.hasOwnProperty("option")) {
                    item.option = root.option
                }

                if (item.hasOwnProperty("optionsModel")) {
                    item.optionsModel = root.optionsModel
                }

                if (item.hasOwnProperty("navigationPanel")) {
                    item.navigationPanel = Qt.binding(function() { return root.navigationPanel })
                }
            }
        }
    }
}
