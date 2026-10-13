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

//! NOTE: the tool strip of the Preview mode: where the score goes from here
//! (the printer, a file, MuseScore.com, Audio.com), and the preview as a window of its own
Rectangle {
    id: root

    property NavigationSection navigationSection: null

    property bool keyHintsVisible: true

    //! NOTE: the export goes through the Export pane (its settings are there)
    signal exportRequested()
    signal separateWindowRequested()

    implicitHeight: strip.height + 9
    color: AtStyle.island

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "PublishToolBar"
        section: root.navigationSection
        order: 1
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("publish", "Publish toolbar")
    }

    AtelierActionsModel {
        id: actions

        actionCodes: ["print", "file-export", "file-publish", "file-share-audio"]

        Component.onCompleted: {
            load()
        }
    }

    //! NOTE: the actions by their codes
    readonly property var items: {
        var result = {}
        var list = actions.items

        for (var i = 0; i < list.length; ++i) {
            result[list[i].code] = list[i]
        }

        return result
    }

    component ActionButton: AtStripButton {
        property string code: ""
        readonly property var item: root.items[code]

        //! NOTE: the actions which leave the computer are quieter
        property bool quiet: false

        enabled: Boolean(item) && item.enabled

        text: Boolean(item) ? AtStyle.plainTitle(item.title) : ""
        textFont: AtStyle.font(12.5, Font.Medium)
        contentColor: quiet ? AtStyle.text2 : AtStyle.text

        keyHint: Boolean(item) ? AtStyle.keyHint(item.shortcuts) : ""
        keyHintsVisible: root.keyHintsVisible

        navigation.panel: root.navigationPanel
        navigation.name: code
    }

    Row {
        id: strip

        anchors.horizontalCenter: parent.horizontalCenter
        y: 5

        spacing: 2

        ActionButton {
            code: "print"
            shape: "print"
            navigation.order: 1

            onClicked: {
                actions.trigger(code)
            }
        }

        ActionButton {
            code: "file-export"
            shape: "export"
            navigation.order: 2

            onClicked: {
                root.exportRequested()
            }
        }

        AtDivider {
            y: 5
            lineHeight: 22
            sideMargin: 8
        }

        ActionButton {
            code: "file-publish"
            shape: "cloud-up"
            quiet: true
            navigation.order: 3

            toolTipTitle: qsTrc("project/save", "Share this score and its audio on MuseScore.com")

            onClicked: {
                actions.trigger(code)
            }
        }

        ActionButton {
            code: "file-share-audio"
            shape: "audio"
            quiet: true
            navigation.order: 4

            toolTipTitle: qsTrc("project/save", "Share the audio from this score on Audio.com")

            onClicked: {
                actions.trigger(code)
            }
        }

        AtDivider {
            y: 5
            lineHeight: 22
            sideMargin: 8
        }

        AtStripButton {
            shape: "window"

            text: qsTrc("appshell", "Separate window")
            textFont: AtStyle.font(12.5, Font.Medium)
            contentColor: AtStyle.text

            toolTipTitle: qsTrc("appshell", "Show the preview as a separate window")

            keyHintsVisible: root.keyHintsVisible

            navigation.panel: root.navigationPanel
            navigation.name: "SeparateWindow"
            navigation.order: 5

            onClicked: {
                root.separateWindowRequested()
            }
        }
    }
}
