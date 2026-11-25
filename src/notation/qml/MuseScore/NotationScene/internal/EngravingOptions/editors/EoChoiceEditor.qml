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

//! NOTE: the control of a choice option (kind "choice"): a segmented track or a drop-down list.
//! "list" and "tiles" are wide and have controls of their own (EoListEditor, EoTilesEditor)
Loader {
    id: root

    required property var option
    required property var optionsModel

    property var session: null

    property NavigationPanel navigationPanel: null
    property int navigationRow: 0
    property int navigationColumn: 1

    //! NOTE: in a cell of a grid a choice is always a drop-down list: a track does not fit
    property bool compact: false

    readonly property var styleItem: Boolean(option.sid) ? optionsModel.item(option.sid) : null

    sourceComponent: option.presentation === "dropdown" || compact ? dropdownComp : segmentedComp

    Component {
        id: segmentedComp

        AtSegmented {
            model: root.option.choices
            currentValue: root.styleItem.value

            segmentWidth: EoMusic.segmentWidthFor(root.option.choices)

            navigationPanel: root.navigationPanel
            navigationRow: root.navigationRow
            navigationOrderStart: root.navigationColumn
            navigationNamePrefix: "Value_" + root.option.sid

            onActivated: function(value) {
                root.styleItem.value = value
            }
        }
    }

    Component {
        id: dropdownComp

        StyledDropdown {
            width: root.compact ? parent.width : 220

            model: root.option.choices
            textRole: "title"
            valueRole: "value"

            currentIndex: indexOfValue(root.styleItem.value)

            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow
            navigation.column: root.navigationColumn
            navigation.name: "Value_" + root.option.sid
            navigation.accessible.name: EoTitles.plain(root.option.fullTitle)

            onActivated: function(index, value) {
                root.styleItem.value = value
            }
        }
    }
}
