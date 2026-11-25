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

//! NOTE: the control of a scale option (kind "scale"): the width and the height, and between them the lock which
//! keeps them the same — side by side, or in a cell of a grid one under the other with the lock beside them
Item {
    id: root

    required property var option
    required property var optionsModel

    property var session: null

    property NavigationPanel navigationPanel: null
    property int navigationRow: 0
    property int navigationColumn: 1
    property bool compact: false

    readonly property var styleItem: Boolean(option.sid) ? optionsModel.item(option.sid) : null
    readonly property var lockItem: Boolean(option.lockSid) ? optionsModel.item(option.lockSid) : null
    readonly property bool isLocked: Boolean(lockItem) && lockItem.value === true

    readonly property string plainTitle: EoTitles.plain(option.fullTitle)

    readonly property int fieldWidth: compact ? 96 : 88
    readonly property int spacing: 4

    implicitWidth: compact ? fieldWidth + spacing + lockButton.width
                           : 2 * fieldWidth + 2 * spacing + lockButton.width
    implicitHeight: compact ? widthField.height + spacing + heightField.height : widthField.height
    width: implicitWidth
    height: implicitHeight

    function setDimension(dimension, number) {
        var value = root.styleItem.value
        if (root.isLocked) {
            root.styleItem.value = Qt.size(number, number)
        } else if (dimension === "width") {
            root.styleItem.value = Qt.size(number, value.height)
        } else {
            root.styleItem.value = Qt.size(value.width, number)
        }
    }

    component DimensionField: IncrementalPropertyControl {
        required property string dimension

        width: root.fieldWidth

        prefixIcon: dimension === "width" ? IconCode.HORIZONTAL : IconCode.VERTICAL
        currentValue: dimension === "width" ? root.styleItem.value.width : root.styleItem.value.height

        minValue: root.option.min
        maxValue: root.option.max
        step: root.option.step
        decimals: root.option.decimals

        measureUnitsSymbol: "×"

        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow
        navigation.name: "Value_" + root.option.sid + "_" + dimension

        onValueEdited: function(newValue) {
            root.setDimension(dimension, newValue)
        }
    }

    DimensionField {
        id: widthField

        dimension: "width"

        navigation.column: root.navigationColumn
        navigation.accessible.name: root.plainTitle + " " + qsTrc("notation/engravingoptions", "Width")
    }

    AtToolButton {
        id: lockButton

        x: root.fieldWidth + root.spacing
        y: (root.height - height) / 2
        width: 24
        height: 24

        visible: Boolean(root.lockItem)

        icon: root.isLocked ? IconCode.LOCK_CLOSED : IconCode.LOCK_OPEN
        checked: root.isLocked
        checkedStyle: "soft"

        toolTipTitle: root.isLocked ? qsTrc("notation/engravingoptions", "Width and height are kept the same")
                                    : qsTrc("notation/engravingoptions", "Keep width and height the same")

        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow
        navigation.column: root.navigationColumn + 1
        navigation.name: "Lock_" + root.option.sid
        navigation.accessible.name: root.plainTitle + ": " + toolTipTitle

        onClicked: {
            var locking = !root.isLocked
            root.lockItem.value = locking

            //! NOTE: locking makes the height the width, as the style dialog did
            if (locking) {
                var width = root.styleItem.value.width
                root.styleItem.value = Qt.size(width, width)
            }
        }
    }

    DimensionField {
        id: heightField

        x: root.compact ? 0 : lockButton.x + lockButton.width + root.spacing
        y: root.compact ? widthField.height + root.spacing : 0

        dimension: "height"

        navigation.column: root.navigationColumn + 2
        navigation.accessible.name: root.plainTitle + " " + qsTrc("notation/engravingoptions", "Height")
    }
}
