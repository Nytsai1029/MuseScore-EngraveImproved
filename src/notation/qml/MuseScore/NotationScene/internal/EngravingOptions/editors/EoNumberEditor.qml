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

import "../EoMusic.js" as EoMusic
import "../EoTitles.js" as EoTitles

//! NOTE: the control of a number option (kind "number"): a field with a value and a unit,
//! which is edited, scrolled and dragged up and down
IncrementalPropertyControl {
    id: root

    //! NOTE: the EoNumber and the EngravingOptionsModel
    required property var option
    required property var optionsModel

    //! NOTE: the EoSession: while the handle of the preview is dragged, the field shows
    //! the value under the pointer
    property var session: null

    property NavigationPanel navigationPanel: null
    property int navigationRow: 0
    property int navigationColumn: 1

    //! NOTE: in a cell of a grid the field asks only for the room its numbers need
    property bool compact: false

    readonly property var styleItem: Boolean(option.sid) ? optionsModel.item(option.sid) : null
    readonly property string plainTitle: EoTitles.plain(option.fullTitle)

    readonly property bool isDragged: Boolean(session) && Boolean(option.sid) && session.dragSid === option.sid
                                      && session.dragValue !== null

    //! NOTE: the value of the option in the units of the style; for a point, the coordinate of the option
    readonly property var plainValue: {
        var value = styleItem.value
        if (option.axis === "x") {
            return value.x
        }
        if (option.axis === "y") {
            return value.y
        }
        return value
    }

    readonly property var storedValue: isDragged ? session.dragValue : plainValue

    //! NOTE: in a cell the field takes the width it is given, up to the width it has in a row
    width: compact ? Math.min(112, parent.width) : 112

    //! NOTE: stored values are shown in their own units: a ratio as a percentage,
    //! a number in quarter staff spaces in staff spaces
    currentValue: EoMusic.toShown(option, storedValue)

    minValue: option.min
    maxValue: option.max
    step: option.step
    decimals: option.decimals

    measureUnitsSymbol: {
        //! NOTE: the cells of a compact grid have no room for the unit; the column title says it
        if (compact) {
            return ""
        }
        if (option.percent) {
            return "%"
        }
        if (option.unit !== "auto") {
            return option.unit
        }
        return optionsModel.isInStaffSpaces(option.sid) ? qsTrc("global", "sp") : ""
    }

    navigation.panel: root.navigationPanel
    navigation.row: root.navigationRow
    navigation.column: root.navigationColumn
    navigation.name: "Value_" + root.option.sid
    navigation.accessible.name: root.plainTitle

    onValueEdited: function(newValue) {
        var number = EoMusic.toStored(root.option, newValue)

        if (root.option.axis === "x") {
            root.styleItem.value = Qt.point(number, root.styleItem.value.y)
        } else if (root.option.axis === "y") {
            root.styleItem.value = Qt.point(root.styleItem.value.x, number)
        } else {
            root.styleItem.value = number
        }
    }
}
