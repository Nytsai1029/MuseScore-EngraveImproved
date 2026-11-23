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
        hint: qsTrc("notation/engravingoptions",
                    "These values shape the strokes of single-note tremolos. The strokes of two-note tremolos are " +
                    "drawn like beams, with the beam thickness and beam distance of the Beams page.")
        music: "\\stemUp c''8:16 \\noBeam d''8:32 \\noBeam"

        EoNumber {
            sid: "tremoloWidth"
            title: qsTrc("notation", "Stroke width:")
            hint: qsTrc("notation/engravingoptions",
                        "Length of each stroke of a single-note tremolo, from its left end to its right end.")
        }

        EoNumber {
            sid: "tremoloLineWidth"
            title: qsTrc("notation", "Stroke thickness:")
            hint: qsTrc("notation/engravingoptions", "Thickness of each stroke of a single-note tremolo.")
        }

        EoNumber {
            sid: "tremoloDistance"
            title: qsTrc("notation", "Distance between strokes:")
            hint: qsTrc("notation/engravingoptions",
                        "Measured from the upper edge of one stroke to the upper edge of the next, so it includes " +
                        "the thickness of a stroke.")
        }

        EoNumber {
            sid: "tremoloNoteSidePadding"
            title: qsTrc("notation", "Distance from notehead:")
            hint: qsTrc("notation/engravingoptions",
                        "The strokes stay at least this far from the notehead: where the stem is too short for that, " +
                        "it is made longer.")
        }

        EoNumber {
            sid: "tremoloOutSidePadding"
            title: qsTrc("notation", "Distance from stem end:")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between the end of the stem and the stroke nearest to it. On flagged and beamed notes " +
                        "the room for the flag or the beams comes on top.")
        }

        EoChoice {
            sid: "tremoloStyle"
            music: "\\tremolo 32 { c''2 e''2 }"
            title: qsTrc("notation/engravingoptions", "Strokes between two half notes")
            hint: qsTrc("notation/engravingoptions",
                        "‘Default’: all strokes stop short of both stems. ‘Traditional’: all strokes join the two " +
                        "stems like beams. ‘Traditional alternative’: only the outer stroke joins the stems, the " +
                        "others stop short of them. Not used on tablature staves.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Default") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Traditional") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Traditional alternative") }
            ]
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
                        "On a note with a straight flag the strokes are set this much further from the stem end than " +
                        "‘Distance from stem end’, to leave room for the flag.")
            max: 3
        }

        EoNumber {
            sid: "tremoloFlagOffsetUp"
            title: qsTrc("notation/engravingoptions", "Extra distance from stem end, curved flag, stem up")
            hint: qsTrc("notation/engravingoptions", "The same on an up-stem note with a curved flag.")
            max: 3
        }

        EoNumber {
            sid: "tremoloFlagOffsetDown"
            title: qsTrc("notation/engravingoptions", "Extra distance from stem end, curved flag, stem down")
            hint: qsTrc("notation/engravingoptions",
                        "The same on a down-stem note with a curved flag, whose flag is shaped differently.")
            max: 3
        }

        EoNumber {
            sid: "tremoloMultiFlagOffsetStraight"
            music: "\\stemUp c'16:8"
            musicStyle: ({ useStraightNoteFlags: true })
            title: qsTrc("notation/engravingoptions", "Further distance for two or more straight flags")
            hint: qsTrc("notation/engravingoptions",
                        "Added once more when the note has two or more straight flags (sixteenth notes and shorter).")
            max: 3
        }

        EoNumber {
            sid: "tremoloMultiFlagOffset"
            music: "\\stemUp c'16:8"
            title: qsTrc("notation/engravingoptions", "Further distance for two or more curved flags")
            hint: qsTrc("notation/engravingoptions",
                        "Added once more when the note has two or more curved flags (sixteenth notes and shorter).")
            max: 3
        }

        EoNumber {
            sid: "tremoloBuzzRollOffsetUp"
            music: "\\stemUp c'4:z"
            title: qsTrc("notation/engravingoptions", "Buzz roll extra distance from stem end, stem up")
            hint: qsTrc("notation/engravingoptions",
                        "A buzz roll (the Z on the stem) on an up-stem note is set this much further from the stem " +
                        "end.")
            max: 2
        }

        EoNumber {
            sid: "tremoloSingleNoteSlant"
            title: qsTrc("notation/engravingoptions", "Slant of single-note tremolo strokes")
            hint: qsTrc("notation/engravingoptions",
                        "How much each stroke rises from its left end to its right end, as a multiple of the stroke " +
                        "thickness: 1 lifts the right end by one thickness, 0 draws level strokes.")
            max: 3
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Two-note tremolos")
        music: "\\tremolo 16 { c''8 d''8 }"

        EoNumber {
            sid: "tremoloTwoNoteStemGap"
            music: "\\tremolo 32 { c''2 e''2 }"
            title: qsTrc("notation/engravingoptions", "Gap between strokes and stems")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal gap between each of the two stems and the ends of the strokes, where the strokes " +
                        "do not join the stems (in the traditional alternate style: for the inner strokes).")
            max: 3
        }

        EoNumber {
            sid: "tremoloTwoNoteMinStrokeLength"
            music: "\\tremolo 16 { c''8 d''8 }"
            title: qsTrc("notation/engravingoptions", "Shortest stroke before the gap is reduced")
            hint: qsTrc("notation/engravingoptions",
                        "When the two notes stand so close that the strokes would be shorter than this, the gap to " +
                        "the stems is narrowed to keep them this long.")
            max: 3
        }

        EoNumber {
            sid: "tremoloTwoNoteMinStemGap"
            music: "\\tremolo 16 { c''8 d''8 }"
            title: qsTrc("notation/engravingoptions", "Smallest gap between strokes and stems")
            hint: qsTrc("notation/engravingoptions",
                        "The narrowing never takes the gap below this, even if the strokes then become shorter than " +
                        "‘Shortest stroke before the gap is reduced’.")
            max: 3
        }
    }
}
