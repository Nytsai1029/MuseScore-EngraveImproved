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

    title: qsTrc("fontdesign", "New font")

    contentWidth: 440
    contentHeight: contentColumn.implicitHeight + 2 * margins
    margins: 20

    property string fontName: ""
    property string fontVersion: "1.0"
    property int unitsPerEm: 1000
    property string copyrightText: ""
    property string folderPath: ""

    NewFontModel {
        id: newFontModel
    }

    Component.onCompleted: {
        root.folderPath = newFontModel.defaultFolder()
    }

    ColumnLayout {
        id: contentColumn

        anchors.fill: parent

        spacing: 16

        StyledTextLabel {
            text: qsTrc("fontdesign", "New font")
            font: ui.theme.bodyBoldFont
        }

        StyledTextLabel {
            Layout.fillWidth: true

            text: qsTrc("fontdesign", "The font and its SMuFL metadata will be written to the chosen folder on first save.")
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
        }

        GridLayout {
            Layout.fillWidth: true

            columns: 2
            columnSpacing: 12
            rowSpacing: 12

            StyledTextLabel {
                text: qsTrc("fontdesign", "Name:")
                horizontalAlignment: Text.AlignLeft
            }

            TextInputField {
                Layout.fillWidth: true

                currentText: root.fontName
                hint: qsTrc("fontdesign", "e.g. MyMusicFont")

                onTextChanged: function(newTextValue) {
                    root.fontName = newTextValue
                }
            }

            StyledTextLabel {
                text: qsTrc("fontdesign", "Version:")
                horizontalAlignment: Text.AlignLeft
            }

            TextInputField {
                Layout.preferredWidth: 120

                currentText: root.fontVersion

                onTextChanged: function(newTextValue) {
                    root.fontVersion = newTextValue
                }
            }

            StyledTextLabel {
                text: qsTrc("fontdesign", "Units per em:")
                horizontalAlignment: Text.AlignLeft
            }

            IncrementalPropertyControl {
                Layout.preferredWidth: 120

                currentValue: root.unitsPerEm
                minValue: 256
                maxValue: 16384
                step: 250
                decimals: 0

                onValueEdited: function(newValue) {
                    root.unitsPerEm = newValue
                }
            }

            StyledTextLabel {
                text: qsTrc("fontdesign", "Copyright:")
                horizontalAlignment: Text.AlignLeft
            }

            TextInputField {
                Layout.fillWidth: true

                currentText: root.copyrightText
                hint: qsTrc("fontdesign", "Optional")

                onTextChanged: function(newTextValue) {
                    root.copyrightText = newTextValue
                }
            }

            StyledTextLabel {
                Layout.alignment: Qt.AlignTop

                text: qsTrc("fontdesign", "Save in:")
                horizontalAlignment: Text.AlignLeft
            }

            FilePicker {
                Layout.fillWidth: true

                pickerType: FilePicker.PickerType.Directory
                path: root.folderPath

                dialogTitle: qsTrc("fontdesign", "Choose folder")
                dir: root.folderPath

                onPathEdited: function(newPath) {
                    root.folderPath = newPath
                }
            }
        }

        StyledTextLabel {
            id: errorLabel

            Layout.fillWidth: true

            visible: text !== ""
            color: "#e05353"
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
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
                text: qsTrc("fontdesign", "Create")
                accentButton: true
                enabled: root.fontName.trim() !== "" && root.folderPath !== ""

                onClicked: {
                    var err = newFontModel.create(root.fontName, root.fontVersion, root.unitsPerEm,
                                                  root.copyrightText, root.folderPath)
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
