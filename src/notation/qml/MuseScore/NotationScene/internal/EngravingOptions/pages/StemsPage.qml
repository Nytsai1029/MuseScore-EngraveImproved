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
        title: qsTrc("notation/engravingoptions", "Stem length")

        EoNumber {
            sid: "shortestStemWideBeams"
            title: qsTrc("notation/engravingoptions", "Shortest stem with wide beam distance")
            hint: qsTrc("notation/engravingoptions",
                         "Lower limit for stems shortened outside the staff when the wide beam distance is on (replaces ‘Shortest stem’ in that case).")
            min: 1
            max: 6
            step: 0.25
        }

        EoNumber {
            sid: "stemLengthThreeBeamsAddition"
            title: qsTrc("notation/engravingoptions", "Extra stem length for three beams")
            hint: qsTrc("notation/engravingoptions",
                         "Length added to the standard stem length of beamed notes with three beams (each further beam adds one beam distance). Rounded to quarter spaces.")
            max: 2
            step: 0.25
        }

        EoNumber {
            sid: "stemOpticalAdjustment"
            title: qsTrc("notation/engravingoptions", "Stem shortening to seat the beam on a staff line")
            hint: qsTrc("notation/engravingoptions",
                         "Stems of notes with one or two beams that would end in the middle of a staff space are shortened by this amount, so that the beam touches a staff line (0 = off). Rounded to quarter spaces.")
            max: 0.5
            step: 0.25
        }

        EoNumber {
            sid: "stemMinInnerLengthNoBeam"
            title: qsTrc("notation/engravingoptions", "Minimum stem between notehead and beams, no beam")
            hint: qsTrc("notation/engravingoptions",
                         "Shortest free stem between the notehead and the innermost beam for a note in a beamed group or two-note tremolo with no beam of its own; the height of the beams is added to it. Rounded to quarter spaces.")
            min: 0.5
            max: 6
            step: 0.25
        }

        EoNumber {
            sid: "stemMinInnerLengthOneBeam"
            title: qsTrc("notation/engravingoptions", "Minimum stem between notehead and beams, one beam")
            hint: qsTrc("notation/engravingoptions",
                         "Shortest free stem between the notehead and the innermost beam for a note in a beamed group or two-note tremolo with one beam; the height of the beams is added to it. Rounded to quarter spaces.")
            min: 0.5
            max: 6
            step: 0.25
        }

        EoNumber {
            sid: "stemMinInnerLengthTwoBeams"
            title: qsTrc("notation/engravingoptions", "Minimum stem between notehead and beams, two beams")
            hint: qsTrc("notation/engravingoptions",
                         "Shortest free stem between the notehead and the innermost beam for a note in a beamed group or two-note tremolo with two beams; the height of the beams is added to it. Rounded to quarter spaces.")
            min: 0.5
            max: 6
            step: 0.25
        }

        EoNumber {
            sid: "stemMinInnerLengthThreeBeams"
            title: qsTrc("notation/engravingoptions", "Minimum stem between notehead and beams, three or more beams")
            hint: qsTrc("notation/engravingoptions",
                         "Shortest free stem between the notehead and the innermost beam for a note in a beamed group or two-note tremolo with three or more beams; the height of the beams is added to it. Rounded to quarter spaces.")
            min: 0.5
            max: 6
            step: 0.25
        }

        EoNumber {
            sid: "stemMinStaffOverlap"
            title: qsTrc("notation/engravingoptions", "Stem reach into the staff")
            hint: qsTrc("notation/engravingoptions",
                         "Stems of notes far outside the staff are lengthened so that their tip reaches at least this far into the staff, measured from the staff line nearest the notehead (2 sp = the middle line of a five-line staff). Applies to unbeamed and flagged notes and to notes with up to two beams. Rounded to quarter spaces.")
            max: 6
            step: 0.25
        }

        EoNumber {
            sid: "stemMinStaffOverlapThreeBeams"
            title: qsTrc("notation/engravingoptions", "Stem reach into the staff, three beams")
            hint: qsTrc("notation/engravingoptions",
                         "The same for normal-size notes with three beams. Rounded to quarter spaces.")
            max: 6
            step: 0.25
        }

        EoNumber {
            sid: "stemMinStaffOverlapFourBeams"
            title: qsTrc("notation/engravingoptions", "Stem reach into the staff, four beams")
            hint: qsTrc("notation/engravingoptions",
                         "The same for normal-size notes with four beams and the regular beam distance; each further beam adds one beam distance. Rounded to quarter spaces.")
            max: 6
            step: 0.25
        }

        EoNumber {
            sid: "stemMinStaffOverlapFourBeamsWide"
            title: qsTrc("notation/engravingoptions", "Stem reach into the staff, four beams, wide distance")
            hint: qsTrc("notation/engravingoptions",
                         "The same for normal-size notes with four beams when the wide beam distance is on. Rounded to quarter spaces.")
            max: 6
            step: 0.25
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Stem direction")

        EoToggle {
            sid: "stemUpOnMiddleLine"
            title: qsTrc("notation/engravingoptions", "Stems up on the middle line")
            hint: qsTrc("notation/engravingoptions",
                         "Stem direction of a note on the middle staff line, and of chords and beamed groups whose notes balance around it: down when off, up when on.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Flags")

        EoNumber {
            sid: "flagStemCutoutUp"
            title: qsTrc("notation/engravingoptions", "Flag height not counted for the stem length, stem up")
            hint: qsTrc("notation/engravingoptions",
                         "Stems of flagged notes are lengthened until the flags clear the notehead; this much of the flag height of an up-stem note with curved flags is ignored, so a larger value gives shorter stems on notes with many flags. Rounded to quarter spaces.")
            max: 3
            step: 0.25
        }

        EoNumber {
            sid: "flagStemCutoutDown"
            title: qsTrc("notation/engravingoptions", "Flag height not counted for the stem length, stem down")
            hint: qsTrc("notation/engravingoptions",
                         "The same for down-stem notes with curved flags. Rounded to quarter spaces.")
            max: 3
            step: 0.25
        }

        EoNumber {
            sid: "flagStemMaxHeight"
            title: qsTrc("notation/engravingoptions", "Largest flag height counted for the stem length")
            hint: qsTrc("notation/engravingoptions",
                         "Upper limit of the flag height taken into account when lengthening the stem of a flagged note; keeps fonts with very long flags from producing overlong stems. Rounded to quarter spaces.")
            min: 1
            max: 8
            step: 0.25
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Grace-note stem slashes")

        EoNumber {
            sid: "stemSlashPositionReduction"
            title: qsTrc("notation/engravingoptions", "Slash position factor without a curved flag")
            hint: qsTrc("notation/engravingoptions",
                         "Factor applied to the stem slash position (its distance from the stem end) on beamed grace notes, on stems without a flag and with straight flags.")
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "stemSlashMultiFlagAngleFactor"
            title: qsTrc("notation/engravingoptions", "Slash angle factor for two or more curved flags")
            hint: qsTrc("notation/engravingoptions",
                         "Factor applied to the stem slash angle on grace notes with two or more curved flags, to follow their heavier flags (Bravura, Finale Maestro and Gonville only).")
            min: 0.5
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "stemSlashBeamedLength"
            title: qsTrc("notation/engravingoptions", "Slash length on beamed grace notes")
            hint: qsTrc("notation/engravingoptions",
                         "Length of the slash drawn through the first stem and the beam of a beamed group of grace notes.")
            min: 0.5
            max: 5
        }

        EoNumber {
            sid: "stemSlashBeamedObtuseLengthFactor"
            title: qsTrc("notation/engravingoptions", "Slash length factor when the beam slants away")
            hint: qsTrc("notation/engravingoptions",
                         "Factor lengthening that slash when the beam slants away from it: a rising beam on up-stem grace notes or a falling beam on down-stem grace notes.")
            min: 0.5
            max: 2
            step: 0.01
        }
    }
}
