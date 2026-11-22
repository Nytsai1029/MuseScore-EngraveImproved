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

    code: "glissando-note-line"
    legacyCodes: ["glissando-note-line"]
    title: qsTrc("EditStyleBase", "Glissandos & note-anchored lines")
    group: qsTrc("notation/engravingoptions", "Lines")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Glissandos and note-anchored lines")

        EoNumber {
            sid: "noteAnchoredLineNoteDistance"
            title: qsTrc("notation/engravingoptions", "Distance from notes")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between each end of a glissando or note-anchored line and the notehead, dots or accidental next to it.")
            max: 2
        }

        EoNumber {
            sid: "noteAnchoredLineSameLineSlant"
            title: qsTrc("notation/engravingoptions", "Slant between notes at the same staff position")
            hint: qsTrc("notation/engravingoptions",
                         "When both notes sit on the same line or space, each end of the line is moved vertically by this amount so that the line is not horizontal.")
            max: 1
        }

        EoNumber {
            sid: "noteAnchoredLineTabSlant"
            title: qsTrc("notation/engravingoptions", "Tablature: slant (× string distance)")
            hint: qsTrc("notation/engravingoptions",
                         "On tablature, each end of a glissando is moved vertically by this fraction of the distance between string lines, so that slides along one string are slanted.")
            max: 1
        }

        EoNumber {
            sid: "glissandoTextDistance"
            title: qsTrc("notation/engravingoptions", "Text distance above straight glissando")
            hint: qsTrc("notation/engravingoptions",
                         "How far the ‘gliss.’ text is raised above a straight glissando line.")
            max: 2
        }

        EoNumber {
            sid: "glissandoTextDistanceWavy"
            title: qsTrc("notation/engravingoptions", "Text distance above wavy glissando")
            hint: qsTrc("notation/engravingoptions",
                         "How far the ‘gliss.’ text is raised above the centre of a wavy glissando line.")
            max: 2
        }
    }
}
