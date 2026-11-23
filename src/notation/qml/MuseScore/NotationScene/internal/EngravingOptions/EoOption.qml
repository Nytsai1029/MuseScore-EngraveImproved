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

//! NOTE: what all the options have. An option is one style value (`sid`) with a control for it;
//! the types which are made of this one say which control (EoNumber, EoToggle, EoChoice…)
QtObject {
    //! NOTE: which control shows the option; set by the type of the option
    property string kind: ""

    //! NOTE: the name of the style value (the tag which it has in a style file)
    property string sid: ""

    property string title: ""

    //! NOTE: a sentence under the title: what the option changes on the page
    property string hint: ""

    //! NOTE: other words by which the search finds the option
    property var keywords: []

    //! NOTE: the music which the preview of the option shows (see PreviewMusic); the one of the section or
    //! of the page if the option has none
    property string music: ""

    //! NOTE: style values by name which the picture of the music sets whatever the score has (the situation
    //! needs a style switch); merged with those of the section and the page, the nearer one wins
    property var musicStyle: ({})

    //! NOTE: the width of the systems the music is laid out with, in staff spaces (0: as wide as the
    //! music needs). It belongs to the music: it counts on the object which gives it
    property real systemWidth: 0

    property bool enabled: true
    property bool visible: true

    //! NOTE: more style values which the control of the option changes;
    //! they are reset and counted with the one of the option
    property var extraSids: []

    //! NOTE: for a cell of an EoGrid: the grid, and the row and the column of the cell in it;
    //! the grid sets them on its cells itself
    property var grid: null
    property int gridRow: -1
    property int gridColumn: -1

    //! NOTE: the title under which the option is searched and read aloud: its own, or the one
    //! which the grid of a cell makes of its row and column titles when the cell has none of its own
    readonly property string fullTitle: Boolean(title) ? title
                                        : (Boolean(grid) ? grid.cellTitle(gridRow, gridColumn) : "")
}
