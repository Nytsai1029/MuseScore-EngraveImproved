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

    ProjectsPageModel {
        id: projectsModel
    }

    InstalledFontsModel {
        id: installedModel
    }

    property var recentList: []

    function refreshRecent() {
        root.recentList = projectsModel.recentFonts()
    }

    Component.onCompleted: {
        installedModel.load()
        refreshRecent()
    }

    onVisibleChanged: {
        if (visible) {
            refreshRecent()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 36

        spacing: 20

        // 头部：图标 + 标题 + 操作
        Column {
            Layout.fillWidth: true

            spacing: 16

            StyledIconLabel {
                anchors.horizontalCenter: parent.horizontalCenter

                iconCode: IconCode.BRUSH
                font.pixelSize: 48
            }

            StyledTextLabel {
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.min(parent.width, 520)

                text: qsTrc("fontdesign", "Font design")
                font: ui.theme.headerBoldFont
                horizontalAlignment: Text.AlignHCenter
            }

            StyledTextLabel {
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.min(parent.width, 520)

                text: qsTrc("fontdesign", "Design SMuFL music fonts: open an existing font, edit glyphs and metadata, and try the result directly in your scores.")
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter

                spacing: 12

                FlatButton {
                    text: qsTrc("fontdesign", "New font…")
                    accentButton: true

                    onClicked: {
                        projectsModel.newFont()
                    }
                }

                FlatButton {
                    text: qsTrc("fontdesign", "Open font…")

                    onClicked: {
                        projectsModel.openFont()
                    }
                }

                FlatButton {
                    text: qsTrc("fontdesign", "Open font design workspace")

                    onClicked: {
                        projectsModel.openFontDesignPage()
                    }
                }
            }
        }

        // 最近打开的字体
        StyledTextLabel {
            visible: root.recentList.length > 0

            text: qsTrc("fontdesign", "Recent fonts")
            font: ui.theme.bodyBoldFont
        }

        Flow {
            Layout.fillWidth: true

            visible: root.recentList.length > 0

            spacing: 8

            Repeater {
                model: root.recentList

                delegate: FlatButton {
                    text: modelData.name
                    toolTipTitle: modelData.path

                    onClicked: {
                        projectsModel.openFontFile(modelData.path)
                    }
                }
            }
        }

        SeparatorLine {
            Layout.fillWidth: true
        }

        // 已安装字体管理（MuseScore 私有音乐字体目录）
        RowLayout {
            Layout.fillWidth: true

            spacing: 8

            StyledTextLabel {
                text: qsTrc("fontdesign", "Installed music fonts")
                font: ui.theme.bodyBoldFont
            }

            StyledTextLabel {
                text: installedModel.count > 0 ? "(" + installedModel.count + ")" : ""
                opacity: 0.6
            }

            Item {
                Layout.fillWidth: true
            }

            FlatButton {
                text: qsTrc("fontdesign", "Refresh")
                transparent: true

                onClicked: {
                    installedModel.load()
                }
            }
        }

        StyledTextLabel {
            Layout.fillWidth: true

            visible: installedModel.count === 0

            text: qsTrc("fontdesign", "No fonts installed yet. Use “Install” in the font design editor to make a font available to MuseScore.")
            horizontalAlignment: Text.AlignLeft
            opacity: 0.7
            wrapMode: Text.WordWrap
        }

        StyledListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            visible: installedModel.count > 0

            model: installedModel
            clip: true

            delegate: Item {
                width: ListView.view.width
                height: 48

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 4
                    anchors.rightMargin: 4

                    spacing: 8

                    StyledTextLabel {
                        text: model.name
                        font: ui.theme.bodyBoldFont
                        horizontalAlignment: Text.AlignLeft
                    }

                    StyledTextLabel {
                        visible: !model.hasMetadata

                        text: qsTrc("fontdesign", "No metadata")
                        opacity: 0.55
                        horizontalAlignment: Text.AlignLeft
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    FlatButton {
                        text: qsTrc("fontdesign", "Open")
                        transparent: true
                        toolTipTitle: qsTrc("fontdesign", "Open in the font design editor")

                        onClicked: {
                            projectsModel.openFontFile(model.fontFile)
                        }
                    }

                    FlatButton {
                        text: qsTrc("fontdesign", "New window")
                        transparent: true
                        toolTipTitle: qsTrc("fontdesign", "Open in a new window")

                        onClicked: {
                            installedModel.openInNewWindow(index)
                        }
                    }

                    FlatButton {
                        text: qsTrc("fontdesign", "Reveal")
                        transparent: true
                        toolTipTitle: qsTrc("fontdesign", "Show the font file in the file manager")

                        onClicked: {
                            installedModel.revealInFileBrowser(index)
                        }
                    }

                    FlatButton {
                        text: qsTrc("fontdesign", "Uninstall")
                        transparent: true
                        toolTipTitle: qsTrc("fontdesign", "Move the installed font folder to the trash")

                        onClicked: {
                            installedModel.uninstall(index)
                        }
                    }
                }

                SeparatorLine {
                    anchors.bottom: parent.bottom
                }
            }
        }
    }
}
