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

import ".."

EoPage {
    id: page

    code: "fermatas"
    legacyCodes: ["fermatas"]
    title: qsTrc("EditStyleBase", "Fermatas")
    group: qsTrc("notation/engravingoptions", "Signs")
    music: "<< { c''1 \\fermata } \\\\ { e'1 \\fermata } >>"

    EoSection {
        title: qsTrc("notation/engravingoptions", "Fermatas")

        EoOffset {
            sid: "fermataPosAbove"
            title: qsTrc("notation/engravingoptions", "Position above the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal shift and height of a fermata placed above, measured from the top staff line " +
                        "(negative is up); notes and signs in the way still push it further out.")
        }

        EoOffset {
            sid: "fermataPosBelow"
            title: qsTrc("notation/engravingoptions", "Position below the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal shift and depth of a fermata placed below, measured from the bottom staff line " +
                        "(positive is down); notes and signs in the way still push it further out.")
        }

        EoNumber {
            sid: "fermataMinDistance"
            music: "a''2 \\fermata c''2 \\fermata"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a fermata and the note, articulations or other signs which lie " +
                        "between it and the staff.")
            max: 1000
            step: 0.2
        }
    }
}
