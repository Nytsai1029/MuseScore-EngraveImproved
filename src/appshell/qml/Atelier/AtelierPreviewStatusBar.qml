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
import MuseScore.NotationScene 1.0
import MuseScore.Atelier 1.0

//! NOTE: the status bar of the Preview mode: the page which is shown on the left;
//! how the page is fitted in the view and the zoom on the right. The zoom is the one of the preview only
Rectangle {
    id: root

    //! NOTE: a PreviewPagesModel
    required property var pagesModel

    //! NOTE: the paint view of the preview (a NotationPaintView)
    property var view: null

    property NavigationSection navigationSection: NavigationSection {
        name: "NotationStatusBar"
        enabled: root.enabled && root.visible
        order: 8
    }

    readonly property int zoomKind: Boolean(root.view) ? root.view.zoomKind : NotationPaintView.ZoomByPercentage
    readonly property int zoomPercentage: Boolean(root.view) ? root.view.zoomPercentage : 100

    readonly property var zoomKinds: [
        { id: "whole-page", kind: NotationPaintView.ZoomToWholePage, title: qsTrc("notation", "Whole page") },
        { id: "page-width", kind: NotationPaintView.ZoomToPageWidth, title: qsTrc("notation", "Page width") },
        { id: "two-pages", kind: NotationPaintView.ZoomToTwoPages, title: qsTrc("notation", "Two pages") }
    ]

    readonly property string zoomKindTitle: {
        for (var i = 0; i < zoomKinds.length; ++i) {
            if (zoomKinds[i].kind === root.zoomKind) {
                return zoomKinds[i].title
            }
        }

        return qsTrc("notation", "Zoom")
    }

    height: AtStyle.statusBarHeight
    color: AtStyle.island

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

        //: The page which is shown, and the number of pages; e.g. "Page 2 of 5"
        text: root.pagesModel.pageCount > 0 ? qsTrc("appshell", "Page %1 of %2").arg(root.pagesModel.currentPageIndex + 1)
                                                                               .arg(root.pagesModel.pageCount)
                                            : ""
        font: AtStyle.font(11.5)
        color: AtStyle.text2
    }

    Row {
        id: controls

        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter

        spacing: 2

        enabled: Boolean(root.view)

        AtToolButton {
            anchors.verticalCenter: parent.verticalCenter
            height: 22
            radius: 6

            text: root.zoomKindTitle
            textFont: AtStyle.font(11.5)
            hasMenuArrow: true

            checked: zoomKindMenu.isMenuOpened
            checkedStyle: "soft"

            navigation.name: "ZoomKind"
            navigation.panel: navPanel
            navigation.order: 1

            onClicked: {
                var items = []
                for (var i = 0; i < root.zoomKinds.length; ++i) {
                    items.push({ id: root.zoomKinds[i].id, title: root.zoomKinds[i].title,
                                 checkable: true, checked: root.zoomKinds[i].kind === root.zoomKind })
                }

                zoomKindMenu.toggleOpened(items)
            }

            StyledMenuLoader {
                id: zoomKindMenu

                onHandleMenuItem: function(itemId) {
                    switch (itemId) {
                    case "whole-page": root.view.zoomToWholePage(); break
                    case "page-width": root.view.zoomToPageWidth(); break
                    case "two-pages": root.view.zoomToTwoPages(); break
                    }
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

            toolTipTitle: qsTrc("notation", "Zoom out")

            navigation.name: "ZoomOut"
            navigation.panel: navPanel
            navigation.order: 2

            onClicked: {
                root.view.zoomOut()
            }
        }

        StyledTextLabel {
            anchors.verticalCenter: parent.verticalCenter
            width: 44

            text: root.zoomPercentage + "%"
            font: AtStyle.font(11.5)
            color: AtStyle.text2
        }

        AtToolButton {
            anchors.verticalCenter: parent.verticalCenter
            width: 22
            height: 22
            radius: 6

            shape: "plus"

            toolTipTitle: qsTrc("notation", "Zoom in")

            navigation.name: "ZoomIn"
            navigation.panel: navPanel
            navigation.order: 3

            onClicked: {
                root.view.zoomIn()
            }
        }
    }
}
