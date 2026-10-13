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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0
import MuseScore.Atelier 1.0

import "../common"
import "appearance/internal"

//! NOTE: whether the selection is shown, in the modes of the Atelier skin. In the Input mode this group is at the top
//! of the properties (see InspectorForm); in the Engrave mode it is the last group of the General section
//! (see GeneralModeView)
Column {
    id: root

    property QtObject model: null
    property string mode: "input"

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    readonly property bool isInput: root.mode === "input"
    readonly property QtObject appearance: root.model ? root.model.appearanceSettingsModel : null

    width: parent ? parent.width : 0
    spacing: 12

    function navigationRow(row) {
        return root.navigationRowStart + row
    }

    AtSectionTitle {
        width: parent.width
        text: qsTrc("inspector", "Show")
    }

    PropertyCheckBox {
        navigation.name: "Visible"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow(0)

        text: qsTrc("inspector", "Visible")
        propertyItem: root.model ? root.model.isVisible : null
        isIndeterminate: enabled && propertyItem && propertyItem.isUndefined
    }

    PropertyCheckBox {
        visible: root.isInput && Boolean(propertyItem) && propertyItem.isVisible

        navigation.name: "Cue size"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow(1)

        text: qsTrc("inspector", "Cue size")
        propertyItem: root.model ? root.model.isSmall : null
        isIndeterminate: enabled && propertyItem && propertyItem.isUndefined
    }

    PropertyCheckBox {
        visible: root.isInput && Boolean(propertyItem) && propertyItem.isVisible

        navigation.name: "Auto-place"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow(2)

        text: qsTrc("inspector", "Auto-place")
        propertyItem: root.model ? root.model.isAutoPlaceAllowed : null
        isIndeterminate: enabled && propertyItem && propertyItem.isUndefined
    }

    Loader {
        width: parent.width

        active: !root.isInput
        visible: active

        sourceComponent: ColorSection {
            propertyItem: root.appearance ? root.appearance.color : null

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRow(10)
        }
    }
}
