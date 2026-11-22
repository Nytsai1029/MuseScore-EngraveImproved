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

//! NOTE: a page of the Engraving Options dialog: its name, its place in the list of the pages and its sections.
//! A page is a description, not a view. The dialog shows it, searches it and counts its changed values,
//! so a new option is one line in a page and nothing else
QtObject {
    id: root

    //! NOTE: the name by which the page is opened from outside (e.g. by the "Style…" command of an element)
    property string code: ""

    //! NOTE: the names of the pages of the style dialog which this page stands for.
    //! A command which asks for one of them opens this page
    property var legacyCodes: []

    property string title: ""

    //! NOTE: the title of the group of pages in the list which the page belongs to
    property string group: ""

    //! NOTE: other words by which the search finds the page
    property var keywords: []

    //! NOTE: the EngravingOptionsModel; set by the dialog
    property var model: null

    //! NOTE: for a page which is not made of sections: what is shown in the place of them. The component is made
    //! with these properties, if it has them: `page` (this object), `optionsModel`, `navigationPanel`
    //! and `subPageCode` (which part of the page was asked for from outside)
    property Component view: null

    default property list<QtObject> sections

    //! NOTE: the value of a style value, for an option which depends on another one,
    //! e.g. `enabled: page.value("showMeasureNumber")`
    function value(sid) {
        if (!Boolean(root.model) || !root.model.isLoaded) {
            return undefined
        }

        var item = root.model.item(sid)
        return Boolean(item) ? item.value : undefined
    }
}
