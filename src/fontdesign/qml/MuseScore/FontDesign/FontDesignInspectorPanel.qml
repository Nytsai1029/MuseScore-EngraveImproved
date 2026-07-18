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

    component PropertyLabel: StyledTextLabel {
        Layout.preferredWidth: 130
        horizontalAlignment: Text.AlignLeft
        elide: Text.ElideMiddle
    }

    component SpValueControl: IncrementalPropertyControl {
        Layout.fillWidth: true
        step: 0.01
        decimals: 3
        minValue: -99
        maxValue: 99
        measureUnitsSymbol: qsTrc("fontdesign", "sp")
    }

    FontInfoModel {
        id: infoModel
    }

    EngravingDefaultsModel {
        id: defaultsModel
    }

    GlyphPropertiesModel {
        id: glyphModel
    }

    AnchorsModel {
        id: anchorsModel
    }

    AlternatesModel {
        id: alternatesModel
    }

    LigaturesModel {
        id: ligaturesModel
    }

    OptionalGlyphsModel {
        id: optionalGlyphsModel
    }

    SetsModel {
        id: setsModel
    }

    Component.onCompleted: {
        infoModel.init()
        defaultsModel.init()
        glyphModel.init()
        anchorsModel.init()
        alternatesModel.init()
        ligaturesModel.init()
        optionalGlyphsModel.init()
        setsModel.init()
    }

    StyledFlickable {
        anchors.fill: parent
        anchors.margins: 12

        contentWidth: width
        contentHeight: contentColumn.height + 12
        clip: true

        Column {
            id: contentColumn

            width: parent.width

            spacing: 16

            // ① 字体信息
            ExpandableBlank {
                title: qsTrc("fontdesign", "Font")
                isExpanded: false

                contentItemComponent: Column {
                    spacing: 8

                    RowLayout {
                        width: parent.width
                        spacing: 8

                        PropertyLabel { text: qsTrc("fontdesign", "Name") }

                        TextInputField {
                            Layout.fillWidth: true
                            currentText: infoModel.fontName

                            onTextEditingFinished: function(newTextValue) {
                                infoModel.fontName = newTextValue
                            }
                        }
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 8

                        PropertyLabel { text: qsTrc("fontdesign", "Version") }

                        IncrementalPropertyControl {
                            Layout.fillWidth: true
                            currentValue: infoModel.fontVersion
                            step: 0.001
                            decimals: 3
                            minValue: 0
                            maxValue: 9999

                            onValueEdited: function(newValue) {
                                infoModel.fontVersion = newValue
                            }
                        }
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 8

                        PropertyLabel { text: qsTrc("fontdesign", "Text font family") }

                        TextInputField {
                            Layout.fillWidth: true
                            currentText: infoModel.textFontFamily

                            onTextEditingFinished: function(newTextValue) {
                                infoModel.textFontFamily = newTextValue
                            }
                        }
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 8

                        PropertyLabel { text: qsTrc("fontdesign", "Design size (dpt)") }

                        IncrementalPropertyControl {
                            Layout.fillWidth: true
                            currentValue: infoModel.designSize
                            step: 1
                            decimals: 0
                            minValue: 0
                            maxValue: 9999

                            onValueEdited: function(newValue) {
                                infoModel.designSize = newValue
                            }
                        }
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 8

                        PropertyLabel { text: qsTrc("fontdesign", "Size range (dpt)") }

                        IncrementalPropertyControl {
                            Layout.fillWidth: true
                            currentValue: infoModel.sizeRangeMin
                            step: 1
                            decimals: 0
                            minValue: 0
                            maxValue: 9999

                            onValueEdited: function(newValue) {
                                infoModel.sizeRangeMin = newValue
                            }
                        }

                        IncrementalPropertyControl {
                            Layout.fillWidth: true
                            currentValue: infoModel.sizeRangeMax
                            step: 1
                            decimals: 0
                            minValue: 0
                            maxValue: 9999

                            onValueEdited: function(newValue) {
                                infoModel.sizeRangeMax = newValue
                            }
                        }
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 8

                        PropertyLabel { text: qsTrc("fontdesign", "Units per em") }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            text: infoModel.upem
                            horizontalAlignment: Text.AlignLeft
                        }
                    }
                }
            }

            // ② 当前字形
            ExpandableBlank {
                title: qsTrc("fontdesign", "Glyph")
                isExpanded: true

                contentItemComponent: Column {
                    spacing: 8

                    RowLayout {
                        width: parent.width
                        spacing: 8

                        PropertyLabel { text: qsTrc("fontdesign", "Name") }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            text: glyphModel.glyphName + "  " + glyphModel.codepointHex
                            horizontalAlignment: Text.AlignLeft
                            elide: Text.ElideRight
                        }
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 8

                        PropertyLabel { text: qsTrc("fontdesign", "Advance") }

                        SpValueControl {
                            currentValue: glyphModel.advanceSp

                            onValueEdited: function(newValue) {
                                glyphModel.advanceSp = newValue
                            }
                        }
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 8

                        PropertyLabel { text: qsTrc("fontdesign", "Bounding box") }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            text: glyphModel.bboxText
                            horizontalAlignment: Text.AlignLeft
                            elide: Text.ElideRight
                        }
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 8
                        visible: glyphModel.classesText.length > 0

                        PropertyLabel { text: qsTrc("fontdesign", "Classes") }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            text: glyphModel.classesText
                            horizontalAlignment: Text.AlignLeft
                            wrapMode: Text.WordWrap
                            maximumLineCount: 3
                            elide: Text.ElideRight
                        }
                    }
                }
            }

            // ③ 锚点
            ExpandableBlank {
                title: qsTrc("fontdesign", "Anchors")
                isExpanded: true

                contentItemComponent: Column {
                    spacing: 8

                    Repeater {
                        model: anchorsModel

                        delegate: Column {
                            width: parent.width
                            spacing: 2

                            RowLayout {
                                width: parent.width
                                spacing: 6

                                PropertyLabel {
                                    Layout.preferredWidth: 100
                                    text: model.name
                                    font: ui.theme.bodyFont
                                }

                                FlatButton {
                                    Layout.preferredWidth: 24
                                    icon: IconCode.QUESTION_MARK
                                    transparent: true
                                    toolTipTitle: model.name
                                    toolTipDescription: model.description
                                }

                                SpValueControl {
                                    currentValue: model.anchorX

                                    onValueEdited: function(newValue) {
                                        anchorsModel.setX(model.index, newValue)
                                    }
                                }

                                SpValueControl {
                                    currentValue: model.anchorY

                                    onValueEdited: function(newValue) {
                                        anchorsModel.setY(model.index, newValue)
                                    }
                                }

                                FlatButton {
                                    icon: IconCode.DELETE_TANK
                                    transparent: true

                                    onClicked: {
                                        anchorsModel.removeAnchor(model.index)
                                    }
                                }
                            }
                        }
                    }

                    StyledTextLabel {
                        width: parent.width
                        visible: anchorsModel.hasGlyph && anchorsModel.rowCount() === 0
                        text: qsTrc("fontdesign", "No anchors")
                        horizontalAlignment: Text.AlignLeft
                        opacity: 0.6
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 8

                        visible: anchorsModel.hasGlyph

                        StyledDropdown {
                            id: addAnchorDropdown

                            Layout.fillWidth: true

                            model: anchorsModel.availableAnchorNames
                            currentIndex: -1

                            displayText: qsTrc("fontdesign", "Add anchor…")

                            onActivated: function(index, value) {
                                anchorsModel.addAnchor(value)
                                addAnchorDropdown.currentIndex = -1
                            }
                        }
                    }
                }
            }

            // ④ 雕版默认值（懒加载 + ListView；根项须有 implicitHeight，否则 Loader 高度为 0）
            ExpandableBlank {
                id: defaultsSection
                title: qsTrc("fontdesign", "Engraving defaults")
                isExpanded: false
                contentItemComponent: defaultsSection.isExpanded ? defaultsContent : null
            }

            Component {
                id: defaultsContent
                Item {
                    width: parent ? parent.width : 0
                    height: defaultsList.height
                    implicitHeight: height

                    ListView {
                        id: defaultsList
                        width: parent.width
                        height: Math.min(320, Math.max(count * 36, 36))
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        model: defaultsModel
                        spacing: 4
                        delegate: RowLayout {
                            width: ListView.view ? ListView.view.width : 0
                            height: 32
                            spacing: 6
                            PropertyLabel {
                                Layout.preferredWidth: 165
                                text: model.key
                                opacity: model.isSet ? 1.0 : 0.5
                            }
                            SpValueControl {
                                currentValue: model.value
                                onValueEdited: function(newValue) { defaultsModel.setValue(model.index, newValue) }
                            }
                        }
                    }
                }
            }

            // ⑤–⑧ 大表：折叠时不实例化；展开用 ListView 回收委托
            ExpandableBlank {
                id: alternatesSection
                title: qsTrc("fontdesign", "Alternates")
                isExpanded: false
                contentItemComponent: alternatesSection.isExpanded ? alternatesContent : null
            }

            Component {
                id: alternatesContent
                Column {
                    width: parent ? parent.width : 0
                    spacing: 8

                    SearchField {
                        width: parent.width
                        onSearchTextChanged: alternatesModel.filterText = searchText
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 6
                        TextInputField {
                            id: altBaseField
                            Layout.fillWidth: true
                            hint: qsTrc("fontdesign", "Base glyph")
                        }
                        TextInputField {
                            id: altNameField
                            Layout.fillWidth: true
                            hint: qsTrc("fontdesign", "Alternate name")
                        }
                        FlatButton {
                            text: qsTrc("fontdesign", "Add")
                            enabled: alternatesModel.hasProject
                            onClicked: alternatesModel.addAlternate(altBaseField.currentText, altNameField.currentText)
                        }
                    }

                    ListView {
                        width: parent.width
                        height: 260
                        implicitHeight: height
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        model: alternatesModel
                        spacing: 8
                        delegate: Column {
                            width: ListView.view ? ListView.view.width : 0
                            spacing: 4
                            RowLayout {
                                width: parent.width
                                spacing: 4
                                TextInputField {
                                    Layout.fillWidth: true
                                    currentText: model.baseName
                                    onTextEditingFinished: function(v) { alternatesModel.setBaseName(model.index, v) }
                                }
                                TextInputField {
                                    Layout.fillWidth: true
                                    currentText: model.altName
                                    onTextEditingFinished: function(v) { alternatesModel.setAltName(model.index, v) }
                                }
                                FlatButton {
                                    icon: IconCode.DELETE_TANK
                                    transparent: true
                                    onClicked: alternatesModel.removeRowAt(model.index)
                                }
                            }
                            RowLayout {
                                width: parent.width
                                spacing: 4
                                TextInputField {
                                    Layout.fillWidth: true
                                    currentText: model.codepoint
                                    onTextEditingFinished: function(v) { alternatesModel.setCodepoint(model.index, v) }
                                }
                                FlatButton {
                                    text: qsTrc("fontdesign", "PUA")
                                    onClicked: alternatesModel.assignNextPua(model.index)
                                }
                            }
                        }
                    }
                }
            }

            ExpandableBlank {
                id: ligaturesSection
                title: qsTrc("fontdesign", "Ligatures")
                isExpanded: false
                contentItemComponent: ligaturesSection.isExpanded ? ligaturesContent : null
            }

            Component {
                id: ligaturesContent
                Column {
                    width: parent ? parent.width : 0
                    spacing: 8

                    SearchField {
                        width: parent.width
                        onSearchTextChanged: ligaturesModel.filterText = searchText
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 6
                        TextInputField {
                            id: ligNameField
                            Layout.fillWidth: true
                            hint: qsTrc("fontdesign", "Ligature name")
                        }
                        FlatButton {
                            text: qsTrc("fontdesign", "Add")
                            enabled: ligaturesModel.hasProject
                            onClicked: ligaturesModel.addLigature(ligNameField.currentText)
                        }
                    }

                    ListView {
                        width: parent.width
                        height: 260
                        implicitHeight: height
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        model: ligaturesModel
                        spacing: 8
                        delegate: Column {
                            width: ListView.view ? ListView.view.width : 0
                            spacing: 4
                            RowLayout {
                                width: parent.width
                                spacing: 4
                                TextInputField {
                                    Layout.fillWidth: true
                                    currentText: model.name
                                    onTextEditingFinished: function(v) { ligaturesModel.setName(model.index, v) }
                                }
                                TextInputField {
                                    Layout.preferredWidth: 90
                                    currentText: model.codepoint
                                    onTextEditingFinished: function(v) { ligaturesModel.setCodepoint(model.index, v) }
                                }
                                FlatButton {
                                    text: qsTrc("fontdesign", "PUA")
                                    onClicked: ligaturesModel.assignNextPua(model.index)
                                }
                                FlatButton {
                                    icon: IconCode.DELETE_TANK
                                    transparent: true
                                    onClicked: ligaturesModel.removeRowAt(model.index)
                                }
                            }
                            TextInputField {
                                width: parent.width
                                currentText: model.components
                                hint: qsTrc("fontdesign", "Components (comma-separated)")
                                onTextEditingFinished: function(v) { ligaturesModel.setComponents(model.index, v) }
                            }
                            TextInputField {
                                width: parent.width
                                currentText: model.description
                                hint: qsTrc("fontdesign", "Description")
                                onTextEditingFinished: function(v) { ligaturesModel.setDescription(model.index, v) }
                            }
                        }
                    }
                }
            }

            ExpandableBlank {
                id: optionalSection
                title: qsTrc("fontdesign", "Optional glyphs")
                isExpanded: false
                contentItemComponent: optionalSection.isExpanded ? optionalContent : null
            }

            Component {
                id: optionalContent
                Column {
                    width: parent ? parent.width : 0
                    spacing: 8

                    SearchField {
                        width: parent.width
                        onSearchTextChanged: optionalGlyphsModel.filterText = searchText
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 6
                        TextInputField {
                            id: optNameField
                            Layout.fillWidth: true
                            hint: qsTrc("fontdesign", "Optional glyph name")
                        }
                        FlatButton {
                            text: qsTrc("fontdesign", "Add")
                            enabled: optionalGlyphsModel.hasProject
                            onClicked: optionalGlyphsModel.addOptional(optNameField.currentText)
                        }
                    }

                    ListView {
                        width: parent.width
                        height: 260
                        implicitHeight: height
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        model: optionalGlyphsModel
                        spacing: 8
                        delegate: Column {
                            width: ListView.view ? ListView.view.width : 0
                            spacing: 4
                            RowLayout {
                                width: parent.width
                                spacing: 4
                                TextInputField {
                                    Layout.fillWidth: true
                                    currentText: model.name
                                    onTextEditingFinished: function(v) { optionalGlyphsModel.setName(model.index, v) }
                                }
                                TextInputField {
                                    Layout.preferredWidth: 90
                                    currentText: model.codepoint
                                    onTextEditingFinished: function(v) { optionalGlyphsModel.setCodepoint(model.index, v) }
                                }
                                FlatButton {
                                    text: qsTrc("fontdesign", "PUA")
                                    onClicked: optionalGlyphsModel.assignNextPua(model.index)
                                }
                                FlatButton {
                                    icon: IconCode.DELETE_TANK
                                    transparent: true
                                    onClicked: optionalGlyphsModel.removeRowAt(model.index)
                                }
                            }
                            TextInputField {
                                width: parent.width
                                currentText: model.classes
                                hint: qsTrc("fontdesign", "Classes (comma-separated)")
                                onTextEditingFinished: function(v) { optionalGlyphsModel.setClasses(model.index, v) }
                            }
                            TextInputField {
                                width: parent.width
                                currentText: model.description
                                hint: qsTrc("fontdesign", "Description")
                                onTextEditingFinished: function(v) { optionalGlyphsModel.setDescription(model.index, v) }
                            }
                        }
                    }
                }
            }

            ExpandableBlank {
                id: setsSection
                title: qsTrc("fontdesign", "Sets")
                isExpanded: false
                contentItemComponent: setsSection.isExpanded ? setsContent : null
            }

            Component {
                id: setsContent
                Column {
                    width: parent ? parent.width : 0
                    spacing: 8

                    SearchField {
                        width: parent.width
                        onSearchTextChanged: setsModel.filterText = searchText
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 6
                        TextInputField {
                            id: setIdField
                            Layout.fillWidth: true
                            hint: qsTrc("fontdesign", "Set id (e.g. ss01)")
                        }
                        FlatButton {
                            text: qsTrc("fontdesign", "Add set")
                            enabled: setsModel.hasProject
                            onClicked: setsModel.addSet(setIdField.currentText)
                        }
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 6
                        TextInputField {
                            id: setGlyphSetField
                            Layout.fillWidth: true
                            hint: qsTrc("fontdesign", "Set id")
                        }
                        TextInputField {
                            id: setGlyphNameField
                            Layout.fillWidth: true
                            hint: qsTrc("fontdesign", "Glyph name")
                        }
                        FlatButton {
                            text: qsTrc("fontdesign", "Add glyph")
                            enabled: setsModel.hasProject
                            onClicked: setsModel.addGlyphToSet(setGlyphSetField.currentText, setGlyphNameField.currentText)
                        }
                    }

                    ListView {
                        width: parent.width
                        height: 260
                        implicitHeight: height
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        model: setsModel
                        spacing: 8
                        delegate: Column {
                            width: ListView.view ? ListView.view.width : 0
                            spacing: 4
                            RowLayout {
                                width: parent.width
                                spacing: 4
                                StyledTextLabel {
                                    Layout.preferredWidth: 50
                                    text: model.setId
                                    horizontalAlignment: Text.AlignLeft
                                }
                                TextInputField {
                                    Layout.fillWidth: true
                                    currentText: model.glyphName
                                    hint: qsTrc("fontdesign", "Glyph")
                                    onTextEditingFinished: function(v) { setsModel.setGlyphName(model.index, v) }
                                }
                                TextInputField {
                                    Layout.preferredWidth: 90
                                    currentText: model.codepoint
                                    onTextEditingFinished: function(v) { setsModel.setCodepoint(model.index, v) }
                                }
                                FlatButton {
                                    text: qsTrc("fontdesign", "PUA")
                                    onClicked: setsModel.assignNextPua(model.index)
                                }
                                FlatButton {
                                    icon: IconCode.DELETE_TANK
                                    transparent: true
                                    onClicked: setsModel.removeRowAt(model.index)
                                }
                            }
                            RowLayout {
                                width: parent.width
                                spacing: 4
                                TextInputField {
                                    Layout.fillWidth: true
                                    currentText: model.alternateFor
                                    hint: qsTrc("fontdesign", "alternateFor")
                                    onTextEditingFinished: function(v) { setsModel.setAlternateFor(model.index, v) }
                                }
                                TextInputField {
                                    Layout.fillWidth: true
                                    currentText: model.setType
                                    hint: qsTrc("fontdesign", "Set type")
                                    onTextEditingFinished: function(v) { setsModel.setSetType(model.index, v) }
                                }
                            }
                            TextInputField {
                                width: parent.width
                                currentText: model.setDescription
                                hint: qsTrc("fontdesign", "Set description")
                                onTextEditingFinished: function(v) { setsModel.setSetDescription(model.index, v) }
                            }
                        }
                    }
                }
            }
        }
    }
}
