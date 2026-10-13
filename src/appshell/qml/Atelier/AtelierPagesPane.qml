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
import QtQuick.Controls 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.AppShell 1.0
import MuseScore.NotationScene 1.0
import MuseScore.Atelier 1.0

//! NOTE: the left pane of the Preview mode: the pages of the score as small pictures
//! (the one which the preview shows is marked, a click shows another one), and the facts of the page
Item {
    id: root

    //! NOTE: a PreviewPagesModel
    required property var pagesModel

    property NavigationSection navigationSection: null
    property int navigationOrderStart: 1

    readonly property int thumbnailWidth: 148

    NavigationPanel {
        id: navPanel
        name: "PagesPane"
        section: root.navigationSection
        order: root.navigationOrderStart
        direction: NavigationPanel.Vertical
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("appshell", "Pages")
    }

    NavigationPanel {
        id: footerNavPanel
        name: "PagesPaneFooter"
        section: root.navigationSection
        order: root.navigationOrderStart + 1
        enabled: root.enabled && root.visible
    }

    AtelierActionsModel {
        id: actions

        actionCodes: ["page-settings"]

        Component.onCompleted: {
            load()
        }
    }

    AtPaneHeader {
        id: header

        width: parent.width

        title: qsTrc("appshell", "Pages")
        leftPadding: 14
        rightPadding: 14

        StyledTextLabel {
            y: 5

            //: The number of pages of the score
            text: qsTrc("appshell", "%n page(s)", "", root.pagesModel.pageCount)
            font: AtStyle.font(12)
            color: AtStyle.text2
        }
    }

    StyledListView {
        id: pagesView

        anchors.top: header.bottom
        anchors.topMargin: 2
        anchors.bottom: footer.top
        anchors.left: parent.left
        anchors.leftMargin: 14
        anchors.right: parent.right
        anchors.rightMargin: 14

        clip: true
        spacing: 4

        scrollBarPolicy: ScrollBar.AsNeeded

        model: root.pagesModel.pageCount

        delegate: Item {
            id: pageItem

            readonly property bool isCurrent: index === root.pagesModel.currentPageIndex

            width: ListView.view.width
            height: 14 + thumbnail.height + 8 + numberLabel.height + 9

            Rectangle {
                anchors.fill: parent

                radius: 10
                color: pageItem.isCurrent ? AtStyle.soft
                                          : (mouseArea.containsMouse ? AtStyle.hover : "transparent")

                NavigationFocusBorder {
                    navigationCtrl: pageNav
                }
            }

            NavigationControl {
                id: pageNav

                name: "Page" + (index + 1)
                panel: navPanel
                row: index

                accessible.role: MUAccessible.ListItem
                //: The number of a page of the score
                accessible.name: qsTrc("appshell", "Page %1").arg(index + 1)

                onTriggered: {
                    root.pagesModel.showPage(index)
                }
            }

            //! NOTE: the edge of the paper
            Rectangle {
                anchors.fill: thumbnail
                anchors.margins: -1

                color: "transparent"
                border.width: 1
                border.color: AtStyle.stroke2
            }

            PageThumbnailView {
                id: thumbnail

                anchors.horizontalCenter: parent.horizontalCenter
                y: 14

                width: root.thumbnailWidth
                height: Math.round(root.thumbnailWidth / root.pagesModel.pageAspectRatio)

                pageIndex: index
            }

            StyledTextLabel {
                id: numberLabel

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: thumbnail.bottom
                anchors.topMargin: 8

                text: index + 1
                font: AtStyle.font(12, Font.DemiBold)
                color: pageItem.isCurrent ? AtStyle.ink : AtStyle.text2
            }

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                hoverEnabled: true

                onClicked: {
                    pageNav.requestActiveByInteraction()
                    root.pagesModel.showPage(index)
                }
            }
        }

        footer: Column {
            width: ListView.view.width

            Item {
                width: 1
                height: 18
            }

            AtSectionTitle {
                width: parent.width
                text: qsTrc("appshell", "Page")
            }

            Item {
                width: 1
                height: 4
            }

            Repeater {
                model: [
                    { title: qsTrc("appshell", "Paper"), value: root.pagesModel.paperTitle },
                    { title: qsTrc("appshell", "Printable width"), value: root.pagesModel.printableWidthTitle },
                    { title: qsTrc("appshell", "Staff space"), value: root.pagesModel.staffSpaceTitle }
                ]

                delegate: Item {
                    width: parent.width
                    height: 26

                    StyledTextLabel {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter

                        text: modelData.title
                        font: AtStyle.font(12.5)
                        color: AtStyle.text2
                    }

                    StyledTextLabel {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        text: modelData.value
                        font: AtStyle.font(12.5)
                    }
                }
            }

            Item {
                width: 1
                height: 14
            }
        }
    }

    Item {
        id: footer

        anchors.bottom: parent.bottom
        width: parent.width
        height: 51

        Rectangle {
            width: parent.width
            height: 1
            color: AtStyle.stroke
        }

        AtOutlineButton {
            x: 12
            y: 11
            width: parent.width - 24

            readonly property var item: actions.items.length > 0 ? actions.items[0] : null

            //! NOTE: the size of the paper is a part of the preview, so the settings can be opened from it
            text: Boolean(item) ? AtStyle.plainTitle(item.title) : ""

            navigation.name: "PageSettings"
            navigation.panel: footerNavPanel

            onClicked: {
                actions.trigger("page-settings")
            }
        }
    }
}
