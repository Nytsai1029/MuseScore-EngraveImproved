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

    code: "barlines"
    legacyCodes: ["barlines"]
    title: qsTrc("EditStyleBase", "Barlines")
    group: qsTrc("notation/engravingoptions", "Notations")

    EoSection {
        title: qsTrc("EditStyleBase", "Barlines")

        EoToggle {
            sid: "repeatBarTips"
            title: qsTrc("EditStyleBase", "Show repeat barline tips (“winged” repeats)")
        }

        EoToggle {
            sid: "startBarlineSingle"
            title: qsTrc("EditStyleBase", "Barline at start of single staff")
        }

        EoToggle {
            sid: "startBarlineMultiple"
            title: qsTrc("EditStyleBase", "Barline at start of multiple staves")
        }

        EoToggle {
            sid: "scaleBarlines"
            title: qsTrc("EditStyleBase", "Scale barlines to staff size")
        }

        EoToggle {
            sid: "maskBarlinesForText"
            title: qsTrc("EditStyleBase", "Mask barlines when intersecting text")
        }

        EoNumber {
            sid: "barWidth"
            title: qsTrc("EditStyleBase", "Thin barline thickness:")
            step: 0.01
        }

        EoNumber {
            sid: "endBarWidth"
            title: qsTrc("EditStyleBase", "Thick barline thickness:")
            step: 0.01
        }

        EoNumber {
            sid: "endBarDistance"
            title: qsTrc("EditStyleBase", "Thick barline distance:")
            step: 0.01
        }

        EoNumber {
            sid: "doubleBarWidth"
            title: qsTrc("EditStyleBase", "Double barline thickness:")
            step: 0.01
        }

        EoNumber {
            sid: "doubleBarDistance"
            title: qsTrc("EditStyleBase", "Double barline distance:")
            step: 0.01
        }

        EoNumber {
            sid: "repeatBarlineDotSeparation"
            title: qsTrc("EditStyleBase", "Repeat barline to dots distance:")
            step: 0.01
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Barline details")

        EoNumber {
            sid: "barlineOneLineStaffSpanFrom"
            title: qsTrc("notation/engravingoptions", "One-line staff: barline top (half spaces)")
            hint: qsTrc("notation/engravingoptions",
                         "Where barlines start on a one-line staff, in half staff spaces from the staff line (negative is above); the staff is treated as this tall for brackets and staff spacing.")
            min: -16
            max: 0
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "barlineOneLineStaffSpanTo"
            title: qsTrc("notation/engravingoptions", "One-line staff: barline bottom (half spaces)")
            hint: qsTrc("notation/engravingoptions",
                         "Where barlines end on a one-line staff, in half staff spaces below the staff line; the staff is treated as this tall for brackets and staff spacing.")
            max: 16
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "bracketOvershoot"
            title: qsTrc("notation/engravingoptions", "Bracket overshoot beyond the staff")
            hint: qsTrc("notation/engravingoptions",
                         "How far the thick line of a bracket extends above the top staff and below the bottom staff before its tips, with music fonts other than Leland.")
            max: 2
        }

        EoNumber {
            sid: "bracketOvershootLeland"
            title: qsTrc("notation/engravingoptions", "Bracket overshoot beyond the staff (Leland)")
            hint: qsTrc("notation/engravingoptions",
                         "How far the thick line of a bracket extends above the top staff and below the bottom staff before its tips, with the music font Leland.")
            max: 2
        }

        EoNumber {
            sid: "bracketSquareHookLength"
            title: qsTrc("notation/engravingoptions", "Square bracket hook length")
            hint: qsTrc("notation/engravingoptions",
                         "Length of the horizontal hooks of a square (sub-)bracket, which is also the room the bracket takes up left of the staves.")
            max: 3
        }

        EoNumber {
            sid: "bracketLineWidthRatio"
            title: qsTrc("notation/engravingoptions", "Line bracket thickness (× bracket thickness)")
            hint: qsTrc("notation/engravingoptions",
                         "Thickness of a line bracket as a multiple of the thickness of a regular bracket.")
            min: 0.1
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "braceWidthGapFactor"
            title: qsTrc("notation/engravingoptions", "Brace width growth per staff gap")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal stretch of a brace is one glyph width per staff it spans plus this much per gap between those staves; lower values give a slimmer brace.")
            max: 5
            step: 0.025
            decimals: 3
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Barline masks")

        EoNumber {
            sid: "maskBarlinesCollisionPadding"
            title: qsTrc("notation/engravingoptions", "Barline masks: detection margin around text")
            hint: qsTrc("notation/engravingoptions",
                         "A barline is interrupted when it comes closer than this to a text; scales with the font size.")
            max: 2
        }

        EoNumber {
            sid: "maskBarlinesPadding"
            title: qsTrc("notation/engravingoptions", "Barline masks: gap around text")
            hint: qsTrc("notation/engravingoptions",
                         "White space left between an unframed text and the ends of the interrupted barline; scales with the font size.")
            max: 3
        }

        EoNumber {
            sid: "maskBarlinesPaddingMin"
            title: qsTrc("notation/engravingoptions", "Barline masks: minimum gap around text")
            hint: qsTrc("notation/engravingoptions",
                         "Lower limit of the gap around text, which matters for small font sizes.")
            max: 3
        }

        EoNumber {
            sid: "maskBarlinesPaddingMax"
            title: qsTrc("notation/engravingoptions", "Barline masks: maximum gap around text")
            hint: qsTrc("notation/engravingoptions",
                         "Upper limit of the gap around text, which matters for large font sizes.")
            max: 5
        }

        EoNumber {
            sid: "maskBarlinesMinFragmentLength"
            title: qsTrc("notation/engravingoptions", "Barline masks: shortest barline fragment")
            hint: qsTrc("notation/engravingoptions",
                         "Barline pieces shorter than this, left between two masks or at an end of the barline, are removed.")
            max: 3
        }
    }
}
