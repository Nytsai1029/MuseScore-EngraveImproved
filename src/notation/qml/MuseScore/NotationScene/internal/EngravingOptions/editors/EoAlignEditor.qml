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

import "../EoTitles.js" as EoTitles

//! NOTE: the control of an alignment option (kind "align"): a list for the horizontal part and one for the
//! vertical part of the text which the score file has for it ("left,baseline")
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

    spacing: 6

    Repeater {
        model: [
            [
                { title: qsTrc("notation/engravingoptions", "Left"), value: "left" },
                { title: qsTrc("notation/engravingoptions", "Center"), value: "center" },
                { title: qsTrc("notation/engravingoptions", "Right"), value: "right" }
            ],
            [
                { title: qsTrc("notation/engravingoptions", "Top"), value: "top" },
                { title: qsTrc("notation/engravingoptions", "Middle"), value: "center" },
                { title: qsTrc("notation/engravingoptions", "Bottom"), value: "bottom" },
                { title: qsTrc("notation/engravingoptions", "Baseline"), value: "baseline" }
            ]
        ]

        delegate: StyledDropdown {
            readonly property int part: index

            width: root.compact ? 90 : 104

            model: modelData
            textRole: "title"
            valueRole: "value"

            currentIndex: Boolean(root.styleItem) ? indexOfValue(String(root.styleItem.value).split(",")[part]) : -1

            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow
            navigation.column: root.navigationColumn + part
            navigation.name: "Value_" + root.option.sid + "_" + part
            navigation.accessible.name: EoTitles.plain(root.option.fullTitle)

            onActivated: function(index, value) {
                var parts = String(root.styleItem.value).split(",")
                parts[part] = value
                root.styleItem.value = parts.join(",")
            }
        }
    }
}
