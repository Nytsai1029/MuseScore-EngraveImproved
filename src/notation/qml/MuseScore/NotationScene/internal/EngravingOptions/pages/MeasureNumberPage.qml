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

    code: "measure-number"
    legacyCodes: ["measure-number"]
    title: qsTrc("EditStyleBase", "Measure numbers")
    group: qsTrc("notation/engravingoptions", "Layout")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Measure numbers")
        music: "c''2 c''2 | c''1 | c''1 | c''1"

        EoToggle {
            sid: "showMeasureNumber"
            music: "c''4 c''4 | c''2 | c''2"
            musicStyle: ({ measureNumberSystem: false, measureNumberInterval: 1 })
            title: qsTrc("notation/editstyle/voltas", "Show measure numbers")
        }

        EoToggle {
            sid: "showMeasureNumberOne"
            music: "c''4 c''4 | c''2 | c''2"
            musicStyle: ({ showMeasureNumber: true })
            title: qsTrc("notation/editstyle/voltas", "Show initial measure number")
            hint: qsTrc("notation/engravingoptions",
                        "The first measure of the score, and of every section, gets its number too. Numbering at a " +
                        "regular interval then counts 1, 6, 11 … instead of 5, 10, 15 …")
            enabled: Boolean(page.value("showMeasureNumber"))
        }

        EoChoice {
            sid: "measureNumberSystem"
            music: "c''4 c''4 | c''2 \\break c''2 | c''2"
            systemWidth: 32
            musicStyle: ({ showMeasureNumber: true, measureNumberInterval: 1 })
            title: qsTrc("notation/editstyle/voltas", "Frequency")
            hint: qsTrc("notation/engravingoptions",
                        "Whether the first measure of every system is numbered, or measures at a regular interval " +
                        "wherever they fall.")
            choices: [
                { value: true, title: qsTrc("notation/editstyle/voltas", "Start of each system") },
                { value: false, title: qsTrc("notation/engravingoptions", "At a regular interval") }
            ]
            enabled: Boolean(page.value("showMeasureNumber"))
        }

        EoNumber {
            sid: "measureNumberInterval"
            music: "c''4 | c''4 | c''4 | c''4 | c''4 | c''4 | c''4"
            musicStyle: ({ showMeasureNumber: true, measureNumberSystem: false })
            title: qsTrc("notation/engravingoptions", "Interval")
            hint: qsTrc("notation/engravingoptions",
                        "A number is shown every this many measures: 5 numbers measures 5, 10, 15 … Only with the " +
                        "frequency ‘At a regular interval’.")
            min: 1
            max: 99
            step: 1
            decimals: 0
            unit: ""
            enabled: Boolean(page.value("showMeasureNumber")) && !Boolean(page.value("measureNumberSystem"))
        }

        EoChoice {
            sid: "measureNumberPlacementMode"
            music: "\\staves 2 c''1 | c''1"
            musicStyle: ({ showMeasureNumber: true, measureNumberSystem: false, measureNumberInterval: 1 })
            title: qsTrc("notation/engravingoptions", "Placement in the system")
            hint: qsTrc("notation/engravingoptions",
                        "The numbers stand above the top staff of each system, below its bottom staff, on the staves " +
                        "which also carry tempo and rehearsal marks, or on every staff.")
            presentation: "dropdown"
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above system") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below system") },
                { value: 2, title: qsTrc("notation/engravingoptions", "At system marking positions") },
                { value: 3, title: qsTrc("notation/engravingoptions", "On all staves") }
            ]
            enabled: Boolean(page.value("showMeasureNumber"))
        }

        EoChoice {
            sid: "measureNumberTextStyle"
            music: "c''4 c''4 | c''2 | c''2"
            musicStyle: ({ showMeasureNumber: true, measureNumberSystem: false, measureNumberInterval: 1 })
            title: qsTrc("notation/editstyle/voltas", "Text style")
            hint: qsTrc("notation/engravingoptions",
                        "The text style which gives the numbers their font, size and frame; ‘Measure number " +
                        "(alternate)’ is a second, bolder and framed style kept for this purpose.")
            presentation: "dropdown"
            choices: page.model.choices("textStyles")
            enabled: Boolean(page.value("showMeasureNumber"))
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Position")
        music: "c''2 c''2 | c''1 | c''1 | c''1"
        musicStyle: ({ showMeasureNumber: true, measureNumberSystem: false, measureNumberInterval: 1 })

        EoChoice {
            sid: "measureNumberVPlacement"
            music: "c''4 c''4 | c''2 | c''2"
            musicStyle: ({
                showMeasureNumber: true,
                measureNumberSystem: false,
                measureNumberInterval: 1,
                measureNumberPlacementMode: 3
            })
            title: qsTrc("notation/engravingoptions", "Above or below the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Used only with the placement ‘On all staves’; with the other placements the numbers stand " +
                        "above the staff, or below it for ‘Below system’.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
            enabled: Boolean(page.value("showMeasureNumber")) && page.value("measureNumberPlacementMode") === 3
        }

        EoChoice {
            sid: "measureNumberHPlacement"
            music: "c''4 c''4 | c''2 | c''2"
            musicStyle: ({ showMeasureNumber: true, measureNumberSystem: false, measureNumberInterval: 1 })
            title: qsTrc("notation/editstyle/voltas", "Alignment")
            hint: qsTrc("notation/engravingoptions",
                        "Whether the number begins at, is centred on, or ends at the point it is aligned to: the " +
                        "barline or the measure, as chosen under ‘Align’.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Left") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Center") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Right") }
            ]
            enabled: Boolean(page.value("showMeasureNumber"))
        }

        EoChoice {
            sid: "measureNumberAlignToBarline"
            music: "c''4 c''4 | c''2 | c''2"
            musicStyle: ({
                showMeasureNumber: true,
                measureNumberSystem: false,
                measureNumberInterval: 1,
                measureNumberHPlacement: 2
            })
            title: qsTrc("notation/editstyle/voltas", "Align")
            hint: qsTrc("notation/engravingoptions",
                        "To the barline: the number sits at the barline which begins its measure. To the measure: at " +
                        "the left end, the middle or the right end of the measure, after any clef and key signature.")
            choices: [
                { value: true, title: qsTrc("notation/editstyle/voltas", "To the barline") },
                { value: false, title: qsTrc("notation/editstyle/voltas", "To the measure") }
            ]
            enabled: Boolean(page.value("showMeasureNumber"))
        }

        EoGrid {
            rowTitles: [
                qsTrc("notation/engravingoptions", "Offset"),
                qsTrc("notation/engravingoptions", "Offset with the alternate text style")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Above the staff"),
                qsTrc("notation/engravingoptions", "Below the staff")
            ]

            EoOffset {
                sid: "measureNumberPosAbove"
                music: "c''4 c''4 | c''2 | c''2"
                musicStyle: ({ showMeasureNumber: true, measureNumberSystem: false, measureNumberInterval: 1 })
                hint: qsTrc("notation/engravingoptions",
                            "The sideways and vertical shift of a number which stands above the staff; a negative " +
                            "vertical value moves it further up. Applies with the text style ‘Measure number’.")
                enabled: Boolean(page.value("showMeasureNumber"))
            }

            EoOffset {
                sid: "measureNumberPosBelow"
                music: "c''4 c''4 | c''2 | c''2"
                musicStyle: ({
                    showMeasureNumber: true,
                    measureNumberSystem: false,
                    measureNumberInterval: 1,
                    measureNumberPlacementMode: 1
                })
                hint: qsTrc("notation/engravingoptions",
                            "The sideways and vertical shift of a number which stands below the staff; a positive " +
                            "vertical value moves it further down. Applies with the text style ‘Measure number’.")
                enabled: Boolean(page.value("showMeasureNumber"))
            }

            EoOffset {
                sid: "measureNumberAlternatePosAbove"
                music: "c''4 c''4 | c''2 | c''2"
                musicStyle: ({
                    showMeasureNumber: true,
                    measureNumberSystem: false,
                    measureNumberInterval: 1,
                    measureNumberTextStyle: 16
                })
                title: qsTrc("notation/engravingoptions", "Offset above the staff with the alternate text style")
                hint: qsTrc("notation/engravingoptions",
                            "Takes the place of the offset above the staff while the text style is ‘Measure number " +
                            "(alternate)’; a negative vertical value moves the number further up.")
                enabled: Boolean(page.value("showMeasureNumber"))
            }

            EoOffset {
                sid: "measureNumberAlternatePosBelow"
                music: "c''4 c''4 | c''2 | c''2"
                musicStyle: ({
                    showMeasureNumber: true,
                    measureNumberSystem: false,
                    measureNumberInterval: 1,
                    measureNumberTextStyle: 16,
                    measureNumberPlacementMode: 1
                })
                title: qsTrc("notation/engravingoptions", "Offset below the staff with the alternate text style")
                hint: qsTrc("notation/engravingoptions",
                            "Takes the place of the offset below the staff while the text style is ‘Measure number " +
                            "(alternate)’.")
                enabled: Boolean(page.value("showMeasureNumber"))
            }
        }

        EoNumber {
            sid: "measureNumberOneLineStaffOffset"
            music: "\\lines 1 f''1 | f''1 | f''1"
            musicStyle: ({ showMeasureNumber: true, measureNumberSystem: false, measureNumberInterval: 1 })
            title: qsTrc("notation/engravingoptions", "Distance from a one-line staff")
            hint: qsTrc("notation/engravingoptions",
                        "On a staff with a single line, such as a percussion line, the numbers start this far above " +
                        "or below the line, because the barlines reach beyond it. Also used for the measure number " +
                        "ranges at multimeasure rests.")
            max: 6
            step: 0.1
            enabled: Boolean(page.value("showMeasureNumber"))
        }

        EoNumber {
            sid: "measureNumberMinBarlineDistance"
            music: "\\staves 2 \\spanBarlines c''1 | c''1 | c''1"
            musicStyle: ({
                showMeasureNumber: true,
                measureNumberSystem: false,
                measureNumberInterval: 1,
                measureNumberPlacementMode: 3,
                measureNumberVPlacement: 0,
                measureNumberHPlacement: 0
            })
            title: qsTrc("notation/engravingoptions", "Least gap to a barline")
            hint: qsTrc("notation/engravingoptions",
                        "A left- or right-aligned number which would touch a barline running between the staves is " +
                        "moved sideways until it is this far from it; centred numbers stay where they are.")
            max: 2
            enabled: Boolean(page.value("showMeasureNumber"))
        }

        EoNumber {
            sid: "measureNumberMinDistance"
            music: "c''2 | a'''8 a''' a''' a''' | c''2"
            musicStyle: ({ showMeasureNumber: true, measureNumberSystem: false, measureNumberInterval: 1 })
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a measure number and the notes or other signs which lie between " +
                        "it and the staff.")
            enabled: Boolean(page.value("showMeasureNumber"))
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Measure number ranges at multimeasure rests")
        music: "c''1 | R1 | R1 | R1 | c''1"
        musicStyle: ({ createMultiMeasureRests: true })

        EoToggle {
            sid: "mmRestShowMeasureNumberRange"
            music: "c''1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("notation/editstyle/voltas", "Show measure number ranges")
            hint: qsTrc("notation/engravingoptions",
                        "A multimeasure rest shows the first and the last measure number it covers, such as 12–19.")
        }

        EoChoice {
            sid: "mmRestRangeBracketType"
            music: "c''1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true, mmRestShowMeasureNumberRange: true })
            title: qsTrc("notation/editstyle/voltas", "Bracket type")
            hint: qsTrc("notation/engravingoptions", "What encloses the range: [12–19], (12–19) or 12–19.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Square brackets") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Parentheses") },
                { value: 2, title: qsTrc("notation/engravingoptions", "None") }
            ]
            enabled: Boolean(page.value("mmRestShowMeasureNumberRange"))
        }

        EoChoice {
            sid: "mmRestRangeTextStyle"
            music: "c''1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true, mmRestShowMeasureNumberRange: true })
            title: qsTrc("notation/editstyle/voltas", "Text style")
            hint: qsTrc("notation/engravingoptions", "The text style which gives the range its font and size.")
            presentation: "dropdown"
            choices: page.model.choices("textStyles")
            enabled: Boolean(page.value("mmRestShowMeasureNumberRange"))
        }

        EoChoice {
            sid: "mmRestRangeVPlacement"
            music: "c''1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true, mmRestShowMeasureNumberRange: true })
            title: qsTrc("notation/engravingoptions", "Above or below the staff")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
            enabled: Boolean(page.value("mmRestShowMeasureNumberRange"))
        }

        EoChoice {
            sid: "mmRestRangeHPlacement"
            music: "c''1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true, mmRestShowMeasureNumberRange: true })
            title: qsTrc("notation/editstyle/voltas", "Alignment")
            hint: qsTrc("notation/engravingoptions",
                        "The range stands at the left end of the measure, centred over it, or at its right end.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Left") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Center") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Right") }
            ]
            enabled: Boolean(page.value("mmRestShowMeasureNumberRange"))
        }

        EoGrid {
            rowTitles: [
                qsTrc("notation/engravingoptions", "Offset")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Above the staff"),
                qsTrc("notation/engravingoptions", "Below the staff")
            ]

            EoOffset {
                sid: "mmRestRangePosAbove"
                music: "c''1 | R1 | R1 | R1 | c''1"
                musicStyle: ({
                    createMultiMeasureRests: true,
                    mmRestShowMeasureNumberRange: true,
                    mmRestRangeVPlacement: 0
                })
                hint: qsTrc("notation/engravingoptions",
                            "The sideways and vertical shift of a range which stands above the staff; a negative " +
                            "vertical value moves it further up.")
                enabled: Boolean(page.value("mmRestShowMeasureNumberRange"))
            }

            EoOffset {
                sid: "mmRestRangePosBelow"
                music: "c''1 | R1 | R1 | R1 | c''1"
                musicStyle: ({
                    createMultiMeasureRests: true,
                    mmRestShowMeasureNumberRange: true,
                    mmRestRangeVPlacement: 1
                })
                hint: qsTrc("notation/engravingoptions",
                            "The sideways and vertical shift of a range which stands below the staff; a positive " +
                            "vertical value moves it further down.")
                enabled: Boolean(page.value("mmRestShowMeasureNumberRange"))
            }
        }

        EoText {
            sid: "mmRestRangeSeparator"
            music: "c''1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true, mmRestShowMeasureNumberRange: true })
            title: qsTrc("notation/engravingoptions", "Separator")
            hint: qsTrc("notation/engravingoptions",
                        "The character between the first and the last number of the range; an en dash by default, as " +
                        "in 12–19.")
            enabled: Boolean(page.value("mmRestShowMeasureNumberRange"))
        }

        EoNumber {
            sid: "mmRestRangeMinDistance"
            music: "c''1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true, mmRestShowMeasureNumberRange: true })
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a measure number range and the notes or other signs which lie " +
                        "between it and the staff.")
            enabled: Boolean(page.value("mmRestShowMeasureNumberRange"))
        }
    }
}
