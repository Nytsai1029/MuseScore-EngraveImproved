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

import "EoMusic.js" as EoMusic

//! NOTE: an option of a page as the dialog shows it: the title on the left, the control which the kind
//! of the option asks for on the right (or under the title, if it needs the width), and a button
//! which sets the option back to its default while it differs from it
FocusScope {
    id: root

    //! NOTE: an EoOption, the EoSection and the EoPage it is in
    required property var option
    property var section: null
    property var page: null

    //! NOTE: the EoSession: the option under the pointer or with the focus is the one which the preview pane shows
    property var session: null

    readonly property bool isActive: Boolean(session) && session.activeOption === option

    //! NOTE: while the handle of the preview is dragged, the field shows the value under the pointer
    readonly property bool isDragged: Boolean(session) && Boolean(option.sid) && session.dragSid === option.sid
                                      && session.dragValue !== null

    readonly property var musicInfo: EoMusic.musicInfoOf(option, section, page)
    readonly property string music: musicInfo.music
    readonly property real musicSystemWidth: musicInfo.systemWidth
    readonly property var musicStyle: EoMusic.musicStyleOf(option, section, page)

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

    readonly property bool hasTitleRow: option.kind !== "note"
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

    function activate() {
        if (Boolean(session)) {
            session.activate(option, section, page)
        }
    }

    onActiveFocusChanged: {
        if (activeFocus) {
            activate()
        }
    }

    HoverHandler {
        onHoveredChanged: {
            if (hovered) {
                root.activate()
            }
        }
    }

    //! NOTE: the option which the preview pane shows is marked quietly
    Rectangle {
        anchors.fill: parent
        anchors.leftMargin: -8
        anchors.rightMargin: -8
        anchors.topMargin: -2
        anchors.bottomMargin: -2

        radius: 7
        color: AtStyle.cell
        visible: root.isActive
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

            readonly property var shownValue: root.isDragged ? root.session.dragValue : plainValue

            currentValue: root.option.percent ? Number((shownValue * 100).toFixed(6)) : shownValue

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
                            root.styleItem.value = axis === "x" ? Qt.point(newValue, value.y)
                                                   : Qt.point(value.x, newValue)
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

            segmentWidth: EoMusic.segmentWidthFor(root.option.choices)

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

                        music: root.music
                        overrides: tile.tileOverrides
                        systemWidth: root.musicSystemWidth

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
