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
import "playback"
import "appearance/internal"

//! NOTE: the General section in the modes of the Atelier skin. In the Input mode it has how the selection sounds
//! (whether it is shown is at the top of the properties, see InspectorForm); in the Engrave mode, where it sits
//! (what the Appearance popup of the other skins has, laid out in the section itself) and whether it is shown
Column {
    id: root

    property QtObject model: null
    property string mode: "input"

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    readonly property bool isInput: root.mode === "input"
    readonly property QtObject appearance: root.model ? root.model.appearanceSettingsModel : null
    readonly property bool hasPlaybackSettings: Boolean(root.model) && !root.model.playbackProxyModel.isEmpty

    width: parent ? parent.width : 0
    spacing: 10

    function navigationRow(row) {
        return root.navigationRowStart + row
    }

    component GroupLine: Rectangle {
        width: root.width
        height: 1
        color: AtStyle.stroke
    }

    // ---------- Input: how it sounds ----------

    Column {
        width: parent.width
        spacing: 12

        visible: root.isInput

        AtSectionTitle {
            width: parent.width
            text: qsTrc("inspector", "Playback")
        }

        PropertyCheckBox {
            navigation.name: "Play"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow(1)

            text: qsTrc("inspector", "Play")
            propertyItem: root.model ? root.model.isPlayable : null
            isIndeterminate: enabled && propertyItem && propertyItem.isUndefined
        }

        Loader {
            width: parent.width

            active: root.isInput && root.hasPlaybackSettings
            visible: active

            sourceComponent: PlaybackSettings {
                proxyModel: root.model ? root.model.playbackProxyModel : null
                navigationPanel: root.navigationPanel
            }
        }
    }

    // ---------- Engrave: where it sits ----------

    Column {
        width: parent.width
        spacing: 12

        visible: !root.isInput

        AtSectionTitle {
            width: parent.width
            text: qsTrc("inspector", "Position")
        }

        AppearanceOffsetSection {
            id: offsetSection

            offset: root.appearance ? root.appearance.offset : null
            isSnappedToGrid: root.appearance ? root.appearance.isSnappedToGrid : false
            isVerticalOffsetAvailable: root.appearance ? root.appearance.isVerticalOffsetAvailable : false

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRow(10)

            onSnapToGridToggled: function(snap) {
                if (root.appearance) {
                    root.appearance.isSnappedToGrid = snap
                }
            }

            onConfigureGridRequested: {
                if (root.appearance) {
                    root.appearance.configureGrid()
                }
            }
        }

        PropertyCheckBox {
            navigation.name: "Auto-place"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow(40)

            text: qsTrc("inspector", "Auto-place")
            propertyItem: root.model ? root.model.isAutoPlaceAllowed : null
            isIndeterminate: enabled && propertyItem && propertyItem.isUndefined
        }

        VerticalSpacingSection {
            id: verticalSpacingSection

            minimumDistance: root.appearance ? root.appearance.minimumDistance : null

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRow(50)
        }

        HorizontalSpacingSection {
            id: horizontalSpacingSection

            leadingSpace: root.appearance ? root.appearance.leadingSpace : null
            measureWidth: root.appearance ? root.appearance.measureWidth : null

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRow(60)
        }
    }

    GroupLine {
        visible: !root.isInput
    }

    Loader {
        width: parent.width

        active: !root.isInput && Boolean(root.appearance)
        visible: active

        sourceComponent: ArrangeSection {
            arrangeOrderProperty: root.appearance.arrangeOrder

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRow(80)

            onPushBackwardsRequested: {
                root.appearance.pushBackwardsInOrder()
            }

            onPushForwardsRequested: {
                root.appearance.pushForwardsInOrder()
            }

            onPushToBackRequested: {
                root.appearance.pushToBackInOrder()
            }

            onPushToFrontRequested: {
                root.appearance.pushToFrontInOrder()
            }
        }
    }

    GroupLine {
        visible: !root.isInput
    }

    // ---------- Engrave: whether it is shown ----------

    //! NOTE: in the Input mode, this group is at the top of the properties (see InspectorForm)
    Loader {
        width: parent.width

        active: !root.isInput
        visible: active

        sourceComponent: GeneralShowGroup {
            model: root.model
            mode: root.mode

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRow(100)
        }
    }
}
