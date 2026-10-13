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
import QtQuick.Controls 2.15

import Muse.Ui 1.0
import Muse.Shortcuts 1.0
import MuseScore.AppShell 1.0

import Muse.Tours 1.0

ApplicationWindow {
    id: root

    default property alias windowContent: windowContentItem.data

    //! NOTE The title bar is drawn over the top of the content, which leaves room for the buttons of the window.
    //! It is merged only where it can be (macOS): read the value to know whether it is.
    //! titleBarHeight is the height of the bar of the content which takes the place of the title bar
    property alias titleBarMerged: bridge.titleBarMerged
    property alias titleBarHeight: bridge.titleBarHeight
    readonly property int titleBarLeftInset: bridge.titleBarLeftInset
    readonly property int titleBarRightInset: bridge.titleBarRightInset

    objectName: "ApplicationWindow"

    title: titleProvider.title

    width: 1150
    height: 800

    minimumWidth: 1050
    minimumHeight: 500

    visible: false

    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        ui.rootItem = root.contentItem
        titleProvider.load()
    }

    MainWindowTitleProvider {
        id: titleProvider
    }

    MainWindowBridge {
        id: bridge

        window: root

        //! NOTE These properties of QWindow (of which ApplicationWindow is derived)
        //!      are not available in QML, so we access them via MainWindowBridge
        filePath: titleProvider.filePath
        fileModified: titleProvider.fileModified
    }

    GraphicsTestObject {}

    ToolTipProvider { }

    ToursProvider { }

    //! NOTE Need only create
    Shortcuts { }

    Item {
        id: windowContentItem
        anchors.fill: parent
    }

    function showMinimizedWithSavePreviousState() {
        bridge.showMinimizedWithSavePreviousState()
    }

    function handleTitleBarPress() {
        bridge.handleTitleBarPress()
    }
}
