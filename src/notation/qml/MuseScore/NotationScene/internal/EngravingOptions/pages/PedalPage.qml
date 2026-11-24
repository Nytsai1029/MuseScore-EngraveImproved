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

    code: "pedal"
    legacyCodes: ["pedal"]
    title: qsTrc("EditStyleBase", "Pedal lines")
    group: qsTrc("notation/engravingoptions", "Lines")
    music: "c'4 \\startPedal \\hooks none 90 e'4 g'4 c''4 | e''2 \\stopPedal c''2"

    EoSection {
        title: qsTrc("notation/engravingoptions", "Pedal lines")

        EoChoice {
            sid: "pedalPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which pedal lines are placed, unless one is flipped by hand.")
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
                sid: "pedalPosAbove"
                musicStyle: ({ pedalPlacement: 0 })
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and height of a pedal line placed above, measured from the top staff " +
                            "line (negative is up); notes and signs in the way still push it further out.")
            }

            EoOffset {
                sid: "pedalPosBelow"
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and depth of a pedal line placed below, measured from the bottom staff " +
                            "line (positive is down); notes and signs in the way still push it further out.")
            }
        }

        EoNumber {
            sid: "pedalLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            step: 0.01
        }

        EoChoice {
            sid: "pedalLineStyle"
            title: qsTrc("EditStyleBase", "Line style:")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Solid") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Dashed") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Dotted") }
            ]
        }

        EoNumber {
            sid: "pedalDashLineLen"
            musicStyle: ({ pedalLineStyle: 1 })
            title: qsTrc("notation/engravingoptions", "Dash length")
            hint: qsTrc("notation/engravingoptions",
                        "Length of each dash as a multiple of the line thickness. Only with the line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("pedalLineStyle") === 1
        }

        EoNumber {
            sid: "pedalDashGapLen"
            musicStyle: ({ pedalLineStyle: 1 })
            title: qsTrc("notation/engravingoptions", "Gap length")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between two dashes as a multiple of the line thickness; it is evened out slightly so " +
                        "that the line ends with a full dash. Only with the line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("pedalLineStyle") === 1
        }

        EoNumber {
            sid: "pedalSymbolLeftShift"
            title: qsTrc("notation/engravingoptions", "Ped. symbol: shift to the left")
            hint: qsTrc("notation/engravingoptions",
                        "How far to the left of its note a pedal line which begins with the Ped. symbol starts.")
            min: -2
            max: 3
        }

        EoNumber {
            sid: "pedalEndInset"
            title: qsTrc("notation/engravingoptions", "Gap between the line’s end and the next note")
            hint: qsTrc("notation/engravingoptions",
                        "How far a pedal line, or its release sign, stops before the note or barline at which it " +
                        "ends.")
            max: 5
        }

        EoNumber {
            sid: "pedalEndInsetBeforeNextPedal"
            music: "c'4 \\startPedal \\hooks none 90 e'4 \\stopPedal g'4 \\startPedal \\hooks none 90 c''4 | e''2 " +
                   "\\stopPedal c''2"
            title: qsTrc("notation/engravingoptions", "Gap between the line’s end and a following pedal line")
            hint: qsTrc("notation/engravingoptions",
                        "Used instead of the gap above when the next pedal line starts on the very note at which " +
                        "this one ends, to leave room for its Ped. symbol.")
            max: 5
        }

        EoNumber {
            sid: "pedalHookHeight"
            title: qsTrc("notation/engravingoptions", "Hook height")
            hint: qsTrc("notation/engravingoptions",
                        "Length of the hooks at the start and the end of a pedal line; they point towards the staff, " +
                        "a negative value turns them away.")
        }

        EoNumber {
            sid: "pedalMinDistance"
            music: "c'4 \\startPedal \\hooks none 90 a4 f4 a4 \\stopPedal | c'1"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a pedal line and the notes or other signs which lie between it " +
                        "and the staff.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Texts")
        hint: qsTrc("notation/engravingoptions",
                    "A symbol of the music font is written as <sym>name</sym>: <sym>keyboardPedalPed</sym> is the " +
                    "Ped. sign, <sym>keyboardPedalUp</sym> the release sign.")

        EoText {
            sid: "pedalText"
            title: qsTrc("notation/engravingoptions", "Text at the start")
            hint: qsTrc("notation/engravingoptions",
                        "For pedal lines which begin without a hook: the Ped. sign by default.")
        }

        EoText {
            sid: "pedalHookText"
            music: "c'4 \\startPedal \\hooks 90 90 e'4 g'4 c''4 | e''2 \\stopPedal c''2"
            title: qsTrc("notation/engravingoptions", "Text at the start of a line which begins with a hook")
            hint: qsTrc("notation/engravingoptions",
                        "Empty by default, so that a bracket-style pedal line shows the hook alone.")
        }

        EoText {
            sid: "pedalContinueText"
            music: "c'4 \\startPedal \\hooks none 90 e'4 g'4 c''4 \\break e''2 \\stopPedal c''2"
            systemWidth: 16
            title: qsTrc("notation/engravingoptions", "Text on following systems")
            hint: qsTrc("notation/engravingoptions",
                        "Repeated at the start of each following system of a pedal line which begins without a hook: " +
                        "the Ped. sign in parentheses by default.")
        }

        EoText {
            sid: "pedalContinueHookText"
            music: "c'4 \\startPedal \\hooks 90 90 e'4 g'4 c''4 \\break e''2 \\stopPedal c''2"
            systemWidth: 16
            title: qsTrc("notation/engravingoptions", "Text on following systems of a line which begins with a hook")
            hint: qsTrc("notation/engravingoptions",
                        "Repeated at the start of each following system of a pedal line which begins with a hook; " +
                        "empty by default, so that only the line goes on.")
        }

        EoText {
            sid: "pedalEndText"
            title: qsTrc("notation/engravingoptions", "Text at the end of a visible line")
            hint: qsTrc("notation/engravingoptions", "Empty by default: a pedal drawn with a line ends with its hook.")
        }

        EoText {
            sid: "pedalRosetteEndText"
            music: "c'4 \\startPedal \\noLine e'4 g'4 c''4 | e''2 \\stopPedal c''2"
            title: qsTrc("notation/engravingoptions", "Text at the end when the line is hidden")
            hint: qsTrc("notation/engravingoptions",
                        "For the pedal marking written as Ped. … ✱ without a line: the release sign by default.")
        }
    }
}
