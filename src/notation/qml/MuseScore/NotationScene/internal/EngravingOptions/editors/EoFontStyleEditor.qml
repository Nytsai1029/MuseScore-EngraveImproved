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

//! NOTE: the control of a font style option (kind "fontStyle"): a toggle for each of bold, italic, underlined
//! and struck through, which adds its bit to the value or takes it away (FontStyle)
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

    spacing: 4

    Repeater {
        model: [
            { iconCode: IconCode.TEXT_BOLD, bit: 1, title: qsTrc("inspector", "Bold") },
            { iconCode: IconCode.TEXT_ITALIC, bit: 2, title: qsTrc("inspector", "Italic") },
            { iconCode: IconCode.TEXT_UNDERLINE, bit: 4, title: qsTrc("inspector", "Underline") },
            { iconCode: IconCode.TEXT_STRIKE, bit: 8, title: qsTrc("inspector", "Strikethrough") }
        ]

        delegate: FlatToggleButton {
            icon: modelData.iconCode
            toolTipTitle: modelData.title

            checked: Boolean(root.styleItem) && (root.styleItem.value & modelData.bit) !== 0

            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow
            navigation.column: root.navigationColumn + index
            navigation.name: "Value_" + root.option.sid + "_" + index
            navigation.accessible.name: modelData.title

            onToggled: {
                var value = root.styleItem.value
                root.styleItem.value = checked ? (value & ~modelData.bit) : (value | modelData.bit)
            }
        }
    }
}
