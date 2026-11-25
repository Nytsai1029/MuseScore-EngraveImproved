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

    code: "beams"
    legacyCodes: ["beams"]
    title: qsTrc("EditStyleBase", "Beams")
    group: qsTrc("notation/engravingoptions", "Notes and rests")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Beams")
        music: "c'32[ e'32 g'32 e'32 g'32]"

        EoChoice {
            sid: "useWideBeams"
            title: qsTrc("notation", "Beam distance")
            hint: qsTrc("notation/engravingoptions",
                        "Distance from one beam to the next in a group with several beams, measured between the same " +
                        "edges: ¾ sp (regular) or 1 sp (wide).")
            presentation: "tiles"
            choices: [
                { value: false, title: qsTrc("notation/engravingoptions", "Regular") },
                { value: true, title: qsTrc("notation/engravingoptions", "Wide") }
            ]
            tileWidth: 200
            tileHeight: 96
        }

        EoNumber {
            sid: "beamWidth"
            title: qsTrc("notation", "Beam thickness")
        }

        EoNumber {
            sid: "beamMinLen"
            music: "c'8.[ c'16 c'8. c'16]"
            title: qsTrc("notation/engravingoptions", "Length of beamlets")
            hint: qsTrc("notation/engravingoptions",
                        "Length of the short stub of beam on a single note, as on the sixteenth after a dotted " +
                        "eighth.")
        }

        EoNumber {
            sid: "beamletPadding"
            music: "c'16[ e'8.]"
            title: qsTrc("notation/engravingoptions", "Gap after a beamlet")
            hint: qsTrc("notation/engravingoptions",
                        "Smallest horizontal gap between the free end of a beamlet which points to the right and the " +
                        "note or sign that follows. The same gap is kept after the strokes of a single-note tremolo.")
        }

        EoChoice {
            sid: "frenchStyleBeams"
            music: "c'32[ e'32 g'32 b'32 g'32 e'32]"
            title: qsTrc("notation", "Beam style")
            presentation: "tiles"
            choices: [
                { value: false, title: qsTrc("notation/engravingoptions", "Draw inner stems through beams") },
                { value: true, title: qsTrc("notation/engravingoptions",
                                            "Draw inner stems to nearest beam (“French” style)") }
            ]
            tileWidth: 200
            tileHeight: 96
        }

        EoToggle {
            sid: "snapCustomBeamsToGrid"
            music: "\\beamAt 4.4 4.4 c'8[ d'8 e'8 c''8]"
            title: qsTrc("notation", "Snap manually adjusted beams to quarter spaces")
            hint: qsTrc("notation/engravingoptions",
                        "A beam which was moved by hand has each of its ends rounded to the nearest quarter of a " +
                        "staff space.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Beam slant")
        music: "c'8[ g' e' c''8]"

        EoToggle {
            sid: "beamNoSlope"
            title: qsTrc("notation", "Flatten all beams")
            hint: qsTrc("notation/engravingoptions",
                        "Every beam is drawn horizontal, whatever its notes. A single beam can still be changed in " +
                        "its own properties.")
        }

        EoToggle {
            sid: "useDefaultBeamSlantRules"
            title: qsTrc("notation", "Apply default beam slant rules")
            hint: qsTrc("notation/engravingoptions",
                        "On: the slant follows the built-in rules of ‘Slant: default rules’, which also keep the " +
                        "beam clear of the staff lines. Off: the slant is taken from the tables of ‘Slant: custom " +
                        "rules’.")
        }

        EoToggle {
            sid: "beamFlattenConcave"
            music: "\\stemUp d'8[ a'8 f'8]"
            title: qsTrc("notation/engravingoptions", "Flatten concave beam groups")
            hint: qsTrc("notation/engravingoptions",
                        "A group in which a note between the ends lies nearer the beam than both end notes gets a " +
                        "horizontal beam; when off, such a beam slants by its end notes.")
        }

        EoToggle {
            sid: "beamTwoPitchSlantRule"
            music: "\\stemUp g'8[ e'8 e'8 e'8] e'8[ g'8 g'8 g'8] e'8[ g'8 e'8 g'8]"
            title: qsTrc("notation/engravingoptions", "Groups on two pitches follow their own slant rule")
            hint: qsTrc("notation/engravingoptions",
                        "Groups of three or more notes which use only two pitches: if a single note at one end " +
                        "differs from all the others, the beam slants when that note is the one nearer the beam and " +
                        "stays horizontal when it is the farther one; two pitches in equal numbers slant the beam. " +
                        "When off, such groups follow the general rules.")
        }

        EoNumber {
            sid: "beamCrossStaffLevelNotesSlant"
            music: "\\\staves 2 \\\beamMove -1 c''8[ \\\crossStaff down e'8 f'8 a''8]"
            title: qsTrc("notation/engravingoptions", "Slant of a cross-staff beam over level notes")
            hint: qsTrc("notation/engravingoptions",
                        "For a cross-staff beam which lies above or below both staves: when the notes on the staff " +
                        "next to the beam are level but the group begins and ends on different staves, the beam gets " +
                        "this slant towards the other staff.")
            max: 2
            step: 0.25
            decimals: 2
            unitScale: 0.25
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Slant: default rules")
        hint: qsTrc("notation/engravingoptions",
                    "Used while ‘Apply default beam slant rules’ is on. A slant is the difference in height between " +
                    "the two ends of the beam.")
        music: "\\stemDown c'''32[ a''']"

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Maximum slant by interval and by beam length")
            hint: qsTrc("notation/engravingoptions",
                        "A beam slants by the smaller of two limits: the maximum slant for the interval between its " +
                        "first and last notes, and the maximum slant of the first row whose length it stays under; " +
                        "so a short beam cannot slant much, whatever its interval. Lengths are measured from the " +
                        "first to the last stem.")
            cornerTitle: qsTrc("notation/engravingoptions", "Interval between first and last note")
            rowTitles: [
                qsTrc("notation/engravingoptions", "2nd"),
                qsTrc("notation/engravingoptions", "3rd"),
                qsTrc("notation/engravingoptions", "4th"),
                qsTrc("notation/engravingoptions", "5th"),
                qsTrc("notation/engravingoptions", "6th"),
                qsTrc("notation/engravingoptions", "7th"),
                qsTrc("notation/engravingoptions", "Octave and more")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Maximum slant (quarter spaces)"),
                qsTrc("notation/engravingoptions", "Also the most for beams shorter than")
            ]

            EoNumber {
                sid: "beamDefaultMaxSlantSecondInterval"
                music: "c''8[ d''] c''8[ e'']"
                max: 3
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamDefaultSlantSecondMaxWidth"
                music: "\\stemDown c'16[ d' e']"
                max: 40
                step: 0.5
            }

            EoNumber {
                sid: "beamDefaultMaxSlantThirdInterval"
                music: "\\stemDown c'8[ d' e']"
                max: 3
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamDefaultSlantThirdMaxWidth"
                music: "b'8[ d'' e'' f'']"
                musicStyle: ({ measureSpacing: 2.05 })
                max: 40
                step: 0.5
            }

            EoNumber {
                sid: "beamDefaultMaxSlantFourthInterval"
                music: "\\stemUp f'8[ g'8 a'8 b'8]"
                max: 3
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamDefaultSlantFourthMaxWidth"
                music: "d''8[ f'' a'' b'']"
                musicStyle: ({ measureSpacing: 1.35 })
                max: 40
                step: 0.5
            }

            EoNumber {
                sid: "beamDefaultMaxSlantFifthInterval"
                music: "\\stemUp f'8[ g'8 a'8 b'8 c''8]"
                max: 3
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamDefaultSlantFifthMaxWidth"
                music: "c'8[ e' f' g' a']"
                musicStyle: ({ measureSpacing: 1.36 })
                max: 40
                step: 0.5
            }

            EoNumber {
                sid: "beamDefaultMaxSlantSixthInterval"
                music: "\\stemUp c'8[ d'8 e'8 f'8 g'8 a'8]"
                max: 3
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamDefaultSlantSixthMaxWidth"
                music: "c''8[ d'' e'' f'' g'' a'' b'']"
                musicStyle: ({ measureSpacing: 1.37 })
                max: 40
                step: 0.5
            }

            EoNumber {
                sid: "beamDefaultMaxSlantSeventhInterval"
                music: "\\stemDown c'8[ d' e' f' g' a' b' c'']"
                max: 3
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamDefaultSlantSeventhMaxWidth"
                music: "b'8[ c'' d'' e'' f'' g'' a'' b'']"
                musicStyle: ({ measureSpacing: 1.21 })
                max: 40
                step: 0.5
            }

            EoNumber {
                sid: "beamDefaultMaxSlantOctave"
                music: "d''8[ e'' f'' g'' a'' b'' c''' d''']"
                musicStyle: ({ measureSpacing: 1.15 })
                max: 3
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoGap {
            }
        }

        EoNumber {
            sid: "beamDefaultRepeatedEndNoteSlant"
            music: "\\stemUp a'8[ a'8 f'8 d'8]"
            title: qsTrc("notation/engravingoptions", "Slant when the end note is repeated")
            hint: qsTrc("notation/engravingoptions",
                        "When the end note nearest the beam is repeated by the note next to it, the beam would be " +
                        "horizontal; it gets this small slant instead.")
            max: 3
            step: 0.25
            decimals: 2
            unitScale: 0.25
        }

        EoNumber {
            sid: "beamMiddleLineSlant"
            music: "\\stemDown e'''8[ a'''8]"
            title: qsTrc("notation/engravingoptions", "Slant of beams extended to the middle of the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Notes far outside the staff have their stems lengthened to the middle of the staff, which " +
                        "would leave the beam lying level on one line; when the end notes differ it gets this slant " +
                        "instead.")
            max: 2
            step: 0.25
            decimals: 2
            unitScale: 0.25
        }

        EoNumber {
            sid: "beamMiddleLineSlantSmall"
            music: "\\stemDown c'''8[ d''']"
            title: qsTrc("notation/engravingoptions", "Smaller slant of beams extended to the middle of the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Used instead of the value above when the end notes are a second apart, when the rules allow " +
                        "only a quarter-space slant, or when the group has two beams.")
            max: 2
            step: 0.25
            decimals: 2
            unitScale: 0.25
        }

        EoNumber {
            sid: "beamEighthSpaceSlant"
            music: "\\stemDown c'''32[ a''']"
            title: qsTrc("notation/engravingoptions", "Slight slant of level three-beam groups")
            hint: qsTrc("notation/engravingoptions",
                        "A group with three beams which the staff lines force to lie level although its end notes " +
                        "differ has one end moved by this much, too little to cut wedges against the staff lines; 0 " +
                        "keeps it level. Not with the wide beam distance.")
            max: 0.5
            step: 0.0125
            decimals: 4
        }

        EoNumber {
            sid: "beamEighthSpaceSlantExtraDistance"
            music: "\\stemDown c'''32[ a''']"
            title: qsTrc("notation/engravingoptions", "Extra beam distance in such a group")
            hint: qsTrc("notation/engravingoptions",
                        "Added to the distance between the three beams of a group which gets the slight slant above, " +
                        "so that the slanted beams still meet the staff lines cleanly.")
            max: 0.25
            step: 0.0125
            decimals: 4
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Slant: custom rules")
        hint: qsTrc("notation/engravingoptions",
                    "Used while ‘Apply default beam slant rules’ is off. A slant is the difference in height between " +
                    "the two ends of the beam.")
        musicStyle: ({ useDefaultBeamSlantRules: false })

        EoToggle {
            sid: "beamCustomPositioningRules"
            music: "\\stemUp d'8[ f'8 a'8]"
            title: qsTrc("notation", "Apply positioning rules for staff lines, ledger lines and stem length")
            hint: qsTrc("notation/engravingoptions",
                        "On: a beam inside the staff is moved so that neither end floats in the middle of a space, a " +
                        "group entirely on ledger lines gets a slant of its own, and no stem is left shorter than " +
                        "normal (see the last four values of this section). Off: the beam keeps the slant of the " +
                        "table and is only moved away from the notes until its ends touch a staff line and its stems " +
                        "are long enough.")
        }

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Slant by interval (quarter spaces)")
            hint: qsTrc("notation/engravingoptions",
                        "The slant a beam takes for the interval between its first and last notes; beams of only two " +
                        "notes have a column of their own. A group whose inner notes call for a horizontal beam " +
                        "stays horizontal.")
            cornerTitle: qsTrc("notation/engravingoptions", "Interval between first and last note")
            rowTitles: [
                qsTrc("notation/engravingoptions", "2nd"),
                qsTrc("notation/engravingoptions", "3rd"),
                qsTrc("notation/engravingoptions", "4th"),
                qsTrc("notation/engravingoptions", "5th"),
                qsTrc("notation/engravingoptions", "6th"),
                qsTrc("notation/engravingoptions", "7th"),
                qsTrc("notation/engravingoptions", "8th"),
                qsTrc("notation/engravingoptions", "9th"),
                qsTrc("notation/engravingoptions", "10th"),
                qsTrc("notation/engravingoptions", "Greater than 10th")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Two notes"),
                qsTrc("notation/engravingoptions", "More than two notes")
            ]

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantSecondInterval"
                music: "\\stemUp c'8[ d'8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomMaxSlantSecondInterval"
                music: "\\stemUp c'8[ c'8 d'8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantThirdInterval"
                music: "\\stemUp c'8[ e'8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomMaxSlantThirdInterval"
                music: "\\stemUp c'8[ a,8 e'8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantFourthInterval"
                music: "\\stemUp e'8[ a'8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomMaxSlantFourthInterval"
                music: "\\stemUp c'8[ a,8 f'8]"
                musicStyle: ({ beamCustomPositioningRules: false })
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantFifthInterval"
                music: "\\stemUp c'8[ g'8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomMaxSlantFifthInterval"
                music: "\\stemUp c'8[ a,8 g'8]"
                musicStyle: ({ beamCustomPositioningRules: false })
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantSixthInterval"
                music: "\\stemUp c'8[ a'8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomMaxSlantSixthInterval"
                music: "\\stemUp c'8[ a,8 a'8]"
                musicStyle: ({ beamCustomPositioningRules: false })
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantSeventhInterval"
                music: "\\stemUp c'8[ b'8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomMaxSlantSeventhInterval"
                music: "\\stemUp c'8[ a,8 b'8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantOctaveInterval"
                music: "\\stemUp c'8[ c''8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomMaxSlantOctave"
                music: "\\stemUp c'8[ e'8 g'8 c''8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantNinthInterval"
                music: "\\stemUp c'8[ d''8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomMaxSlantNinthInterval"
                music: "\\stemUp c'8[ a'8 d''8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantTenthInterval"
                music: "\\stemUp c'8[ e''8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomMaxSlantTenthInterval"
                music: "\\stemUp c'8[ b'8 e''8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantGreaterThanTenthInterval"
                music: "\\stemUp c'8[ g''8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }

            EoNumber {
                sid: "beamCustomMaxSlantGreaterThanTenthInterval"
                music: "\\stemUp c'8[ e'8 g''8]"
                max: 4
                step: 0.25
                decimals: 2
                unitScale: 0.25
            }
        }

        EoNote {
            title: qsTrc("notation",
                         "These options determine the ideal slant of the beam in terms of the vertical distance " +
                         "between the tips of the stems of the first and last notes within the beam, based on the " +
                         "interval between the first and last notes within the beam, provided the contour of the " +
                         "other notes within the beam is such that the beam should be slanted.")
        }

        EoNumber {
            sid: "beamCustomRepeatedEndNoteSlant"
            music: "\\stemUp a'8[ a'8 f'8 d'8]"
            musicStyle: ({ useDefaultBeamSlantRules: false, beamCustomPositioningRules: false })
            title: qsTrc("notation/engravingoptions", "Slant when the end note is repeated (custom rules)")
            hint: qsTrc("notation/engravingoptions",
                        "When the end note nearest the beam is repeated by the note next to it, the beam takes this " +
                        "slant instead of the slant of the table.")
            max: 3
            step: 0.25
            decimals: 2
            unitScale: 0.25
        }

        EoNumber {
            sid: "beamCustomEndStemMaxShortening"
            music: "\\stemUp e'8[ g'8 b'8]"
            musicStyle: ({ useDefaultBeamSlantRules: false })
            title: qsTrc("notation/engravingoptions", "Largest stem shortening at beam ends")
            hint: qsTrc("notation/engravingoptions",
                        "The stems at the two ends of a beam may be this much shorter than their normal length, " +
                        "while the stems between them keep their full length; if the slant asks for more, it is " +
                        "reduced or the beam is moved away from the notes. Only with ‘Apply positioning rules for " +
                        "staff lines, ledger lines and stem length’ on.")
            max: 2
            step: 0.125
            decimals: 3
        }

        EoNumber {
            sid: "beamCustomLedgerGroupMaxShortening"
            music: "\\stemDown a'''8[ c''''8 e''''8 g''''8]"
            musicStyle: ({ useDefaultBeamSlantRules: false })
            title: qsTrc("notation/engravingoptions", "Largest stem shortening in ledger-line groups")
            hint: qsTrc("notation/engravingoptions",
                        "In a group whose notes all lie on ledger lines, a stem may be this much shorter than its " +
                        "normal length; beyond that the whole beam is moved towards the staff by whole staff spaces. " +
                        "Only with ‘Apply positioning rules for staff lines, ledger lines and stem length’ on.")
            max: 3
            step: 0.25
        }

        EoNumber {
            sid: "beamCustomLedgerGroupSlant"
            music: "\\stemDown a'''8[ c''''8]"
            musicStyle: ({ useDefaultBeamSlantRules: false })
            title: qsTrc("notation/engravingoptions", "Slant of ledger-line groups")
            hint: qsTrc("notation/engravingoptions",
                        "Slant of every beam whose notes all lie on ledger lines and are not level, whatever their " +
                        "interval; 0 keeps such beams horizontal. Only with ‘Apply positioning rules for staff " +
                        "lines, ledger lines and stem length’ on.")
            max: 2
            step: 0.25
            decimals: 2
            unitScale: 0.25
        }

        EoNumber {
            sid: "beamCustomLedgerGroupSnapShortening"
            music: "\\stemUp f,8[ a,8]"
            musicStyle: ({ useDefaultBeamSlantRules: false })
            title: qsTrc("notation/engravingoptions", "Stem shortening allowed to seat a ledger-line group")
            hint: qsTrc("notation/engravingoptions",
                        "In a ledger-line group the edge of the beam which faces the staff is set on a line; to get " +
                        "there the stem at the end nearer the notes may be shortened by at most this much, otherwise " +
                        "the beam moves the other way and the stem gets longer. Only with ‘Apply positioning rules " +
                        "for staff lines, ledger lines and stem length’ on.")
            max: 1
            step: 0.25
            decimals: 2
            unitScale: 0.25
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Stems of beamed notes")

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Shortest stem under a beam")
            hint: qsTrc("notation/engravingoptions",
                        "However the beam is slanted, it is moved away from the notes until every stem between its " +
                        "ends is at least this long (from the notehead to the stem tip, rounded to quarter spaces), " +
                        "and the default rules do not shorten the end stems below it. Not used where ‘Apply " +
                        "positioning rules for staff lines, ledger lines and stem length’ places the beam.")
            cornerTitle: qsTrc("notation/engravingoptions", "Number of beams")
            rowTitles: [
                qsTrc("notation/engravingoptions", "1 beam"),
                qsTrc("notation/engravingoptions", "2 beams"),
                qsTrc("notation/engravingoptions", "3 beams"),
                qsTrc("notation/engravingoptions", "4 beams"),
                qsTrc("notation/engravingoptions", "5 beams"),
                qsTrc("notation/engravingoptions", "6 beams"),
                qsTrc("notation/engravingoptions", "7 beams"),
                qsTrc("notation/engravingoptions", "8 beams")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Shortest stem")
            ]

            EoNumber {
                sid: "beamMinStemLengthOneBeam"
                music: "\\stemDown c''8[ a' c''' g']"
                min: 1
                max: 12
                step: 0.25
            }

            EoNumber {
                sid: "beamMinStemLengthTwoBeams"
                music: "\\stemUp c'16[ g' e']"
                min: 1
                max: 12
                step: 0.25
            }

            EoNumber {
                sid: "beamMinStemLengthThreeBeams"
                music: "\\stemUp c''32[ g'' e'']"
                min: 1
                max: 12
                step: 0.25
            }

            EoNumber {
                sid: "beamMinStemLengthFourBeams"
                music: "\\stemUp c''64[ g'' e'']"
                min: 1
                max: 12
                step: 0.25
            }

            EoNumber {
                sid: "beamMinStemLengthFiveBeams"
                music: "\\stemUp c''128[ g''128 e''128]"
                min: 1
                max: 12
                step: 0.25
            }

            EoNumber {
                sid: "beamMinStemLengthSixBeams"
                music: "\\stemUp c''256[ g''256 e''256]"
                min: 1
                max: 12
                step: 0.25
            }

            EoNumber {
                sid: "beamMinStemLengthSevenBeams"
                music: "\\stemUp c''512[ g''512 e''512]"
                min: 1
                max: 12
                step: 0.25
            }

            EoNumber {
                sid: "beamMinStemLengthEightBeams"
                music: "\\stemUp c''1024[ g''1024 e''1024]"
                min: 1
                max: 12
                step: 0.25
            }
        }

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Stems under a slanted beam")
            hint: qsTrc("notation/engravingoptions",
                        "A slanted beam may cut this much off the normal stem length of the notes between its ends; " +
                        "if it would cut more, the whole beam is moved away from the notes. Not used where ‘Apply " +
                        "positioning rules for staff lines, ledger lines and stem length’ places the beam.")
            cornerTitle: qsTrc("notation/engravingoptions", "Slant of the beam")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Small slant (up to ¾ sp)"),
                qsTrc("notation/engravingoptions", "Medium slant (1 to 1½ sp)"),
                qsTrc("notation/engravingoptions", "Large slant (more than 1½ sp)")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Stems between the ends may be shortened by")
            ]

            EoNumber {
                sid: "beamInnerStemShorteningSmallSlant"
                music: "\\stemUp c'8[ e'8 f'8 f'8]"
                max: 2
            }

            EoNumber {
                sid: "beamInnerStemShorteningMediumSlant"
                music: "\\stemUp c'8[ a'8 f'8 g'8]"
                max: 2
            }

            EoNumber {
                sid: "beamInnerStemShorteningLargeSlant"
                music: "\\stemUp c'16[ d'16 e'16 g'16 c''16 e''16]"
                max: 2
            }
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Rests under beams")

        EoNumber {
            sid: "beamRestPadding"
            music: "\\beamAt 1.6 1.6 \\stemUp e'8[ r8 e'8]"
            title: qsTrc("notation/engravingoptions", "Rest to beam clearance")
            hint: qsTrc("notation/engravingoptions",
                        "Smallest vertical gap between a beam and a rest inside its group; a rest which would come " +
                        "closer is moved away from the beam by whole staff spaces.")
            max: 2
        }

        EoNumber {
            sid: "beamRestPaddingShortRests"
            music: "\\beamAt 1.8 1.8 \\stemUp e'16[ r32 e'16]"
            title: qsTrc("notation/engravingoptions", "Rest to beam clearance, 32nd rests and shorter")
            hint: qsTrc("notation/engravingoptions",
                        "The same for 32nd rests and shorter, which are taller and may come closer to the beam.")
            max: 2
        }
    }
}
