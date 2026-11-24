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

    contentWidth: 1320
    contentHeight: 760
    resizable: true
    modal: true

    readonly property var pages: pagesList.pages

    //! NOTE: the widths of the side columns while the dialog is open: from the state at first,
    //! moved live by the handles and written back to the state when a drag ends;
    //! a stored width which does not fit the window is clamped where it is used
    property int sidebarWidth: dialogState.sidebarWidth
    property int previewWidth: dialogState.previewWidth
    readonly property real sidebarWidthShown:
        Math.max(200, Math.min(root.width - previewWidthShown - 420, sidebarWidth))
    readonly property real previewWidthShown:
        Math.max(300, Math.min(root.width - sidebarWidth - 420, previewWidth))

    property int currentPageIndex: 0
    readonly property var currentPage: pages[currentPageIndex]

    //! NOTE: each page begins with its first option in the preview pane
    onCurrentPageChanged: {
        eoSession.begin(currentPage)
    }

    readonly property var searchWords: EoFilter.words(sidebar.searchText)
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
    function openPage(pageIndex) {
        if (pageIndex < 0 || pageIndex >= pages.length) {
            return
        }

        if (root.isFiltered) {
            resultsLoader.item.scrollToPage(pageIndex)
            return
        }

        root.currentPageIndex = pageIndex
        root.currentSubPageCode = ""
        dialogState.lastPageCode = pages[pageIndex].code
    }

    //! NOTE: a page which a link of another page asks for, with what it is to show
    Connections {
        target: eoSession

        function onPageRequested(pageCode, subPageCode) {
            root.openPage(pagesList.indexOfCode(pageCode))
            root.currentSubPageCode = subPageCode
        }
    }

    Component.onCompleted: {
        eoModel.load()

        var pageIndex = pagesList.indexOfCode(root.currentPageCode)
        if (pageIndex < 0) {
            pageIndex = pagesList.indexOfCode(dialogState.lastPageCode)
        }

        root.currentPageIndex = Math.max(0, pageIndex)
        dialogState.lastPageCode = pages[root.currentPageIndex].code
    }

    //! NOTE: closed in any other way than by "OK" (the close button of the window, Esc): as by "Cancel"
    onClosed: {
        eoModel.reject()
    }

    EngravingOptionsModel {
        id: eoModel
    }

    EoSession {
        id: eoSession
    }

    //! NOTE: how the dialog was left — its page, its handle switch and the widths of its side columns
    EngravingOptionsDialogState {
        id: dialogState
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

    // ----- the list of the pages -----

    EoSidebar {
        id: sidebar

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: footer.top

        width: root.sidebarWidthShown

        pages: root.pages
        currentPageIndex: root.currentPageIndex
        currentSectionIndex: Boolean(pageLoader.item) ? pageLoader.item.currentSectionIndex : -1
        pageShownCounts: root.pageShownCounts
        pageModifiedCounts: root.pageModifiedCounts
        modifiedCount: root.modifiedCount
        modifiedOnly: root.modifiedOnly
        isFiltered: root.isFiltered
        navigationPanel: listNavPanel

        onPageActivated: function(pageIndex) {
            root.openPage(pageIndex)
        }
        onSectionActivated: function(sectionIndex) {
            pageLoader.item.scrollToSection(sectionIndex)
        }
        onModifiedOnlyToggled: {
            root.modifiedOnly = !root.modifiedOnly
        }
    }
    //! NOTE: the width of the sidebar is dragged between its smallest and largest room;
    //! a double click gives it back its default width
    EoSplitHandle {
        anchors.top: parent.top
        anchors.bottom: footer.top
        x: sidebar.width - width / 2

        value: root.sidebarWidth
        minimum: 200
        maximum: Math.min(360, root.width - previewWidthShown - 420)
        defaultValue: dialogState.defaultSidebarWidth

        onMoved: function(width) {
            root.sidebarWidth = Math.round(width)
        }
        onDropped: {
            dialogState.sidebarWidth = root.sidebarWidth
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

        EoPreviewPane {
            id: previewPane

            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            width: root.previewWidthShown

            visible: eoModel.isLoaded && (root.isFiltered || root.currentPage.hasPreview)

            session: eoSession
            optionsModel: eoModel
            dialogState: dialogState

            navigationPanel: pageNavPanel
        }

        Loader {
            id: pageLoader

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: previewPane.visible ? previewPane.left : parent.right

            active: eoModel.isLoaded && !root.isFiltered
            visible: active

            sourceComponent: Boolean(root.currentPage.view) ? customPageComp : sectionsPageComp
        }

        //! NOTE: the width of the preview pane is dragged like the sidebar's;
        //! the options always keep 420 px of room
        EoSplitHandle {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            x: previewPane.x - width / 2

            inverted: true

            value: root.previewWidth
            minimum: 300
            maximum: Math.min(760, root.width - sidebarWidthShown - 420)
            defaultValue: dialogState.defaultPreviewWidth

            visible: previewPane.visible

            onMoved: function(width) {
                root.previewWidth = Math.round(width)
            }
            onDropped: {
                dialogState.previewWidth = root.previewWidth
            }
        }

        Loader {
            id: resultsLoader

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: previewPane.visible ? previewPane.left : parent.right

            active: eoModel.isLoaded && root.isFiltered
            visible: active

            sourceComponent: EoResultsView {
                pages: root.pages
                shownCounts: root.pageShownCounts
                optionsModel: eoModel
                session: eoSession

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
            session: eoSession

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

                if (item.hasOwnProperty("session")) {
                    item.session = eoSession
                }

                if (item.hasOwnProperty("subPageCode")) {
                    item.subPageCode = Qt.binding(function() { return root.currentSubPageCode })
                }
            }
        }
    }

    // ----- the buttons of the dialog -----

    EoFooter {
        id: footer

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        optionsModel: eoModel
        modifiedCount: root.modifiedCount
        navigationPanel: buttonsNavPanel

        onAcceptClicked: {
            root.accept()
        }
        onCancelClicked: {
            root.cancel()
        }
    }}
