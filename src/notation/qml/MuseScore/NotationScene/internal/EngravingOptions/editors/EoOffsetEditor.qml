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

import "../EoTitles.js" as EoTitles

//! NOTE: the control of an offset option (kind "offset"): the two directions side by side
Row {
    id: root

    required property var option
    required property var optionsModel

    property var session: null

    property NavigationPanel navigationPanel: null
    property int navigationRow: 0
    property int navigationColumn: 1
    property bool compact: false

    readonly property var styleItem: Boolean(option.sid) ? optionsModel.item(option.sid) : null

    spacing: compact ? 4 : 10

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
                width: root.compact ? Math.min(96, (root.width - 32) / 2) : 96

                currentValue: axis === "x" ? root.styleItem.value.x : root.styleItem.value.y

                minValue: root.option.min
                maxValue: root.option.max
                step: root.option.step
                decimals: root.option.decimals

                measureUnitsSymbol: root.option.unit === "auto" ? qsTrc("global", "sp") : root.option.unit

                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRow
                navigation.column: root.navigationColumn + index
                navigation.name: "Value_" + root.option.sid + "_" + axis
                navigation.accessible.name: EoTitles.plain(root.option.fullTitle) + " " + axis.toUpperCase()

                onValueEdited: function(newValue) {
                    var value = root.styleItem.value
                    root.styleItem.value = axis === "x" ? Qt.point(newValue, value.y)
                                           : Qt.point(value.x, newValue)
                }
            }
        }
    }
}
