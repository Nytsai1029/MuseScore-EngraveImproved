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
import MuseScore.NotationScene 1.0

import MuseScore.Atelier 1.0

Row {
    id: root

    property NavigationPanel navigationPanel: null
    property int navigationOrderStart: 0

    spacing: 1

    UndoRedoToolbarModel {
        id: undoRedoModel
    }

    Component.onCompleted: {
        undoRedoModel.load()
    }

    Repeater {
        model: undoRedoModel

        delegate: AtToolButton {
            readonly property var item: model.itemRole

            anchors.verticalCenter: parent.verticalCenter

            enabled: Boolean(item) && item.enabled

            shape: Boolean(item) ? (item.code === "undo" ? "undo" : (item.code === "redo" ? "redo" : "")) : ""
            icon: Boolean(item) ? item.icon : IconCode.NONE
            iconFont: ui.theme.toolbarIconsFont

            toolTipTitle: Boolean(item) ? item.title : ""
            toolTipDescription: Boolean(item) ? item.description : ""
            toolTipShortcut: Boolean(item) ? item.shortcuts : ""

            navigation.name: Boolean(item) ? item.id : ""
            navigation.panel: root.navigationPanel
            navigation.order: root.navigationOrderStart + model.index

            onClicked: {
                Qt.callLater(item.activate)
            }
        }
    }
}
