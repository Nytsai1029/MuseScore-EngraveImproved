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

    code: "hammer-ons-pull-offs-and-tapping"
    legacyCodes: ["hammer-ons-pull-offs-and-tapping"]
    title: qsTrc("EditStyleBase", "Hammer-ons, pull-offs & tapping")
    group: qsTrc("notation/engravingoptions", "Lines")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Hammer-ons and pull-offs")
        music: "\\hopoOn e'4( g'4 a'4 g'4) \\hopoOff"

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "Aligned letters stand in a row at one height, together with a tapping ‘T’ on the first note " +
                        "of the slur.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Show ‘H’ and ‘P’"),
                qsTrc("notation/engravingoptions", "Align the letters of one slur")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Standard staves"),
                qsTrc("notation/engravingoptions", "Tablature staves")
            ]

            EoToggle {
                sid: "hopoShowOnStandardStaves"
                hint: qsTrc("notation/engravingoptions",
                            "Writes ‘H’ and ‘P’ over hammer-on and pull-off slurs on standard staves.")
            }

            EoToggle {
                sid: "hopoShowOnTabStaves"
                music: "\\tab \\hopoOn e'4( g'4 a'4 g'4) \\hopoOff"
                hint: qsTrc("notation/engravingoptions",
                            "Writes ‘H’ and ‘P’ over hammer-on and pull-off slurs on tablature staves.")
            }

            EoToggle {
                sid: "hopoAlignLettersStandardStaves"
                music: "\\hopoOn c'4( g'4 e''4 g'4) \\hopoOff"
                hint: qsTrc("notation/engravingoptions",
                            "On standard staves the letters under one slur, and a tapping ‘T’ on its first note, " +
                            "stand in a row at one height.")
            }

            EoToggle {
                sid: "hopoAlignLettersTabStaves"
                music: "\\tab \\hopoOn e'4( g'4 a'4 g'4) \\hopoOff"
                hint: qsTrc("notation/engravingoptions",
                            "On tablature staves the letters under one slur, and a tapping ‘T’ on its first note, " +
                            "stand in a row at one height.")
            }
        }

        EoChoice {
            sid: "hopoUpperCase"
            title: qsTrc("notation/editstyle/hammeronpulloff", "Case")
            hint: qsTrc("notation/engravingoptions", "‘H’ and ‘P’, or ‘h’ and ‘p’.")
            choices: [
                { value: true, title: qsTrc("notation/engravingoptions", "Upper case") },
                { value: false, title: qsTrc("notation/engravingoptions", "Lower case") }
            ]
        }

        EoChoice {
            sid: "hopoShowAll"
            title: qsTrc("notation/editstyle/hammeronpulloff", "Consecutive hammer-ons/pull-offs")
            hint: qsTrc("notation/engravingoptions",
                        "What is written when one slur covers several hammer-ons or pull-offs in a row.")
            choices: [
                { value: true, title: qsTrc("notation/engravingoptions",
                                            "Show ‘H’ or ‘P’ between each pair of notes") },
                { value: false, title: qsTrc("notation/engravingoptions",
                                             "Show ‘H’ or ‘P’ once per group of ascending/descending notes") }
            ]
        }

        EoNumber {
            sid: "hopoTextVerticalPadding"
            music: "\\hopoOn e'4( g'4) \\hopoOff r2"
            title: qsTrc("notation/engravingoptions", "Gap between the letters and the slur")
            hint: qsTrc("notation/engravingoptions",
                        "Vertical gap between an ‘H’ or ‘P’ and the slur, or whatever else lies next to it; the " +
                        "letters also stay at least this far outside the staff.")
            max: 3
            step: 0.05
        }

        EoNumber {
            sid: "hopoTextNoteDistanceRatio"
            music: "\\hopoOn \\stemUp g''4( a''4) \\hopoOff r2"
            title: qsTrc("notation/engravingoptions", "Least distance of the letters from the notes")
            hint: qsTrc("notation/engravingoptions",
                        "The letter keeps at least this distance from the nearer of its two notes, given as a " +
                        "multiple of ‘Gap between the letters and the slur’.")
            max: 6
            step: 0.1
            decimals: 1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Tapping")

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "A left-hand tap is marked by a half slur leading into the note, by a symbol over it, or by " +
                        "both; its symbol only matters where a symbol is shown.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Left-hand tapping: symbol"),
                qsTrc("notation/engravingoptions", "Left-hand tapping: shown as"),
                qsTrc("notation/engravingoptions", "Right-hand tapping: symbol")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Standard staves"),
                qsTrc("notation/engravingoptions", "Tablature staves")
            ]

            EoChoice {
                sid: "lhTappingSymbolNormalStave"
                music: "e'4 g'4 \\tapping left a'4 \\tapping left r4"
                musicStyle: ({ lhTappingShowItemsNormalStave: 2 })
                hint: qsTrc("notation/engravingoptions",
                            "The symbol of a left-hand tap on standard staves, where one is shown.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Dot") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Circled T") }
                ]
            }

            EoChoice {
                sid: "lhTappingSymbolTab"
                music: "\\tab e'4 g'4 \\tapping left a'4 \\tapping left r4"
                musicStyle: ({ lhTappingShowItemsTab: 2 })
                hint: qsTrc("notation/engravingoptions",
                            "The symbol of a left-hand tap on tablature staves, where one is shown.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Dot") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Circled T") }
                ]
            }

            EoChoice {
                sid: "lhTappingShowItemsNormalStave"
                music: "e'4 g'4 \\tapping left a'4 \\tapping left r4"
                hint: qsTrc("notation/engravingoptions",
                            "Whether a left-hand tap on standard staves is marked by a half slur leading into the " +
                            "note, by a symbol over it, or by both.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Half slur") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Symbol") },
                    { value: 2, title: qsTrc("notation/engravingoptions", "Both") }
                ]
            }

            EoChoice {
                sid: "lhTappingShowItemsTab"
                music: "\\tab e'4 g'4 \\tapping left a'4 \\tapping left r4"
                hint: qsTrc("notation/engravingoptions",
                            "Whether a left-hand tap on tablature staves is marked by a half slur leading into the " +
                            "note, by a symbol over it, or by both.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Half slur") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Symbol") },
                    { value: 2, title: qsTrc("notation/engravingoptions", "Both") }
                ]
            }

            EoChoice {
                sid: "rhTappingSymbolNormalStave"
                music: "e'4 g'4 \\tapping a'4 r4"
                hint: qsTrc("notation/engravingoptions", "The symbol of a right-hand tap on standard staves.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Letter T") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Plus sign") }
                ]
            }

            EoChoice {
                sid: "rhTappingSymbolTab"
                music: "\\tab e'4 g'4 \\tapping a'4 r4"
                hint: qsTrc("notation/engravingoptions", "The symbol of a right-hand tap on tablature staves.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Letter T") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Plus sign") }
                ]
            }
        }

        EoToggle {
            sid: "lhTappingSlurTopAndBottomNoteOnTab"
            music: "\\tab e'4 <g' c''>4 \\tapping left r2"
            title: qsTrc("notation/engravingoptions", "On tablature, slur both the top and the bottom note of a chord")
            hint: qsTrc("notation/engravingoptions",
                        "Off: a chord tapped with the left hand gets a single half slur, on its top note. Only where " +
                        "left-hand taps are shown with a half slur on tablature staves.")
        }

        EoNumber {
            sid: "lhTappingDotMag"
            music: "e'4 g'4 \\tapping left a'4 \\tapping left r4"
            musicStyle: ({ lhTappingShowItemsNormalStave: 2 })
            title: qsTrc("notation/engravingoptions", "Size of the left-hand tapping dot")
            hint: qsTrc("notation/engravingoptions",
                        "As a fraction of the dot’s normal size in the music font; 0.5 is half size.")
            min: 0.2
            max: 2
        }
    }
}
