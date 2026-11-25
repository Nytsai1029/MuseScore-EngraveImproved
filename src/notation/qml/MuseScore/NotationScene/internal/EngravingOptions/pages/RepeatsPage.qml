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

    code: "repeats"
    legacyCodes: ["repeats"]
    title: qsTrc("EditStyleBase", "Repeats")
    group: qsTrc("notation/engravingoptions", "Signs")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Measure repeats")
        music: "c''2 | c''2 | \\measureRepeat 2"

        EoNumber {
            sid: "measureRepeatNumberPos"
            title: qsTrc("notation", "Number position:")
            hint: qsTrc("notation/engravingoptions",
                        "Height of the number over a measure repeat sign, measured from the top staff line; negative " +
                        "values move it up.")
            min: -99
            step: 0.5
            unit: qsTrc("global", "sp")
        }

        EoToggle {
            sid: "oneMeasureRepeatShow1"
            music: "c''4 c''4 c''4 c''4 | \\measureRepeat 1"
            title: qsTrc("notation/engravingoptions", "Show ‘1’ on 1-measure repeats")
            hint: qsTrc("notation/engravingoptions",
                        "Not used while ‘Number consecutive measure repeats’ is on, which prints its own numbers.")
        }

        EoToggle {
            sid: "fourMeasureRepeatShowExtenders"
            music: "c''4 | c''4 | c''4 | c''4 | \\measureRepeat 4"
            title: qsTrc("notation", "Show extenders on 4-measure repeats")
            hint: qsTrc("notation/engravingoptions",
                        "A line runs from each side of the sign across the four measures it stands for, when they " +
                        "are all on one system.")
        }

        EoToggle {
            sid: "mrNumberSeries"
            music: "c''4 | \\measureRepeat 1 \\measureRepeat 1 \\measureRepeat 1 \\measureRepeat 1 \\measureRepeat 1"
            title: qsTrc("notation", "Number consecutive measure repeats")
            hint: qsTrc("notation/engravingoptions",
                        "In a row of one-measure repeats a running count is printed over the signs, so that players " +
                        "can keep their place.")
        }

        EoNumber {
            sid: "mrNumberEveryXMeasures"
            music: "c''4 | \\measureRepeat 1 \\measureRepeat 1 \\measureRepeat 1 \\measureRepeat 1 \\measureRepeat 1"
            musicStyle: ({ mrNumberSeries: true })
            title: qsTrc("notation/engravingoptions", "Count shown every … measures")
            hint: qsTrc("notation/engravingoptions",
                        "With 4, the count appears on the 4th, 8th, 12th measure of the row; with 1, on every repeat " +
                        "sign. Only with ‘Number consecutive measure repeats’ on.")
            min: 0
            max: 99
            step: 1
            decimals: 0
            enabled: Boolean(page.value("mrNumberSeries"))
        }

        EoToggle {
            sid: "mrNumberSeriesWithParentheses"
            music: "c''4 | \\measureRepeat 1 \\measureRepeat 1 \\measureRepeat 1 \\measureRepeat 1 \\measureRepeat 1"
            musicStyle: ({ mrNumberSeries: true })
            title: qsTrc("notation/engravingoptions", "Count in parentheses")
            hint: qsTrc("notation/engravingoptions",
                        "Prints the count as (4), (8), (12). Only with ‘Number consecutive measure repeats’ on.")
            enabled: Boolean(page.value("mrNumberSeries"))
        }

        EoNumber {
            sid: "mrNumberSeriesFirstNumber"
            music: "c''4 | \\measureRepeat 1 \\measureRepeat 1 \\measureRepeat 1 \\measureRepeat 1 \\measureRepeat 1"
            musicStyle: ({ mrNumberSeries: true })
            title: qsTrc("notation/engravingoptions", "Count starts with")
            hint: qsTrc("notation/engravingoptions",
                        "Whether the measure which is written out counts as number 1, so that the first repeat sign " +
                        "is number 2, or only the repeat signs are counted. Only with ‘Number consecutive measure " +
                        "repeats’ on.")
            min: 1
            max: 2
            step: 1
            decimals: 0
            enabled: Boolean(page.value("mrNumberSeries"))
        }

        EoToggle {
            sid: "twoMeasureRepeatShowNumber"
            title: qsTrc("notation/engravingoptions", "Show ‘2’ on 2-measure repeats")
        }

        EoToggle {
            sid: "fourMeasureRepeatShowNumber"
            music: "c''4 | c''4 | c''4 | c''4 | \\measureRepeat 4"
            title: qsTrc("notation/engravingoptions", "Show ‘4’ on 4-measure repeats")
        }

        EoNumber {
            sid: "measureRepeatExtenderGap"
            music: "c''4 | c''4 | c''4 | c''4 | \\measureRepeat 4"
            musicStyle: ({ fourMeasureRepeatShowExtenders: true })
            title: qsTrc("notation/engravingoptions", "Gap between a 4-measure repeat sign and its extenders")
            hint: qsTrc("notation/engravingoptions",
                        "White space on each side of the sign before the extender line begins. Only with ‘Show " +
                        "extenders on 4-measure repeats’ on.")
            max: 4
            step: 0.05
            enabled: Boolean(page.value("fourMeasureRepeatShowExtenders"))
        }

        EoNumber {
            sid: "measureRepeatNumberOneLineStaffOffset"
            music: "\\lines 1 f''2 | f''2 | \\measureRepeat 2"
            title: qsTrc("notation/engravingoptions", "Extra height of the number on one-line staves")
            hint: qsTrc("notation/engravingoptions",
                        "On a one-line staff the number is raised by this much more, to clear the barlines which " +
                        "reach above the line.")
            max: 6
            step: 0.1
            decimals: 1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Repeat play count")
        music: "\\bar \".|:\" c''2 c''2 \\bar \":|.\" \\playCount 3 c''2 c''2"

        EoToggle {
            sid: "repeatPlayCountShow"
            title: qsTrc("notation/editstyle/repeatplaycount", "Automatically show text at repeat barlines")
            hint: qsTrc("notation/engravingoptions",
                        "An end repeat barline which is played three times or more gets a text such as ‘3x’ above " +
                        "it.")
        }

        EoToggle {
            sid: "repeatPlayCountShowSingleRepeats"
            music: "\\bar \".|:\" c''2 c''2 \\bar \":|.\" c''2 c''2"
            title: qsTrc("notation/editstyle/repeatplaycount", "Show for single repeats (e.g. “x2”)")
            hint: qsTrc("notation/engravingoptions",
                        "Also marks an ordinary repeat, which is played twice. Only with ‘Automatically show text at " +
                        "repeat barlines’ on.")
            enabled: Boolean(page.value("repeatPlayCountShow"))
        }

        EoChoice {
            sid: "repeatPlayCountPreset"
            title: qsTrc("notation", "Preset")
            hint: qsTrc("notation/engravingoptions",
                        "The wording of the automatic text, shown here for a passage played three times.")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle/repeatplaycount", "x3") },
                { value: 1, title: qsTrc("notation/editstyle/repeatplaycount", "3x") },
                { value: 2, title: qsTrc("notation/editstyle/repeatplaycount", "Play 3 times") },
                { value: 3, title: qsTrc("notation/editstyle/repeatplaycount", "3 repeats") }
            ]
            enabled: Boolean(page.value("repeatPlayCountShow"))
        }

        EoChoice {
            sid: "repeatPlayCountPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which the play count texts of repeat barlines (‘3x’) are placed, " +
                        "unless one is flipped by hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoNumber {
            sid: "repeatPlayCountMinDistance"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a play count text and the notes or other signs which lie between " +
                        "it and the staff.")
            step: 0.1
        }

        EoLink {
            title: qsTrc("notation", "Edit text style")
            target: "repeat-play-count"
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Jumps and markers")
        music: "\\segno c''2 c''2 \\toCoda | c''1 | \\coda c''4 c''4 c''2 \\jump \"D.S. al Coda\""

        EoNumber {
            sid: "repeatTextBarlinePadding"
            title: qsTrc("notation/engravingoptions", "Distance from the barline on lower staves")
            hint: qsTrc("notation/engravingoptions",
                        "A jump or marker which is shown on a staff below the top one keeps this distance from the " +
                        "barline coming down from the staff above; given for 10 pt text and growing with the text " +
                        "size.")
            max: 3
        }

        EoChoice {
            sid: "repeatLeftPlacement"
            title: qsTrc("notation/engravingoptions", "Markers: position")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which markers (Segno, Coda, Fine, To Coda…) are placed, unless one is " +
                        "flipped by hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoChoice {
            sid: "repeatRightPlacement"
            title: qsTrc("notation/engravingoptions", "Jumps: position")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which jumps (D.C., D.S. al Coda…) are placed, unless one is flipped by " +
                        "hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoNumber {
            sid: "repeatMinDistance"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a jump or marker and the notes or other signs which lie between " +
                        "it and the staff.")
            step: 0.1
        }
    }
}
