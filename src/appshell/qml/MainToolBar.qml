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
import Muse.UiComponents 1.0
import MuseScore.AppShell 1.0

import "./PublishPage"

Item {
    id: root

    width: radioButtonList.width
    height: radioButtonList.height

    property alias navigation: navPanel

    property string currentUri: "musescore://home"

    signal selected(string uri)

    property var publishPreviewWindow: null

    function select(uri) {
        root.selected(uri)
    }

    function openPublishPreviewWindow() {
        if (Boolean(root.publishPreviewWindow)) {
            root.publishPreviewWindow.show()
            root.publishPreviewWindow.raise()
            root.publishPreviewWindow.requestActivate()
            return
        }

        root.publishPreviewWindow = publishPreviewWindowComponent.createObject(root)
        if (!root.publishPreviewWindow) {
            return
        }

        root.publishPreviewWindow.closed.connect(function() {
            root.publishPreviewWindow = null
        })
        root.publishPreviewWindow.show()
        root.publishPreviewWindow.requestActivate()
    }

    function focusOnFirst() {
        var btn = radioButtonList.itemAtIndex(0)
        if (btn) {
            btn.navigation.requestActive()
        }
    }

    MainToolBarModel {
        id: toolBarModel
    }

    Component.onCompleted: {
        toolBarModel.load()
    }

    NavigationPanel {
        id: navPanel
        name: "MainToolBar"
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("appshell", "Main toolbar") + " " + navPanel.directionInfo
    }

    Component {
        id: publishPreviewWindowComponent

        PublishPreviewWindow {}
    }

    RadioButtonGroup {
        id: radioButtonList
        spacing: 0

        model: toolBarModel

        width: Math.max(1, contentItem.childrenRect.width)
        height: Math.max(1, contentItem.childrenRect.height)

        delegate: PageTabButton {
            id: radioButtonDelegate

            ButtonGroup.group: radioButtonList.radioButtonGroup

            spacing: 0
            leftPadding: 12

            normalStateFont: model.isTitleBold ? ui.theme.largeBodyBoldFont : ui.theme.largeBodyFont

            navigation.name: model.title
            navigation.panel: navPanel
            navigation.order: model.index

            checked: model.uri === root.currentUri
            title: model.title

            onToggled: {
                root.selected(model.uri)
            }

            MouseArea {
                id: publishPreviewMenuMouseArea

                anchors.fill: parent

                enabled: model.uri === "musescore://publish"
                acceptedButtons: Qt.RightButton

                onClicked: function(mouse) {
                    if (mouse.button === Qt.RightButton) {
                        publishPreviewMenuLoader.show(Qt.point(mouse.x, mouse.y))
                    }
                }

                ContextMenuLoader {
                    id: publishPreviewMenuLoader

                    items: [
                        { id: "show-as-window", title: qsTrc("appshell", "显示为单独窗口") }
                    ]

                    onHandleMenuItem: function(itemId) {
                        if (itemId === "show-as-window") {
                            root.openPublishPreviewWindow()
                        }
                    }
                }
            }
        }
    }
}
