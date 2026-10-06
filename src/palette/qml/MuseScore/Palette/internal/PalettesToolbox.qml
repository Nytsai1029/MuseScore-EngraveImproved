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
import QtQuick.Controls 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Palette 1.0

//! NOTE: A narrow strip with a button for each palette, which is used to choose the palette to show
Item {
    id: root

    //! NOTE: the top level rows of this model are the palettes
    property var paletteModel: null

    property alias navigation: navPanel

    readonly property int buttonSize: 32

    //! NOTE: the icons are never larger than this size
    readonly property int iconSize: 20

    signal paletteClicked(int row)

    function firstCellIcon(row) {
        if (!root.paletteModel) {
            return undefined
        }

        const paletteIndex = root.paletteModel.index(row, 0)
        if (root.paletteModel.rowCount(paletteIndex) === 0) {
            return undefined
        }

        return root.paletteModel.data(root.paletteModel.index(0, 0, paletteIndex), Qt.DecorationRole)
    }

    NavigationPanel {
        id: navPanel
        name: "PalettesToolbox"
        direction: NavigationPanel.Vertical
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("palette", "Palettes toolbox")
    }

    StyledListView {
        id: view

        anchors.fill: parent
        anchors.topMargin: 6
        anchors.bottomMargin: 6

        spacing: 4
        scrollBarPolicy: ScrollBar.AlwaysOff

        model: root.paletteModel

        delegate: FlatButton {
            id: button

            //! NOTE: the palettes without their own icon are represented by their first cell
            readonly property bool hasIcon: Boolean(model.paletteIcon) && model.paletteIcon !== IconCode.NONE
            readonly property var cellIcon: hasIcon ? undefined : root.firstCellIcon(model.index)
            readonly property size cellSize: model.gridSize

            x: (ListView.view.width - width) / 2
            width: root.buttonSize
            height: root.buttonSize

            contentItem: iconComp

            accentButton: Boolean(model.expanded)
            transparent: !accentButton

            toolTipTitle: model.display

            navigation.panel: navPanel
            navigation.name: model.display
            navigation.row: model.index
            navigation.accessible.name: model.display

            onClicked: {
                root.paletteClicked(model.index)
            }

            Component {
                id: iconComp

                Item {
                    implicitWidth: root.iconSize
                    implicitHeight: root.iconSize

                    StyledIconLabel {
                        anchors.centerIn: parent
                        visible: button.hasIcon

                        //! NOTE: some glyphs are wider than the others, so they are scaled down to fit the icon
                        scale: Math.min(1, root.iconSize / Math.max(implicitWidth, 1),
                                        root.iconSize / Math.max(implicitHeight, 1))

                        iconCode: button.hasIcon ? model.paletteIcon : IconCode.NONE
                    }

                    IconView {
                        anchors.centerIn: parent
                        visible: !button.hasIcon && Boolean(button.cellIcon)

                        //! NOTE: the cell is painted in its own size, and then it is scaled down to fit the icon
                        width: button.cellSize.width
                        height: button.cellSize.height
                        scale: Math.min(1, root.iconSize / Math.max(width, 1), root.iconSize / Math.max(height, 1))

                        icon: button.cellIcon
                    }

                    StyledTextLabel {
                        anchors.centerIn: parent
                        visible: !button.hasIcon && !Boolean(button.cellIcon)
                        font: ui.theme.bodyBoldFont
                        text: model.display.charAt(0)
                    }
                }
            }
        }
    }
}
