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

    code: "tempo-text"
    legacyCodes: ["tempo-text"]
    title: qsTrc("EditStyleBase", "Tempo text")
    group: qsTrc("notation/engravingoptions", "Text")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Tempo text")
        music: "c''4 \\tempo \"Allegro\" e''4 g''4 e''4"

        EoChoice {
            sid: "tempoPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which tempo markings are placed, unless one is flipped by hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "Offset: X moves the text to the right, Y moves it down from the top staff line (above the " +
                        "staff) or from the bottom staff line (below the staff), so a Y of −2 above means 2 sp over " +
                        "the staff. Rit. and accel. lines use the same offsets.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Offset")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Above the staff"),
                qsTrc("notation/engravingoptions", "Below the staff")
            ]

            EoOffset {
                sid: "tempoPosAbove"
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and height of a tempo marking placed above, measured from the top " +
                            "staff line (negative is up); notes and signs in the way still push it further out.")
            }

            EoOffset {
                sid: "tempoPosBelow"
                musicStyle: ({ tempoPlacement: 1 })
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and depth of a tempo marking placed below, measured from the bottom " +
                            "staff line (positive is down); notes and signs in the way still push it further out.")
            }
        }

        EoNumber {
            sid: "tempoMinDistance"
            music: "a''4 \\tempo \"Allegro\" c'''4 e'''4 c'''4"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a tempo marking and the notes or other signs which lie between " +
                        "it and the staff.")
            min: -100
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "tempoRehearsalMarkPadding"
            music: "c''1 | c''4 \\mark \"B\" \\tempo \"Allegro\" e''4 g''4 e''4"
            title: qsTrc("notation/engravingoptions", "Gap to a rehearsal mark")
            hint: qsTrc("notation/engravingoptions",
                        "A tempo marking which would collide with a rehearsal mark at the same place moves to the " +
                        "right of it and keeps this gap; the gap grows with the font size of the tempo text.")
            max: 3
        }

        EoNumber {
            sid: "tempoTimeSigAboveStavesDistance"
            music: "\\time 4/4 c''4 \\tempo \"Allegro\" e''4 g''4 e''4"
            musicStyle: ({ timeSigPlacement: 1 })
            title: qsTrc("notation/engravingoptions", "Gap to a time signature above the staves")
            hint: qsTrc("notation/engravingoptions",
                        "Where time signatures are placed above the staves, a tempo marking at the start of the " +
                        "measure begins this far to the right of the time signature. Only with ‘Align with the time " +
                        "signature’ on.")
            max: 5
            step: 0.1
        }

        EoToggle {
            sid: "tempoAlignWithTimeSig"
            music: "\\time 4/4 c''4 \\tempo \"Allegro\" e''4 g''4 e''4"
            title: qsTrc("notation/engravingoptions", "Align with the time signature")
            hint: qsTrc("notation/engravingoptions",
                        "A tempo marking on the first beat of a measure which begins with a time signature starts at " +
                        "the left edge of the time signature; off, it starts at the first note.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Gradual tempo change lines")
        music: "c''4 \\startTempoChange rit e''4 g''4 e''4 \\stopTempoChange | c''1"

        EoNumber {
            sid: "tempoChangeLineSnapDistance"
            music: "c''4 \\startTempoChange rit e''4 g''4 e''4 \\stopTempoChange | c''1 \\tempo \"a tempo\""
            title: qsTrc("notation/engravingoptions", "Gap before the following tempo marking")
            hint: qsTrc("notation/engravingoptions",
                        "A rit. or accel. line which leads up to a tempo marking, or to another such line, on the " +
                        "same system stops this far before it; the gap grows with the font size of that marking.")
            max: 5
            step: 0.1
            decimals: 1
        }

        EoChoice {
            sid: "tempoChangePlacement"
            title: qsTrc("EditStyleBase", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which rit., accel. and similar lines are placed, unless one is flipped " +
                        "by hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoOffset {
            sid: "tempoChangePosBelow"
            musicStyle: ({ tempoChangePlacement: 1 })
            title: qsTrc("notation/engravingoptions", "Offset below the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal shift and depth of a rit. or accel. line placed below, measured from the bottom " +
                        "staff line (positive is down); notes and signs in the way still push it further out. The " +
                        "offset above the staff is the ‘Offset’ of the text style ‘Gradual tempo change’.")
        }

        EoNumber {
            sid: "tempoChangeMinDistance"
            music: "a''4 \\startTempoChange rit c'''4 e'''4 c'''4 \\stopTempoChange | c''1"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a rit. or accel. line and the notes or other signs which lie " +
                        "between it and the staff.")
        }

        EoNumber {
            sid: "tempoChangeLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            step: 0.01
        }

        EoChoice {
            sid: "tempoChangeLineStyle"
            title: qsTrc("EditStyleBase", "Line style:")
            hint: qsTrc("notation/engravingoptions", "The line after the word: dashed by default.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Solid") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Dashed") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Dotted") }
            ]
        }

        EoNumber {
            sid: "tempoChangeDashLineLen"
            title: qsTrc("notation/engravingoptions", "Dash length")
            hint: qsTrc("notation/engravingoptions",
                        "Length of each dash as a multiple of the line thickness. Only with the line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("tempoChangeLineStyle") === 1
        }

        EoNumber {
            sid: "tempoChangeDashGapLen"
            title: qsTrc("notation/engravingoptions", "Gap length")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between two dashes as a multiple of the line thickness; it is evened out slightly so " +
                        "that the line ends with a full dash. Only with the line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("tempoChangeLineStyle") === 1
        }
    }
}
