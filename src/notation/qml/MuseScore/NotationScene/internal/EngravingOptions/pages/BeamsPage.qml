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
        title: qsTrc("notation/engravingoptions", "Beam slant: default rules")

        EoNumber {
            sid: "beamDefaultMaxSlantSecondInterval"
            title: qsTrc("notation/engravingoptions", "Maximum slant, 2nd")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: the largest slant of a beam whose first and last notes are a second apart. Value in quarter spaces (1 = ¼ sp).")
            max: 12
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamDefaultMaxSlantThirdInterval"
            title: qsTrc("notation/engravingoptions", "Maximum slant, 3rd")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: the largest slant of a beam whose first and last notes are a third apart. Value in quarter spaces (1 = ¼ sp).")
            max: 12
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamDefaultMaxSlantFourthInterval"
            title: qsTrc("notation/engravingoptions", "Maximum slant, 4th")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: the largest slant of a beam whose first and last notes are a fourth apart. Value in quarter spaces (1 = ¼ sp).")
            max: 12
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamDefaultMaxSlantFifthInterval"
            title: qsTrc("notation/engravingoptions", "Maximum slant, 5th")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: the largest slant of a beam whose first and last notes are a fifth apart. Value in quarter spaces (1 = ¼ sp).")
            max: 12
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamDefaultMaxSlantSixthInterval"
            title: qsTrc("notation/engravingoptions", "Maximum slant, 6th")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: the largest slant of a beam whose first and last notes are a sixth apart. Value in quarter spaces (1 = ¼ sp).")
            max: 12
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamDefaultMaxSlantSeventhInterval"
            title: qsTrc("notation/engravingoptions", "Maximum slant, 7th")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: the largest slant of a beam whose first and last notes are a seventh apart. Value in quarter spaces (1 = ¼ sp).")
            max: 12
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamDefaultMaxSlantOctave"
            title: qsTrc("notation/engravingoptions", "Maximum slant, octave or more")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: the largest slant of a beam whose first and last notes are an octave or more apart. Value in quarter spaces (1 = ¼ sp).")
            max: 12
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamDefaultSlantSecondMaxWidth"
            title: qsTrc("notation/engravingoptions", "Beam width limit for the slant of a 2nd")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: a beam narrower than this, measured between its first and last stems, slants no more than the maximum slant of a second, whatever its interval.")
            max: 40
            step: 0.5
        }

        EoNumber {
            sid: "beamDefaultSlantThirdMaxWidth"
            title: qsTrc("notation/engravingoptions", "Beam width limit for the slant of a 3rd")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: a beam narrower than this, measured between its first and last stems, slants no more than the maximum slant of a third, whatever its interval.")
            max: 40
            step: 0.5
        }

        EoNumber {
            sid: "beamDefaultSlantFourthMaxWidth"
            title: qsTrc("notation/engravingoptions", "Beam width limit for the slant of a 4th")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: a beam narrower than this, measured between its first and last stems, slants no more than the maximum slant of a fourth, whatever its interval.")
            max: 40
            step: 0.5
        }

        EoNumber {
            sid: "beamDefaultSlantFifthMaxWidth"
            title: qsTrc("notation/engravingoptions", "Beam width limit for the slant of a 5th")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: a beam narrower than this, measured between its first and last stems, slants no more than the maximum slant of a fifth, whatever its interval.")
            max: 40
            step: 0.5
        }

        EoNumber {
            sid: "beamDefaultSlantSixthMaxWidth"
            title: qsTrc("notation/engravingoptions", "Beam width limit for the slant of a 6th")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: a beam narrower than this, measured between its first and last stems, slants no more than the maximum slant of a sixth, whatever its interval.")
            max: 40
            step: 0.5
        }

        EoNumber {
            sid: "beamDefaultSlantSeventhMaxWidth"
            title: qsTrc("notation/engravingoptions", "Beam width limit for the slant of a 7th")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: a beam narrower than this, measured between its first and last stems, slants no more than the maximum slant of a seventh, whatever its interval.")
            max: 40
            step: 0.5
        }

        EoNumber {
            sid: "beamDefaultRepeatedEndNoteSlant"
            title: qsTrc("notation/engravingoptions", "Slant when the end note is repeated")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: slant of a beam whose end note nearest the beam is repeated by the note next to it (the beam would otherwise be flat). Value in quarter spaces (1 = ¼ sp).")
            max: 12
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamMiddleLineSlant"
            title: qsTrc("notation/engravingoptions", "Slant of beams extended to the middle of the staff")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: when the stems of notes far outside the staff are lengthened to the middle of the staff the beam would lie flat on one line; it is given this slant instead if the end notes differ. Value in quarter spaces (1 = ¼ sp).")
            max: 8
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamMiddleLineSlantSmall"
            title: qsTrc("notation/engravingoptions", "Small slant of beams extended to the middle of the staff")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: the same, used when the end notes are a second apart, when the contour only allows a small slant, or when the group has two beams. Value in quarter spaces (1 = ¼ sp).")
            max: 8
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamEighthSpaceSlant"
            title: qsTrc("notation/engravingoptions", "Eighth-space slant of three-beam groups")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: a group with three beams that the staff-line rules keep level although its end notes differ is tilted by this amount at one end (0 = keep it level).")
            max: 0.5
            step: 0.0125
            decimals: 4
        }

        EoNumber {
            sid: "beamEighthSpaceSlantExtraDistance"
            title: qsTrc("notation/engravingoptions", "Extra beam distance with the eighth-space slant")
            hint: qsTrc("notation/engravingoptions",
                         "Default slant rules: amount added to the distance between the beams of a three-beam group that receives the eighth-space slant.")
            max: 0.25
            step: 0.0125
            decimals: 4
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Beam slant: contour rules")

        EoToggle {
            sid: "beamFlattenConcave"
            title: qsTrc("notation/engravingoptions", "Flatten concave beam groups")
            hint: qsTrc("notation/engravingoptions",
                         "Keeps the beam horizontal when a note inside the group lies beyond both end notes on the beam side; when off, such a beam slants according to its end notes.")
        }

        EoToggle {
            sid: "beamTwoPitchSlantRule"
            title: qsTrc("notation/engravingoptions", "Fixed slants for two-pitch groups")
            hint: qsTrc("notation/engravingoptions",
                         "Groups of three or more notes on only two pitches: a single different note at one end slants the beam if it is the note nearer the beam and leaves it flat otherwise; evenly alternating pitches slant the beam. When off, the general contour rules apply.")
        }

        EoNumber {
            sid: "beamCrossStaffLevelNotesSlant"
            title: qsTrc("notation/engravingoptions", "Cross-staff beam slant over level notes")
            hint: qsTrc("notation/engravingoptions",
                         "Cross-staff beam lying above or below both staves: when the notes on the staff nearest the beam are level, the beam slants this much towards the notes of the other staff. Value in quarter spaces (1 = ¼ sp).")
            max: 8
            step: 1
            decimals: 0
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Beam positioning: custom rules")

        EoNumber {
            sid: "beamCustomRepeatedEndNoteSlant"
            title: qsTrc("notation/engravingoptions", "Slant when the end note is repeated (custom rules)")
            hint: qsTrc("notation/engravingoptions",
                         "Custom slant rules: slant of a beam whose end note nearest the beam is repeated by the note next to it. Value in quarter spaces (1 = ¼ sp).")
            max: 12
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamCustomEndStemMaxShortening"
            title: qsTrc("notation/engravingoptions", "Largest stem shortening at beam ends")
            hint: qsTrc("notation/engravingoptions",
                         "Custom positioning rules: how far the stems at the two ends of a beam (and of inner chords level with the end note nearest the beam) may fall short of their default length before the slant is reduced or the beam is moved away from the notes.")
            max: 2
            step: 0.125
            decimals: 3
        }

        EoNumber {
            sid: "beamCustomLedgerGroupMaxShortening"
            title: qsTrc("notation/engravingoptions", "Largest stem shortening in ledger-line groups")
            hint: qsTrc("notation/engravingoptions",
                         "Custom positioning rules, groups whose notes are all on ledger lines: how far a stem may fall short of its default length before the whole beam is moved towards the staff, a full space at a time.")
            max: 3
            step: 0.25
        }

        EoNumber {
            sid: "beamCustomLedgerGroupSlant"
            title: qsTrc("notation/engravingoptions", "Slant of ledger-line groups")
            hint: qsTrc("notation/engravingoptions",
                         "Custom positioning rules: slant of every beam whose notes are all on ledger lines and not level (0 = always flat). Value in quarter spaces (1 = ¼ sp).")
            max: 8
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "beamCustomLedgerGroupSnapShortening"
            title: qsTrc("notation/engravingoptions", "Stem shortening allowed when snapping ledger-line groups")
            hint: qsTrc("notation/engravingoptions",
                         "Custom positioning rules, ledger-line groups: the most a stem may be shortened to rest the beam on the staff-line grid; if more would be needed the stem is lengthened to the next line instead. Value in quarter spaces (1 = ¼ sp).")
            max: 4
            step: 1
            decimals: 0
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Beams: minimum stem length")

        EoNumber {
            sid: "beamMinStemLengthOneBeam"
            title: qsTrc("notation/engravingoptions", "Minimum stem length under a beam, one beam")
            hint: qsTrc("notation/engravingoptions",
                         "Shortest stem a beam may leave to a chord with one beam: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
            min: 1
            max: 12
            step: 0.25
        }

        EoNumber {
            sid: "beamMinStemLengthTwoBeams"
            title: qsTrc("notation/engravingoptions", "Minimum stem length under a beam, two beams")
            hint: qsTrc("notation/engravingoptions",
                         "Shortest stem a beam may leave to a chord with two beams: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
            min: 1
            max: 12
            step: 0.25
        }

        EoNumber {
            sid: "beamMinStemLengthThreeBeams"
            title: qsTrc("notation/engravingoptions", "Minimum stem length under a beam, three beams")
            hint: qsTrc("notation/engravingoptions",
                         "Shortest stem a beam may leave to a chord with three beams: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
            min: 1
            max: 12
            step: 0.25
        }

        EoNumber {
            sid: "beamMinStemLengthFourBeams"
            title: qsTrc("notation/engravingoptions", "Minimum stem length under a beam, four beams")
            hint: qsTrc("notation/engravingoptions",
                         "Shortest stem a beam may leave to a chord with four beams: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
            min: 1
            max: 12
            step: 0.25
        }

        EoNumber {
            sid: "beamMinStemLengthFiveBeams"
            title: qsTrc("notation/engravingoptions", "Minimum stem length under a beam, five beams")
            hint: qsTrc("notation/engravingoptions",
                         "Shortest stem a beam may leave to a chord with five beams: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
            min: 1
            max: 12
            step: 0.25
        }

        EoNumber {
            sid: "beamMinStemLengthSixBeams"
            title: qsTrc("notation/engravingoptions", "Minimum stem length under a beam, six beams")
            hint: qsTrc("notation/engravingoptions",
                         "Shortest stem a beam may leave to a chord with six beams: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
            min: 1
            max: 12
            step: 0.25
        }

        EoNumber {
            sid: "beamMinStemLengthSevenBeams"
            title: qsTrc("notation/engravingoptions", "Minimum stem length under a beam, seven beams")
            hint: qsTrc("notation/engravingoptions",
                         "Shortest stem a beam may leave to a chord with seven beams: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
            min: 1
            max: 12
            step: 0.25
        }

        EoNumber {
            sid: "beamMinStemLengthEightBeams"
            title: qsTrc("notation/engravingoptions", "Minimum stem length under a beam, eight beams")
            hint: qsTrc("notation/engravingoptions",
                         "Shortest stem a beam may leave to a chord with eight beams: the beam is moved away from the notes until the chords between its ends reach this length, and the end stems are not shortened below it when the beam is seated on a staff line. Rounded to quarter spaces.")
            min: 1
            max: 12
            step: 0.25
        }

        EoNumber {
            sid: "beamInnerStemShorteningSmallSlant"
            title: qsTrc("notation/engravingoptions", "Inner stem shortening, beam slant up to ¾ sp")
            hint: qsTrc("notation/engravingoptions",
                         "How far a slanted beam (slant up to ¾ sp) may cut into the default stem length of the chords between its ends before it is moved away from the notes.")
            max: 2
        }

        EoNumber {
            sid: "beamInnerStemShorteningMediumSlant"
            title: qsTrc("notation/engravingoptions", "Inner stem shortening, beam slant from 1 to 1½ sp")
            hint: qsTrc("notation/engravingoptions",
                         "How far a slanted beam (slant from 1 to 1½ sp) may cut into the default stem length of the chords between its ends before it is moved away from the notes.")
            max: 2
        }

        EoNumber {
            sid: "beamInnerStemShorteningLargeSlant"
            title: qsTrc("notation/engravingoptions", "Inner stem shortening, beam slant more than 1½ sp")
            hint: qsTrc("notation/engravingoptions",
                         "How far a slanted beam (slant more than 1½ sp) may cut into the default stem length of the chords between its ends before it is moved away from the notes.")
            max: 2
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Rests under beams")

        EoNumber {
            sid: "beamRestPadding"
            title: qsTrc("notation/engravingoptions", "Rest to beam clearance")
            hint: qsTrc("notation/engravingoptions",
                         "Smallest vertical gap between a beam and a rest inside the beamed group; a rest that comes closer is moved away from the beam by whole staff spaces.")
            max: 2
        }

        EoNumber {
            sid: "beamRestPaddingShortRests"
            title: qsTrc("notation/engravingoptions", "Rest to beam clearance, 32nd rests and shorter")
            hint: qsTrc("notation/engravingoptions", "The same clearance for 32nd rests and shorter ones.")
            max: 2
        }
    }
}
