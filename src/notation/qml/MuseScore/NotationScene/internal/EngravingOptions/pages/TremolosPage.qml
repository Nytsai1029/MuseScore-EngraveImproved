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

    code: "tremolos"
    title: qsTrc("notation/engravingoptions", "Tremolos")
    group: qsTrc("notation/engravingoptions", "Notes and rests")


    EoSection {
        title: qsTrc("notation/engravingoptions", "Strokes")
        music: "\\stemUp c''8:16 \\noBeam d''8:32 \\noBeam"

        EoNumber {
            sid: "tremoloWidth"
            title: qsTrc("notation/engravingoptions", "Tremolo stroke width")
        }

        EoNumber {
            sid: "tremoloLineWidth"
            title: qsTrc("notation/engravingoptions", "Tremolo stroke thickness")
        }

        EoNumber {
            sid: "tremoloDistance"
            title: qsTrc("notation/engravingoptions", "Distance between tremolo strokes")
        }

        EoNumber {
            sid: "tremoloNoteSidePadding"
            title: qsTrc("notation/engravingoptions", "Gap between a tremolo and the note side")
        }

        EoNumber {
            sid: "tremoloOutSidePadding"
            title: qsTrc("notation/engravingoptions", "Gap between a tremolo and what is outside it")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Single-note tremolos")
        music: "\\stemUp c''8:16 \\noBeam d''8:32 \\noBeam \\stemDown e'8:16 \\noBeam"

        EoNumber {
            sid: "tremoloFlagOffsetStraight"
            music: "\\stemUp c'8:16"
            musicStyle: ({ useStraightNoteFlags: true })
            title: qsTrc("notation/engravingoptions", "Extra distance from stem end, straight flag")
            hint: qsTrc("notation/engravingoptions",
                         "Single-note tremolo on a note with straight flags: distance added between the stem end and the tremolo to leave room for the flag.")
            max: 3
        }

        EoNumber {
            sid: "tremoloFlagOffsetUp"
            title: qsTrc("notation/engravingoptions", "Extra distance from stem end, curved flag, stem up")
            hint: qsTrc("notation/engravingoptions",
                         "Single-note tremolo on an up-stem note with a curved flag: distance added between the stem end and the tremolo to leave room for the flag.")
            max: 3
        }

        EoNumber {
            sid: "tremoloFlagOffsetDown"
            title: qsTrc("notation/engravingoptions", "Extra distance from stem end, curved flag, stem down")
            hint: qsTrc("notation/engravingoptions", "The same for a down-stem note with a curved flag.")
            max: 3
        }

        EoNumber {
            sid: "tremoloMultiFlagOffsetStraight"
            music: "\\stemUp c'16:8"
            musicStyle: ({ useStraightNoteFlags: true })
            title: qsTrc("notation/engravingoptions", "Further distance for two or more straight flags")
            hint: qsTrc("notation/engravingoptions",
                         "Single-note tremolo on a note with two or more straight flags: distance added on top of the straight-flag distance.")
            max: 3
        }

        EoNumber {
            sid: "tremoloMultiFlagOffset"
            music: "\\stemUp c'16:8"
            title: qsTrc("notation/engravingoptions", "Further distance for two or more curved flags")
            hint: qsTrc("notation/engravingoptions",
                         "Single-note tremolo on a note with two or more curved flags: distance added on top of the curved-flag distance.")
            max: 3
        }

        EoNumber {
            sid: "tremoloBuzzRollOffsetUp"
            music: "\\stemUp c'4:z"
            title: qsTrc("notation/engravingoptions", "Buzz roll extra distance from stem end, stem up")
            hint: qsTrc("notation/engravingoptions",
                         "Distance added between the stem end and a buzz roll on an up-stem note.")
            max: 2
        }

        EoNumber {
            sid: "tremoloSingleNoteSlant"
            title: qsTrc("notation/engravingoptions", "Slant of single-note tremolo strokes")
            hint: qsTrc("notation/engravingoptions",
                         "How much each stroke of a single-note tremolo rises from its left to its right end, as a multiple of the stroke thickness (0 = level strokes).")
            max: 3
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Two-note tremolos")
        music: "\\tremolo 16 { c'2 g'2 }"

        EoNumber {
            sid: "tremoloTwoNoteStemGap"
            music: "\\tremolo 32 { c''2 e''2 }"
            title: qsTrc("notation/engravingoptions", "Gap between strokes and stems")
            hint: qsTrc("notation/engravingoptions",
                         "Two-note tremolo in the default style (and the inner strokes of the traditional alternate style): horizontal gap between each stem and the ends of the strokes.")
            max: 3
        }

        EoNumber {
            sid: "tremoloTwoNoteMinStrokeLength"
            music: "\\tremolo 16 { c''8 d''8 }"
            title: qsTrc("notation/engravingoptions", "Shortest stroke before the gap is reduced")
            hint: qsTrc("notation/engravingoptions",
                         "Two-note tremolo between closely spaced notes: if the strokes would become shorter than this, the gap to the stems is reduced to lengthen them.")
            max: 3
        }

        EoNumber {
            sid: "tremoloTwoNoteMinStemGap"
            music: "\\tremolo 16 { c''8 d''8 }"
            title: qsTrc("notation/engravingoptions", "Smallest gap between strokes and stems")
            hint: qsTrc("notation/engravingoptions",
                         "Two-note tremolo between closely spaced notes: the gap to the stems is never reduced below this value.")
            max: 3
        }
    }
}
