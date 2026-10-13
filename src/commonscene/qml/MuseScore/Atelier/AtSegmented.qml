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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

//! NOTE: one choice of several, shown as a track with a raised key on the chosen segment.
//! The model is a list of { "title", "value", "enabled", "icon" }
Rectangle {
    id: root

    property var model: []
    property var currentValue: undefined

    property int segmentWidth: 84
    property int segmentHeight: 26
    property real segmentRadius: 7

    property NavigationPanel navigationPanel: null
    property int navigationOrderStart: 0

    signal activated(var value)

    implicitWidth: row.width + 4
    implicitHeight: segmentHeight + 4

    width: implicitWidth
    height: implicitHeight

    radius: segmentRadius + 2
    color: AtStyle.button

    Row {
        id: row

        x: 2
        y: 2
        spacing: 2

        Repeater {
            model: root.model

            delegate: Item {
                id: segment

                readonly property bool isCurrent: modelData.value === root.currentValue
                readonly property bool isEnabled: modelData.enabled === undefined || Boolean(modelData.enabled)

                width: root.segmentWidth
                height: root.segmentHeight

                opacity: isEnabled ? 1.0 : ui.theme.itemOpacityDisabled

                NavigationControl {
                    id: navCtrl

                    name: "Segment_" + modelData.value
                    panel: root.navigationPanel
                    order: root.navigationOrderStart + model.index
                    enabled: segment.isEnabled && root.visible

                    accessible.role: MUAccessible.RadioButton
                    accessible.name: modelData.title
                    accessible.checked: segment.isCurrent

                    onTriggered: {
                        root.activated(modelData.value)
                    }
                }

                Rectangle {
                    anchors.fill: parent

                    radius: root.segmentRadius
                    color: segment.isCurrent ? AtStyle.raised
                                             : (mouseArea.containsMouse && segment.isEnabled ? AtStyle.hover : "transparent")
                    border.width: segment.isCurrent ? 1 : 0
                    border.color: AtStyle.stroke

                    NavigationFocusBorder {
                        navigationCtrl: navCtrl
                    }
                }

                StyledIconLabel {
                    anchors.centerIn: parent
                    visible: Boolean(modelData.icon)
                    iconCode: Boolean(modelData.icon) ? modelData.icon : IconCode.NONE
                    color: segment.isCurrent ? AtStyle.text : AtStyle.text2
                }

                StyledTextLabel {
                    anchors.fill: parent
                    anchors.leftMargin: 4
                    anchors.rightMargin: 4

                    visible: !Boolean(modelData.icon)
                    text: modelData.title
                    font: AtStyle.font(13, segment.isCurrent ? Font.DemiBold : Font.Medium)
                    color: segment.isCurrent ? AtStyle.text : AtStyle.text2
                }

                MouseArea {
                    id: mouseArea

                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: segment.isEnabled

                    onClicked: {
                        navCtrl.requestActiveByInteraction()
                        root.activated(modelData.value)
                    }
                }
            }
        }
    }
}
