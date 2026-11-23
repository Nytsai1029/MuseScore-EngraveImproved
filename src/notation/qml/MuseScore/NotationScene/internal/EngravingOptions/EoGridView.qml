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

import "EoFilter.js" as EoFilter
import "EoMusic.js" as EoMusic
import "EoTitles.js" as EoTitles

//! NOTE: an EoGrid as the dialog shows it: a line with the title of the grid, then the table —
//! the titles of the columns over it, the titles of the rows on the left, a small editor in each
//! cell and a dot for a cell which is changed. When the search leaves only some of the cells,
//! the ones which do not match fade. When the cells are wider than the room, the table scrolls
//! sideways while the titles of the rows stay
FocusScope {
    id: root

    //! NOTE: the EoGrid, the EoSection and the EoPage it is in
    required property var option
    property var section: null
    property var page: null

    //! NOTE: the EoSession: the cell under the pointer or with the focus is the one which the preview pane shows
    property var session: null

    //! NOTE: the EngravingOptionsModel
    required property var optionsModel

    //! NOTE: the filter of the dialog; the cells which do not match fade to a third of their opacity
    property var searchWords: []
    property bool modifiedOnly: false

    property NavigationPanel navigationPanel: null

    //! NOTE: the first navigation row of the block the grid has; each row of the table uses one
    property int navigationRow: 0

    readonly property var grid: option
    readonly property int columnCount: Math.max(1, grid.columnTitles.length)
    readonly property int rowCount: Math.max(1, Math.ceil(grid.cells.length / columnCount))

    readonly property bool isFiltered: searchWords.length > 0 || modifiedOnly

    //! NOTE: the smallest width a control asks for in a cell of its kind
    function minWidthOf(cell) {
        switch (cell.kind) {
        case "number": return grid.compact ? 56 : 76
        case "offset": return 150
        case "toggle": return 44
        case "choice": return 110
        case "text": return 120
        case "color": return 56
        case "font": return 140
        }
        return 80
    }

    function cellAt(row, column) {
        var index = row * columnCount + column
        return index < grid.cells.length ? grid.cells[index] : null
    }

    function isCellShown(cell) {
        if (!isFiltered) {
            return true
        }
        optionsModel.revision
        return EoFilter.isOptionShown(cell, root.section, root.page, root.searchWords, root.modifiedOnly,
                                      root.optionsModel)
    }

    function cellSids(cell) {
        var result = Boolean(cell.sid) ? [cell.sid] : []
        return result.concat(cell.extraSids)
    }

    function isCellModified(cell) {
        optionsModel.revision
        var sids = cellSids(cell)
        return sids.length > 0 && optionsModel.modifiedCount(sids) > 0
    }

    implicitHeight: content.implicitHeight
    height: implicitHeight

    enabled: option.enabled
    opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled

    //! NOTE: the height of a row of the table: the tallest editor of its cells, plus the air around it
    function rowHeightOf(row) {
        var height = 30
        for (var column = 0; column < columnCount; ++column) {
            var cell = cellAt(row, column)
            if (Boolean(cell) && cell.kind === "text" && cell.multiline) {
                height = Math.max(height, 48)
            }
        }
        return height + 6
    }

    readonly property var rowHeights: {
        var result = []
        for (var row = 0; row < rowCount; ++row) {
            result.push(rowHeightOf(row))
        }
        return result
    }
    readonly property int tableHeight: rowHeights.reduce(function(sum, h) { return sum + h }, 0)

    readonly property int headerHeight: grid.columnTitles.some(function(title) { return Boolean(title) }) ? 22 : 0

    //! NOTE: the column of the row titles: the widest title measured with its font,
    //! at most two fifths of the grid's width; a longer title wraps
    readonly property int rowTitleWidth: {
        if (grid.rowTitles.length === 0 && !Boolean(grid.cornerTitle)) {
            return 0
        }
        var longest = rowTitleMetrics.advanceWidth(grid.cornerTitle)
        for (var i = 0; i < grid.rowTitles.length; ++i) {
            longest = Math.max(longest, rowTitleMetrics.advanceWidth(String(grid.rowTitles[i])))
        }
        return Math.min(width * 0.4, longest + 16)
    }

    //! NOTE: the smallest width a column may take: what the widest cell kind of it asks for,
    //! plus the room of the reset dot
    function minColumnWidth(column) {
        var width = 0
        for (var row = 0; row < rowCount; ++row) {
            var cell = cellAt(row, column)
            if (Boolean(cell) && cell.kind !== "gap" && cell.visible) {
                width = Math.max(width, minWidthOf(cell))
            }
        }
        return width + 20
    }

    //! NOTE: the columns share the room beside the row titles equally, each at least its minimum;
    //! the ones whose minimum is larger than an equal share are taken out of the sharing
    readonly property var columnWidths: {
        var mins = []
        var fixed = 0
        var shared = 0
        for (var column = 0; column < columnCount; ++column) {
            mins.push(minColumnWidth(column))
        }
        var rest = Math.max(0, width - rowTitleWidth)
        var share = rest / columnCount
        for (var c = 0; c < columnCount; ++c) {
            if (mins[c] >= share) {
                fixed += mins[c]
                shared += 1
            }
        }
        var each = columnCount > shared ? (rest - fixed) / (columnCount - shared) : 0
        var result = []
        for (var col = 0; col < columnCount; ++col) {
            result.push(Math.max(mins[col], each))
        }
        return result
    }
    readonly property int tableWidth: columnWidths.reduce(function(sum, w) { return sum + w }, 0)

    FontMetrics {
        id: rowTitleMetrics

        font: AtStyle.font(12.5)
    }

    Column {
        id: content

        width: parent.width
        spacing: 8

        // ----- the title of the grid, as a small title over the table -----

        Column {
            width: parent.width
            spacing: 2

            visible: Boolean(root.grid.title) || Boolean(root.grid.hint)

            StyledTextLabel {
                width: parent.width
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.WordWrap

                text: EoTitles.plain(root.grid.title)
                font: AtStyle.font(13)
            }

            StyledTextLabel {
                width: parent.width
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.WordWrap

                visible: Boolean(root.grid.hint)
                text: root.grid.hint
                font: AtStyle.font(11.5)
                color: AtStyle.text3
            }
        }

        // ----- the table: the titles of the rows stay, the cells scroll sideways -----

        Row {
            width: parent.width
            spacing: 0

            Column {
                width: root.rowTitleWidth
                spacing: 0

                visible: width > 0

                Item {
                    width: parent.width
                    height: root.headerHeight

                    StyledTextLabel {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        horizontalAlignment: Text.AlignLeft

                        visible: Boolean(root.grid.cornerTitle)
                        text: root.grid.cornerTitle
                        font: AtStyle.font(11.5, Font.Medium)
                        color: AtStyle.text3
                        elide: Text.ElideRight
                    }
                }

                Repeater {
                    model: root.rowCount

                    delegate: Item {
                        width: parent.width
                        height: root.rowHeights[index]

                        StyledTextLabel {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            horizontalAlignment: Text.AlignLeft

                            visible: index < root.grid.rowTitles.length
                            text: index < root.grid.rowTitles.length ? root.grid.rowTitles[index] : ""
                            font: AtStyle.font(12.5)
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }

            StyledFlickable {
                width: parent.width - root.rowTitleWidth
                height: root.headerHeight + root.tableHeight

                contentWidth: table.width
                contentHeight: height
                flickableDirection: Flickable.HorizontalFlick
                boundsBehavior: Flickable.StopAtBounds

                interactive: table.width > width

                Column {
                    id: table

                    width: root.tableWidth
                    spacing: 0

                    Item {
                        width: parent.width
                        height: root.headerHeight

                        Row {
                            anchors.fill: parent

                            Repeater {
                                model: root.columnCount

                                delegate: Item {
                                    width: root.columnWidths[index]
                                    height: parent.height

                                    StyledTextLabel {
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.bottom: parent.bottom
                                        horizontalAlignment: Text.AlignLeft

                                        visible: index < root.grid.columnTitles.length
                                        text: index < root.grid.columnTitles.length
                                              ? root.grid.columnTitles[index] : ""
                                        font: AtStyle.font(11.5, Font.Medium)
                                        color: AtStyle.text3
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }

                    Repeater {
                        model: root.rowCount

                        delegate: Row {
                            readonly property int rowIndex: index

                            width: parent.width
                            height: root.rowHeights[index]

                            Repeater {
                                model: root.columnCount

                                delegate: FocusScope {
                                    id: cellItem

                                    readonly property var cell: root.cellAt(rowIndex, index)
                                    readonly property int cellRow: rowIndex
                                    readonly property int cellColumn: index
                                    readonly property bool hasCell: Boolean(cell) && cell.kind !== "gap"
                                                                    && cell.visible
                                    readonly property bool isActive: hasCell && Boolean(root.session)
                                                                     && root.session.activeOption === cell
                                    readonly property bool isShown: hasCell ? root.isCellShown(cell) : false
                                    readonly property int cellNavRow: root.navigationRow + cellRow

                                    width: root.columnWidths[cellColumn]
                                    height: root.rowHeights[cellRow]

                                    enabled: hasCell ? cell.enabled : false
                                    opacity: !hasCell ? 0.0 : (isShown ? 1.0 : 0.35)

                                    function activateCell() {
                                        if (hasCell && isShown && Boolean(root.session)) {
                                            root.session.activate(cell, root.section, root.page)
                                        }
                                    }

                                    onActiveFocusChanged: {
                                        if (activeFocus) {
                                            activateCell()
                                        }
                                    }

                                    HoverHandler {
                                        enabled: cellItem.hasCell && cellItem.isShown
                                        onHoveredChanged: {
                                            if (hovered) {
                                                cellItem.activateCell()
                                            }
                                        }
                                    }

                                    Rectangle {
                                        anchors.fill: parent
                                        anchors.leftMargin: -4
                                        anchors.rightMargin: -4
                                        anchors.topMargin: 1
                                        anchors.bottomMargin: 1

                                        radius: 5
                                        color: AtStyle.cell
                                        visible: cellItem.isActive
                                    }

                                    EoEditor {
                                        anchors.left: parent.left
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: cellItem.width - 20

                                        active: cellItem.hasCell

                                        option: cellItem.cell
                                        optionsModel: root.optionsModel
                                        session: root.session
                                        navigationPanel: root.navigationPanel
                                        navigationRow: cellItem.cellNavRow
                                        navigationColumn: 1 + 2 * cellItem.cellColumn
                                        compact: true

                                        musicInfo: cellItem.hasCell ? EoMusic.musicInfoOf(cellItem.cell, root.section,
                                                                                          root.page) : null
                                        musicStyle: cellItem.hasCell ? EoMusic.musicStyleOf(cellItem.cell, root.section,
                                                                                            root.page) : ({})
                                    }

                                    //! NOTE: the cell is set back to its default by a small dot,
                                    //! not by the button of a whole option
                                    Item {
                                        anchors.right: parent.right
                                        anchors.verticalCenter: parent.verticalCenter

                                        width: 16
                                        height: 16

                                        visible: cellItem.hasCell && root.isCellModified(cellItem.cell)

                                        Rectangle {
                                            anchors.centerIn: parent

                                            width: 6
                                            height: 6
                                            radius: 3

                                            color: AtStyle.accent
                                        }

                                        NavigationControl {
                                            id: cellResetNav

                                            name: "Reset_" + (cellItem.hasCell ? cellItem.cell.sid : "")
                                            panel: root.navigationPanel
                                            row: cellItem.cellNavRow
                                            column: 2 + 2 * cellItem.cellColumn
                                            enabled: parent.visible && cellItem.isShown

                                            accessible.role: MUAccessible.Button
                                            accessible.name: qsTrc("notation/engravingoptions", "Reset to default")
                                                             + ": " + (cellItem.hasCell
                                                                       ? EoTitles.plain(cellItem.cell.fullTitle) : "")

                                            onTriggered: {
                                                root.optionsModel.resetValues(root.cellSids(cellItem.cell))
                                            }
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            hoverEnabled: true

                                            //! NOTE: what the dot does, as a tip over it
                                            onContainsMouseChanged: {
                                                if (containsMouse && Boolean(cellItem.cell)) {
                                                    ui.tooltip.show(parent,
                                                                    qsTrc("notation/engravingoptions",
                                                                          "Reset to default"))
                                                } else {
                                                    ui.tooltip.hide(parent)
                                                }
                                            }

                                            onClicked: {
                                                cellResetNav.requestActiveByInteraction()
                                                root.optionsModel.resetValues(root.cellSids(cellItem.cell))
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
