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

//! NOTE: the control of a font option (kind "font"): a drop-down list of the families of the system
StyledDropdown {
    id: root

    required property var option
    required property var optionsModel

    property var session: null

    property NavigationPanel navigationPanel: null
    property int navigationRow: 0
    property int navigationColumn: 1
    property bool compact: false

    readonly property var styleItem: Boolean(option.sid) ? optionsModel.item(option.sid) : null

    width: compact ? parent.width : 220

    //! NOTE: the families of the system, and the family of the value if the system does not have it
    model: {
        var families = Qt.fontFamilies()
        var current = String(styleItem.value)
        var result = []

        if (Boolean(current) && families.indexOf(current) < 0) {
            result.push({ text: current, value: current })
        }

        for (var i = 0; i < families.length; ++i) {
            result.push({ text: families[i], value: families[i] })
        }

        return result
    }

    currentIndex: indexOfValue(String(styleItem.value))

    navigation.panel: root.navigationPanel
    navigation.row: root.navigationRow
    navigation.column: root.navigationColumn
    navigation.name: "Value_" + root.option.sid
    navigation.accessible.name: EoTitles.plain(root.option.fullTitle)

    onActivated: function(index, value) {
        root.styleItem.value = value
    }
}
