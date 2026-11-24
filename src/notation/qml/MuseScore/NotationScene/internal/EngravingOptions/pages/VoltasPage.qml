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

    code: "voltas"
    legacyCodes: ["volta"]
    title: qsTrc("notation/engravingoptions", "Voltas")
    group: qsTrc("notation/engravingoptions", "Signs")
    music: "c''2 c''2 | \\volta \"1.\" 1 c''2 c''2 \\bar \":|.\" \\volta \"2.\" 1 c''2 c''2 \\bar \"|.\""

    EoSection {
        title: qsTrc("notation/engravingoptions", "Voltas")

        EoNumber {
            sid: "voltaPosAbove"
            title: qsTrc("notation/editstyle/voltas", "Vertical position:")
            hint: qsTrc("notation/engravingoptions",
                        "Height of the volta line above the top staff line (negative numbers are above the staff); " +
                        "notes and signs in the way still push the volta higher.")
            min: -100
            max: 0
            step: 0.25
            decimals: 2
            unit: qsTrc("global", "sp")
            axis: "y"
        }

        EoNumber {
            sid: "voltaHook"
            title: qsTrc("notation/editstyle/voltas", "Hook height:")
            hint: qsTrc("notation/engravingoptions",
                        "Length of the vertical strokes which close the volta bracket at its ends.")
            step: 0.1
        }

        EoNumber {
            sid: "voltaLineWidth"
            title: qsTrc("notation", "Line thickness:")
            step: 0.01
        }

        EoChoice {
            sid: "voltaLineStyle"
            title: qsTrc("notation", "Line style:")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Solid") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Dashed") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Dotted") }
            ]
        }

        EoNumber {
            sid: "voltaDashLineLen"
            musicStyle: ({ voltaLineStyle: 1 })
            title: qsTrc("notation/engravingoptions", "Dash length")
            hint: qsTrc("notation/engravingoptions",
                        "Length of each dash as a multiple of the line thickness. Only with the line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("voltaLineStyle") === 1
        }

        EoNumber {
            sid: "voltaDashGapLen"
            music: "\\volta \"1.\" 2 c''1 | c''1"
            musicStyle: ({ voltaLineStyle: 1, voltaDashLineLen: 3 })
            title: qsTrc("notation/engravingoptions", "Gap length")
            hint: qsTrc("notation/engravingoptions",
                        "Length of the gap between two dashes as a multiple of the line thickness. Only with the " +
                        "line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("voltaLineStyle") === 1
        }

        EoNumber {
            sid: "voltaMinDistance"
            music: "c''2 c''2 | \\volta \"1.\" 1 c'''2 c''2"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a volta and the notes or other signs which lie between it and " +
                        "the staff.")
            step: 0.1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Alignment")

        EoChoice {
            sid: "voltaAlignStartBeforeKeySig"
            music: "\\key 3 \\volta \"1.\" 1 c''1 | c''1"
            title: qsTrc("notation/engravingoptions",
                         "At the start of a system, start the volta over the key signature")
            hint: qsTrc("notation/engravingoptions",
                        "Off, a volta which begins or continues at the start of a system starts just before the " +
                        "first note, after the key signature.")
            choices: [
                { value: false, title: qsTrc("notation/editstyle/voltas", "After key signature") },
                { value: true, title: qsTrc("notation/editstyle/voltas", "On key signature") }
            ]
        }

        EoChoice {
            sid: "voltaAlignEndLeftOfBarline"
            title: qsTrc("notation/engravingoptions",
                         "After an end repeat, start the hook at the left of the thick barline")
            hint: qsTrc("notation/engravingoptions",
                        "Off, the hook of a volta which follows an end repeat barline stands at the right edge of " +
                        "that barline.")
            choices: [
                { value: false, title: qsTrc("notation/editstyle/voltas", "To right side of barlines") },
                { value: true, title: qsTrc("notation/editstyle/voltas", "To left side of barlines") }
            ]
        }
    }
}
