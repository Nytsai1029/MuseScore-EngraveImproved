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
import MuseScore.NotationScene 1.0
import MuseScore.Atelier 1.0

//! NOTE: the options of all the pages which a search finds, or which are changed, page by page
StyledFlickable {
    id: root

    //! NOTE: the EoPage objects
    required property var pages

    //! NOTE: how many options of each page are found, in the order of the pages
    required property var shownCounts

    //! NOTE: the EngravingOptionsModel
    required property var optionsModel

    property var searchWords: []
    property bool modifiedOnly: false

    property var session: null

    property NavigationPanel navigationPanel: null

    readonly property int sideMargin: 28
    readonly property int maxContentWidth: 780

    function scrollToPage(pageIndex) {
        var item = pagesRepeater.itemAt(pageIndex)
        if (!Boolean(item) || !item.visible) {
            return
        }

        var maxContentY = Math.max(0, contentHeight - height)
        contentY = Math.min(pagesColumn.y + item.y - 8, maxContentY)
    }

    contentWidth: width
    contentHeight: pagesColumn.y + pagesColumn.height + 32

    Column {
        id: pagesColumn

        x: root.sideMargin
        y: 8
        width: Math.min(root.width - 2 * root.sideMargin, root.maxContentWidth)

        spacing: 36

        Repeater {
            id: pagesRepeater

            model: root.pages

            delegate: Column {
                id: pageItem

                readonly property var page: modelData
                readonly property int pageIndex: index
                readonly property int shownCount: root.shownCounts[index] === undefined ? 0 : root.shownCounts[index]

                width: pagesColumn.width
                visible: shownCount > 0
                spacing: 14

                Row {
                    spacing: 8

                    StyledTextLabel {
                        anchors.baseline: pageTitleLabel.baseline

                        visible: Boolean(pageItem.page.group)
                        text: pageItem.page.group
                        font: AtStyle.font(13)
                        color: AtStyle.text3
                    }

                    StyledTextLabel {
                        anchors.baseline: pageTitleLabel.baseline

                        visible: Boolean(pageItem.page.group)
                        text: "›"
                        font: AtStyle.font(13)
                        color: AtStyle.text3
                    }

                    StyledTextLabel {
                        id: pageTitleLabel

                        text: pageItem.page.title
                        font: AtStyle.font(18, Font.DemiBold)
                    }
                }

                //! NOTE: the options of a page are made when the page has something to show
                Loader {
                    width: parent.width

                    active: pageItem.shownCount > 0

                    sourceComponent: EoPageContent {
                        page: pageItem.page
                        optionsModel: root.optionsModel
                        session: root.session

                        searchWords: root.searchWords
                        modifiedOnly: root.modifiedOnly

                        navigationPanel: root.navigationPanel
                        navigationRowStart: (pageItem.pageIndex + 1) * 100000
                    }
                }
            }
        }
    }
}
