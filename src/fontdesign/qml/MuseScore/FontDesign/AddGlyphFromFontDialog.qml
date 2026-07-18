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

StyledDialogView {
    id: root

    title: qsTrc("fontdesign", "Add glyph from font")

    contentWidth: 560
    contentHeight: contentColumn.implicitHeight + 2 * margins
    margins: 20

    property string codepointHex: ""
    property string filePath: ""
    property string textCharacter: ""

    AddGlyphSourceModel {
        id: sourceModel
    }

    property var engravingFontsList: sourceModel.engravingFontNames()
    property var textFamiliesList: sourceModel.textFontFamilies()
    property int engravingFontIndex: 0
    property int textFamilyIndex: -1

    Component.onCompleted: {
        root.codepointHex = sourceModel.currentCodepointHex()
    }

    ColumnLayout {
        id: contentColumn

        anchors.fill: parent

        spacing: 14

        StyledTextLabel {
            text: qsTrc("fontdesign", "Add glyph from font")
            font: ui.theme.bodyBoldFont
        }

        StyledTextLabel {
            Layout.fillWidth: true

            text: qsTrc("fontdesign", "The loaded outline is appended to the current glyph, scaled to this font’s units per em.")
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
            opacity: 0.75
        }

        // 来源码位（音乐字体 / 字体文件共用）
        RowLayout {
            Layout.fillWidth: true

            spacing: 8

            StyledTextLabel {
                text: qsTrc("fontdesign", "Source codepoint (hex):")
            }

            TextInputField {
                Layout.preferredWidth: 110

                currentText: root.codepointHex

                onTextChanged: function(newTextValue) {
                    root.codepointHex = newTextValue
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }

        SeparatorLine {}

        // 来源 1：已安装的雕版（音乐）字体
        RowLayout {
            Layout.fillWidth: true

            spacing: 8

            StyledTextLabel {
                text: qsTrc("fontdesign", "Music font:")
            }

            StyledDropdown {
                Layout.fillWidth: true

                model: root.engravingFontsList
                currentIndex: root.engravingFontIndex

                onActivated: function(index, value) {
                    root.engravingFontIndex = index
                }
            }

            FlatButton {
                text: qsTrc("fontdesign", "Load")
                enabled: root.engravingFontsList.length > 0

                onClicked: {
                    errorLabel.text = sourceModel.loadFromEngravingFont(
                                root.engravingFontsList[root.engravingFontIndex], root.codepointHex)
                }
            }
        }

        // 来源 2：任意字体文件
        RowLayout {
            Layout.fillWidth: true

            spacing: 8

            StyledTextLabel {
                text: qsTrc("fontdesign", "Font file:")
            }

            FilePicker {
                Layout.fillWidth: true

                pickerType: FilePicker.PickerType.File
                path: root.filePath
                filter: [qsTrc("fontdesign", "Fonts") + " (*.otf *.ttf)"]
                dialogTitle: qsTrc("fontdesign", "Choose font file")

                onPathEdited: function(newPath) {
                    root.filePath = newPath
                }
            }

            FlatButton {
                text: qsTrc("fontdesign", "Load")
                enabled: root.filePath !== ""

                onClicked: {
                    errorLabel.text = sourceModel.loadFromFile(root.filePath, root.codepointHex)
                }
            }
        }

        // 来源 3：系统文本字体字符
        RowLayout {
            Layout.fillWidth: true

            spacing: 8

            StyledTextLabel {
                text: qsTrc("fontdesign", "Text font:")
            }

            StyledDropdown {
                Layout.fillWidth: true

                model: root.textFamiliesList
                currentIndex: root.textFamilyIndex

                onActivated: function(index, value) {
                    root.textFamilyIndex = index
                }
            }

            TextInputField {
                Layout.preferredWidth: 64

                hint: qsTrc("fontdesign", "Char")
                currentText: root.textCharacter
                maximumLength: 2

                onTextChanged: function(newTextValue) {
                    root.textCharacter = newTextValue
                }
            }

            FlatButton {
                text: qsTrc("fontdesign", "Load")
                enabled: root.textFamilyIndex >= 0 && root.textCharacter !== ""

                onClicked: {
                    errorLabel.text = sourceModel.loadFromTextCharacter(
                                root.textFamiliesList[root.textFamilyIndex], root.textCharacter)
                }
            }
        }

        SeparatorLine {}

        // 预览
        RowLayout {
            Layout.fillWidth: true

            spacing: 12

            Rectangle {
                Layout.preferredWidth: 110
                Layout.preferredHeight: 110

                color: ui.theme.backgroundSecondaryColor
                border.color: ui.theme.strokeColor
                border.width: 1
                radius: 4

                OutlinePreview {
                    anchors.fill: parent

                    sourceModel: sourceModel
                    fillColor: ui.theme.fontPrimaryColor
                }

                StyledTextLabel {
                    anchors.centerIn: parent

                    visible: !sourceModel.hasLoadedGlyph

                    text: qsTrc("fontdesign", "No glyph loaded")
                    opacity: 0.5
                }
            }

            ColumnLayout {
                Layout.fillWidth: true

                spacing: 6

                StyledTextLabel {
                    Layout.fillWidth: true

                    text: sourceModel.loadedInfo
                    horizontalAlignment: Text.AlignLeft
                    elide: Text.ElideMiddle
                }

                StyledTextLabel {
                    id: errorLabel

                    Layout.fillWidth: true

                    visible: text !== ""
                    color: "#e05353"
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.WordWrap
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 12

            Item {
                Layout.fillWidth: true
            }

            FlatButton {
                text: qsTrc("global", "Cancel")

                onClicked: {
                    root.hide()
                }
            }

            FlatButton {
                text: qsTrc("fontdesign", "Add to current glyph")
                accentButton: true
                enabled: sourceModel.hasLoadedGlyph

                onClicked: {
                    var err = sourceModel.addToCurrentGlyph()
                    if (err === "") {
                        root.hide()
                    } else {
                        errorLabel.text = err
                    }
                }
            }
        }
    }
}
