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

    code: "fretboard-diagrams"
    legacyCodes: ["fretboard-diagrams"]
    title: qsTrc("EditStyleBase", "Fretboard diagrams")
    group: qsTrc("notation/engravingoptions", "Fretted instruments")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Details")

        EoNumber {
            sid: "fretLineThickness"
            title: qsTrc("notation/engravingoptions", "Fretboard diagrams: string and fret line thickness")
            hint: qsTrc("notation/engravingoptions", "Thickness of the string and fret lines of fretboard diagrams.")
            min: 0.01
            max: 0.5
            step: 0.01
        }

        EoNumber {
            sid: "fretMarkerSizeRatio"
            title: qsTrc("notation/engravingoptions", "Fretboard diagrams: open and mute marker size")
            hint: qsTrc("notation/engravingoptions",
                         "Size of the ○ and × markers above the nut as a fraction of the string spacing.")
            min: 0.2
            max: 1.5
        }

        EoNumber {
            sid: "fretMarkerNutDistance"
            title: qsTrc("notation/engravingoptions", "Fretboard diagrams: marker distance from the nut")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between the ○ and × markers and the nut of a fretboard diagram.")
            max: 2
        }

        EoNumber {
            sid: "fretNumPadding"
            title: qsTrc("notation/engravingoptions", "Fretboard diagrams: fret number distance")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between the fret number and the diagram, as a fraction of the string spacing.")
            max: 3
        }

        EoNumber {
            sid: "fretDiagramFingeringDistance"
            title: qsTrc("notation/engravingoptions", "Fretboard diagrams: fingering distance")
            hint: qsTrc("notation/engravingoptions", "Gap between a fretboard diagram and its fingering numbers.")
            max: 2
        }
    }
}
