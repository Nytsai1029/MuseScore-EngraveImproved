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
            presentation: "tiles"
            choices: [
                { value: false, title: qsTrc("notation", "Regular") },
                { value: true, title: qsTrc("notation", "Wide") }
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
            music: "c'8[ e'16]"
            title: qsTrc("notation/engravingoptions", "Shortest beam or beamlet")
        }

        EoNumber {
            sid: "beamletPadding"
            music: "c'16[ e'8.]"
            title: qsTrc("notation/engravingoptions", "Gap before a beamlet")
        }


        EoChoice {
            sid: "frenchStyleBeams"
            music: "c'16[ e'16 g'16 e'16]"
            title: qsTrc("notation", "Beam style")
            presentation: "tiles"
            choices: [
                { value: false, title: qsTrc("notation", "Draw inner stems through beams") },
                { value: true, title: qsTrc("notation", "Draw inner stems to nearest beam (“French” style)") }
            ]
            tileWidth: 200
            tileHeight: 96
        }

        EoToggle {
            sid: "snapCustomBeamsToGrid"
            music: "\\beamAt 4.4 4.4 c'8[ d'8 e'8 c''8]"
            title: qsTrc("notation/engravingoptions", "Snap custom beam positions to the grid")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Beam slant")
        music: "c'8[ g' e' c''8]"

        EoToggle {
            sid: "beamNoSlope"
            title: qsTrc("notation", "Flatten all beams")
        }

        EoToggle {
            sid: "useDefaultBeamSlantRules"
            title: qsTrc("notation/engravingoptions", "Slant by the default rules")
        }

        EoToggle {
            sid: "beamFlattenConcave"
            music: "\\stemUp d'8[ a'8 f'8]"
            title: qsTrc("notation/engravingoptions", "Flatten concave beam groups")
            hint: qsTrc("notation/engravingoptions",
                         "Keeps the beam horizontal when a note inside the group lies beyond both end notes on the beam side; when off, such a beam slants according to its end notes.")
        }

        EoToggle {
            sid: "beamTwoPitchSlantRule"
            music: "\\stemUp g'8[ e'8 e'8 e'8] e'8[ g'8 g'8 g'8] e'8[ g'8 e'8 g'8]"
            title: qsTrc("notation/engravingoptions", "Fixed slants for two-pitch groups")
            hint: qsTrc("notation/engravingoptions",
                         "Groups of three or more notes on only two pitches: a single different note at one end slants the beam if it is the note nearer the beam and leaves it flat otherwise; evenly alternating pitches slant the beam. When off, the general contour rules apply.")
        }

        EoNumber {
            sid: "beamCrossStaffLevelNotesSlant"
            music: "\\\staves 2 \\\beamMove -1 c''8[ \\\crossStaff down e'8 f'8 a''8]"
            title: qsTrc("notation/engravingoptions", "Cross-staff beam slant over level notes")
            hint: qsTrc("notation/engravingoptions",
                         "Cross-staff beam lying above or below both staves: when the notes on the staff nearest the beam are level, the beam slants this much towards the notes of the other staff. Value in quarter spaces (1 = ¼ sp).")
            max: 8
            step: 1
            decimals: 0
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Slant: default rules")
        music: "c''8[ d''] c''8[ e'']"

        EoGrid {
            cornerTitle: qsTrc("notation/engravingoptions", "Slant: default rules")
            rowTitles: [
                qsTrc("notation/engravingoptions", "2nd"), qsTrc("notation/engravingoptions", "3rd"),
                qsTrc("notation/engravingoptions", "4th"), qsTrc("notation/engravingoptions", "5th"),
                qsTrc("notation/engravingoptions", "6th"), qsTrc("notation/engravingoptions", "7th"),
                qsTrc("notation/engravingoptions", "Octave and more")
            ]
            columnTitles: [qsTrc("notation/engravingoptions", "Maximum slant"),
                           qsTrc("notation/engravingoptions", "Applies up to a beam width of")]
            compact: true

            EoNumber {
                sid: "beamDefaultMaxSlantSecondInterval"
                hint: qsTrc("notation/engravingoptions",
                                     "Default slant rules: the largest slant of a beam whose first and last notes are a second apart. Value in quarter spaces (1 = ¼ sp).")
                max: 12
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamDefaultSlantSecondMaxWidth"
                music: "\\stemDown c'16[ d' e']"
                hint: qsTrc("notation/engravingoptions",
                                     "Default slant rules: a beam narrower than this, measured between its first and last stems, slants no more than the maximum slant of a second, whatever its interval.")
                max: 40
                step: 0.5
            }

            EoNumber {
                sid: "beamDefaultMaxSlantThirdInterval"
                music: "\\stemDown c'8[ d' e']"
                hint: qsTrc("notation/engravingoptions",
                                     "Default slant rules: the largest slant of a beam whose first and last notes are a third apart. Value in quarter spaces (1 = ¼ sp).")
                max: 12
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamDefaultSlantThirdMaxWidth"
                music: "b'8[ d'' e'' f'']"
                musicStyle: ({ measureSpacing: 2.05 })
                hint: qsTrc("notation/engravingoptions",
                                     "Default slant rules: a beam narrower than this, measured between its first and last stems, slants no more than the maximum slant of a third, whatever its interval.")
                max: 40
                step: 0.5
            }

            EoNumber {
                sid: "beamDefaultMaxSlantFourthInterval"
                music: "\\stemUp f'8[ g'8 a'8 b'8]"
                hint: qsTrc("notation/engravingoptions",
                                     "Default slant rules: the largest slant of a beam whose first and last notes are a fourth apart. Value in quarter spaces (1 = ¼ sp).")
                max: 12
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamDefaultSlantFourthMaxWidth"
                music: "d''8[ f'' a'' b'']"
                musicStyle: ({ measureSpacing: 1.35 })
                hint: qsTrc("notation/engravingoptions",
                                     "Default slant rules: a beam narrower than this, measured between its first and last stems, slants no more than the maximum slant of a fourth, whatever its interval.")
                max: 40
                step: 0.5
            }

            EoNumber {
                sid: "beamDefaultMaxSlantFifthInterval"
                music: "\\stemUp f'8[ g'8 a'8 b'8 c''8]"
                hint: qsTrc("notation/engravingoptions",
                                     "Default slant rules: the largest slant of a beam whose first and last notes are a fifth apart. Value in quarter spaces (1 = ¼ sp).")
                max: 12
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamDefaultSlantFifthMaxWidth"
                music: "c'8[ e' f' g' a']"
                musicStyle: ({ measureSpacing: 1.36 })
                hint: qsTrc("notation/engravingoptions",
                                     "Default slant rules: a beam narrower than this, measured between its first and last stems, slants no more than the maximum slant of a fifth, whatever its interval.")
                max: 40
                step: 0.5
            }

            EoNumber {
                sid: "beamDefaultMaxSlantSixthInterval"
                music: "\\stemUp c'8[ d'8 e'8 f'8 g'8 a'8]"
                hint: qsTrc("notation/engravingoptions",
                                     "Default slant rules: the largest slant of a beam whose first and last notes are a sixth apart. Value in quarter spaces (1 = ¼ sp).")
                max: 12
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamDefaultSlantSixthMaxWidth"
                music: "c''8[ d'' e'' f'' g'' a'' b'']"
                musicStyle: ({ measureSpacing: 1.37 })
                hint: qsTrc("notation/engravingoptions",
                                     "Default slant rules: a beam narrower than this, measured between its first and last stems, slants no more than the maximum slant of a sixth, whatever its interval.")
                max: 40
                step: 0.5
            }

            EoNumber {
                sid: "beamDefaultMaxSlantSeventhInterval"
                music: "\\stemDown c'8[ d' e' f' g' a' b' c'']"
                hint: qsTrc("notation/engravingoptions",
                                     "Default slant rules: the largest slant of a beam whose first and last notes are a seventh apart. Value in quarter spaces (1 = ¼ sp).")
                max: 12
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamDefaultSlantSeventhMaxWidth"
                music: "b'8[ c'' d'' e'' f'' g'' a'' b'']"
                musicStyle: ({ measureSpacing: 1.21 })
                hint: qsTrc("notation/engravingoptions",
                                     "Default slant rules: a beam narrower than this, measured between its first and last stems, slants no more than the maximum slant of a seventh, whatever its interval.")
                max: 40
                step: 0.5
            }

            EoNumber {
                sid: "beamDefaultMaxSlantOctave"
                music: "d''8[ e'' f'' g'' a'' b'' c''' d''']"
                musicStyle: ({ measureSpacing: 1.15 })
                title: qsTrc("notation/engravingoptions", "Maximum slant, octave or more")
                hint: qsTrc("notation/engravingoptions",
                                     "Default slant rules: the largest slant of a beam whose first and last notes are an octave or more apart. Value in quarter spaces (1 = ¼ sp).")
                max: 12
                step: 1
                decimals: 0
            }

            EoGap {
            }
        }

        EoNumber {
            sid: "beamDefaultRepeatedEndNoteSlant"
            music: "\\stemUp a'8[ a'8 f'8 d'8]"
            title: qsTrc("notation/engravingoptions", "Slant when the end note is repeated")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: slant of a beam whose end note nearest the beam is repeated by the note next to it (the beam would otherwise be flat). Value in quarter spaces (1 = ¼ sp).")
            max: 12
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamMiddleLineSlant"
            music: "\\stemDown e'''8[ a'''8]"
            title: qsTrc("notation/engravingoptions", "Slant of beams extended to the middle of the staff")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: when the stems of notes far outside the staff are lengthened to the middle of the staff the beam would lie flat on one line; it is given this slant instead if the end notes differ. Value in quarter spaces (1 = ¼ sp).")
            max: 8
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamMiddleLineSlantSmall"
            music: "\\stemDown c'''8[ d''']"
            title: qsTrc("notation/engravingoptions", "Small slant of beams extended to the middle of the staff")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: the same, used when the end notes are a second apart, when the contour only allows a small slant, or when the group has two beams. Value in quarter spaces (1 = ¼ sp).")
            max: 8
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamEighthSpaceSlant"
            music: "\\stemDown c'''32[ a''']"
            title: qsTrc("notation/engravingoptions", "Eighth-space slant of three-beam groups")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: a group with three beams that the staff-line rules keep level although its end notes differ is tilted by this amount at one end (0 = keep it level).")
            max: 0.5
            step: 0.0125
            decimals: 4
        }

        EoNumber {
            sid: "beamEighthSpaceSlantExtraDistance"
            music: "\\stemDown c'''32[ a''']"
            title: qsTrc("notation/engravingoptions", "Extra beam distance with the eighth-space slant")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: amount added to the distance between the beams of a three-beam group that receives the eighth-space slant.")
            max: 0.25
            step: 0.0125
            decimals: 4
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Slant: custom rules")
        music: "\\stemUp c'8[ e'8 g'8 c''8]"
        musicStyle: ({ useDefaultBeamSlantRules: false })

        EoToggle {
            sid: "beamCustomPositioningRules"
            music: "\\stemUp d'8[ f'8 a'8]"
            title: qsTrc("notation/engravingoptions", "Stems positioned by the custom rules")
        }

        EoGrid {
            cornerTitle: qsTrc("notation/engravingoptions", "Slant: custom rules")
            rowTitles: [
                qsTrc("notation/engravingoptions", "2nd"), qsTrc("notation/engravingoptions", "3rd"),
                qsTrc("notation/engravingoptions", "4th"), qsTrc("notation/engravingoptions", "5th"),
                qsTrc("notation/engravingoptions", "6th"), qsTrc("notation/engravingoptions", "7th"),
                qsTrc("notation/engravingoptions", "8th"), qsTrc("notation/engravingoptions", "9th"),
                qsTrc("notation/engravingoptions", "10th"),
                qsTrc("notation/engravingoptions", "Greater than 10th")
            ]
            columnTitles: [qsTrc("notation/engravingoptions", "Two notes"),
                           qsTrc("notation/engravingoptions", "More than two notes")]
            compact: true

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantSecondInterval"
                music: "\\stemUp c'8[ d'8]"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomMaxSlantSecondInterval"
                music: "\\stemUp c'8[ c'8 d'8]"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantThirdInterval"
                music: "\\stemUp c'8[ e'8]"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomMaxSlantThirdInterval"
                music: "\\stemUp c'8[ a,8 e'8]"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantFourthInterval"
                music: "\\stemUp e'8[ a'8]"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomMaxSlantFourthInterval"
                music: "\\stemUp c'8[ a,8 f'8]"
                musicStyle: ({ beamCustomPositioningRules: false })
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantFifthInterval"
                music: "\\stemUp c'8[ g'8]"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomMaxSlantFifthInterval"
                music: "\\stemUp c'8[ a,8 g'8]"
                musicStyle: ({ beamCustomPositioningRules: false })
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantSixthInterval"
                music: "\\stemUp c'8[ a'8]"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomMaxSlantSixthInterval"
                music: "\\stemUp c'8[ a,8 a'8]"
                musicStyle: ({ beamCustomPositioningRules: false })
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantSeventhInterval"
                music: "\\stemUp c'8[ b'8]"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomMaxSlantSeventhInterval"
                music: "\\stemUp c'8[ a,8 b'8]"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantOctaveInterval"
                music: "\\stemUp c'8[ c''8]"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomMaxSlantOctave"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantNinthInterval"
                music: "\\stemUp c'8[ d''8]"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomMaxSlantNinthInterval"
                music: "\\stemUp c'8[ a'8 d''8]"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantTenthInterval"
                music: "\\stemUp c'8[ e''8]"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomMaxSlantTenthInterval"
                music: "\\stemUp c'8[ b'8 e''8]"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomTwoNoteMaxSlantGreaterThanTenthInterval"
                music: "\\stemUp c'8[ g''8]"
                max: 16
                step: 1
                decimals: 0
            }

            EoNumber {
                sid: "beamCustomMaxSlantGreaterThanTenthInterval"
                music: "\\stemUp c'8[ e'8 g''8]"
                max: 16
                step: 1
                decimals: 0
            }
        }

        EoNumber {
            sid: "beamCustomRepeatedEndNoteSlant"
            music: "\\stemUp a'8[ a'8 f'8 d'8]"
            musicStyle: ({ useDefaultBeamSlantRules: false, beamCustomPositioningRules: false })
            title: qsTrc("notation/engravingoptions", "Slant when the end note is repeated (custom rules)")
            hint: qsTrc("notation/engravingoptions",
                         "Custom slant rules: slant of a beam whose end note nearest the beam is repeated by the note next to it. Value in quarter spaces (1 = ¼ sp).")
            max: 12
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamCustomEndStemMaxShortening"
            music: "\\stemUp e'8[ g'8 b'8]"
            musicStyle: ({ useDefaultBeamSlantRules: false })
            title: qsTrc("notation/engravingoptions", "Largest stem shortening at beam ends")
            hint: qsTrc("notation/engravingoptions",
                         "Custom positioning rules: how far the stems at the two ends of a beam (and of inner chords level with the end note nearest the beam) may fall short of their default length before the slant is reduced or the beam is moved away from the notes.")
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
                         "Custom positioning rules, groups whose notes are all on ledger lines: how far a stem may fall short of its default length before the whole beam is moved towards the staff, a full space at a time.")
            max: 3
            step: 0.25
        }

        EoNumber {
            sid: "beamCustomLedgerGroupSlant"
            music: "\\stemDown a'''8[ c''''8]"
            musicStyle: ({ useDefaultBeamSlantRules: false })
            title: qsTrc("notation/engravingoptions", "Slant of ledger-line groups")
            hint: qsTrc("notation/engravingoptions",
                         "Custom positioning rules: slant of every beam whose notes are all on ledger lines and not level (0 = always flat). Value in quarter spaces (1 = ¼ sp).")
            max: 8
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamCustomLedgerGroupSnapShortening"
            music: "\\stemUp f,8[ a,8]"
            musicStyle: ({ useDefaultBeamSlantRules: false })
            title: qsTrc("notation/engravingoptions", "Stem shortening allowed when snapping ledger-line groups")
            hint: qsTrc("notation/engravingoptions",
                         "Custom positioning rules, ledger-line groups: the most a stem may be shortened to rest the beam on the staff-line grid; if more would be needed the stem is lengthened to the next line instead. Value in quarter spaces (1 = ¼ sp).")
            max: 4
            step: 1
            decimals: 0
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Stems of beamed notes")
        music: "\\stemUp c'8[ g' e']"

        EoGrid {
            cornerTitle: qsTrc("notation/engravingoptions", "Minimum stem length")
            rowTitles: [
                qsTrc("notation/engravingoptions", "1 beam"), qsTrc("notation/engravingoptions", "2 beams"),
                qsTrc("notation/engravingoptions", "3 beams"), qsTrc("notation/engravingoptions", "4 beams"),
                qsTrc("notation/engravingoptions", "5 beams"), qsTrc("notation/engravingoptions", "6 beams"),
                qsTrc("notation/engravingoptions", "7 beams"), qsTrc("notation/engravingoptions", "8 beams")
            ]
            columnTitles: [qsTrc("notation/engravingoptions", "Shortest stem")]
            compact: true

            EoNumber {
                sid: "beamMinStemLengthOneBeam"
                music: "\\stemDown c''8[ a' c''' g']"
                hint: qsTrc("notation/engravingoptions",
                                     "Shortest stem a beam may leave to a chord with one beam: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
                min: 1
                max: 12
                step: 0.25
            }


            EoNumber {
                sid: "beamMinStemLengthTwoBeams"
                music: "\\stemUp c'16[ g' e']"
                hint: qsTrc("notation/engravingoptions",
                                     "Shortest stem a beam may leave to a chord with two beams: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
                min: 1
                max: 12
                step: 0.25
            }


            EoNumber {
                sid: "beamMinStemLengthThreeBeams"
                music: "\\stemUp c''32[ g'' e'']"
                hint: qsTrc("notation/engravingoptions",
                                     "Shortest stem a beam may leave to a chord with three beams: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
                min: 1
                max: 12
                step: 0.25
            }


            EoNumber {
                sid: "beamMinStemLengthFourBeams"
                music: "\\stemUp c''64[ g'' e'']"
                hint: qsTrc("notation/engravingoptions",
                                     "Shortest stem a beam may leave to a chord with four beams: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
                min: 1
                max: 12
                step: 0.25
            }


            EoNumber {
                sid: "beamMinStemLengthFiveBeams"
                music: "\\stemUp c''128[ g''128 e''128]"
                hint: qsTrc("notation/engravingoptions",
                                     "Shortest stem a beam may leave to a chord with five beams: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
                min: 1
                max: 12
                step: 0.25
            }


            EoNumber {
                sid: "beamMinStemLengthSixBeams"
                music: "\\stemUp c''256[ g''256 e''256]"
                hint: qsTrc("notation/engravingoptions",
                                     "Shortest stem a beam may leave to a chord with six beams: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
                min: 1
                max: 12
                step: 0.25
            }


            EoNumber {
                sid: "beamMinStemLengthSevenBeams"
                music: "\\stemUp c''512[ g''512 e''512]"
                hint: qsTrc("notation/engravingoptions",
                                     "Shortest stem a beam may leave to a chord with seven beams: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
                min: 1
                max: 12
                step: 0.25
            }


            EoNumber {
                sid: "beamMinStemLengthEightBeams"
                music: "\\stemUp c''1024[ g''1024 e''1024]"
                hint: qsTrc("notation/engravingoptions",
                                     "Shortest stem a beam may leave to a chord with eight beams: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
                min: 1
                max: 12
                step: 0.25
            }
        }

        EoGrid {
            cornerTitle: qsTrc("notation/engravingoptions", "Inner stems")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Small slant"),
                qsTrc("notation/engravingoptions", "Medium slant"),
                qsTrc("notation/engravingoptions", "Large slant")
            ]
            columnTitles: [qsTrc("notation/engravingoptions", "Shortening of the inner stems")]
            compact: true


            EoNumber {
                sid: "beamInnerStemShorteningSmallSlant"
                music: "\\stemUp c'8[ e'8 f'8 f'8]"
                hint: qsTrc("notation/engravingoptions",
                                     "How far a slanted beam (slant up to ¾ sp) may cut into the default stem length of the chords between its ends before it is moved away from the notes.")
                max: 2
            }


            EoNumber {
                sid: "beamInnerStemShorteningMediumSlant"
                music: "\\stemUp c'8[ a'8 f'8 g'8]"
                hint: qsTrc("notation/engravingoptions",
                                     "How far a slanted beam (slant from 1 to 1½ sp) may cut into the default stem length of the chords between its ends before it is moved away from the notes.")
                max: 2
            }


            EoNumber {
                sid: "beamInnerStemShorteningLargeSlant"
                music: "\\stemUp c'16[ d'16 e'16 g'16 c''16 e''16]"
                hint: qsTrc("notation/engravingoptions",
                                     "How far a slanted beam (slant more than 1½ sp) may cut into the default stem length of the chords between its ends before it is moved away from the notes.")
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
                         "Smallest vertical gap between a beam and a rest inside the beamed group; a rest that comes closer is moved away from the beam by whole staff spaces.")
            max: 2
        }

        EoNumber {
            sid: "beamRestPaddingShortRests"
            music: "\\beamAt 1.8 1.8 \\stemUp e'16[ r32 e'16]"
            title: qsTrc("notation/engravingoptions", "Rest to beam clearance, 32nd rests and shorter")
            hint: qsTrc("notation/engravingoptions", "The same clearance for 32nd rests and shorter ones.")
            max: 2
        }
    }
}
