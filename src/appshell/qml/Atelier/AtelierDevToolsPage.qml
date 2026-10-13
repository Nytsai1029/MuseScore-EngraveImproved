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

import Muse.Audio 1.0

import "../DevTools"
import "../DevTools/Gallery"
import "../DevTools/Interactive"
import "../DevTools/CrashHandler"
import "../DevTools/KeyNav"
import "../DevTools/Preferences"

//! NOTE: the tools of the developers (shown in the development mode only)
Item {
    id: root

    required property AtelierShellView shell

    readonly property string pageUri: "musescore://devtools"

    readonly property string leftPaneName: ""
    readonly property string leftPaneTitle: ""
    readonly property string rightPaneName: ""
    readonly property string rightPaneTitle: ""

    property string current: "settings"

    Rectangle {
        id: menuPane

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 240

        color: AtStyle.island2

        DevToolsMenu {
            anchors.fill: parent
            anchors.rightMargin: 1

            model: [
                { "name": "settings", "title": "Settings" },
                { "name": "gallery", "title": "UI Gallery" },
                { "name": "interactive", "title": "Interactive" },
                { "name": "crashhandler", "title": "Crash handler" },
                { "name": "corruptscore", "title": "Corrupt score" },
                { "name": "mpe", "title": "MPE" },
                { "name": "extensions", "title": "Extensions" },
                { "name": "navigation", "title": "KeyNav" }
            ]

            onSelected: function(name) {
                root.current = name
            }
        }

        Rectangle {
            anchors.right: parent.right
            width: 1
            height: parent.height
            color: AtStyle.stroke
        }
    }

    Loader {
        anchors.left: menuPane.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        active: root.visible

        sourceComponent: {
            switch (root.current) {
            case "settings": return settingsComp
            case "gallery": return galleryComp
            case "interactive": return interactiveComp
            case "crashhandler": return crashhandlerComp
            case "corruptscore": return corruptScoreComp
            case "mpe": return mpeComp
            case "extensions": return extensionsComp
            case "navigation": return keynavComp
            }

            return settingsComp
        }
    }

    Component {
        id: settingsComp

        SettingsPage {}
    }

    Component {
        id: galleryComp

        GeneralComponentsGallery {}
    }

    Component {
        id: interactiveComp

        InteractiveTests {}
    }

    Component {
        id: crashhandlerComp

        CrashHandlerDevTools {}
    }

    Component {
        id: corruptScoreComp

        Loader {
            source: "qrc:/qml/DevTools/CorruptScore/CorruptScoreDevTools.qml"
        }
    }

    Component {
        id: mpeComp

        Loader {
            source: "qrc:/qml/DevTools/MPE/ArticulationsProfileEditorView.qml"
        }
    }

    Component {
        id: extensionsComp

        Loader {
            source: "qrc:/qml/DevTools/Extensions/ExtensionsListView.qml"
        }
    }

    Component {
        id: keynavComp

        KeyNavExample {}
    }
}
