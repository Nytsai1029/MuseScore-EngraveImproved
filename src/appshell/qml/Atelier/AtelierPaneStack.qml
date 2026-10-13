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

//! NOTE: a zone of the window which shows one of several panels.
//! The open panels of the zone share it: if there is more than one, their titles are shown as tabs above them.
//! The entries are { "name", "title", "component", "size" (preferred height of a drawer) }
Item {
    id: root

    required property AtelierShellView shell
    property string pageUri: ""

    property var entries: []

    //! NOTE: tabs are always shown (with the close button), even for a single panel
    property bool alwaysShowTabs: false

    readonly property var openEntries: {
        var result = []
        for (var i = 0; i < entries.length; ++i) {
            if (root.shell.isPanelOpen(root.pageUri, entries[i].name, root.shell.panelsRevision)) {
                result.push(entries[i])
            }
        }
        return result
    }

    readonly property bool hasOpenPanels: openEntries.length > 0

    property string currentName: ""

    readonly property var currentEntry: {
        for (var i = 0; i < openEntries.length; ++i) {
            if (openEntries[i].name === root.currentName) {
                return openEntries[i]
            }
        }
        return openEntries.length > 0 ? openEntries[0] : null
    }

    readonly property real preferredSize: Boolean(currentEntry) && Boolean(currentEntry.size) ? currentEntry.size : 200
    readonly property real contentY: tabs.visible ? tabs.height : 0
    readonly property alias currentItem: contentLoader.item

    //! NOTE: the panel which has just been opened comes to the front
    property var prevOpenNames: []
    onOpenEntriesChanged: {
        var names = []
        for (var i = 0; i < openEntries.length; ++i) {
            names.push(openEntries[i].name)
            if (prevOpenNames.indexOf(openEntries[i].name) === -1) {
                root.currentName = openEntries[i].name
            }
        }
        prevOpenNames = names
    }

    AtTabs {
        id: tabs

        width: parent.width
        visible: root.hasOpenPanels && (root.alwaysShowTabs || root.openEntries.length > 1)

        model: root.openEntries
        currentName: Boolean(root.currentEntry) ? root.currentEntry.name : ""

        onActivated: function(name) {
            root.currentName = name
        }

        onCloseRequested: function(name) {
            root.shell.setPanelOpen(root.pageUri, name, false)
        }
    }

    Loader {
        id: contentLoader

        anchors.fill: parent
        anchors.topMargin: root.contentY

        active: root.visible && Boolean(root.currentEntry)
        sourceComponent: Boolean(root.currentEntry) ? root.currentEntry.component : null
    }
}
