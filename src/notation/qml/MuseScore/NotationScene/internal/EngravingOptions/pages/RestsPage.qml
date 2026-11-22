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

        EoToggle {
            sid: "createMultiMeasureRests"
            title: qsTrc("notation/engravingoptions", "Create multimeasure rests")
        }

        EoNumber {
            sid: "minEmptyMeasures"
            title: qsTrc("EditStyleBase", "Minimum number of empty measures:")
            min: 1
            max: 99
            step: 1
            decimals: 0
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoToggle {
            sid: "oldStyleMultiMeasureRests"
            title: qsTrc("notation/engravingoptions", "Use old-style multimeasure rests")
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestOldStyleMaxMeasures"
            title: qsTrc("EditStyleBase", "Maximum number of measures:")
            max: 99
            step: 1
            decimals: 0
            enabled: Boolean(page.value("oldStyleMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestOldStyleSpacing"
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
            title: qsTrc("notation/engravingoptions", "Show number ‘1’")
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Width")

        EoNumber {
            sid: "mmRestReferenceWidth"
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
            title: qsTrc("EditStyleBase", "Minimum width:")
            min: 2
            step: 1.0
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestMaxWidthIncrease"
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
            title: qsTrc("EditStyleBase", "Vertical position:")
            min: -99.99
            step: 0.5
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoToggle {
            sid: "mmRestNumberMaskHBar"
            title: qsTrc("EditStyleBase", "Break H-bar if number overlaps")
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoToggle {
            sid: "mmRestBetweenStaves"
            title: qsTrc("EditStyleBase", "Center between staves on multi-staff instruments")
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "H-bar")

        EoNumber {
            sid: "mmRestHBarThickness"
            title: qsTrc("EditStyleBase", "Horizontal stroke thickness:")
            max: 4
            step: 0.1
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "multiMeasureRestMargin"
            title: qsTrc("EditStyleBase", "Margin within barlines:")
            step: 0.1
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestHBarVStrokeThickness"
            title: qsTrc("EditStyleBase", "Vertical stroke thickness:")
            max: 4
            enabled: Boolean(page.value("createMultiMeasureRests"))
        }

        EoNumber {
            sid: "mmRestHBarVStrokeHeight"
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
            title: qsTrc("notation/engravingoptions", "Rest to notes of another voice")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum vertical gap between a rest and the notes of another voice at the same position; applies to all rests except whole and half rests.")
            max: 2
        }

        EoNumber {
            sid: "restChordMinClearanceWholeHalf"
            title: qsTrc("notation/engravingoptions", "Whole and half rests to notes of another voice")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum vertical gap between a whole or half rest and the notes of another voice at the same position.")
            max: 2
        }

        EoNumber {
            sid: "restRestMinClearance"
            title: qsTrc("notation/engravingoptions", "Rest to rest of another voice")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum vertical gap between the rests of two voices at the same position.")
            max: 2
        }

        EoNumber {
            sid: "fullMeasureRestMinHorizontalClearance"
            title: qsTrc("notation/engravingoptions", "Full-measure rest: horizontal range checked for collisions")
            hint: qsTrc("notation/engravingoptions",
                         "In multi-voice measures, notes of other voices closer than this horizontally to a full-measure rest can push the rest up or down.")
            max: 10
            step: 0.25
        }

        EoNumber {
            sid: "fullMeasureRestMinVerticalClearance"
            title: qsTrc("notation/engravingoptions", "Full-measure rest: vertical clearance from other voices")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum vertical gap between a full-measure rest and nearby notes of other voices in the same measure.")
            max: 3
        }
    }
}
