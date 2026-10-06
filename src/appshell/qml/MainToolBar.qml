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

    //! NOTE: the Dorico skin shows the page tabs as one segmented control
    readonly property bool segmented: ui.theme.skin === "dorico"
    readonly property int segmentedMargin: 6

    width: radioButtonList.width + (root.segmented ? 2 * root.segmentedMargin : 0)
    height: root.segmented ? 36 : radioButtonList.height

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

    Rectangle {
        anchors.fill: radioButtonList

        visible: root.segmented
        radius: 4
        color: Utils.colorWithAlpha(ui.theme.buttonColor, ui.theme.buttonOpacityNormal)
    }

    RadioButtonGroup {
        id: radioButtonList

        anchors.verticalCenter: parent.verticalCenter
        x: root.segmented ? root.segmentedMargin : 0

        spacing: 0

        model: toolBarModel

        width: Math.max(1, contentItem.childrenRect.width)
        height: Math.max(1, contentItem.childrenRect.height)

        delegate: PageTabButton {
            id: radioButtonDelegate

            ButtonGroup.group: radioButtonList.radioButtonGroup

            segmented: root.segmented
            height: root.segmented ? 28 : 36

            spacing: 0
            leftPadding: root.segmented ? 10 : 12
            rightPadding: root.segmented ? 10 : 0

            //! NOTE: the segments use the smaller font, so that the segmented control is not wider than the tabs
            normalStateFont: root.segmented ? (model.isTitleBold ? ui.theme.bodyBoldFont : ui.theme.bodyFont)
                                            : (model.isTitleBold ? ui.theme.largeBodyBoldFont : ui.theme.largeBodyFont)
            selectedStateFont: root.segmented ? ui.theme.bodyBoldFont : ui.theme.largeBodyBoldFont

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
