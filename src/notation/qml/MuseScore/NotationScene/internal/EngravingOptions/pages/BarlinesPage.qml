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
    group: qsTrc("notation/engravingoptions", "Signs")
    music: "c''2 \\bar \"||\" c''2 \\bar \":|.\" c''2 \\bar \"|.\""

    EoSection {
        title: qsTrc("notation/engravingoptions", "Barlines")

        EoNumber {
            sid: "barWidth"
            title: qsTrc("EditStyleBase", "Thin barline thickness:")
            hint: qsTrc("notation/engravingoptions", "Normal barlines, and the thin line of final and repeat barlines.")
            step: 0.01
        }

        EoGrid {
            rowTitles: [
                qsTrc("notation/engravingoptions", "Double barline"),
                qsTrc("notation/engravingoptions", "Thick line of final and repeat barlines")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Thickness"),
                qsTrc("notation/engravingoptions", "Distance between the lines")
            ]

            EoNumber {
                sid: "doubleBarWidth"
                title: qsTrc("EditStyleBase", "Double barline thickness:")
                hint: qsTrc("notation/engravingoptions", "Thickness of each of the two lines of a double barline.")
                step: 0.01
            }

            EoNumber {
                sid: "doubleBarDistance"
                title: qsTrc("EditStyleBase", "Double barline distance:")
                hint: qsTrc("notation/engravingoptions", "White space between the two lines of a double barline.")
                step: 0.01
            }

            EoNumber {
                sid: "endBarWidth"
                title: qsTrc("EditStyleBase", "Thick barline thickness:")
                hint: qsTrc("notation/engravingoptions",
                            "The thick line of final and repeat barlines, and the heavy barline.")
                step: 0.01
            }

            EoNumber {
                sid: "endBarDistance"
                title: qsTrc("EditStyleBase", "Thick barline distance:")
                hint: qsTrc("notation/engravingoptions",
                            "White space between the thick line and the thin line beside it in final and repeat " +
                            "barlines.")
                step: 0.01
            }
        }

        EoNumber {
            sid: "repeatBarlineDotSeparation"
            title: qsTrc("notation/engravingoptions", "Repeat barline to dots distance")
            hint: qsTrc("notation/engravingoptions",
                        "White space between the thin line of a repeat barline and its two dots.")
            step: 0.01
        }

        EoToggle {
            sid: "repeatBarTips"
            title: qsTrc("notation/engravingoptions", "Show repeat barline tips (“winged” repeats)")
        }

        EoToggle {
            sid: "scaleBarlines"
            music: "\\smallStaff c''1 | c''1"
            title: qsTrc("notation/engravingoptions", "Scale barlines to staff size")
            hint: qsTrc("notation/engravingoptions",
                        "On a small staff the barlines become thinner and their lines closer together in proportion; " +
                        "off keeps the full-size thicknesses on every staff.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "At the start of the system")
        music: "\\staves 2 c''1 | c''1"

        EoToggle {
            sid: "startBarlineSingle"
            music: "c''1 | c''1"
            title: qsTrc("EditStyleBase", "Barline at start of single staff")
            hint: qsTrc("notation/engravingoptions",
                        "Draws a barline at the left end of every system which shows one staff only; a system with a " +
                        "bracket has it in any case.")
        }

        EoToggle {
            sid: "startBarlineMultiple"
            title: qsTrc("EditStyleBase", "Barline at start of multiple staves")
            hint: qsTrc("notation/engravingoptions",
                        "Draws the line which joins all staves at the left end of every system with more than one " +
                        "staff.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Before a start repeat with a new key or time signature")
        hint: qsTrc("notation/engravingoptions",
                    "These three settings act only at a start repeat barline; double barlines before ordinary " +
                    "changes are set under Clefs, key and time signatures.")
        music: "c''2 c''2 \\bar \".|:\" \\key 2 \\time 3/4 c''2. | c''2."

        EoToggle {
            sid: "barlineBeforeSigChange"
            title: qsTrc("notation/editstyle/barlines", "Show barline before key and time signature changes")
            hint: qsTrc("notation/engravingoptions",
                        "When a measure opening with a start repeat also brings a new key or time signature, a " +
                        "barline closes the measure before, in front of the signatures; off, they follow the last " +
                        "note with no barline.")
        }

        EoToggle {
            sid: "doubleBarlineBeforeKeySig"
            musicStyle: ({ barlineBeforeSigChange: true })
            title: qsTrc("notation/editstyle/barlines", "Use double barline before key signatures")
            hint: qsTrc("notation/engravingoptions",
                        "That barline is a double barline when the key changes. Only with ‘Show barline before key " +
                        "and time signature changes’ on.")
            enabled: Boolean(page.value("barlineBeforeSigChange"))
        }

        EoToggle {
            sid: "doubleBarlineBeforeTimeSig"
            musicStyle: ({ barlineBeforeSigChange: true })
            title: qsTrc("notation/editstyle/barlines", "Use double barline before time signatures")
            hint: qsTrc("notation/engravingoptions",
                        "That barline is a double barline when the time signature changes. Only with ‘Show barline " +
                        "before key and time signature changes’ on.")
            enabled: Boolean(page.value("barlineBeforeSigChange"))
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "One-line staves")
        music: "\\lines 1 f''1 | f''1"

        EoNumber {
            sid: "barlineOneLineStaffSpanFrom"
            title: qsTrc("notation/engravingoptions", "Top end of the barline")
            hint: qsTrc("notation/engravingoptions",
                        "How far barlines reach above the line of a one-line staff; negative numbers are above the " +
                        "line (−2 = 2 sp above it).")
            min: -8
            max: 0
            step: 0.5
            decimals: 1
            unit: qsTrc("global", "sp")
            unitScale: 0.5
        }

        EoNumber {
            sid: "barlineOneLineStaffSpanTo"
            title: qsTrc("notation/engravingoptions", "Bottom end of the barline")
            hint: qsTrc("notation/engravingoptions",
                        "How far barlines reach below the line of a one-line staff (2 = 2 sp below it).")
            max: 8
            step: 0.5
            decimals: 1
            unit: qsTrc("global", "sp")
            unitScale: 0.5
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Barlines which cross text")
        music: "\\staves 2 \\spanBarlines c''1 | \\staff 2 c''2 c''2^\"espressivo\" | c''1"

        EoToggle {
            sid: "maskBarlinesForText"
            title: qsTrc("EditStyleBase", "Mask barlines when intersecting text")
            hint: qsTrc("notation/engravingoptions",
                        "A barline which joins two staves is broken off where it would run through a text between " +
                        "them, such as a dynamic or a lyric; off, only a text whose own ‘Mask barlines’ property is " +
                        "on does this.")
        }

        EoNumber {
            sid: "maskBarlinesCollisionPadding"
            music: "\\staves 2 \\spanBarlines c''1 | \\staff 2 c''2^\"espressivo\" c''2 | c''1"
            musicStyle: ({ maskBarlinesForText: true, measureSpacing: 1.0 })
            title: qsTrc("notation/engravingoptions", "Distance at which a barline is broken")
            hint: qsTrc("notation/engravingoptions",
                        "A barline which passes closer than this to a text is broken there, even if it does not " +
                        "touch it; given for 10 pt text and growing with the text size.")
            max: 2
        }

        EoNumber {
            sid: "maskBarlinesPadding"
            musicStyle: ({ maskBarlinesForText: true })
            title: qsTrc("notation/engravingoptions", "Gap between text and barline")
            hint: qsTrc("notation/engravingoptions",
                        "White space left between the text and the ends of the broken barline; given for 10 pt text " +
                        "and growing with the text size. Framed text gets no extra gap.")
            max: 3
        }

        EoNumber {
            sid: "maskBarlinesPaddingMin"
            musicStyle: ({ maskBarlinesForText: true, maskBarlinesPadding: 0.0 })
            title: qsTrc("notation/engravingoptions", "Smallest gap")
            hint: qsTrc("notation/engravingoptions",
                        "The gap between text and barline never becomes smaller than this, however small the text.")
            max: 3
        }

        EoNumber {
            sid: "maskBarlinesPaddingMax"
            musicStyle: ({ maskBarlinesForText: true, maskBarlinesPadding: 1.5 })
            title: qsTrc("notation/engravingoptions", "Largest gap")
            hint: qsTrc("notation/engravingoptions",
                        "The gap between text and barline never becomes larger than this, however large the text.")
            max: 5
        }

        EoNumber {
            sid: "maskBarlinesMinFragmentLength"
            musicStyle: ({ maskBarlinesForText: true, maskBarlinesPadding: 0.25 })
            title: qsTrc("notation/engravingoptions", "Shortest piece of barline left standing")
            hint: qsTrc("notation/engravingoptions",
                        "A stub of barline shorter than this, between two texts or between a text and the end of the " +
                        "barline, is removed as well.")
            max: 3
        }
    }
}
