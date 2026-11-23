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

    code: "rests"
    legacyCodes: ["rests"]
    title: qsTrc("EditStyleBase", "Rests")
    group: qsTrc("notation/engravingoptions", "Notes and rests")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Rests in voices")
        music: "<< { R1 } \\\\ { c'4 e'4 c'''4 e'4 } >>"

        EoChoice {
            sid: "multiVoiceRestTwoSpaceOffset"
            music: "<< { r4 r4 r2 } \\\\ { c'4 d'4 e'2 } >>"
            title: qsTrc("EditStyleBase", "Default vertical offset:")
            hint: qsTrc("notation/engravingoptions",
                        "How far a rest moves up (voices 1 and 3) or down (voices 2 and 4) from its normal place " +
                        "when its staff has more than one voice.")
            presentation: "tiles"
            choices: [
                { value: false, title: qsTrc("notation/engravingoptions", "1 space") },
                { value: true, title: qsTrc("notation/engravingoptions", "2 spaces") }
            ]
        }

        EoToggle {
            sid: "mergeMatchingRests"
            music: "<< { r4 c''4 r2 } \\\\ { r4 g'4 r2 } >>"
            title: qsTrc("notation", "Merge matching rests")
            hint: qsTrc("notation/engravingoptions",
                        "When all the voices of a staff rest at the same moment for the same duration, one rest is " +
                        "printed in the normal place instead of one for each voice (as in choral music).")
        }

        EoNumber {
            sid: "restChordMinClearance"
            music: "<< { r8 r8 r4 r2 } \\\\ { b'4 r4 r2 } >>"
            title: qsTrc("notation/engravingoptions", "Rest to notes of another voice")
            hint: qsTrc("notation/engravingoptions",
                        "The smallest vertical gap between a rest and the notes which another voice has at the same " +
                        "moment; a rest which would be nearer moves further away. For all rests except whole and " +
                        "half rests.")
            max: 2
        }

        EoNumber {
            sid: "restChordMinClearanceWholeHalf"
            music: "<< { r2 r2 } \\\\ { c'''2 c'''2 } >>"
            title: qsTrc("notation/engravingoptions", "Whole and half rests to notes of another voice")
            hint: qsTrc("notation/engravingoptions",
                        "The same gap for whole and half rests, which move away by whole staff spaces because they " +
                        "hang from or sit on a line.")
            max: 2
        }

        EoNumber {
            sid: "restRestMinClearance"
            music: "<< { r4 r4 r4 r4 } \\\\ { r1 } >>"
            title: qsTrc("notation/engravingoptions", "Rest to rest of another voice")
            hint: qsTrc("notation/engravingoptions",
                        "The smallest vertical gap between the rests which two voices have at the same moment; when " +
                        "they are nearer, both move apart.")
            max: 2
        }

        EoNumber {
            sid: "fullMeasureRestMinHorizontalClearance"
            music: "<< { R1 } \\\\ { c'4 e'4 c'''4 e'4 } >>"
            title: qsTrc("notation/engravingoptions", "Full-measure rest: horizontal distance within which notes count")
            hint: qsTrc("notation/engravingoptions",
                        "Only notes of the other voices which are nearer to the rest than this, measured sideways, " +
                        "can push it up or down.")
            max: 10
            step: 0.25
        }

        EoNumber {
            sid: "fullMeasureRestMinVerticalClearance"
            music: "<< { R1 } \\\\ { c'4 e'4 c'''4 e'4 } >>"
            title: qsTrc("notation/engravingoptions", "Full-measure rest: vertical gap to notes of other voices")
            hint: qsTrc("notation/engravingoptions",
                        "The rest moves up (voice 1) or down (other voices) by whole staff spaces until it is at " +
                        "least this far from those notes.")
            max: 3
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Multimeasure rests")

        EoToggle {
            sid: "createMultiMeasureRests"
            music: "R1 | R1 | R1 | R1"
            title: qsTrc("notation/engravingoptions", "Create multimeasure rests")
            hint: qsTrc("notation/engravingoptions",
                        "Empty measures which follow each other are printed as one rest with the number of measures " +
                        "above it.")
        }

        EoNumber {
            sid: "minEmptyMeasures"
            music: "R1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Minimum number of empty measures:")
            hint: qsTrc("notation/engravingoptions",
                        "Fewer empty measures in a row than this are left as single measures.")
            min: 1
            max: 99
            step: 1
            decimals: 0
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoChoice {
            sid: "singleMeasureMMRestUseNormalRest"
            music: "R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true, minEmptyMeasures: 1 })
            title: qsTrc("EditStyleBase", "On single empty measure")
            hint: qsTrc("notation/engravingoptions",
                        "What a multimeasure rest of one measure looks like. Only matters when ‘Minimum number of " +
                        "empty measures’ is 1.")
            presentation: "tiles"
            choices: [
                { value: true, title: qsTrc("notation/engravingoptions", "Use normal rest") },
                { value: false, title: qsTrc("notation/engravingoptions", "Use H-bar") }
            ]
            enabled: Boolean(page.value("createMultiMeasureRests")) && page.value("minEmptyMeasures") === 1
                     && !Boolean(page.value("oldStyleMultiMeasureRests"))
        }

        EoToggle {
            sid: "singleMeasureMMRestShowNumber"
            music: "R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true, minEmptyMeasures: 1 })
            title: qsTrc("notation/engravingoptions", "Show number ‘1’")
            hint: qsTrc("notation/engravingoptions",
                        "Prints a 1 above the whole rest of a single empty measure. An H-bar always has its number.")
            enabled: Boolean(page.value("createMultiMeasureRests"))
                     && Boolean(page.value("singleMeasureMMRestUseNormalRest"))
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Width of multimeasure rests")
        music: "R1 | R1 | R1 | R1 | c''1"

        EoChoice {
            sid: "mmRestConstantWidth"
            music: "\\time 3/4 R2. | R2. | R2. | c''2."
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("notation/engravingoptions", "Basic width")
            hint: qsTrc("notation/engravingoptions",
                        "Whether a multimeasure rest takes the room of one measure of the time signature (wider in " +
                        "4/4 than in 2/4) or the same room in every time signature.")
            choices: [
                { value: false, title: qsTrc("notation/engravingoptions", "Proportional to time signature") },
                { value: true, title: qsTrc("notation/engravingoptions", "The same in every time signature") }
            ]
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestReferenceWidth"
            music: "\\time 3/4 R2. | R2. | R2. | c''2."
            musicStyle: ({ createMultiMeasureRests: true, mmRestConstantWidth: true })
            title: qsTrc("notation/engravingoptions", "Fixed basic width, in quarter notes")
            hint: qsTrc("notation/engravingoptions",
                        "The rest takes the room of a note of this many quarter notes: 2 the room of a half note, 4 " +
                        "that of a whole note. Only with ‘Basic width’ set to ‘The same in every time signature’.")
            min: 2
            max: 8
            step: 1
            decimals: 0
            unit: ""
            enabled: Boolean(page.value("createMultiMeasureRests")) && Boolean(page.value("mmRestConstantWidth"))
        }

        EoNumber {
            sid: "minMMRestWidth"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Minimum width:")
            hint: qsTrc("notation/engravingoptions",
                        "A measure with a multimeasure rest is never narrower than this, however tightly the system " +
                        "is spaced.")
            min: 2
            step: 1.0
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestMaxWidthIncrease"
            music: "R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("notation/engravingoptions", "Number of further measures which widen the rest")
            hint: qsTrc("notation/engravingoptions",
                        "Every measure beyond the ‘Longest rest at the basic width’ widens the rest by one beat (one " +
                        "quarter note when the basic width is fixed); after this many it grows no more.")
            max: 99
            step: 1
            decimals: 0
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestBaseWidthMeasures"
            music: "R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("notation/engravingoptions", "Longest rest at the basic width")
            hint: qsTrc("notation/engravingoptions",
                        "A multimeasure rest of up to this many measures has the basic width; longer rests are " +
                        "wider.")
            min: 1
            max: 20
            step: 1
            decimals: 0
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Number")
        music: "R1 | R1 | R1 | R1 | c''1"
        musicStyle: ({ createMultiMeasureRests: true })

        EoNumber {
            sid: "mmRestNumberPos"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Vertical position:")
            hint: qsTrc("notation/engravingoptions",
                        "Measured from the top staff line to the lower edge of the number; negative values are above " +
                        "the line.")
            min: -99.99
            step: 0.5
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestNumberPosOneLineStaff"
            music: "\\lines 1 R1 | R1 | R1 | R1 | c'1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("notation/engravingoptions", "Lowest position on one-line staves")
            hint: qsTrc("notation/engravingoptions",
                        "On a staff of one line the number is never lower than this, whatever ‘Vertical position’ " +
                        "says; negative values are above the line.")
            min: -6
            max: 2
            step: 0.1
            decimals: 1
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoToggle {
            sid: "mmRestNumberMaskHBar"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Break H-bar if number overlaps")
            hint: qsTrc("notation/engravingoptions",
                        "When the number is placed so low that it stands on the H-bar, the bar is interrupted around " +
                        "it.")
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestNumberMaskHBarPadding"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true, mmRestNumberMaskHBar: true, mmRestNumberPos: 2 })
            title: qsTrc("notation/engravingoptions", "Space beside the number in a broken H-bar")
            hint: qsTrc("notation/engravingoptions",
                        "The bar is interrupted for the width of the number plus this much, half of it on each side. " +
                        "Only with ‘Break H-bar if number overlaps’ on.")
            max: 4
            step: 0.1
            decimals: 1
            enabled: Boolean(page.value("createMultiMeasureRests")) && Boolean(page.value("mmRestNumberMaskHBar"))
        }

        EoToggle {
            sid: "mmRestBetweenStaves"
            music: "\\staves 2 R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Center between staves on multi-staff instruments")
            hint: qsTrc("notation/engravingoptions",
                        "On a piano or another instrument with several staves the number is printed once, halfway " +
                        "between two staves, instead of above each staff.")
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "H-bar")
        hint: qsTrc("notation/engravingoptions", "The thick horizontal bar with a short vertical stroke at each end.")
        music: "R1 | R1 | R1 | R1 | c''1"
        musicStyle: ({ createMultiMeasureRests: true })

        EoNumber {
            sid: "mmRestHBarThickness"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            max: 4
            step: 0.1
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestHBarVStrokeThickness"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            max: 4
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestHBarVStrokeHeight"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            max: 10
            step: 0.5
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "multiMeasureRestMargin"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Margin within barlines:")
            hint: qsTrc("notation/engravingoptions",
                        "The space left free between each end of the H-bar and the barline (or the clef, key or time " +
                        "signature) next to it; a larger value makes the bar shorter.")
            step: 0.1
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Old-style multimeasure rests")
        musicStyle: ({ createMultiMeasureRests: true })

        EoToggle {
            sid: "oldStyleMultiMeasureRests"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("notation/engravingoptions", "Use old-style multimeasure rests")
            hint: qsTrc("notation/engravingoptions",
                        "Short multimeasure rests are drawn with whole, double whole and longa rest symbols under " +
                        "the number, instead of the H-bar.")
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestOldStyleMaxMeasures"
            music: "R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1"
            musicStyle: ({ createMultiMeasureRests: true, oldStyleMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Maximum number of measures:")
            hint: qsTrc("notation/engravingoptions", "Rests of more measures than this are drawn with the H-bar.")
            max: 99
            step: 1
            decimals: 0
            enabled: Boolean(page.value("oldStyleMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestOldStyleSpacing"
            music: "R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1"
            musicStyle: ({ createMultiMeasureRests: true, oldStyleMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Spacing of symbols:")
            hint: qsTrc("notation/engravingoptions",
                        "The gap between neighbouring rest symbols of an old-style multimeasure rest.")
            max: 4
            step: 0.01
            enabled: Boolean(page.value("oldStyleMultiMeasureRests"))
        }
    }
}
