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

    code: "rehearsal-marks"
    legacyCodes: ["rehearsal-marks"]
    title: qsTrc("EditStyleBase", "Rehearsal marks")
    group: qsTrc("notation/engravingoptions", "Text")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Rehearsal marks")
        music: "c''1 | c''4 \\mark \"B\" e''4 g''4 e''4"

        EoChoice {
            sid: "rehearsalMarkPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which rehearsal marks are placed, unless one is flipped by hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "Offset: X moves the mark to the right, Y moves it down from the top staff line (above the " +
                        "staff) or from the bottom staff line (below the staff), so a Y of −2 above means 2 sp over " +
                        "the staff.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Offset")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Above the staff"),
                qsTrc("notation/engravingoptions", "Below the staff")
            ]

            EoOffset {
                sid: "rehearsalMarkPosAbove"
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and height of a rehearsal mark placed above, measured from the top " +
                            "staff line (negative is up); notes and signs in the way still push it further out.")
            }

            EoOffset {
                sid: "rehearsalMarkPosBelow"
                musicStyle: ({ rehearsalMarkPlacement: 1 })
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and depth of a rehearsal mark placed below, measured from the bottom " +
                            "staff line (positive is down); notes and signs in the way still push it further out.")
            }
        }

        EoNumber {
            sid: "rehearsalMarkMinDistance"
            music: "c''1 | a''4 \\mark \"B\" c'''4 e'''4 c'''4"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a rehearsal mark and the notes or other signs which lie between " +
                        "it and the staff.")
            min: -100
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "rehearsalMarkTimeSigMargin"
            music: "c''1 | \\time 3/4 c''4 \\mark \"B\" e''4 g''4"
            musicStyle: ({ timeSigPlacement: 1 })
            title: qsTrc("notation/engravingoptions", "Gap to a time signature above the staves")
            hint: qsTrc("notation/engravingoptions",
                        "Where time signatures are placed above the staves, a rehearsal mark is moved to the right " +
                        "until it clears the time signature by this much, given as a multiple of the x-height of its " +
                        "font.")
            max: 5
            step: 0.1
        }

        EoToggle {
            sid: "rehearsalMarkAfterHeaderAtSystemStart"
            music: "\\key 2 \\time 4/4 d''4 \\mark \"A\" fis''4 a''4 fis''4"
            title: qsTrc("notation/engravingoptions",
                         "At the start of a system, place after the clef and key signature")
            hint: qsTrc("notation/engravingoptions",
                        "A rehearsal mark on the first note of a system starts where the clef, key signature and " +
                        "time signature end; off, it starts at the left end of the staff, like a rehearsal mark over " +
                        "a barline.")
        }
    }
}
