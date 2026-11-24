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

import "EoMusic.js" as EoMusic

//! NOTE: what the dialog and the parts of it share while it is open: the option which is being edited, which the
//! preview pane shows, and the value which is being dragged on a preview, which the field of the option shows
QtObject {
    id: root

    property var activeOption: null
    property var activeSection: null
    property var activePage: null

    //! NOTE: the name of the style value which is dragged, and the value under the pointer
    property string dragSid: ""
    property var dragValue: null

    //! NOTE: a page is asked for (by its code), with what it is to show (the code of a text style)
    signal pageRequested(string pageCode, string subPageCode)

    function openPage(pageCode, subPageCode) {
        root.pageRequested(pageCode, subPageCode)
    }

    //! NOTE: an option which is not a style value (a picture, a paragraph) cannot be edited, so it is not taken
    function activate(option, section, page) {
        if (!Boolean(option) || !Boolean(option.sid) || option === root.activeOption) {
            return
        }

        root.activeOption = option
        root.activeSection = section
        root.activePage = page
    }

    //! NOTE: for a page which is opened: it begins with its first option
    function begin(page) {
        var first = Boolean(page) ? EoMusic.firstOption(page) : null

        root.activeOption = first ? first.option : null
        root.activeSection = first ? first.section : null
        root.activePage = page
    }
}
