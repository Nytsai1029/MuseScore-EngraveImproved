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

//! NOTE: a group of options of a page under a title
QtObject {
    property string title: ""

    //! NOTE: a sentence under the title
    property string hint: ""

    //! NOTE: the music of the options of the section which have none of their own
    property string music: ""

    //! NOTE: style values by name which the picture of the music sets whatever the score has (the situation
    //! needs a style switch); merged with those of the section and the page, the nearer one wins
    property var musicStyle: ({})

    //! NOTE: the width of the systems the music is laid out with, in staff spaces (0: as wide as the
    //! music needs). It belongs to the music: it counts on the object which gives it
    property real systemWidth: 0

    //! NOTE: "page": the music is laid out on pages as the style has them (their size and margins, the header
    //! and the footer, the vertical spacing), and the picture is the pages, whole — for the options of the page
    //! itself; "music": the music alone. The nearest object which says it counts (the option, its section, its
    //! page); no one: the music alone
    property string previewScope: ""

    property bool visible: true

    default property list<QtObject> options
}
