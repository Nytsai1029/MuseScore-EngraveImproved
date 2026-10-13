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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Palette 1.0
import MuseScore.Atelier 1.0

import "internal"

//! NOTE: the palettes as a library (the Atelier skin): a rail with a button for each palette,
//! and beside it the name of the chosen palette, the search field and its cells
Item {
    id: root

    property NavigationSection navigationSection: null
    property int navigationOrderStart: 1

    readonly property PaletteProvider paletteProvider: paletteRootModel.paletteProvider
    readonly property var paletteModel: paletteTree.paletteModel

    //! NOTE: the row of the palette which is shown
    property int currentRow: -1

    readonly property bool isSearching: Boolean(searchField.searchText)

    readonly property string currentTitle: {
        if (root.isSearching) {
            return qsTrc("palette", "Search results")
        }

        return root.currentRow >= 0 && Boolean(root.paletteModel) ? paletteTree.paletteName(root.currentRow) : ""
    }

    function applyCurrentPaletteElement() {
        paletteTree.applyCurrentElement()
    }

    function paletteIndex(row) {
        return root.paletteModel.index(row, 0)
    }

    function isExpanded(row) {
        return Boolean(root.paletteModel.data(paletteIndex(row), PaletteTreeModel.PaletteExpandedRole))
    }

    //! NOTE: exactly one palette is shown
    function selectPalette(row) {
        if (!Boolean(root.paletteModel) || row < 0 || row >= root.paletteModel.rowCount()) {
            return
        }

        for (var idx = 0; idx < root.paletteModel.rowCount(); idx++) {
            if (idx !== row && isExpanded(idx)) {
                root.paletteModel.setData(paletteIndex(idx), false, PaletteTreeModel.PaletteExpandedRole)
            }
        }

        if (!isExpanded(row)) {
            root.paletteModel.setData(paletteIndex(row), true, PaletteTreeModel.PaletteExpandedRole)
        }

        root.currentRow = row
        paletteTree.currentIndex = row
        paletteTree.positionViewAtBeginning()
    }

    function selectPaletteByName(name) {
        if (Boolean(root.paletteModel)) {
            selectPalette(paletteTree.rowOfPalette(name))
        }
    }

    function syncCurrentRow() {
        if (!Boolean(root.paletteModel) || root.isSearching) {
            return
        }

        var count = root.paletteModel.rowCount()
        if (count === 0) {
            root.currentRow = -1
            return
        }

        var row = -1
        for (var idx = 0; idx < count; idx++) {
            if (isExpanded(idx)) {
                row = idx
                break
            }
        }

        selectPalette(row >= 0 ? row : Math.min(Math.max(root.currentRow, 0), count - 1))
    }

    Component.onCompleted: {
        Qt.callLater(root.syncCurrentRow)
    }

    Connections {
        target: root.paletteModel

        function onRowsInserted() { Qt.callLater(root.syncCurrentRow) }
        function onRowsRemoved() { Qt.callLater(root.syncCurrentRow) }
        function onModelReset() { Qt.callLater(root.syncCurrentRow) }
    }

    PaletteRootModel {
        id: paletteRootModel

        onPaletteSearchRequested: {
            searchField.forceActiveFocus()
        }

        onApplyCurrentPaletteElementRequested: {
            root.applyCurrentPaletteElement()
        }
    }

    // ---------- the rail ----------

    Rectangle {
        id: rail

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: AtStyle.railWidth

        color: AtStyle.island2

        NavigationPanel {
            id: railNavPanel
            name: "PaletteLibraryRail"
            section: root.navigationSection
            order: root.navigationOrderStart
            direction: NavigationPanel.Vertical
            enabled: root.enabled && root.visible
            accessible.name: qsTrc("palette", "Palettes")
        }

        function firstCellIcon(row) {
            if (!Boolean(root.paletteModel)) {
                return undefined
            }

            var index = root.paletteIndex(row)
            if (root.paletteModel.rowCount(index) === 0) {
                return undefined
            }

            return root.paletteModel.data(root.paletteModel.index(0, 0, index), Qt.DecorationRole)
        }

        StyledListView {
            id: railView

            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.bottom: addPalettesButton.top
            anchors.bottomMargin: 4
            width: parent.width

            spacing: 2
            scrollBarPolicy: ScrollBar.AlwaysOff

            model: paletteProvider ? paletteProvider.mainPaletteModel : null

            delegate: AtToolButton {
                id: railButton

                //! NOTE: the palettes without their own icon are represented by their first cell
                readonly property bool hasIcon: Boolean(model.paletteIcon) && model.paletteIcon !== IconCode.NONE
                readonly property var cellIcon: hasIcon ? undefined : rail.firstCellIcon(model.index)
                readonly property size cellSize: model.gridSize

                x: (ListView.view.width - width) / 2
                width: 36
                height: 36
                radius: 8

                icon: hasIcon ? model.paletteIcon : IconCode.NONE

                checked: !root.isSearching && model.index === root.currentRow
                checkedStyle: "soft"

                toolTipTitle: model.display

                navigation.name: model.display
                navigation.panel: railNavPanel
                navigation.row: model.index
                navigation.accessible.name: model.display

                onClicked: {
                    if (root.isSearching) {
                        //! NOTE: while searching, the rail shows only the palettes with found cells,
                        //! so the rows are not the rows of the whole list
                        var name = model.display
                        searchField.clear()
                        Qt.callLater(root.selectPaletteByName, name)
                        return
                    }

                    root.selectPalette(model.index)
                }

                IconView {
                    anchors.centerIn: parent
                    visible: !railButton.hasIcon && Boolean(railButton.cellIcon)

                    //! NOTE: the cell is painted in its own size, and then it is scaled down to fit the button
                    width: railButton.cellSize.width
                    height: railButton.cellSize.height
                    scale: Math.min(1, 22 / Math.max(width, 1), 22 / Math.max(height, 1))

                    icon: railButton.cellIcon
                }

                StyledTextLabel {
                    anchors.centerIn: parent
                    visible: !railButton.hasIcon && !Boolean(railButton.cellIcon)
                    font: AtStyle.font(13, Font.DemiBold)
                    color: railButton.checked ? AtStyle.ink : AtStyle.text2
                    text: model.display.charAt(0)
                }
            }
        }

        AtToolButton {
            id: addPalettesButton

            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            anchors.horizontalCenter: parent.horizontalCenter

            width: 36
            height: 32
            radius: 8

            shape: "dots"
            toolTipTitle: qsTrc("palette", "Add palettes")

            navigation.name: "AddPalettes"
            navigation.panel: railNavPanel
            navigation.row: 100000

            onClicked: {
                popups.toggle(addPalettesPopup, paletteProvider.availableExtraPalettesModel())
            }
        }

        Rectangle {
            anchors.right: parent.right
            width: 1
            height: parent.height
            color: AtStyle.stroke
        }
    }

    QtObject {
        id: popups

        property var opened: null

        function toggle(popup, model) {
            var wasThis = opened === popup && popup.isOpened

            if (Boolean(opened) && opened.isOpened) {
                opened.close()
            }

            opened = null

            if (wasThis) {
                return
            }

            if (Boolean(model)) {
                popup.model = model
            }

            opened = popup
            popup.open()
        }
    }

    AddPalettesPopup {
        id: addPalettesPopup

        paletteProvider: root.paletteProvider

        popupAvailableWidth: panel.width
        maxHeight: panel.height - header.height
        anchorItem: panel

        onAddCustomPaletteRequested: {
            popups.toggle(createCustomPalettePopup)
        }
    }

    CreateCustomPalettePopup {
        id: createCustomPalettePopup

        popupAvailableWidth: panel.width
        anchorItem: panel

        onAddCustomPaletteRequested: function(paletteName) {
            paletteTree.insertCustomPalette(0, paletteName)
            Qt.callLater(root.selectPalette, 0)
        }
    }

    // ---------- the chosen palette ----------

    Item {
        id: panel

        anchors.left: rail.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        NavigationPanel {
            id: headerNavPanel
            name: "PaletteLibraryHeader"
            section: root.navigationSection
            order: root.navigationOrderStart + 1
            direction: NavigationPanel.Horizontal
            enabled: root.enabled && root.visible
        }

        AtPaneHeader {
            id: header

            width: parent.width

            title: root.currentTitle
            leftPadding: 14

            AtToolButton {
                id: optionsButton

                width: 26
                height: 26

                shape: "dots"
                visible: !root.isSearching && root.currentRow >= 0
                checked: optionsMenu.isMenuOpened
                checkedStyle: "soft"

                toolTipTitle: qsTrc("palette", "Palette options")

                navigation.name: "PaletteOptions"
                navigation.panel: headerNavPanel
                navigation.order: 1

                readonly property bool isCustom: root.currentRow >= 0 && Boolean(root.paletteModel)
                                                 && Boolean(root.paletteModel.data(root.paletteIndex(root.currentRow),
                                                                                   PaletteTreeModel.CustomRole))

                readonly property bool isEditingEnabled: root.currentRow >= 0 && Boolean(root.paletteModel)
                                                         && Boolean(root.paletteModel.data(root.paletteIndex(root.currentRow),
                                                                                           PaletteTreeModel.EditableRole))

                onClicked: {
                    optionsMenu.toggleOpened([
                        { id: "hide", title: isCustom ? qsTrc("palette", "Hide/Delete palette") : qsTrc("palette", "Hide palette") },
                        { id: "new", title: qsTrc("palette", "Insert new palette") },
                        { id: "", title: "" },
                        { id: "edit", title: qsTrc("palette", "Enable editing"), checkable: true, checked: isEditingEnabled },
                        { id: "", title: "" },
                        { id: "reset", title: qsTrc("palette", "Reset palette") },
                        { id: "save", title: qsTrc("palette", "Save palette…") },
                        { id: "load", title: qsTrc("palette", "Load palette…") },
                        { id: "", title: "" },
                        { id: "properties", title: qsTrc("palette", "Palette properties…") }
                    ])
                }

                StyledMenuLoader {
                    id: optionsMenu

                    onHandleMenuItem: function(itemId) {
                        var row = root.currentRow
                        var index = root.paletteIndex(row)

                        switch (itemId) {
                        case "hide": paletteTree.hidePaletteAtRow(row); break
                        case "new":
                            paletteTree.insertCustomPalette(row, qsTrc("palette", "Untitled palette"))
                            Qt.callLater(root.selectPalette, row)
                            break
                        case "edit":
                            root.paletteModel.setData(index, !optionsButton.isEditingEnabled, PaletteTreeModel.EditableRole)
                            break
                        case "reset": root.paletteProvider.resetPalette(index); break
                        case "save": root.paletteProvider.savePalette(index); break
                        case "load": root.paletteProvider.loadPalette(index); break
                        case "properties": Qt.callLater(paletteTree.paletteController.editPaletteProperties, index); break
                        }
                    }
                }
            }
        }

        SearchField {
            id: searchField

            anchors.top: header.bottom
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.right: parent.right
            anchors.rightMargin: 12

            height: 28

            hint: qsTrc("palette", "Search symbols")

            background.color: AtStyle.button
            background.border.color: "transparent"

            navigation.name: "SearchSymbols"
            navigation.panel: headerNavPanel
            navigation.order: 2

            Keys.onEscapePressed: {
                searchField.clear()
            }

            onAccepted: {
                root.applyCurrentPaletteElement()
            }
        }

        StyledTextLabel {
            anchors.top: searchField.bottom
            anchors.topMargin: 16
            anchors.left: parent.left
            anchors.leftMargin: 14
            anchors.right: parent.right
            anchors.rightMargin: 14

            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
            color: AtStyle.text2

            visible: root.isSearching && !paletteTree.isResultFound
            text: qsTrc("global", "No results found")
        }

        PaletteTree {
            id: paletteTree

            anchors.top: searchField.bottom
            anchors.topMargin: 10
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.leftMargin: 9
            anchors.right: parent.right
            anchors.rightMargin: 9

            clip: true
            paletteProvider: root.paletteProvider

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 2

            toolboxMode: true
            libraryMode: true
            libraryTileColor: AtStyle.cell

            filter: searchField.searchText
            searchOpened: root.isSearching
            enableAnimations: false

            onSearchOpenedChanged: {
                if (!searchOpened) {
                    Qt.callLater(root.syncCurrentRow)
                }
            }
        }
    }
}
