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

import "internal/EngravingOptions"
import "internal/EngravingOptions/EoFilter.js" as EoFilter

//! NOTE: the options of the engraving (the style of the score) in the Atelier skin: the pages in a list on the left,
//! the options of the chosen page on the right, each with what it does shown by the engraving engine.
//! A change is seen in the score at once; "Cancel" takes all the changes back, "OK" keeps them as one step
//! of the undo history.
//! The pages are descriptions (see EoPage and EoPages); the dialog shows them, searches them and counts
//! their changed values
StyledDialogView {
    id: root

    //! NOTE: the page to open, by its code or by the code of a page of the style dialog; the last one if empty
    property string currentPageCode: ""
    property string currentSubPageCode: ""

    objectName: "EngravingOptionsDialog"

    title: qsTrc("notation/engravingoptions", "Engraving options")

    contentWidth: 1160
    contentHeight: 760
    resizable: true
    modal: true

    readonly property var pages: pagesList.pages
    property int currentPageIndex: 0
    readonly property var currentPage: pages[currentPageIndex]

    readonly property var searchWords: EoFilter.words(searchField.searchText)
    property bool modifiedOnly: false
    readonly property bool isFiltered: searchWords.length > 0 || modifiedOnly

    //! NOTE: the names of the style values of each page, in the order of the pages
    readonly property var pageSids: {
        var result = []
        for (var i = 0; i < pages.length; ++i) {
            result.push(EoFilter.pageSids(pages[i]))
        }
        return result
    }

    //! NOTE: how many values of each page differ from their defaults
    readonly property var pageModifiedCounts: {
        eoModel.revision

        var result = []
        for (var i = 0; i < pages.length; ++i) {
            result.push(eoModel.isLoaded ? eoModel.modifiedCount(pageSids[i]) : 0)
        }
        return result
    }

    readonly property int modifiedCount: {
        var count = 0
        for (var i = 0; i < pageModifiedCounts.length; ++i) {
            count += pageModifiedCounts[i]
        }
        return count
    }

    //! NOTE: how many options of each page the search finds; all of them are found when nothing is searched
    readonly property var pageShownCounts: {
        eoModel.revision

        var result = []
        for (var i = 0; i < pages.length; ++i) {
            result.push(isFiltered && eoModel.isLoaded
                        ? EoFilter.shownOptionCountOfPage(pages[i], searchWords, modifiedOnly, eoModel)
                        : -1)
        }
        return result
    }

    readonly property int shownCount: {
        var count = 0
        for (var i = 0; i < pageShownCounts.length; ++i) {
            count += Math.max(0, pageShownCounts[i])
        }
        return count
    }

    function accept() {
        eoModel.accept()
        root.hide()
    }

    function cancel() {
        eoModel.reject()
        root.hide()
    }

    //! NOTE: scrolls the list of the pages so that the lines of the chosen page, with its sections, are seen
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

    function openPage(pageIndex) {
        if (pageIndex < 0 || pageIndex >= pages.length) {
            return
        }

        if (root.isFiltered) {
            resultsLoader.item.scrollToPage(pageIndex)
            return
        }

        root.currentPageIndex = pageIndex
        eoModel.lastPageCode = pages[pageIndex].code
    }

    Component.onCompleted: {
        eoModel.load()

        var pageIndex = pagesList.indexOfCode(root.currentPageCode)
        if (pageIndex < 0) {
            pageIndex = pagesList.indexOfCode(eoModel.lastPageCode)
        }

        root.currentPageIndex = Math.max(0, pageIndex)
        eoModel.lastPageCode = pages[root.currentPageIndex].code
    }

    //! NOTE: closed in any other way than by "OK" (the close button of the window, Esc): as by "Cancel"
    onClosed: {
        eoModel.reject()
    }

    EngravingOptionsModel {
        id: eoModel
    }

    EoPages {
        id: pagesList

        model: eoModel
    }

    // ----- the keyboard navigation: the list, the page, the buttons -----

    NavigationPanel {
        id: listNavPanel
        name: "EngravingOptionsPages"
        section: root.navigationSection
        order: 1
        direction: NavigationPanel.Vertical
        accessible.name: qsTrc("notation/engravingoptions", "Pages")
    }

    NavigationPanel {
        id: pageNavPanel
        name: "EngravingOptionsPage"
        section: root.navigationSection
        order: 2
        direction: NavigationPanel.Both
        accessible.name: root.isFiltered ? resultsTitle.text : root.currentPage.title
    }

    NavigationPanel {
        id: buttonsNavPanel
        name: "EngravingOptionsButtons"
        section: root.navigationSection
        order: 3
        direction: NavigationPanel.Horizontal
        accessible.name: qsTrc("notation/engravingoptions", "Dialog buttons")
    }

    // ----- a line of the list: a page, or a section of the chosen page -----

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

    // ----- the list of the pages -----

    Rectangle {
        id: sidebar

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: footer.top

        width: 272
        color: AtStyle.island2

        SearchField {
            id: searchField

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 12

            hint: qsTrc("notation/engravingoptions", "Search options")

            navigation.panel: listNavPanel
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

            navigation.panel: listNavPanel
            navigation.row: 2
            navigation.name: "ChangedOnly"

            onToggled: {
                root.modifiedOnly = !root.modifiedOnly
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

                            navigation.panel: listNavPanel
                            navigation.row: 10 + pageLines.pageIndex * 100
                            navigation.name: "Page_" + pageLines.page.code
                            navigation.accessible.name: pageLines.page.title

                            onClicked: {
                                root.openPage(pageLines.pageIndex)
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
                                isCurrent: Boolean(pageLoader.item) && pageLoader.item.currentSectionIndex === index

                                navigation.panel: listNavPanel
                                navigation.row: 10 + pageLines.pageIndex * 100 + 1 + index
                                navigation.name: "Section_" + pageLines.page.code + "_" + index
                                navigation.accessible.name: modelData.title

                                onClicked: {
                                    pageLoader.item.scrollToSection(index)
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

    // ----- the title of what is shown -----

    Item {
        id: header

        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.top: parent.top

        height: 76

        Column {
            anchors.left: parent.left
            anchors.leftMargin: 28
            anchors.right: headerButtons.left
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter

            spacing: 3

            StyledTextLabel {
                width: parent.width
                horizontalAlignment: Text.AlignLeft

                text: {
                    if (!root.isFiltered) {
                        return root.currentPage.group
                    }

                    //: %n is the number of options which a search has found
                    return qsTrc("notation/engravingoptions", "%n option(s)", "", root.shownCount)
                }
                font: AtStyle.font(12)
                color: AtStyle.text3
            }

            StyledTextLabel {
                id: resultsTitle

                width: parent.width
                horizontalAlignment: Text.AlignLeft

                text: {
                    if (!root.isFiltered) {
                        return root.currentPage.title
                    }

                    return root.searchWords.length > 0 ? qsTrc("notation/engravingoptions", "Search results")
                                                       : qsTrc("notation/engravingoptions", "Changed options")
                }
                font: AtStyle.font(21, Font.DemiBold)
            }
        }

        Row {
            id: headerButtons

            anchors.right: parent.right
            anchors.rightMargin: 24
            anchors.verticalCenter: parent.verticalCenter

            AtToolButton {
                visible: !root.isFiltered && root.pageModifiedCounts[root.currentPageIndex] > 0

                shape: "reset"
                text: qsTrc("notation/engravingoptions", "Reset page")
                textFont: AtStyle.font(12.5, Font.Medium)

                navigation.panel: pageNavPanel
                navigation.row: 0
                navigation.column: 1000
                navigation.name: "ResetPage"

                onClicked: {
                    eoModel.resetValues(root.pageSids[root.currentPageIndex])
                }
            }
        }

        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            height: 1
            color: AtStyle.stroke
        }
    }

    // ----- the page, or what is found -----

    Item {
        id: body

        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: footer.top

        clip: true

        Loader {
            id: pageLoader

            anchors.fill: parent

            active: eoModel.isLoaded && !root.isFiltered
            visible: active

            sourceComponent: Boolean(root.currentPage.view) ? customPageComp : sectionsPageComp
        }

        Loader {
            id: resultsLoader

            anchors.fill: parent

            active: eoModel.isLoaded && root.isFiltered
            visible: active

            sourceComponent: EoResultsView {
                pages: root.pages
                shownCounts: root.pageShownCounts
                optionsModel: eoModel

                searchWords: root.searchWords
                modifiedOnly: root.modifiedOnly

                navigationPanel: pageNavPanel
            }
        }

        StyledTextLabel {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 72

            visible: root.isFiltered && root.shownCount === 0

            text: root.searchWords.length > 0 ? qsTrc("notation/engravingoptions", "No option matches the search")
                                              : qsTrc("notation/engravingoptions", "No option is changed")
            font: AtStyle.font(13)
            color: AtStyle.text3
        }
    }

    Component {
        id: sectionsPageComp

        EoPageView {
            page: root.currentPage
            optionsModel: eoModel

            navigationPanel: pageNavPanel
        }
    }

    Component {
        id: customPageComp

        //! NOTE: a page with a view of its own
        Loader {
            readonly property int currentSectionIndex: 0

            function scrollToSection(sectionIndex) {
            }

            sourceComponent: root.currentPage.view

            onLoaded: {
                if (item.hasOwnProperty("page")) {
                    item.page = Qt.binding(function() { return root.currentPage })
                }

                if (item.hasOwnProperty("optionsModel")) {
                    item.optionsModel = eoModel
                }

                if (item.hasOwnProperty("navigationPanel")) {
                    item.navigationPanel = pageNavPanel
                }

                if (item.hasOwnProperty("subPageCode")) {
                    item.subPageCode = root.currentSubPageCode
                }
            }
        }
    }

    // ----- the buttons of the dialog -----

    Rectangle {
        id: footer

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        height: 56
        color: AtStyle.island

        Rectangle {
            width: parent.width
            height: 1

            color: AtStyle.stroke
        }

        Row {
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter

            spacing: 8

            //! NOTE: all the options are reset with the second click, so that it is not done by mistake
            AtOutlineButton {
                id: resetAllButton

                property bool isArmed: false

                enabled: root.modifiedCount > 0 || isArmed

                text: isArmed ? qsTrc("notation/engravingoptions", "Click again to reset all options")
                              : qsTrc("notation/engravingoptions", "Reset all to defaults")
                toolTipTitle: qsTrc("notation/engravingoptions",
                                    "Sets all the options to their defaults, except the page size and margins and the staff size")

                navigation.panel: buttonsNavPanel
                navigation.column: 1
                navigation.name: "ResetAll"

                onClicked: {
                    if (isArmed) {
                        isArmed = false
                        eoModel.resetAllValues()
                        return
                    }

                    isArmed = true
                    disarmTimer.restart()
                }

                Timer {
                    id: disarmTimer

                    interval: 4000

                    onTriggered: {
                        resetAllButton.isArmed = false
                    }
                }
            }

            AtOutlineButton {
                visible: eoModel.canApplyToAllParts

                text: qsTrc("notation", "Apply to all parts")

                navigation.panel: buttonsNavPanel
                navigation.column: 2
                navigation.name: "ApplyToAllParts"

                onClicked: {
                    eoModel.applyToAllParts()
                }
            }
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter

            spacing: 8

            AtOutlineButton {
                width: Math.max(96, implicitWidth)

                text: qsTrc("global", "Cancel")

                navigation.panel: buttonsNavPanel
                navigation.column: 3
                navigation.name: "Cancel"

                onClicked: {
                    root.cancel()
                }
            }

            FlatButton {
                height: 28
                minWidth: 96
                margins: 16

                text: qsTrc("global", "OK")
                accentButton: true
                backgroundRadius: 7
                textFont: AtStyle.font(13, Font.Medium)

                navigation.panel: buttonsNavPanel
                navigation.column: 4
                navigation.name: "OK"

                onClicked: {
                    root.accept()
                }
            }
        }
    }
}
