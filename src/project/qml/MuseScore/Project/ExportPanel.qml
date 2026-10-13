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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Project 1.0
import MuseScore.Atelier 1.0

import "internal/Export"

//! NOTE: the export of the score as a panel which stays open (the Atelier skin).
//! It has the settings of the Export dialog, and works with the same model
Item {
    id: root

    property NavigationSection navigationSection: null
    property int navigationOrderStart: 1

    //! NOTE: shown under the title, e.g. the name of the score and the number of its pages
    property string subtitle: ""

    readonly property bool canExport: exportModel.selectionLength > 0

    signal printRequested()

    function reload() {
        exportModel.load()
    }

    function exportScores() {
        if (exportModel.exportScores()) {
            exportModel.updateExportInfo()
        }
    }

    ExportDialogModel {
        id: exportModel
    }

    Component.onCompleted: {
        exportModel.load()
    }

    //! NOTE: the parts of the score may have changed while the panel was not shown
    onVisibleChanged: {
        if (root.visible) {
            exportModel.load()
        } else {
            exportModel.updateExportInfo()
        }
    }

    NavigationPanel {
        id: partsNavPanel
        name: "ExportPanelParts"
        section: root.navigationSection
        order: root.navigationOrderStart + 1
        direction: NavigationPanel.Vertical
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("project/export", "Select parts to export")
    }

    NavigationPanel {
        id: footerNavPanel
        name: "ExportPanelFooter"
        section: root.navigationSection
        order: root.navigationOrderStart + 3
        direction: NavigationPanel.Horizontal
        enabled: root.enabled && root.visible
    }

    AtPaneHeader {
        id: header

        width: parent.width

        title: qsTrc("project/export", "Export")
        subtitle: root.subtitle
        separatorVisible: true
    }

    StyledFlickable {
        id: flickable

        anchors.top: header.bottom
        anchors.bottom: footer.top
        width: parent.width

        clip: true

        contentHeight: content.height + 8

        Column {
            id: content

            x: 16
            width: parent.width - 32

            // ----- the format -----

            Item {
                width: parent.width
                height: formatView.height + 22

                ExportOptionsView {
                    id: formatView

                    y: 12
                    width: parent.width

                    exportModel: exportModel
                    showSettings: false

                    navigation.section: root.navigationSection
                    navigation.order: root.navigationOrderStart
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: AtStyle.stroke
                }
            }

            // ----- the parts -----

            Item {
                width: parent.width
                height: partsColumn.height + 22

                Column {
                    id: partsColumn

                    y: 12
                    width: parent.width

                    Item {
                        width: parent.width
                        height: Math.max(partsTitle.implicitHeight, selectAllButton.height) + 4

                        AtSectionTitle {
                            id: partsTitle

                            anchors.left: parent.left
                            anchors.right: selectAllButton.left
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter

                            text: qsTrc("project/export", "Select parts to export")
                        }

                        AtToolButton {
                            id: selectAllButton

                            readonly property bool allSelected: partsRepeater.count > 0
                                                                && exportModel.selectionLength === partsRepeater.count

                            enabled: partsRepeater.count > 0

                            anchors.right: parent.right
                            anchors.rightMargin: -5
                            anchors.verticalCenter: parent.verticalCenter

                            height: 20
                            radius: 5

                            text: allSelected ? qsTrc("project/export", "Clear selection") : qsTrc("project/export", "Select all")
                            textFont: AtStyle.font(11.5)
                            contentColor: AtStyle.ink

                            navigation.name: "SelectAll"
                            navigation.panel: partsNavPanel
                            navigation.row: 0

                            onClicked: {
                                exportModel.setAllSelected(!allSelected)
                            }
                        }
                    }

                    Repeater {
                        id: partsRepeater

                        model: exportModel

                        delegate: AtSwitchRow {
                            width: partsColumn.width

                            text: model.isMain ? qsTrc("project/export", "Main score") : model.title
                            checked: model.isSelected

                            navigation.panel: partsNavPanel
                            navigation.row: 1 + model.index

                            onToggled: {
                                exportModel.setSelected(model.index, !model.isSelected)
                            }
                        }
                    }
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: AtStyle.stroke
                }
            }

            // ----- the settings of the format -----

            Item {
                width: parent.width
                height: settingsColumn.height + 20

                Column {
                    id: settingsColumn

                    y: 12
                    width: parent.width

                    spacing: 8

                    AtSectionTitle {
                        width: parent.width
                        text: qsTrc("project/export", "Export settings")
                    }

                    ExportOptionsView {
                        width: parent.width

                        exportModel: exportModel
                        showFormat: false

                        navigation.section: root.navigationSection
                        navigation.order: root.navigationOrderStart + 2
                    }
                }
            }
        }
    }

    Item {
        id: footer

        anchors.bottom: parent.bottom
        width: parent.width
        height: 51

        Rectangle {
            width: parent.width
            height: 1
            color: AtStyle.stroke
        }

        Row {
            x: 12
            y: 11
            width: parent.width - 24

            spacing: 8

            AtOutlineButton {
                width: (parent.width - parent.spacing) / 2

                //: The dialog of the printer
                text: qsTrc("project/export", "Print…")

                navigation.name: "Print"
                navigation.panel: footerNavPanel
                navigation.order: 1

                onClicked: {
                    root.printRequested()
                }
            }

            FlatButton {
                width: (parent.width - parent.spacing) / 2
                height: 28
                minWidth: 0

                text: qsTrc("project/export", "Export…")
                accentButton: true
                enabled: root.canExport

                navigation.name: "Export"
                navigation.panel: footerNavPanel
                navigation.order: 2

                onClicked: {
                    root.exportScores()
                }
            }
        }
    }
}
