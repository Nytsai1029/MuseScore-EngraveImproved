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
        title: qsTrc("EditStyleBase", "Rests")
        music: "R1 | R1 | R1 | R1"

        EoToggle {
            sid: "createMultiMeasureRests"
            title: qsTrc("notation/engravingoptions", "Create multimeasure rests")
        }

        EoNumber {
            sid: "minEmptyMeasures"
            music: "R1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Minimum number of empty measures:")
            min: 1
            max: 99
            step: 1
            decimals: 0
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoToggle {
            sid: "oldStyleMultiMeasureRests"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("notation/engravingoptions", "Use old-style multimeasure rests")
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestOldStyleMaxMeasures"
            music: "R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1"
            musicStyle: ({ createMultiMeasureRests: true, oldStyleMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Maximum number of measures:")
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
            max: 4
            step: 0.01
            enabled: Boolean(page.value("oldStyleMultiMeasureRests"))
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "On single empty measure")

        EoToggle {
            sid: "singleMeasureMMRestShowNumber"
            music: "R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true, minEmptyMeasures: 1 })
            title: qsTrc("notation/engravingoptions", "Show number ‘1’")
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Width")

        EoNumber {
            sid: "mmRestReferenceWidth"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("notation/engravingoptions", "Reference width (in quarter notes)")
            min: 2
            max: 8
            step: 1
            decimals: 0
            unit: ""
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "minMMRestWidth"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Minimum width:")
            min: 2
            step: 1.0
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestMaxWidthIncrease"
            music: "R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1 | R1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Cap width after:")
            max: 99
            step: 1
            decimals: 0
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Number layout")

        EoNumber {
            sid: "mmRestNumberPos"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Vertical position:")
            min: -99.99
            step: 0.5
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoToggle {
            sid: "mmRestNumberMaskHBar"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Break H-bar if number overlaps")
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoToggle {
            sid: "mmRestBetweenStaves"
            music: "\\staves 2 R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Center between staves on multi-staff instruments")
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "H-bar")

        EoNumber {
            sid: "mmRestHBarThickness"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Horizontal stroke thickness:")
            max: 4
            step: 0.1
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "multiMeasureRestMargin"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Margin within barlines:")
            step: 0.1
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestHBarVStrokeThickness"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Vertical stroke thickness:")
            max: 4
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestHBarVStrokeHeight"
            music: "R1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true })
            title: qsTrc("EditStyleBase", "Vertical stroke height:")
            max: 10
            step: 0.5
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Rests in multi-voice contexts")

        EoNumber {
            sid: "restChordMinClearance"
            music: "<< { r8 r8 r4 r2 } \\\\ { b'4 r4 r2 } >>"
            title: qsTrc("notation/engravingoptions", "Rest to notes of another voice")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum vertical gap between a rest and the notes of another voice at the same position; applies to all rests except whole and half rests.")
            max: 2
        }

        EoNumber {
            sid: "restChordMinClearanceWholeHalf"
            music: "<< { r2 r2 } \\\\ { c'''2 c'''2 } >>"
            title: qsTrc("notation/engravingoptions", "Whole and half rests to notes of another voice")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum vertical gap between a whole or half rest and the notes of another voice at the same position.")
            max: 2
        }

        EoNumber {
            sid: "restRestMinClearance"
            music: "<< { r4 r4 r4 r4 } \\\\ { r1 } >>"
            title: qsTrc("notation/engravingoptions", "Rest to rest of another voice")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum vertical gap between the rests of two voices at the same position.")
            max: 2
        }

        EoNumber {
            sid: "fullMeasureRestMinHorizontalClearance"
            music: "<< { R1 } \\\\ { c'4 e'4 c'''4 e'4 } >>"
            title: qsTrc("notation/engravingoptions", "Full-measure rest: horizontal range checked for collisions")
            hint: qsTrc("notation/engravingoptions",
                         "In multi-voice measures, notes of other voices closer than this horizontally to a full-measure rest can push the rest up or down.")
            max: 10
            step: 0.25
        }

        EoNumber {
            sid: "fullMeasureRestMinVerticalClearance"
            music: "<< { R1 } \\\\ { c'4 e'4 c'''4 e'4 } >>"
            title: qsTrc("notation/engravingoptions", "Full-measure rest: vertical clearance from other voices")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum vertical gap between a full-measure rest and nearby notes of other voices in the same measure.")
            max: 3
        }
    }
}
