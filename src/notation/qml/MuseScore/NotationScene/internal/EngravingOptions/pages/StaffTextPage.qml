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

    code: "staff-text"
    legacyCodes: ["staff-text", "expression"]
    title: qsTrc("EditStyleBase", "Staff text")
    group: qsTrc("notation/engravingoptions", "Text")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Staff text")
        music: "c''4^\"pizz.\" e''4 g''4 e''4"

        EoChoice {
            sid: "staffTextPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which staff text is placed, unless one is flipped by hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "Offset: X moves the text to the right, Y moves it down from the top staff line (above the " +
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
                sid: "staffTextPosAbove"
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and height of a staff text placed above, measured from the top staff " +
                            "line (negative is up); notes and signs in the way still push it further out.")
            }

            EoOffset {
                sid: "staffTextPosBelow"
                musicStyle: ({ staffTextPlacement: 1 })
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and depth of a staff text placed below, measured from the bottom staff " +
                            "line (positive is down); notes and signs in the way still push it further out.")
            }
        }

        EoNumber {
            sid: "staffTextMinDistance"
            music: "a''4^\"pizz.\" c'''4 e'''4 c'''4"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a staff text and the notes or other signs which lie between it " +
                        "and the staff.")
            min: -100
            step: 0.5
            decimals: 1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Expression text")
        music: "c'4 \\p \\expression \"dolce\" e'4 g'4 e'4"

        EoToggle {
            sid: "snapToDynamics"
            title: qsTrc("notation/engravingoptions", "Snap to dynamics")
            hint: qsTrc("notation/engravingoptions",
                        "Expression text beside a dynamic is set at the height of the dynamic, and the two then move " +
                        "up or down together.")
        }

        EoToggle {
            sid: "snapBeforeDynamics"
            title: qsTrc("notation/engravingoptions", "Place expression before dynamic")
            hint: qsTrc("notation/engravingoptions",
                        "Off: expression text on the same note as a dynamic follows it (‘p dolce’). On: it stands " +
                        "before it (‘dolce p’).")
        }

        EoOffset {
            sid: "expressionPosAbove"
            music: "\\instrument voice c''4 \\expression \"dolce\" e''4 g''4 e''4"
            title: qsTrc("notation/engravingoptions", "Offset above the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal shift and height of expression text placed above, measured from the top staff " +
                        "line (negative is up); notes and signs in the way still push it further out.")
        }

        EoOffset {
            sid: "expressionPosBelow"
            music: "c'4 \\expression \"dolce\" e'4 g'4 e'4"
            title: qsTrc("notation/engravingoptions", "Offset below the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal shift and depth of expression text placed below, measured from the bottom staff " +
                        "line (positive is down); notes and signs in the way still push it further out.")
        }

        EoNumber {
            sid: "expressionMinDistance"
            music: "a4 \\expression \"dolce\" f4 d4 f4"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between expression text and the notes or other signs which lie between " +
                        "it and the staff.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "System text")
        hint: qsTrc("notation/engravingoptions",
                    "Text which belongs to the whole system instead of one staff, and so appears in every part.")
        music: "c''4 \\systemText \"Tutti\" e''4 g''4 e''4"

        EoChoice {
            sid: "systemTextPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which system text is placed, unless one is flipped by hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoOffset {
            sid: "systemTextPosBelow"
            musicStyle: ({ systemTextPlacement: 1 })
            title: qsTrc("notation/engravingoptions", "Offset below the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal shift and depth of system text placed below, measured from the bottom staff line " +
                        "(positive is down); notes and signs in the way still push it further out. The offset above " +
                        "the staff is the ‘Offset’ of the text style ‘System’.")
        }

        EoNumber {
            sid: "systemTextMinDistance"
            music: "a''4 \\systemText \"Tutti\" c'''4 e'''4 c'''4"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between system text and the notes or other signs which lie between it " +
                        "and the staff.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Instrument changes")
        music: "c''4 \\instrumentChange \"To flute\" e''4 g''4 e''4"
        hint: qsTrc("notation/engravingoptions",
                    "The text which tells a player to change to another instrument (‘To Piccolo’).")

        EoChoice {
            sid: "instrumentChangePlacement"
            music: "c''4 \\instrumentChange \"To flute\" e''4 g''4 e''4"
            title: qsTrc("EditStyleBase", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which instrument change texts are placed, unless one is flipped by " +
                        "hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoNumber {
            sid: "instrumentChangeMinDistance"
            music: "a''4 \\instrumentChange \"To flute\" c'''4 e'''4 c'''4"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between an instrument change text and the notes or other signs which lie " +
                        "between it and the staff.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Harp pedal diagrams")
        music: "c''4 \\harpPedals e''4 g''4 e''4"
        hint: qsTrc("notation/engravingoptions",
                    "The pedal setting of the harp is shown either as a diagram or as text (the note names of the " +
                    "seven strings); each kind has its own values.")

        EoChoice {
            sid: "harpPedalDiagramPlacement"
            music: "c''4 \\harpPedals e''4 g''4 e''4"
            title: qsTrc("notation/engravingoptions", "Diagram: position")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which harp pedal diagrams are placed, unless one is flipped by hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoNumber {
            sid: "harpPedalDiagramMinDistance"
            music: "a''4 \\harpPedals c'''4 e'''4 c'''4"
            title: qsTrc("notation/engravingoptions", "Diagram: minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a harp pedal diagram and the notes or other signs which lie " +
                        "between it and the staff.")
        }

        EoChoice {
            sid: "harpPedalTextDiagramPlacement"
            music: "c''4 \\harpPedalsText e''4 g''4 e''4"
            title: qsTrc("notation/engravingoptions", "Text: position")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which harp pedal settings written as text are placed, unless one is " +
                        "flipped by hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoNumber {
            sid: "harpPedalTextDiagramMinDistance"
            music: "a''4 \\harpPedalsText c'''4 e'''4 c'''4"
            title: qsTrc("notation/engravingoptions", "Text: minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a harp pedal setting written as text and the notes or other " +
                        "signs which lie between it and the staff.")
        }
    }
}
