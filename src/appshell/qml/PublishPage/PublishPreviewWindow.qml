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
import QtQuick.Window 2.15

import Muse.Ui 1.0
import MuseScore.AppShell 1.0
import MuseScore.NotationScene 1.0

Window {
    id: root

    objectName: "PublishPreviewWindow"

    signal closed()

    title: titleProvider.title.length > 0
           ? qsTrc("appshell", "Publish preview") + " - " + titleProvider.title
           : qsTrc("appshell", "Publish preview")

    width: 900
    height: 700

    minimumWidth: 640
    minimumHeight: 420

    color: ui.theme.backgroundPrimaryColor

    MainWindowTitleProvider {
        id: titleProvider

        Component.onCompleted: {
            load()
        }
    }

    NotationView {
        anchors.fill: parent

        name: "DetachedPublishNotationView"
        publishMode: true
        syncViewState: false
    }

    onClosing: {
        root.closed()
        root.destroy()
    }
}
