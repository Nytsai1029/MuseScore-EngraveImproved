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

    code: "text-line"
    legacyCodes: ["text-line", "system-text-line"]
    title: qsTrc("EditStyleBase", "Text lines")
    group: qsTrc("notation/engravingoptions", "Lines")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Text lines")
        music: "c''4 \\startTextLine \"sul pont.\" \\hooks none 90 d''4 e''4 f''4 | e''2 \\stopTextLine c''2"

        EoChoice {
            sid: "textLinePlacement"
            title: qsTrc("EditStyleBase", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which text lines are placed, unless one is flipped by hand.")
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
                sid: "textLinePosAbove"
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and height of a text line placed above, measured from the top staff " +
                            "line (negative is up); notes and signs in the way still push it further out.")
            }

            EoOffset {
                sid: "textLinePosBelow"
                musicStyle: ({ textLinePlacement: 1 })
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and depth of a text line placed below, measured from the bottom staff " +
                            "line (positive is down); notes and signs in the way still push it further out.")
            }
        }

        EoNumber {
            sid: "textLineLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            step: 0.01
        }

        EoChoice {
            sid: "textLineLineStyle"
            title: qsTrc("EditStyleBase", "Line style:")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Solid") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Dashed") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Dotted") }
            ]
        }

        EoNumber {
            sid: "textLineDashLineLen"
            musicStyle: ({ textLineLineStyle: 1 })
            title: qsTrc("notation/engravingoptions", "Dash length")
            hint: qsTrc("notation/engravingoptions",
                        "Length of each dash as a multiple of the line thickness. Only with the line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("textLineLineStyle") === 1
        }

        EoNumber {
            sid: "textLineDashGapLen"
            musicStyle: ({ textLineLineStyle: 1 })
            title: qsTrc("notation/engravingoptions", "Gap length")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between two dashes as a multiple of the line thickness; it is evened out slightly so " +
                        "that the line ends with a full dash. Only with the line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("textLineLineStyle") === 1
        }

        EoNumber {
            sid: "textLineHookHeight"
            title: qsTrc("notation/engravingoptions", "Hook height")
            hint: qsTrc("notation/engravingoptions",
                        "Length of the hooks of a text line which has hooks; they point towards the staff, a " +
                        "negative value turns them away.")
        }

        EoNumber {
            sid: "textLineMinDistance"
            music: "c''4 \\startTextLine \"sul pont.\" \\hooks none 90 a''4 c'''4 a''4 \\stopTextLine | c''1"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a text line and the notes or other signs which lie between it " +
                        "and the staff.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "System text lines")
        music: "c''4 \\startSystemTextLine \"Tutti\" \\hooks none 90 d''4 e''4 f''4 | e''2 \\stopSystemTextLine c''2"
        hint: qsTrc("notation/engravingoptions",
                    "Text lines which belong to the whole system instead of one staff, and so appear in every part.")

        EoChoice {
            sid: "systemTextLinePlacement"
            title: qsTrc("EditStyleBase", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which system text lines are placed, unless one is flipped by hand.")
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
                sid: "systemTextLinePosAbove"
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and height of a system text line placed above, measured from the top " +
                            "staff line (negative is up); notes and signs in the way still push it further out.")
            }

            EoOffset {
                sid: "systemTextLinePosBelow"
                musicStyle: ({ systemTextLinePlacement: 1 })
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and depth of a system text line placed below, measured from the bottom " +
                            "staff line (positive is down); notes and signs in the way still push it further out.")
            }
        }

        EoNumber {
            sid: "systemTextLineLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            step: 0.01
        }

        EoChoice {
            sid: "systemTextLineLineStyle"
            title: qsTrc("EditStyleBase", "Line style:")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Solid") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Dashed") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Dotted") }
            ]
        }

        EoNumber {
            sid: "systemTextLineDashLineLen"
            musicStyle: ({ systemTextLineLineStyle: 1 })
            title: qsTrc("notation/engravingoptions", "Dash length")
            hint: qsTrc("notation/engravingoptions",
                        "Length of each dash as a multiple of the line thickness. Only with the line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("systemTextLineLineStyle") === 1
        }

        EoNumber {
            sid: "systemTextLineDashGapLen"
            musicStyle: ({ systemTextLineLineStyle: 1 })
            title: qsTrc("notation/engravingoptions", "Gap length")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between two dashes as a multiple of the line thickness; it is evened out slightly so " +
                        "that the line ends with a full dash. Only with the line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("systemTextLineLineStyle") === 1
        }

        EoNumber {
            sid: "systemTextLineHookHeight"
            title: qsTrc("notation/engravingoptions", "Hook height")
            hint: qsTrc("notation/engravingoptions",
                        "Length of the hooks of a system text line which has hooks; they point towards the staff, a " +
                        "negative value turns them away.")
        }

        EoNumber {
            sid: "systemTextLineMinDistance"
            music: "c''4 \\startSystemTextLine \"Tutti\" \\hooks none 90 a''4 c'''4 a''4 \\stopSystemTextLine | c''1"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a system text line and the notes or other signs which lie " +
                        "between it and the staff.")
        }
    }
}
