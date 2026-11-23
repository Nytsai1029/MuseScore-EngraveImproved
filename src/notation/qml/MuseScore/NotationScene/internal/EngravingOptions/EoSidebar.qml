/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
import MuseScore.Atelier 1.0

//! NOTE: the sidebar of the Engraving Options dialog: the search, the "changed only" switch
//! and the list of the pages with the sections of the chosen one
Rectangle {
    id: root

    required property var pages
    required property int currentPageIndex
    required property int currentSectionIndex
    required property var pageShownCounts
    required property var pageModifiedCounts
    required property int modifiedCount
    required property bool modifiedOnly
    required property bool isFiltered
    required property var navigationPanel

    readonly property string searchText: searchField.searchText

    signal pageActivated(int pageIndex)
    signal sectionActivated(int sectionIndex)
    signal modifiedOnlyToggled()

    color: AtStyle.island2

    component ListLine: FlatButton {
        id: line

        property bool isCurrent: false
        property bool isSection: false

        //! NOTE: a number at the right end; `isBadgeAccented`: it counts changed values
        property string badge: ""
        property bool isBadgeAccented: false

        height: isSection ? 26 : 30

        transparent: true
        isClickOnKeyNavTriggered: true

        backgroundItem: Rectangle {
            radius: 7

            color: {
                if (line.isCurrent && !line.isSection) {
                    return AtStyle.soft
                }

                if (line.mouseArea.pressed) {
                    return AtStyle.pressed
                }

                return line.mouseArea.containsMouse ? AtStyle.hover : "transparent"
            }

            NavigationFocusBorder {
                navigationCtrl: line.navigation
            }
        }

        contentItem: Item {
            implicitWidth: 0
            implicitHeight: 0
        }

        //! NOTE: the mark of the section which is at the top of the page
        Rectangle {
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.verticalCenter: parent.verticalCenter

            width: 2
            height: 14
            radius: 1

            visible: line.isSection
            color: line.isCurrent ? AtStyle.accent : AtStyle.stroke
        }

        StyledTextLabel {
            anchors.left: parent.left
            anchors.leftMargin: line.isSection ? 24 : 10
            anchors.right: badgeItem.visible ? badgeItem.left : parent.right
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter

            horizontalAlignment: Text.AlignLeft

            text: line.text
            font: line.isSection ? AtStyle.font(12, line.isCurrent ? Font.Medium : Font.Normal)
                                 : AtStyle.font(13, line.isCurrent ? Font.DemiBold : Font.Normal)
            color: {
                if (line.isSection) {
                    return line.isCurrent ? AtStyle.text : AtStyle.text2
                }

                return line.isCurrent ? AtStyle.ink : AtStyle.text
            }
        }

        Rectangle {
            id: badgeItem

            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter

            width: Math.max(20, badgeLabel.implicitWidth + 10)
            height: 18
            radius: 9

            visible: Boolean(line.badge)
            color: line.isBadgeAccented ? AtStyle.soft : "transparent"

            StyledTextLabel {
                id: badgeLabel

                anchors.centerIn: parent

                text: line.badge
                font: AtStyle.font(11, Font.Medium)
                color: line.isBadgeAccented ? AtStyle.ink : AtStyle.text3
            }
        }
    }


    //! NOTE: the line of a page is brought into view when it becomes the current one
    function revealInList(pageLines) {
        Qt.callLater(function() {
            var top = listColumn.y + pageLines.y
            var bottom = top + pageLines.height
            var y = listFlickable.contentY

            if (bottom + 8 > y + listFlickable.height) {
                y = bottom + 8 - listFlickable.height
            }

            if (top - 8 < y) {
                y = top - 8
            }

            listFlickable.contentY = Math.max(0, Math.min(y, listFlickable.contentHeight - listFlickable.height))
        })
    }

        SearchField {
            id: searchField

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 12

            hint: qsTrc("notation/engravingoptions", "Search options")

            navigation.panel: root.navigationPanel
            navigation.row: 1
            navigation.name: "Search"
        }

        AtSwitchRow {
            id: modifiedOnlyRow

            anchors.left: parent.left
            anchors.leftMargin: 14
            anchors.right: parent.right
            anchors.rightMargin: 14
            anchors.top: searchField.bottom
            anchors.topMargin: 8

            text: root.modifiedCount > 0
                  //: %1 is the number of options which differ from their defaults
                  ? qsTrc("notation/engravingoptions", "Changed only (%1)").arg(root.modifiedCount)
                  : qsTrc("notation/engravingoptions", "Changed only")
            font: AtStyle.font(12.5)

            checked: root.modifiedOnly

            navigation.panel: root.navigationPanel
            navigation.row: 2
            navigation.name: "ChangedOnly"

            onToggled: {
                root.modifiedOnlyToggled()
            }
        }

        Rectangle {
            id: listTopLine

            anchors.top: modifiedOnlyRow.bottom
            anchors.topMargin: 8
            width: parent.width
            height: 1

            color: AtStyle.stroke
        }

        StyledFlickable {
            id: listFlickable

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: listTopLine.bottom
            anchors.bottom: parent.bottom

            contentWidth: width
            contentHeight: listColumn.y + listColumn.height + 12

            Column {
                id: listColumn

                x: 8
                y: 6
                width: parent.width - 16

                Repeater {
                    model: root.pages

                    delegate: Column {
                        id: pageLines

                        readonly property var page: modelData
                        readonly property int pageIndex: index

                        readonly property bool isCurrent: !root.isFiltered && root.currentPageIndex === pageIndex
                        readonly property int shownCount: root.pageShownCounts[pageIndex]
                        readonly property int changedCount: root.pageModifiedCounts[pageIndex]

                        readonly property bool isShown: !root.isFiltered || shownCount > 0

                        //! NOTE: the title of the group stands above the first page of the group which is shown
                        readonly property bool hasGroupTitle: {
                            if (!isShown || !Boolean(page.group)) {
                                return false
                            }

                            for (var i = pageIndex - 1; i >= 0; --i) {
                                var isOtherShown = !root.isFiltered || root.pageShownCounts[i] > 0
                                if (isOtherShown) {
                                    return root.pages[i].group !== page.group
                                }
                            }

                            return true
                        }

                        width: listColumn.width
                        visible: isShown

                        onIsCurrentChanged: {
                            if (isCurrent) {
                                root.revealInList(pageLines)
                            }
                        }

                        Item {
                            width: parent.width
                            height: 30

                            visible: pageLines.hasGroupTitle

                            AtSectionTitle {
                                anchors.left: parent.left
                                anchors.leftMargin: 10
                                anchors.bottom: parent.bottom
                                anchors.bottomMargin: 5

                                text: pageLines.page.group
                                color: AtStyle.text3
                            }
                        }

                        ListLine {
                            width: parent.width

                            text: pageLines.page.title
                            isCurrent: pageLines.isCurrent

                            badge: {
                                if (root.isFiltered) {
                                    return String(pageLines.shownCount)
                                }

                                return pageLines.changedCount > 0 ? String(pageLines.changedCount) : ""
                            }
                            isBadgeAccented: !root.isFiltered

                            navigation.panel: root.navigationPanel
                            navigation.row: 10 + pageLines.pageIndex * 100
                            navigation.name: "Page_" + pageLines.page.code
                            navigation.accessible.name: pageLines.page.title

                            onClicked: {
                                root.pageActivated(pageLines.pageIndex)
                            }
                        }

                        //! NOTE: the sections of the chosen page, to go to one of them
                        Repeater {
                            model: pageLines.isCurrent && !Boolean(pageLines.page.view) && pageLines.page.sections.length > 1
                                   ? pageLines.page.sections : []

                            delegate: ListLine {
                                width: pageLines.width

                                visible: modelData.visible && Boolean(modelData.title)

                                isSection: true
                                text: modelData.title
                                isCurrent: root.currentSectionIndex === index

                                navigation.panel: root.navigationPanel
                                navigation.row: 10 + pageLines.pageIndex * 100 + 1 + index
                                navigation.name: "Section_" + pageLines.page.code + "_" + index
                                navigation.accessible.name: modelData.title

                                onClicked: {
                                    root.sectionActivated(index)
                                }
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            anchors.right: parent.right
            width: 1
            height: parent.height

            color: AtStyle.stroke
        }
}
