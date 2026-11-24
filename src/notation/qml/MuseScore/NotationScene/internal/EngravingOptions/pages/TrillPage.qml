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

    code: "trill"
    legacyCodes: ["trill", "vibrato"]
    title: qsTrc("notation/engravingoptions", "Trill and vibrato lines")
    group: qsTrc("notation/engravingoptions", "Lines")
    music: "e''8 \\startTrill f''8 g''4 a''2 | c''2 \\stopTrill c''2"

    EoSection {
        title: qsTrc("notation/engravingoptions", "Trill lines")

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "Offset: X moves the line to the right, Y moves it down from the top staff line (above the " +
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
                sid: "trillPosAbove"
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and height of a trill line placed above, measured from the top staff " +
                            "line (negative is up); notes and signs in the way still push it further out.")
            }

            EoOffset {
                sid: "trillPosBelow"
                music: "<< { c''1 } \\\\ { g'2 \\startTrill \\stopTrill e'2 } >> | c''1"
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and depth of a trill line placed below, measured from the bottom staff " +
                            "line (positive is down); notes and signs in the way still push it further out.")
            }
        }

        EoChoice {
            sid: "trillAlwaysShowCueNote"
            title: qsTrc("EditStyleBase", "Trill cue note")
            hint: qsTrc("notation/engravingoptions",
                        "The small note in parentheses after the main note which shows the upper note of the trill.")
            choices: [
                { value: false, title: qsTrc("notation/engravingoptions",
                                             "Show only for intervals other than seconds") },
                { value: true, title: qsTrc("notation/engravingoptions", "Show for all intervals") }
            ]
        }

        EoNumber {
            sid: "trillLineEndInset"
            title: qsTrc("notation/engravingoptions", "Gap between the line’s end and the next note")
            hint: qsTrc("notation/engravingoptions",
                        "How far the wavy line stops before the next note, grace notes, clef or barline. Also used " +
                        "for vibrato lines.")
            max: 5
        }

        EoNumber {
            sid: "trillAccidentalMargin"
            music: "e''1 \\startTrill! \\stopTrill | c''1"
            title: qsTrc("notation/engravingoptions", "Gap between the trill sign and its accidental")
            hint: qsTrc("notation/engravingoptions",
                        "Vertical gap between the sign at the start of a trill line and the accidental above or " +
                        "below it.")
            max: 2
        }

        EoNumber {
            sid: "trillMinDistance"
            music: "e''4 \\startTrill a''4 c'''4 a''4 \\stopTrill | e''1"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a trill line and the notes or other signs which lie between it " +
                        "and the staff.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Vibrato lines")
        music: "e''1 \\startVibrato \\stopVibrato | c''1"

        EoChoice {
            sid: "vibratoPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which vibrato lines are placed, unless one is flipped by hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "Offset: X moves the line to the right, Y moves it down from the top staff line (above the " +
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
                sid: "vibratoPosAbove"
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and height of a vibrato line placed above, measured from the top staff " +
                            "line (negative is up); notes and signs in the way still push it further out.")
            }

            EoOffset {
                sid: "vibratoPosBelow"
                musicStyle: ({ vibratoPlacement: 1 })
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and depth of a vibrato line placed below, measured from the bottom " +
                            "staff line (positive is down); notes and signs in the way still push it further out.")
            }
        }

        EoNumber {
            sid: "vibratoMinDistance"
            music: "e''4 \\startVibrato a''4 c'''4 a''4 \\stopVibrato | e''1"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a vibrato line and the notes or other signs which lie between it " +
                        "and the staff.")
        }
    }
}
