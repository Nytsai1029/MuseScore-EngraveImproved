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

    title: qsTrc("fontdesign", "Check font (SMuFL)")

    contentWidth: 620
    contentHeight: 460
    margins: 16

    FontLintModel {
        id: lintModel
    }

    Component.onCompleted: {
        lintModel.run()
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 12

        RowLayout {
            Layout.fillWidth: true

            spacing: 8

            StyledTextLabel {
                text: qsTrc("fontdesign", "Check font (SMuFL)")
                font: ui.theme.bodyBoldFont
            }

            Item {
                Layout.fillWidth: true
            }

            StyledTextLabel {
                text: lintModel.summary
                opacity: 0.75
            }
        }

        StyledTextLabel {
            Layout.fillWidth: true

            visible: lintModel.count > 0

            text: qsTrc("fontdesign", "Click an issue to jump to the glyph.")
            opacity: 0.6
            horizontalAlignment: Text.AlignLeft
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: ui.theme.backgroundSecondaryColor
            border.color: ui.theme.strokeColor
            border.width: 1
            radius: 4

            StyledTextLabel {
                anchors.centerIn: parent

                visible: lintModel.count === 0

                text: qsTrc("fontdesign", "No issues found")
                opacity: 0.6
            }

            StyledListView {
                anchors.fill: parent
                anchors.margins: 4

                visible: lintModel.count > 0

                model: lintModel
                clip: true

                delegate: ListItemBlank {
                    width: ListView.view.width
                    height: Math.max(30, rowLabel.implicitHeight + 10)

                    onClicked: {
                        lintModel.goToGlyph(index)
                        if (model.codepoint > 0) {
                            root.hide()
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8

                        spacing: 8

                        Rectangle {
                            Layout.preferredWidth: 8
                            Layout.preferredHeight: 8

                            radius: 4
                            color: model.severity === "error" ? "#e05353"
                                 : model.severity === "warning" ? "#e0a030"
                                 : ui.theme.fontPrimaryColor
                            opacity: model.severity === "info" ? 0.4 : 1.0
                        }

                        StyledTextLabel {
                            id: rowLabel

                            Layout.fillWidth: true

                            text: model.message
                            horizontalAlignment: Text.AlignLeft
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            elide: Text.ElideRight
                        }
                    }
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
                text: qsTrc("fontdesign", "Re-run checks")

                onClicked: {
                    lintModel.run()
                }
            }

            FlatButton {
                text: qsTrc("global", "Close")
                accentButton: true

                onClicked: {
                    root.hide()
                }
            }
        }
    }
}
