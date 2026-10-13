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
import MuseScore.AppShell 1.0
import MuseScore.Atelier 1.0

//! NOTE: what is selected on the left; concert pitch, the view mode and the zoom on the right
Rectangle {
    id: root

    property NavigationSection navigationSection: NavigationSection {
        name: "NotationStatusBar"
        enabled: root.enabled && root.visible
        order: 8
    }

    //! NOTE: e.g. "Note; Pitch: F5; Duration: Half; …" (empty if nothing is selected)
    readonly property string selectionInfo: model.accessibilityInfo

    height: AtStyle.statusBarHeight
    color: AtStyle.island

    NotationStatusBarModel {
        id: model
    }

    NavigationPanel {
        id: navPanel
        name: "NotationStatusBar"
        section: root.navigationSection
        order: 0
        direction: NavigationPanel.Horizontal
        enabled: root.enabled && root.visible
    }

    Rectangle {
        width: parent.width
        height: 1
        color: AtStyle.stroke
    }

    StyledTextLabel {
        anchors.left: parent.left
        anchors.leftMargin: 14
        anchors.right: controls.left
        anchors.rightMargin: 16
        anchors.verticalCenter: parent.verticalCenter

        horizontalAlignment: Text.AlignLeft
        text: root.selectionInfo
        font: AtStyle.font(11.5)
        color: AtStyle.text2
    }

    Row {
        id: controls

        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter

        spacing: 2

        AtToolButton {
            anchors.verticalCenter: parent.verticalCenter
            height: 22
            radius: 6

            shape: "concert-pitch"
            text: model.concertPitchItem.title
            textFont: AtStyle.font(11.5)

            checked: model.concertPitchItem.checked
            checkedStyle: "soft"
            enabled: model.concertPitchItem.enabled

            toolTipTitle: model.concertPitchItem.title
            toolTipDescription: model.concertPitchItem.description

            navigation.name: "ConcertPitch"
            navigation.panel: navPanel
            navigation.order: 1

            onClicked: {
                model.toggleConcertPitch()
            }
        }

        AtToolButton {
            anchors.verticalCenter: parent.verticalCenter
            height: 22
            radius: 6

            text: Boolean(model.currentViewMode) ? model.currentViewMode.title : ""
            textFont: AtStyle.font(11.5)
            hasMenuArrow: true

            checked: viewModeMenu.isMenuOpened
            checkedStyle: "soft"

            navigation.name: "ViewMode"
            navigation.panel: navPanel
            navigation.order: 2

            onClicked: {
                viewModeMenu.toggleOpened(model.availableViewModeList)
            }

            StyledMenuLoader {
                id: viewModeMenu

                onHandleMenuItem: function(itemId) {
                    model.setCurrentViewMode(itemId)
                }
            }
        }

        AtDivider {
            anchors.verticalCenter: parent.verticalCenter
            lineHeight: 14
            sideMargin: 5
        }

        AtToolButton {
            anchors.verticalCenter: parent.verticalCenter
            width: 22
            height: 22
            radius: 6

            shape: "minus"
            enabled: model.zoomEnabled && model.currentZoomPercentage > model.minZoomPercentage()

            toolTipTitle: qsTrc("notation", "Zoom out")

            navigation.name: "ZoomOut"
            navigation.panel: navPanel
            navigation.order: 3

            onClicked: {
                model.zoomOut()
            }
        }

        AtToolButton {
            anchors.verticalCenter: parent.verticalCenter
            width: Math.max(44, implicitWidth)
            height: 22
            radius: 6

            text: model.currentZoomPercentage + "%"
            textFont: AtStyle.font(11.5)
            enabled: model.zoomEnabled

            checked: zoomMenu.isMenuOpened
            checkedStyle: "soft"

            toolTipTitle: qsTrc("notation", "Zoom")

            navigation.name: "Zoom"
            navigation.panel: navPanel
            navigation.order: 4

            onClicked: {
                zoomMenu.toggleOpened(model.availableZoomList)
            }

            StyledMenuLoader {
                id: zoomMenu

                onHandleMenuItem: function(itemId) {
                    model.setCurrentZoom(itemId)
                }
            }
        }

        AtToolButton {
            anchors.verticalCenter: parent.verticalCenter
            width: 22
            height: 22
            radius: 6

            shape: "plus"
            enabled: model.zoomEnabled && model.currentZoomPercentage < model.maxZoomPercentage()

            toolTipTitle: qsTrc("notation", "Zoom in")

            navigation.name: "ZoomIn"
            navigation.panel: navPanel
            navigation.order: 5

            onClicked: {
                model.zoomIn()
            }
        }
    }
}
