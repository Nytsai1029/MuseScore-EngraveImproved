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
    group: qsTrc("notation/engravingoptions", "Signs")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Default positions")
        hint: qsTrc("notation/engravingoptions", "These three settings act on dynamics and hairpins alike.")

        EoChoice {
            sid: "dynamicsHairpinVoiceBasedPlacement"
            music: "c''2 \\p \\< d''2 | e''1 \\! \\f"
            title: qsTrc("EditStyleBase", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "‘Based on voice’ puts them below the staff, except where several voices share a staff: " +
                        "there the marks of voices 1 and 3 go above and those of voices 2 and 4 below.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Based on voice") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoToggle {
            sid: "dynamicsHairpinsAboveForVocalStaves"
            music: "\\instrument voice c''2 \\p \\< d''2 | e''1 \\! \\f"
            title: qsTrc("EditStyleBase", "Place above the staff on vocal instruments")
            hint: qsTrc("notation/engravingoptions",
                        "Dynamics and hairpins of singers’ staves go above the staff, out of the way of the lyrics. " +
                        "Only with the position ‘Based on voice’.")
        }

        EoToggle {
            sid: "dynamicsHairpinsAutoCenterOnGrandStaff"
            music: "\\staves 2 c''2 \\p \\< d''2 | e''1 \\! \\f"
            title: qsTrc("EditStyleBase", "Center on grand staff instruments automatically")
            hint: qsTrc("notation/engravingoptions",
                        "On piano, harp and other instruments written on two or more staves, dynamics and hairpins " +
                        "which apply to the whole instrument are centred between the staves.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Dynamics")
        music: "c''2 \\p e''2 \\sfz"

        EoToggle {
            sid: "dynamicsOverrideFont"
            music: "c''2 \\p e''2 \\sfz"
            musicStyle: ({ dynamicsFont: "Petaluma" })
            title: qsTrc("notation/engravingoptions", "Use a different music font for dynamics")
            hint: qsTrc("notation/engravingoptions",
                        "Off, the letters of dynamics come from the musical symbols font of the score.")
        }

        EoChoice {
            sid: "dynamicsFont"
            music: "c''2 \\p e''2 \\sfz"
            musicStyle: ({ dynamicsOverrideFont: true })
            title: qsTrc("EditStyleBase", "Font:")
            hint: qsTrc("notation/engravingoptions",
                        "The music font from which the letters of dynamics are taken. Only with ‘Use a different " +
                        "music font for dynamics’ on.")
            presentation: "dropdown"
            choices: page.model.choices("musicFonts")
            enabled: Boolean(page.value("dynamicsOverrideFont"))
        }

        EoNumber {
            sid: "dynamicsSize"
            music: "c''2 \\p e''2 \\sfz"
            title: qsTrc("EditStyleBase", "Scale:")
            hint: qsTrc("notation/engravingoptions", "Size of dynamics relative to their normal size.")
            max: 1000
            step: 1.0
            decimals: 0
            percent: true
        }

        EoToggle {
            sid: "avoidBarLines"
            music: "\\staves 2 \\spanBarlines c''4 c''4 c''4 c''4 | c''4 \\fff c''4 c''4 c''4"
            title: qsTrc("EditStyleBase", "Avoid barlines")
            hint: qsTrc("notation/engravingoptions",
                        "A dynamic between two staves which are joined by barlines is moved sideways so that it does " +
                        "not sit on the barline.")
        }

        EoGrid {
            rowTitles: [
                qsTrc("notation/engravingoptions", "Position")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Above the staff"),
                qsTrc("notation/engravingoptions", "Below the staff")
            ]

            EoOffset {
                sid: "dynamicsPosAbove"
                music: "c''2 \\p e''2 \\sfz"
                musicStyle: ({ dynamicsHairpinVoiceBasedPlacement: 1 })
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and height of a dynamic placed above, measured from the top staff line " +
                            "(negative is up); notes and signs in the way still push it further out.")
            }

            EoOffset {
                sid: "dynamicsPosBelow"
                music: "c''2 \\p e''2 \\sfz"
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and depth of a dynamic placed below, measured from the bottom staff " +
                            "line (positive is down); notes and signs in the way still push it further out.")
            }
        }

        EoNumber {
            sid: "dynamicsMinDistance"
            music: "c'2 \\p a2 \\f"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a dynamic and the notes, stems, slurs or other signs which lie " +
                        "between it and the staff.")
            max: 1000
            step: 0.2
            decimals: 1
        }

        EoNumber {
            sid: "dynamicsEndOfPreviousDistance"
            music: "c''2 c''2 | c''2 \\p \\atEndOfPrevious c''2"
            title: qsTrc("notation/engravingoptions", "Gap after a dynamic placed before its beat")
            hint: qsTrc("notation/engravingoptions",
                        "For a dynamic set to stand at the end of the note before, such as a p just before a " +
                        "barline: the gap between its right edge and the barline, clef or note at which its beat " +
                        "begins.")
            max: 3
        }

        EoNumber {
            sid: "dynamicsMinBarlineDistance"
            music: "\\staves 2 \\spanBarlines c''2 c''2 | c''2 \\sfz c''2"
            title: qsTrc("notation/engravingoptions", "Minimum distance from barlines")
            hint: qsTrc("notation/engravingoptions",
                        "With ‘Avoid barlines’ on: how far from the barline a dynamic ends up after it has been " +
                        "moved aside.")
            max: 2
        }

        EoNumber {
            sid: "expressionDynamicDistance"
            music: "c''2 \\p \\expression \"dolce\" c''2"
            title: qsTrc("notation/engravingoptions", "Gap between a dynamic and expression text")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal gap between a dynamic and the expression text which follows it on the same beat, " +
                        "as in p dolce; it grows with the size of both.")
            max: 3
        }

        EoNumber {
            sid: "expressionDynamicForteDistance"
            music: "c''2 \\f \\expression \"espress.\" c''2"
            title: qsTrc("notation/engravingoptions", "Gap after a dynamic ending in f")
            hint: qsTrc("notation/engravingoptions",
                        "Used instead of the gap above when the dynamic ends with the letter f, whose top leans to " +
                        "the right and needs less room.")
            max: 3
        }

        EoToggle {
            sid: "dynamicsUseOpticalCenter"
            music: "a'2 \\f a'2 \\sfz"
            title: qsTrc("notation/engravingoptions", "Centre dynamics on their optical centre")
            hint: qsTrc("notation/engravingoptions",
                        "A dynamic centred on a note is aligned by the optical centre which the music font gives for " +
                        "it, not by the middle of its outline; a slanted f, for example, then looks centred.")
        }

        EoToggle {
            sid: "centerOnNotehead"
            music: "c''2 \\p e''2 \\sfz"
            title: qsTrc("inspector", "Center on notehead")
            hint: qsTrc("notation/engravingoptions",
                        "A dynamic is centred under the middle of its notehead. Off: it follows the alignment of the " +
                        "text style ‘Dynamics’, so a left-aligned dynamic starts at the left edge of the note.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Hairpins")
        music: "c''2 \\< d''2 | e''1 \\!"

        EoGrid {
            rowTitles: [
                qsTrc("notation/engravingoptions", "Position")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Above the staff"),
                qsTrc("notation/engravingoptions", "Below the staff")
            ]

            EoOffset {
                sid: "hairpinPosAbove"
                music: "c''2 \\< d''2 | e''1 \\!"
                musicStyle: ({ dynamicsHairpinVoiceBasedPlacement: 1 })
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and height of the middle line of a hairpin placed above, measured from " +
                            "the top staff line (negative is up); notes and signs in the way still push it further " +
                            "out.")
            }

            EoOffset {
                sid: "hairpinPosBelow"
                music: "c''2 \\< d''2 | e''1 \\!"
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and depth of the middle line of a hairpin placed below, measured from " +
                            "the bottom staff line (positive is down); notes and signs in the way still push it " +
                            "further out.")
            }
        }

        EoNumber {
            sid: "hairpinHeight"
            music: "c''2 \\< d''2 | e''1 \\!"
            title: qsTrc("EditStyleBase", "Height:")
            hint: qsTrc("notation/engravingoptions", "Distance between the two lines at the open end of a hairpin.")
            step: 0.1
        }

        EoNumber {
            sid: "hairpinContHeight"
            music: "c''1 \\< \\break d''2 e''2 \\!"
            systemWidth: 16
            title: qsTrc("notation/engravingoptions", "Opening at a system break")
            hint: qsTrc("notation/engravingoptions",
                        "Distance between the two lines at the narrow end of a hairpin which is broken off at the " +
                        "end of a system or taken up again on the next one.")
            step: 0.1
        }

        EoNumber {
            sid: "hairpinLineWidth"
            music: "c''2 \\< d''2 | e''1 \\!"
            title: qsTrc("EditStyleBase", "Line thickness:")
            step: 0.01
        }

        EoToggle {
            sid: "hairpinVerticalEnds"
            music: "c''2 \\< \\diagonal -2 d''2 | e''1 \\!"
            title: qsTrc("notation/engravingoptions", "Vertical ends on diagonal hairpins")
            hint: qsTrc("notation/engravingoptions",
                        "When a hairpin is drawn at an angle, its open end is cut vertically instead of at right " +
                        "angles to its direction.")
        }

        EoNumber {
            sid: "hairpinCircledTipDiameter"
            music: "c''2 \\< \\niente d''2 | e''1 \\!"
            title: qsTrc("notation/engravingoptions", "Diameter of the niente circle")
            hint: qsTrc("notation/engravingoptions",
                        "The small circle at the tip of a hairpin which starts from nothing or fades to nothing.")
            min: 0.2
            max: 2
        }

        EoNumber {
            sid: "hairpinMinLength"
            music: "c''16 \\< d''16 \\! e''8 f''4 g''2"
            title: qsTrc("notation/engravingoptions", "Minimum length")
            hint: qsTrc("notation/engravingoptions",
                        "A hairpin, or the part of it on one system, is never drawn shorter than this.")
            max: 5
            step: 0.1
        }

        EoNumber {
            sid: "hairpinTextDistance"
            music: "c''2 \\< d''2 | e''1 \\!"
            musicStyle: ({ hairpinText: "più" })
            title: qsTrc("notation/engravingoptions", "Gap between a hairpin and its text")
            hint: qsTrc("notation/engravingoptions",
                        "Space between the lines of a hairpin and a text attached to its start or end.")
            max: 3
        }

        EoNumber {
            sid: "autoplaceHairpinDynamicsDistance"
            music: "c''2 \\p \\< d''2 | e''1 \\! \\f"
            title: qsTrc("notation/engravingoptions", "Distance from dynamics")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal gap between the end of a hairpin and the dynamic or expression text before or " +
                        "after it.")
            step: 0.1
        }

        EoNumber {
            sid: "hairpinSnapExtendThreshold"
            music: "c''2 \\p \\< d''2 | e''1 \\! \\f"
            musicStyle: ({ hairpinEndInset: 2.75 })
            title: qsTrc("notation/engravingoptions", "Extend to following dynamic when gap exceeds")
            hint: qsTrc("notation/engravingoptions",
                        "A hairpin which stops short of the dynamic or expression text after it is lengthened to " +
                        "reach it only if the gap left is wider than this; a narrower gap stays open.")
            max: 10
            step: 0.1
        }

        EoNumber {
            sid: "hairpinEndInset"
            music: "c''2 \\< d''2 | e''1 \\!"
            title: qsTrc("notation/engravingoptions", "Distance of the end before the next note or barline")
            hint: qsTrc("notation/engravingoptions",
                        "A hairpin stops this far before the note, rest or barline at which it ends, unless a " +
                        "dynamic there decides where it ends.")
            max: 5
        }

        EoNumber {
            sid: "hairpinMinDistance"
            music: "c'2 \\< a2 \\! | c'1"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a hairpin and the notes or other signs which lie between it and " +
                        "the staff. Cresc. and dim. lines use it too.")
        }

        EoText {
            sid: "hairpinText"
            music: "c''2 \\< d''2 | e''1 \\!"
            title: qsTrc("notation/engravingoptions", "Text at the start of every hairpin")
            hint: qsTrc("notation/engravingoptions",
                        "A text set here appears at the start of every hairpin, crescendo and diminuendo alike, and " +
                        "again on each following system. Normally empty; it has nothing to do with the cresc. and " +
                        "dim. lines of this section.")
        }

        EoChoice {
            sid: "hairpinLineStyle"
            music: "c''2 \\< d''2 | e''1 \\!"
            title: qsTrc("EditStyleBase", "Line style:")
            hint: qsTrc("notation/engravingoptions", "The two lines of a hairpin drawn solid, dashed or dotted.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Solid") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Dashed") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Dotted") }
            ]
        }

        EoNumber {
            sid: "hairpinDashLineLen"
            music: "c''2 \\< d''2 | e''1 \\!"
            musicStyle: ({ hairpinLineStyle: 1 })
            title: qsTrc("notation/engravingoptions", "Dash length")
            hint: qsTrc("notation/engravingoptions",
                        "Length of each dash as a multiple of the line thickness. Only with the line style ‘Dashed’.")
            visible: page.value("hairpinLineStyle") === 1
        }

        EoNumber {
            sid: "hairpinDashGapLen"
            music: "c''2 \\< d''2 | e''1 \\!"
            musicStyle: ({ hairpinLineStyle: 1 })
            title: qsTrc("notation/engravingoptions", "Gap length")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between two dashes as a multiple of the line thickness; it is evened out slightly so " +
                        "that the line ends with a full dash. Only with the line style ‘Dashed’.")
            visible: page.value("hairpinLineStyle") === 1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Cresc. and dim. written as text with a line")
        hint: qsTrc("notation/engravingoptions",
                    "The lines which begin with a word such as ‘cresc.’ or ‘dim.’ and run on as a dashed line, used " +
                    "in place of a hairpin.")
        music: "c''2 \\cresc d''2 | e''1 \\!"

        EoText {
            sid: "hairpinCrescText"
            music: "c''2 \\cresc d''2 | e''1 \\!"
            title: qsTrc("notation/engravingoptions", "Crescendo: text")
            hint: qsTrc("notation/engravingoptions", "The word with which a crescendo line begins.")
        }

        EoText {
            sid: "hairpinCrescContText"
            music: "c''1 \\cresc \\break d''2 e''2 \\!"
            systemWidth: 16
            title: qsTrc("notation/engravingoptions", "Crescendo: text on following systems")
            hint: qsTrc("notation/engravingoptions",
                        "The word which is repeated where a crescendo line goes on after a system break.")
        }

        EoText {
            sid: "hairpinDecrescText"
            music: "c''2 \\dim d''2 | e''1 \\!"
            title: qsTrc("notation/engravingoptions", "Diminuendo: text")
            hint: qsTrc("notation/engravingoptions", "The word with which a diminuendo line begins.")
        }

        EoText {
            sid: "hairpinDecrescContText"
            music: "c''1 \\dim \\break d''2 e''2 \\!"
            systemWidth: 16
            title: qsTrc("notation/engravingoptions", "Diminuendo: text on following systems")
            hint: qsTrc("notation/engravingoptions",
                        "The word which is repeated where a diminuendo line goes on after a system break.")
        }

        EoGrid {
            rowTitles: [
                qsTrc("notation/engravingoptions", "Position")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Above the staff"),
                qsTrc("notation/engravingoptions", "Below the staff")
            ]

            EoOffset {
                sid: "hairpinLinePosAbove"
                music: "c''2 \\cresc d''2 | e''1 \\!"
                musicStyle: ({ dynamicsHairpinVoiceBasedPlacement: 1 })
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and height of a cresc. or dim. line placed above, measured from the " +
                            "top staff line (negative is up); notes and signs in the way still push it further out.")
            }

            EoOffset {
                sid: "hairpinLinePosBelow"
                music: "c''2 \\cresc d''2 | e''1 \\!"
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and depth of a cresc. or dim. line placed below, measured from the " +
                            "bottom staff line (positive is down); notes and signs in the way still push it further " +
                            "out.")
            }
        }

        EoChoice {
            sid: "hairpinLineLineStyle"
            music: "c''2 \\cresc d''2 | e''1 \\!"
            title: qsTrc("EditStyleBase", "Line style:")
            hint: qsTrc("notation/engravingoptions", "The line which follows the word: dashed by default.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Solid") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Dashed") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Dotted") }
            ]
        }

        EoNumber {
            sid: "hairpinLineDashLineLen"
            music: "c''1 \\cresc | d''1 | e''1 \\!"
            title: qsTrc("notation/engravingoptions", "Dash length")
            hint: qsTrc("notation/engravingoptions",
                        "Length of each dash as a multiple of the line thickness. Only with the line style ‘Dashed’.")
            visible: page.value("hairpinLineLineStyle") === 1
        }

        EoNumber {
            sid: "hairpinLineDashGapLen"
            music: "c''1 \\cresc | d''1 | e''1 \\!"
            musicStyle: ({ hairpinLineDashLineLen: 4 })
            title: qsTrc("notation/engravingoptions", "Gap length")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between two dashes as a multiple of the line thickness; it is evened out slightly so " +
                        "that the line ends with a full dash. Only with the line style ‘Dashed’.")
            visible: page.value("hairpinLineLineStyle") === 1
        }
    }
}
