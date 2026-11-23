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
import "EoTitles.js" as EoTitles

//! NOTE: a table of options which belong together, as in the old dialog (the cells of a header/footer,
//! the slants of the beams): columns and rows with titles, one small control in each cell.
//! It is an option itself only so that the description of a page is one kind of object;
//! it has no style value of its own — the cells do.
EoOption {
    kind: "grid"

    //! NOTE: the titles of the rows and of the columns; empty strings are left out
    property var rowTitles: []
    property var columnTitles: []

    //! NOTE: the title of the corner cell, over the titles of the rows
    property string cornerTitle: ""

    //! NOTE: the controls of the cells ask for as little room as they can (a number field with
    //! the numbers of a grid needs less room than the field of a list)
    property bool compact: false

    //! NOTE: the options of the cells, row by row; an EoGap is an empty cell
    default property list<QtObject> cells

    //! NOTE: the title of the cell of a row and a column: the title of the grid (or the corner
    //! title), then ": " and the titles of the row and the column joined by ", " — or just the
    //! joined titles when the grid has no head
    function cellTitle(row, column) {
        var head = Boolean(title) ? EoTitles.plain(title) : cornerTitle
        var titles = []
        if (row >= 0 && row < rowTitles.length && Boolean(rowTitles[row])) {
            titles.push(rowTitles[row])
        }
        if (column >= 0 && column < columnTitles.length && Boolean(columnTitles[column])) {
            titles.push(columnTitles[column])
        }

        if (titles.length === 0) {
            return head
        }
        return (Boolean(head) ? head + ": " : "") + titles.join(", ")
    }

    Component.onCompleted: {
        var columns = Math.max(1, columnTitles.length)
        for (var i = 0; i < cells.length; ++i) {
            var cell = cells[i]
            if (cell.kind === "gap") {
                continue
            }
            cell.grid = this
            cell.gridRow = Math.floor(i / columns)
            cell.gridColumn = i % columns
        }
    }
}
