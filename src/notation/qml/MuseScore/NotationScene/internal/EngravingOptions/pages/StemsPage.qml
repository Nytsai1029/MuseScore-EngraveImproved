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

    code: "stems"
    title: qsTrc("notation/engravingoptions", "Stems and flags")
    group: qsTrc("notation/engravingoptions", "Notes and rests")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Stems")

        EoNumber {
            sid: "stemWidth"
            music: "c'4"
            title: qsTrc("EditStyleBase", "Stem thickness:")
            step: 0.1
        }

        EoNumber {
            sid: "stemLength"
            music: "g''4"
            title: qsTrc("EditStyleBase", "Standard stem length:")
            hint: qsTrc("notation/engravingoptions",
                        "Length of the stem of a note inside the staff, from the notehead to the stem tip (3.5 sp is " +
                        "an octave), before any shortening outside the staff or lengthening for beams and tremolos.")
            min: 1
            max: 10
            step: 0.25
            unit: qsTrc("global", "sp")
        }

        EoToggle {
            sid: "stemUpOnMiddleLine"
            music: "b'4 r4 r2"
            title: qsTrc("notation/engravingoptions", "Stems up on the middle line")
            hint: qsTrc("notation/engravingoptions",
                        "A note on the middle staff line, and a chord or beamed group whose notes balance around it, " +
                        "gets its stem up; when off, down.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Stem shortening outside the staff")
        music: "\\stemDown c,4"

        EoToggle {
            sid: "useDefaultStemShorteningRules"
            music: "\\stemUp f''4 e''4"
            musicStyle: ({ stemCustomLengthFirstLine: 4.0, stemCustomLengthFirstSpace: 4.0 })
            title: qsTrc("notation/editstyle/notes", "Apply default stem shortening rules")
            hint: qsTrc("notation/engravingoptions",
                        "On: stems whose tip stands out beyond the staff are shortened by the built-in rules, " +
                        "governed by the next four values. Off: the stem length is taken from the table ‘Custom stem " +
                        "length by notehead position’ instead.")
        }

        EoToggle {
            sid: "shortenStem"
            music: "\\stemDown c,4"
            title: qsTrc("EditStyleBase", "Shorten stems")
            hint: qsTrc("notation/engravingoptions",
                        "A stem whose tip would stand out beyond the staff (an up-stem on a high note, a down-stem " +
                        "on a low note) is shortened, the more the further it reaches out. Only with ‘Apply default " +
                        "stem shortening rules’ on.")
        }

        EoNumber {
            sid: "shortestStem"
            music: "\\stemDown c,4"
            title: qsTrc("EditStyleBase", "Shortest stem:")
            hint: qsTrc("notation/engravingoptions",
                        "The shortening never makes a stem shorter than this. Only with ‘Shorten stems’ on.")
            min: 1
            step: 0.25
            unit: qsTrc("global", "sp")
            enabled: Boolean(page.value("shortenStem"))
        }

        EoNumber {
            sid: "shortStemStartLocation"
            music: "\\stemUp g'4 a'4 b'4 c''4"
            title: qsTrc("EditStyleBase", "Shortening starts beyond the staff by (half spaces):")
            hint: qsTrc("notation/engravingoptions",
                        "How far the stem tip must stand out beyond the outer staff line before the stem is " +
                        "shortened: 1 is half a staff space, 2 a whole one. Only with ‘Shorten stems’ on.")
            max: 8
            step: 1
            decimals: 0
            enabled: Boolean(page.value("shortenStem"))
        }

        EoNumber {
            sid: "shortestStemWideBeams"
            music: "\\stemUp a''4 r4 r2"
            musicStyle: ({ useWideBeams: true })
            title: qsTrc("notation/engravingoptions", "Shortest stem with wide beam distance")
            hint: qsTrc("notation/engravingoptions",
                        "Takes the place of ‘Shortest stem’ when ‘Beam distance’ on the Beams page is ‘Wide’.")
            min: 1
            max: 6
            step: 0.25
        }

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Custom stem length by notehead position")
            hint: qsTrc("notation/engravingoptions",
                        "Used when ‘Apply default stem shortening rules’ is off. Positions are shown for stems " +
                        "pointing up on a five-line staff; stems pointing down mirror them. On other staves they are " +
                        "counted from the staff line nearest the stem tip. Noteheads beyond the opposite staff line " +
                        "keep the default rules. Flagged notes always use the standard stem length. Stems still " +
                        "lengthen for beams and flags that would otherwise collide.")
            cornerTitle: qsTrc("notation/engravingoptions", "Notehead position")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Top line and above"),
                qsTrc("notation/engravingoptions", "4th space"),
                qsTrc("notation/engravingoptions", "4th line"),
                qsTrc("notation/engravingoptions", "3rd space"),
                qsTrc("notation/engravingoptions", "Middle line"),
                qsTrc("notation/engravingoptions", "2nd space"),
                qsTrc("notation/engravingoptions", "2nd line"),
                qsTrc("notation/engravingoptions", "1st space"),
                qsTrc("notation/engravingoptions", "Bottom line")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Stem length")
            ]

            EoNumber {
                sid: "stemCustomLengthFirstLine"
                music: "\\stemUp f''4"
                musicStyle: ({ useDefaultStemShorteningRules: false })
                min: 0.5
                max: 12
                step: 0.25
                decimals: 2
                unit: qsTrc("global", "sp")
                enabled: Boolean(!page.value("useDefaultStemShorteningRules"))
            }

            EoNumber {
                sid: "stemCustomLengthFirstSpace"
                music: "\\stemUp e''4"
                musicStyle: ({ useDefaultStemShorteningRules: false })
                min: 0.5
                max: 12
                step: 0.25
                decimals: 2
                unit: qsTrc("global", "sp")
                enabled: Boolean(!page.value("useDefaultStemShorteningRules"))
            }

            EoNumber {
                sid: "stemCustomLengthSecondLine"
                music: "\\stemUp d''4"
                musicStyle: ({ useDefaultStemShorteningRules: false })
                min: 0.5
                max: 12
                step: 0.25
                decimals: 2
                unit: qsTrc("global", "sp")
                enabled: Boolean(!page.value("useDefaultStemShorteningRules"))
            }

            EoNumber {
                sid: "stemCustomLengthSecondSpace"
                music: "\\stemUp c''4"
                musicStyle: ({ useDefaultStemShorteningRules: false })
                min: 0.5
                max: 12
                step: 0.25
                decimals: 2
                unit: qsTrc("global", "sp")
                enabled: Boolean(!page.value("useDefaultStemShorteningRules"))
            }

            EoNumber {
                sid: "stemCustomLengthThirdLine"
                music: "\\stemUp b'4"
                musicStyle: ({ useDefaultStemShorteningRules: false })
                min: 0.5
                max: 12
                step: 0.25
                decimals: 2
                unit: qsTrc("global", "sp")
                enabled: Boolean(!page.value("useDefaultStemShorteningRules"))
            }

            EoNumber {
                sid: "stemCustomLengthThirdSpace"
                music: "\\stemUp a'4"
                musicStyle: ({ useDefaultStemShorteningRules: false })
                min: 0.5
                max: 12
                step: 0.25
                decimals: 2
                unit: qsTrc("global", "sp")
                enabled: Boolean(!page.value("useDefaultStemShorteningRules"))
            }

            EoNumber {
                sid: "stemCustomLengthFourthLine"
                music: "\\stemUp g'4"
                musicStyle: ({ useDefaultStemShorteningRules: false })
                min: 0.5
                max: 12
                step: 0.25
                decimals: 2
                unit: qsTrc("global", "sp")
                enabled: Boolean(!page.value("useDefaultStemShorteningRules"))
            }

            EoNumber {
                sid: "stemCustomLengthFourthSpace"
                music: "\\stemUp f'4"
                musicStyle: ({ useDefaultStemShorteningRules: false })
                min: 0.5
                max: 12
                step: 0.25
                decimals: 2
                unit: qsTrc("global", "sp")
                enabled: Boolean(!page.value("useDefaultStemShorteningRules"))
            }

            EoNumber {
                sid: "stemCustomLengthFifthLine"
                music: "\\stemUp e'4"
                musicStyle: ({ useDefaultStemShorteningRules: false })
                min: 0.5
                max: 12
                step: 0.25
                decimals: 2
                unit: qsTrc("global", "sp")
                enabled: Boolean(!page.value("useDefaultStemShorteningRules"))
            }
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Stem length with flags and beams")
        hint: qsTrc("notation/engravingoptions",
                    "Stem lengths are worked out in quarter staff spaces: the values of this section are rounded to " +
                    "0.25 sp.")
        music: "c'8[ e'' c'] c'16[ e'' e'' c'] c'32[ e'' e'' e'' c']"

        EoNumber {
            sid: "stemLengthThreeBeamsAddition"
            title: qsTrc("notation/engravingoptions", "Extra stem length for three beams")
            hint: qsTrc("notation/engravingoptions",
                        "Added to the standard stem length of notes under three beams. With four or more beams the " +
                        "stem grows by one beam distance for every beam beyond three instead.")
            max: 2
            step: 0.25
        }

        EoNumber {
            sid: "stemOpticalAdjustment"
            music: "g'8[ g'8] b'8[ b'8] a'8[ a'8] c''8[ c''8]"
            title: qsTrc("notation/engravingoptions", "Shortening of beamed stems ending in mid-space")
            hint: qsTrc("notation/engravingoptions",
                        "A stem with one or two beams whose tip would fall in the middle of a staff space is " +
                        "shortened by this much; with 0.25 sp the beam then lies centred on a staff line. 0 switches " +
                        "it off.")
            max: 0.5
            step: 0.25
        }

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Free stem under the beams")
            hint: qsTrc("notation/engravingoptions",
                        "A note in a beamed group, or in a two-note tremolo, keeps at least this much stem between " +
                        "its notehead and the nearest beam before the beam is positioned; the height of the beams " +
                        "comes on top.")
            cornerTitle: qsTrc("notation/engravingoptions", "Number of beams")
            rowTitles: [
                qsTrc("notation/engravingoptions", "No beam"),
                qsTrc("notation/engravingoptions", "One beam"),
                qsTrc("notation/engravingoptions", "Two beams"),
                qsTrc("notation/engravingoptions", "Three or more beams")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Shortest stem between notehead and beam")
            ]

            EoNumber {
                sid: "stemMinInnerLengthNoBeam"
                music: "\\stemUp \\tremolo 16 { c'''2 d'''2 }"
                min: 0.5
                max: 6
                step: 0.25
            }

            EoNumber {
                sid: "stemMinInnerLengthOneBeam"
                min: 0.5
                max: 6
                step: 0.25
            }

            EoNumber {
                sid: "stemMinInnerLengthTwoBeams"
                min: 0.5
                max: 6
                step: 0.25
            }

            EoNumber {
                sid: "stemMinInnerLengthThreeBeams"
                min: 0.5
                max: 6
                step: 0.25
            }
        }

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Notes far outside the staff")
            hint: qsTrc("notation/engravingoptions",
                        "The stem of a note on ledger lines is lengthened until its tip is this far inside the " +
                        "staff, measured from the staff line nearest the notehead; 2 sp is the middle line of a " +
                        "five-line staff. Unbeamed and flagged notes, and small notes, use the first row.")
            cornerTitle: qsTrc("notation/engravingoptions", "Number of beams")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Up to two beams"),
                qsTrc("notation/engravingoptions", "Three beams"),
                qsTrc("notation/engravingoptions", "Four beams"),
                qsTrc("notation/engravingoptions", "Four beams (wide beam distance)")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Stem reaches into the staff by")
            ]

            EoNumber {
                sid: "stemMinStaffOverlap"
                music: "\\stemDown c''''4 r4 r2"
                max: 6
                step: 0.25
            }

            EoNumber {
                sid: "stemMinStaffOverlapThreeBeams"
                music: "\\stemUp c'32[ c']"
                max: 6
                step: 0.25
            }

            EoNumber {
                sid: "stemMinStaffOverlapFourBeams"
                music: "\\stemDown c'''64[ e''' c''' e''']"
                max: 6
                step: 0.25
            }

            EoNumber {
                sid: "stemMinStaffOverlapFourBeamsWide"
                music: "\\stemDown c''''64[ d'''' c'''' d'''']"
                musicStyle: ({ useWideBeams: true })
                max: 6
                step: 0.25
            }
        }

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Four beams just outside the staff")
            hint: qsTrc("notation/engravingoptions",
                        "For a note with four beams which sits just outside the staff with its stem pointing towards " +
                        "the staff: on the first ledger line, or one or two steps beyond it.")
            cornerTitle: qsTrc("notation/engravingoptions", "Notehead position")
            rowTitles: [
                qsTrc("notation/engravingoptions", "First ledger line"),
                qsTrc("notation/engravingoptions", "Beyond the first ledger line")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Shortest stem with four beams")
            ]

            EoNumber {
                sid: "stemFourBeamsMinLengthFirstLedger"
                music: "\\stemUp c'64[ c' c' c']"
                min: 2.5
                max: 8
                step: 0.25
            }

            EoNumber {
                sid: "stemFourBeamsMinLengthSecondLedger"
                music: "\\stemUp a64[ a a a]"
                min: 2.5
                max: 8
                step: 0.25
            }
        }

        EoNumber {
            sid: "stemFourBeamsMinLengthInStaffAddition"
            music: "\\stemUp a'64[ a' a' a']"
            title: qsTrc("notation/engravingoptions", "Extra stem length for four or more beams inside the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Added to the shortest stem of a note with four or more beams whose notehead lies inside the " +
                        "staff, in quarter staff spaces: 1 makes those stems 0.25 sp longer.")
            max: 4
            step: 1
            decimals: 0
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Flags")

        EoChoice {
            sid: "useStraightNoteFlags"
            music: "\\stemUp g'8 r8 \\stemDown c''16 r16 r8"
            title: qsTrc("EditStyleBase", "Flag style")
            presentation: "tiles"
            choices: [
                { value: false, title: qsTrc("notation/engravingoptions", "Traditional") },
                { value: true, title: qsTrc("notation/engravingoptions", "Straight") }
            ]
        }

        EoNumber {
            sid: "flagStemCutoutUp"
            music: "\\stemUp c''64 r r r"
            title: qsTrc("notation/engravingoptions", "Flag height the stem need not cover, stem up")
            hint: qsTrc("notation/engravingoptions",
                        "Taken off the height of a curved flag when working out how much stem the flag needs, " +
                        "because its tip curls away from the stem; a larger value allows a shorter stem. It shows on " +
                        "flagged notes with tremolo strokes: ordinary stems are longer than their flags need anyway.")
            max: 3
            step: 0.25
        }

        EoNumber {
            sid: "flagStemCutoutDown"
            music: "\\stemDown c''64 r r r"
            title: qsTrc("notation/engravingoptions", "Flag height the stem need not cover, stem down")
            hint: qsTrc("notation/engravingoptions",
                        "The same for down-stem notes, whose flags are shaped differently.")
            max: 3
            step: 0.25
        }

        EoNumber {
            sid: "flagStemMaxHeight"
            music: "\\stemUp c''32 r r r"
            title: qsTrc("notation/engravingoptions", "Largest flag height the stem makes room for")
            hint: qsTrc("notation/engravingoptions",
                        "A flag taller than this counts as this tall when the stem is made long enough for it, so " +
                        "that music fonts with very long flags do not give overlong stems.")
            min: 1
            max: 8
            step: 0.25
        }
    }
}
