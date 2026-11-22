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

    code: "dynamics-hairpins"
    legacyCodes: ["dynamics-hairpins"]
    title: qsTrc("EditStyleBase", "Dynamics & hairpins")
    group: qsTrc("notation/engravingoptions", "Notations")

    EoSection {
        title: qsTrc("EditStyleBase", "Default positions of dynamics and hairpins")

        EoChoice {
            sid: "dynamicsHairpinVoiceBasedPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Based on voice") },
                { value: 1, title: qsTrc("notation/editstyle", "Above") },
                { value: 2, title: qsTrc("notation/editstyle", "Below") }
            ]
        }

        EoToggle {
            sid: "dynamicsHairpinsAboveForVocalStaves"
            title: qsTrc("EditStyleBase", "Place above the staff on vocal instruments")
        }

        EoToggle {
            sid: "dynamicsHairpinsAutoCenterOnGrandStaff"
            title: qsTrc("EditStyleBase", "Center on grand staff instruments automatically")
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Dynamics")

        EoToggle {
            sid: "dynamicsOverrideFont"
            title: qsTrc("EditStyleBase", "Override score font")
        }

        EoChoice {
            sid: "dynamicsFont"
            title: qsTrc("EditStyleBase", "Font:")
            presentation: "dropdown"
            choices: page.model.choices("musicFonts")
            enabled: Boolean(page.value("dynamicsOverrideFont"))
        }

        EoToggle {
            sid: "avoidBarLines"
            title: qsTrc("EditStyleBase", "Avoid barlines")
        }

        EoNumber {
            sid: "dynamicsSize"
            title: qsTrc("EditStyleBase", "Scale:")
            max: 1000
            step: 1.0
            decimals: 0
            percent: true
        }

        EoOffset {
            sid: "dynamicsPosAbove"
            title: qsTrc("EditStyleBase", "Offset above:")
        }

        EoOffset {
            sid: "dynamicsPosBelow"
            title: qsTrc("EditStyleBase", "Offset below:")
        }

        EoNumber {
            sid: "dynamicsMinDistance"
            title: qsTrc("EditStyleBase", "Autoplace min. distance:")
            max: 1000
            step: 0.2
            decimals: 1
        }

        EoNumber {
            sid: "dynamicsEndOfPreviousDistance"
            title: qsTrc("notation/engravingoptions", "Dynamic anchored to end of previous note: gap")
            hint: qsTrc("notation/engravingoptions",
                         "For a dynamic set to anchor to the end of the previous note, the gap between its right edge and the start of its beat (the barline, clef or note found there).")
            max: 3
        }

        EoNumber {
            sid: "dynamicsMinBarlineDistance"
            title: qsTrc("notation/engravingoptions", "Minimum distance from barlines")
            hint: qsTrc("notation/engravingoptions",
                         "With ‘avoid barlines’ on, a dynamic between staves joined by barlines is moved sideways until it is this far from the barline.")
            max: 2
        }

        EoNumber {
            sid: "expressionDynamicDistance"
            title: qsTrc("notation/engravingoptions", "Gap between dynamic and expression text")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal gap between a dynamic and expression text on the same beat (scaled by the size of both).")
            max: 3
        }

        EoNumber {
            sid: "expressionDynamicForteDistance"
            title: qsTrc("notation/engravingoptions", "Gap after a dynamic ending in f")
            hint: qsTrc("notation/engravingoptions",
                         "The same gap when the dynamic ends with the letter f, whose overhanging top needs less room.")
            max: 3
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Hairpins")

        EoOffset {
            sid: "hairpinPosAbove"
            title: qsTrc("EditStyleBase", "Offset above:")
        }

        EoOffset {
            sid: "hairpinPosBelow"
            title: qsTrc("EditStyleBase", "Offset below:")
        }

        EoNumber {
            sid: "hairpinHeight"
            title: qsTrc("EditStyleBase", "Height:")
            step: 0.1
        }

        EoNumber {
            sid: "hairpinContHeight"
            title: qsTrc("EditStyleBase", "Continue height:")
            step: 0.1
        }

        EoNumber {
            sid: "autoplaceHairpinDynamicsDistance"
            title: qsTrc("EditStyleBase", "Autoplace, distance to dynamics:")
            step: 0.1
        }

        EoNumber {
            sid: "hairpinLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            step: 0.01
        }

        EoToggle {
            sid: "hairpinVerticalEnds"
            title: qsTrc("EditStyleBase", "Vertical ends on diagonal hairpins")
        }

        EoNumber {
            sid: "hairpinTextDistance"
            title: qsTrc("notation/engravingoptions", "Gap between hairpin and its text")
            hint: qsTrc("notation/engravingoptions",
                         "Space between the lines of a hairpin and text attached to its start or end.")
            max: 3
        }

        EoNumber {
            sid: "hairpinMinLength"
            title: qsTrc("notation/engravingoptions", "Minimum hairpin length")
            hint: qsTrc("notation/engravingoptions", "A hairpin segment is never drawn shorter than this.")
            max: 5
            step: 0.1
        }

        EoNumber {
            sid: "hairpinCircledTipDiameter"
            title: qsTrc("notation/engravingoptions", "Niente circle diameter")
            hint: qsTrc("notation/engravingoptions", "Diameter of the circle at the tip of a ‘niente’ hairpin.")
            min: 0.2
            max: 2
        }

        EoNumber {
            sid: "hairpinSnapExtendThreshold"
            title: qsTrc("notation/engravingoptions", "Extend to following dynamic when gap exceeds")
            hint: qsTrc("notation/engravingoptions",
                         "A hairpin that snaps to the dynamic or expression text after it is lengthened to reach it only when the remaining gap is larger than this; smaller gaps are left as they are.")
            max: 10
            step: 0.1
        }

        EoNumber {
            sid: "hairpinEndInset"
            title: qsTrc("notation/engravingoptions", "Hairpin end: distance before end position")
            hint: qsTrc("notation/engravingoptions",
                         "How far before the note, rest or barline at its end position a hairpin stops when no dynamic determines its end.")
            max: 5
        }
    }
}
