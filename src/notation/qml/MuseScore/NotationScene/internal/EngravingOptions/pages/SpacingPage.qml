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

    code: "spacing"
    legacyCodes: ["spacing"]
    title: qsTrc("EditStyleBase", "Horizontal spacing")
    group: qsTrc("notation/engravingoptions", "Layout")

    EoSection {
        title: qsTrc("EditStyleBase", "Horizontal spacing")

        EoNumber {
            sid: "spacingDensity"
            title: qsTrc("EditStyleBase", "System density:")
            min: 10
            max: 1000
            step: 1.0
            decimals: 1
            percent: true
        }

        EoNumber {
            sid: "measureSpacing"
            title: qsTrc("EditStyleBase", "Spacing ratio:")
            min: 1
            max: 2
            step: 0.1
            decimals: 3
        }

        EoNumber {
            sid: "minMeasureWidth"
            title: qsTrc("EditStyleBase", "Minimum measure width:")
            min: 2
            step: 1.0
        }

        EoNumber {
            sid: "lastSystemFillLimit"
            title: qsTrc("EditStyleBase", "Last system fill threshold:")
            max: 100
            step: 1
            decimals: 0
            percent: true
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Padding")

        EoNumber {
            sid: "clefKeyDistance"
            title: qsTrc("EditStyleBase", "Clef to key signature:")
            step: 0.01
        }

        EoNumber {
            sid: "clefTimesigDistance"
            title: qsTrc("EditStyleBase", "Clef to time signature:")
            step: 0.01
        }

        EoNumber {
            sid: "keyTimesigDistance"
            title: qsTrc("EditStyleBase", "Key signature to time signature:")
            step: 0.01
        }

        EoNumber {
            sid: "clefKeyRightMargin"
            title: qsTrc("EditStyleBase", "Clef to note:")
        }

        EoNumber {
            sid: "minNoteDistance"
            title: qsTrc("EditStyleBase", "Note to note:")
            step: 0.1
        }

        EoNumber {
            sid: "noteBarDistance"
            title: qsTrc("EditStyleBase", "Note to barline:")
            step: 0.1
        }

        EoNumber {
            sid: "barNoteDistance"
            title: qsTrc("EditStyleBase", "Barline to note:")
            step: 0.1
        }

        EoNumber {
            sid: "barAccidentalDistance"
            title: qsTrc("EditStyleBase", "Barline to accidental:")
            step: 0.1
        }

        EoNumber {
            sid: "barGraceDistance"
            title: qsTrc("EditStyleBase", "Barline to grace note:")
            step: 0.1
        }

        EoNumber {
            sid: "clefLeftMargin"
            title: qsTrc("EditStyleBase", "Barline to clef:")
            step: 0.1
        }

        EoNumber {
            sid: "clefBarlineDistance"
            title: qsTrc("EditStyleBase", "Clef to barline:")
            step: 0.01
        }

        EoNumber {
            sid: "keysigLeftMargin"
            title: qsTrc("EditStyleBase", "Barline to key signature:")
            step: 0.1
        }

        EoNumber {
            sid: "keyBarlineDistance"
            title: qsTrc("EditStyleBase", "Key signature to barline:")
            step: 0.01
        }

        EoNumber {
            sid: "timesigLeftMargin"
            title: qsTrc("EditStyleBase", "Barline to time signature:")
            step: 0.1
        }

        EoNumber {
            sid: "timesigBarlineDistance"
            title: qsTrc("EditStyleBase", "Time signature to barline:")
            step: 0.1
        }

        EoNumber {
            sid: "graceToMainNoteDist"
            title: qsTrc("EditStyleBase", "Grace note to main note:")
            step: 0.1
        }

        EoNumber {
            sid: "graceToGraceNoteDist"
            title: qsTrc("EditStyleBase", "Grace note to grace note:")
            step: 0.1
        }

        EoNumber {
            sid: "headerToLineStartDistance"
            title: qsTrc("EditStyleBase", "System header to continued tie or line:")
            step: 0.1
        }

        EoNumber {
            sid: "barlineToLineStartDistance"
            title: qsTrc("EditStyleBase", "Barline to continued tie or line:")
            step: 0.1
        }

        EoNumber {
            sid: "lineEndToBarlineDistance"
            title: qsTrc("EditStyleBase", "Continued tie or line to barline:")
            step: 0.1
        }

        EoNumber {
            sid: "systemTrailerRightMargin"
            title: qsTrc("EditStyleBase", "Right margin after courtesy signatures:")
            step: 0.1
        }

        EoNumber {
            sid: "ambitusMargin"
            title: qsTrc("EditStyleBase", "Ambitus padding:")
            step: 0.1
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "System header")

        EoNumber {
            sid: "systemHeaderDistance"
            title: qsTrc("EditStyleBase", "Clef/key signature to first note:")
            step: 0.1
        }

        EoNumber {
            sid: "systemHeaderTimeSigDistance"
            title: qsTrc("EditStyleBase", "Time signature to first note:")
            step: 0.1
        }

        EoNumber {
            sid: "systemHeaderMinStartOfSystemDistance"
            title: qsTrc("EditStyleBase", "Start of the system to first note:")
            step: 1.0
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Japanese-style spacing")

        EoToggle {
            sid: "useAdaptiveSpacingRatio"
            title: qsTrc("EditStyleBase", "Vary spacing ratio by note density of each system")
        }

        EoNumber {
            sid: "spacingRatioDense"
            title: qsTrc("EditStyleBase", "Dense-line spacing ratio:")
            min: 1
            max: 2
            step: 0.1
            decimals: 3
        }

        EoNumber {
            sid: "spacingRatioSparse"
            title: qsTrc("EditStyleBase", "Sparse-line spacing ratio:")
            min: 1
            max: 2
            step: 0.1
            decimals: 3
        }

        EoToggle {
            sid: "keepEqualDurationSpacing"
            title: qsTrc("EditStyleBase", "Keep equal spacing for equal note values (per system)")
        }

        EoToggle {
            sid: "crossStaffBeamSpacingOffset"
            title: qsTrc("EditStyleBase", "Offset spacing of cross-staff beamed notes")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Rhythmic spacing")

        EoNumber {
            sid: "spacingQuarterNoteSpace"
            title: qsTrc("notation/engravingoptions", "Space for a quarter note")
            hint: qsTrc("notation/engravingoptions",
                         "Width given to a quarter note before the spacing density and justification are applied; other note values follow from it through the spacing ratio.")
            min: 0.5
            max: 10
            step: 0.1
        }

        EoNumber {
            sid: "spacingRatioSparseThreshold"
            title: qsTrc("notation/engravingoptions", "Sparse-line density threshold")
            hint: qsTrc("notation/engravingoptions",
                         "Note density (notes per staff space of system width) at or below which a system gets the sparse-line spacing ratio; between the two thresholds the ratio is interpolated.")
            min: 0.01
            max: 0.3
            step: 0.01
        }

        EoNumber {
            sid: "spacingRatioDenseThreshold"
            title: qsTrc("notation/engravingoptions", "Dense-line density threshold")
            hint: qsTrc("notation/engravingoptions",
                         "Note density (notes per staff space of system width) at or above which a system gets the dense-line spacing ratio; keep it above the sparse-line threshold.")
            min: 0.31
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "mmRestBaseWidthMeasures"
            title: qsTrc("notation/engravingoptions", "Multimeasure rest: measures at base width")
            hint: qsTrc("notation/engravingoptions",
                         "A multimeasure rest of up to this many measures keeps the base width; each further measure widens it by one step, up to the maximum width increase.")
            min: 1
            max: 20
            step: 1
            decimals: 0
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Padding limits and kerning")

        EoNumber {
            sid: "spacingDefaultPadding"
            title: qsTrc("notation/engravingoptions", "Default padding between items")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between two items of kinds that have no setting of their own, and between two noteheads that may come closer than the minimum note distance.")
            max: 2
        }

        EoNumber {
            sid: "spacingAbsoluteMinPadding"
            title: qsTrc("notation/engravingoptions", "Least padding between items")
            hint: qsTrc("notation/engravingoptions",
                         "No pair of items gets less padding than this, whatever its own setting is; like every padding it shrinks when an overfull system is squeezed.")
            max: 2
        }

        EoNumber {
            sid: "kerningVerticalClearance"
            title: qsTrc("notation/engravingoptions", "Vertical clearance for kerning")
            hint: qsTrc("notation/engravingoptions",
                         "Two items may overlap horizontally (one tucked above or below the other) only if this much vertical space stays between them.")
            max: 2
        }

        EoNumber {
            sid: "kerningVerticalClearanceAccidental"
            title: qsTrc("notation/engravingoptions", "Vertical clearance for kerning accidentals")
            hint: qsTrc("notation/engravingoptions",
                         "Vertical space that must stay between an accidental and the item before it for the accidental to tuck above or below that item.")
            max: 2
        }

        EoNumber {
            sid: "kerningVerticalClearanceParenthesis"
            title: qsTrc("notation/engravingoptions", "Vertical clearance for kerning parentheses")
            hint: qsTrc("notation/engravingoptions",
                         "Vertical space that must stay between a parenthesis and a neighbouring item for one to tuck above or below the other.")
            max: 2
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Gaps after notes and rests")

        EoNumber {
            sid: "noteRestDistance"
            title: qsTrc("notation/engravingoptions", "Note to rest")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a notehead and a rest that follows it. Also used from a rest to a note and between two rests.")
            max: 5
        }

        EoNumber {
            sid: "noteClefDistance"
            title: qsTrc("notation/engravingoptions", "Note to clef")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a notehead and a clef that follows it. Also used from a rest to a clef.")
            max: 5
        }

        EoNumber {
            sid: "noteArpeggioDistance"
            title: qsTrc("notation/engravingoptions", "Note to arpeggio")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a notehead and an arpeggio that follows it.")
            max: 5
        }

        EoNumber {
            sid: "noteKeyDistance"
            title: qsTrc("notation/engravingoptions", "Note to key signature")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a notehead and a key signature that follows it.")
            max: 5
        }

        EoNumber {
            sid: "noteTimesigDistance"
            title: qsTrc("notation/engravingoptions", "Note to time signature")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a notehead and a time signature that follows it.")
            max: 5
        }

        EoNumber {
            sid: "noteAccidentalDistance"
            title: qsTrc("notation/engravingoptions", "Note to accidental")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a notehead and an accidental that follows it. The accidental to note distance is used instead if it is larger.")
            max: 5
        }

        EoNumber {
            sid: "flagNoteDistance"
            title: qsTrc("notation/engravingoptions", "Flag to note")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a flag and a notehead that follows it. Also used from a flag to a rest.")
            max: 5
        }

        EoNumber {
            sid: "flagAccidentalDistance"
            title: qsTrc("notation/engravingoptions", "Flag to accidental")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a flag and an accidental that follows it.")
            max: 5
        }

        EoNumber {
            sid: "flagClefDistance"
            title: qsTrc("notation/engravingoptions", "Flag to clef")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a flag and a clef that follows it.")
            max: 5
        }

        EoNumber {
            sid: "flagArpeggioDistance"
            title: qsTrc("notation/engravingoptions", "Flag to arpeggio")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a flag and an arpeggio that follows it.")
            max: 5
        }

        EoNumber {
            sid: "flagBarlineDistance"
            title: qsTrc("notation/engravingoptions", "Flag to barline")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a flag and a barline that follows it.")
            max: 5
        }

        EoNumber {
            sid: "flagKeyDistance"
            title: qsTrc("notation/engravingoptions", "Flag to key signature")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a flag and a key signature that follows it.")
            max: 5
        }

        EoNumber {
            sid: "flagTimesigDistance"
            title: qsTrc("notation/engravingoptions", "Flag to time signature")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a flag and a time signature that follows it.")
            max: 5
        }

        EoNumber {
            sid: "dotAccidentalDistance"
            title: qsTrc("notation/engravingoptions", "Augmentation dot to accidental")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between an augmentation dot and an accidental that follows it.")
            max: 5
        }

        EoNumber {
            sid: "dotClefDistance"
            title: qsTrc("notation/engravingoptions", "Augmentation dot to clef")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between an augmentation dot and a clef that follows it.")
            max: 5
        }

        EoNumber {
            sid: "dotArpeggioDistance"
            title: qsTrc("notation/engravingoptions", "Augmentation dot to arpeggio")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between an augmentation dot and an arpeggio that follows it.")
            max: 5
        }

        EoNumber {
            sid: "dotBarlineDistance"
            title: qsTrc("notation/engravingoptions", "Augmentation dot to barline")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between an augmentation dot and a barline that follows it.")
            max: 5
        }

        EoNumber {
            sid: "dotKeyDistance"
            title: qsTrc("notation/engravingoptions", "Augmentation dot to key signature")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between an augmentation dot and a key signature that follows it.")
            max: 5
        }

        EoNumber {
            sid: "dotTimesigDistance"
            title: qsTrc("notation/engravingoptions", "Augmentation dot to time signature")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between an augmentation dot and a time signature that follows it.")
            max: 5
        }

        EoNumber {
            sid: "restAccidentalDistance"
            title: qsTrc("notation/engravingoptions", "Rest to accidental")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a rest and an accidental that follows it.")
            max: 5
        }

        EoNumber {
            sid: "restBarlineDistance"
            title: qsTrc("notation/engravingoptions", "Rest to barline")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a rest and a barline that follows it.")
            max: 5
        }

        EoNumber {
            sid: "restKeyDistance"
            title: qsTrc("notation/engravingoptions", "Rest to key signature")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a rest and a key signature that follows it.")
            max: 5
        }

        EoNumber {
            sid: "restTimesigDistance"
            title: qsTrc("notation/engravingoptions", "Rest to time signature")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a rest and a time signature that follows it.")
            max: 5
        }

        EoNumber {
            sid: "stemStemDistance"
            title: qsTrc("notation/engravingoptions", "Stem to stem")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a stem and a stem that follows it. Applies between the stems of adjacent chords, above all under cross-staff beams.")
            max: 5
        }

        EoNumber {
            sid: "stemAccidentalDistance"
            title: qsTrc("notation/engravingoptions", "Stem to accidental")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a stem and an accidental that follows it.")
            max: 5
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Gaps after clefs, signatures and barlines")

        EoNumber {
            sid: "systemHeaderMinClearDistance"
            title: qsTrc("notation/engravingoptions", "System header: least clear gap")
            hint: qsTrc("notation/engravingoptions",
                         "Least clear space between the right edge of the clef, key or time signature that ends the system header and the leftmost point of what follows: the first note or rest with its accidentals, or a start repeat barline.")
            max: 5
        }

        EoNumber {
            sid: "clefAccidentalDistance"
            title: qsTrc("notation/engravingoptions", "Clef to accidental")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a clef and an accidental that follows it.")
            max: 5
        }

        EoNumber {
            sid: "clefClefDistance"
            title: qsTrc("notation/engravingoptions", "Clef to clef")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a clef and a clef that follows it.")
            max: 5
        }

        EoNumber {
            sid: "clefArpeggioDistance"
            title: qsTrc("notation/engravingoptions", "Clef to arpeggio")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a clef and an arpeggio that follows it.")
            max: 5
        }

        EoNumber {
            sid: "clefStemDistance"
            title: qsTrc("notation/engravingoptions", "Clef to stem")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a clef and a stem that follows it.")
            max: 5
        }

        EoNumber {
            sid: "barArpeggioDistance"
            title: qsTrc("notation/engravingoptions", "Barline to arpeggio")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a barline and an arpeggio that follows it.")
            max: 5
        }

        EoNumber {
            sid: "barBarlineDistance"
            title: qsTrc("notation/engravingoptions", "Barline to barline")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a barline and a barline that follows it. This is the least inner width of a bar whose content takes up no room.")
            max: 5
        }

        EoNumber {
            sid: "keyNoteDistance"
            title: qsTrc("notation/engravingoptions", "Key signature to note")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a key signature and a notehead that follows it. In a system header the system header distance applies instead if it is larger; also used from a key signature to a rest.")
            max: 5
        }

        EoNumber {
            sid: "keyNoteAccidentalDistance"
            title: qsTrc("notation/engravingoptions", "Key signature to accidental")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a key signature and an accidental that follows it. The accidental is that of the first note after the key signature.")
            max: 5
        }

        EoNumber {
            sid: "keyClefDistance"
            title: qsTrc("notation/engravingoptions", "Key signature to clef")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a key signature and a clef that follows it.")
            max: 5
        }

        EoNumber {
            sid: "keyArpeggioDistance"
            title: qsTrc("notation/engravingoptions", "Key signature to arpeggio")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a key signature and an arpeggio that follows it.")
            max: 5
        }

        EoNumber {
            sid: "keyKeyDistance"
            title: qsTrc("notation/engravingoptions", "Key signature to key signature")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a key signature and a key signature that follows it.")
            max: 5
        }

        EoNumber {
            sid: "timesigNoteDistance"
            title: qsTrc("notation/engravingoptions", "Time signature to note")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a time signature and a notehead that follows it. In a system header the system header distance for time signatures applies instead if it is larger; also used from a time signature to a rest.")
            max: 5
        }

        EoNumber {
            sid: "timesigAccidentalDistance"
            title: qsTrc("notation/engravingoptions", "Time signature to accidental")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a time signature and an accidental that follows it.")
            max: 5
        }

        EoNumber {
            sid: "timesigClefDistance"
            title: qsTrc("notation/engravingoptions", "Time signature to clef")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a time signature and a clef that follows it.")
            max: 5
        }

        EoNumber {
            sid: "timesigArpeggioDistance"
            title: qsTrc("notation/engravingoptions", "Time signature to arpeggio")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a time signature and an arpeggio that follows it.")
            max: 5
        }

        EoNumber {
            sid: "timesigTimesigDistance"
            title: qsTrc("notation/engravingoptions", "Time signature to time signature")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a time signature and a time signature that follows it.")
            max: 5
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Gaps around other items")

        EoNumber {
            sid: "lyricsAvoidBarlinesPadding"
            title: qsTrc("notation/engravingoptions", "Lyrics to barline padding (× x-height)")
            hint: qsTrc("notation/engravingoptions",
                         "Gap kept between a lyric syllable and a barline it must avoid, as a multiple of the x-height of the lyrics font.")
            max: 3
        }

        EoNumber {
            sid: "ledgerLineLedgerLineDistance"
            title: qsTrc("notation/engravingoptions", "Ledger line to ledger line")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum gap between the ledger lines of neighbouring chords; automatically shortened ledger lines keep a fraction of it.")
            max: 2
        }

        EoNumber {
            sid: "arpeggioLedgerLineDistance"
            title: qsTrc("notation/engravingoptions", "Arpeggio to ledger line")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between an arpeggio and the ledger lines of the chord it belongs to.")
            max: 2
        }

        EoNumber {
            sid: "accidentalLedgerLineDistance"
            title: qsTrc("notation/engravingoptions", "Accidental to ledger line")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between an accidental and the ledger lines of the chord it belongs to.")
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "breathHorizontalPadding"
            title: qsTrc("notation/engravingoptions", "Padding around breath marks")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a breath mark or caesura and the items before and after it.")
            max: 5
        }

        EoNumber {
            sid: "harmonyFretDiagramPadding"
            title: qsTrc("notation/engravingoptions", "Chord symbol to fretboard diagram")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a chord symbol and a neighbouring fretboard diagram, in either order.")
            max: 5
        }

        EoNumber {
            sid: "fretDiagramPadding"
            title: qsTrc("notation/engravingoptions", "Fretboard diagram to fretboard diagram")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between two neighbouring fretboard diagrams.")
            max: 5
        }

        EoNumber {
            sid: "chordlineHorizontalPadding"
            title: qsTrc("notation/engravingoptions", "Padding around falls, doits and scoops")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a fall, doit, plop or scoop and the items before and after it.")
            max: 5
        }

        EoNumber {
            sid: "chordlineBarlinePadding"
            title: qsTrc("notation/engravingoptions", "Fall, doit or scoop to barline")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a fall, doit, plop or scoop and a barline, in either order.")
            max: 5
        }

        EoNumber {
            sid: "beamletPadding"
            title: qsTrc("notation/engravingoptions", "Padding after beamlets")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a beamlet (the stub of a beam) or a single-note tremolo and the item that follows; also the gap between two beamlets that face each other in a cross-staff beam.")
            max: 5
        }

        EoNumber {
            sid: "articulationHorizontalPadding"
            title: qsTrc("notation/engravingoptions", "Padding around articulations and fermatas")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between an articulation, fermata or tapping sign and neighbouring items it cannot overlap.")
            max: 5
        }

        EoNumber {
            sid: "laissezVibNoteDistance"
            title: qsTrc("notation/engravingoptions", "Laissez vibrer tie to note")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between the end of a laissez vibrer tie and the next note or rest.")
            max: 5
        }

        EoNumber {
            sid: "laissezVibAccidentalDistance"
            title: qsTrc("notation/engravingoptions", "Laissez vibrer tie to accidental")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between the end of a laissez vibrer tie and an accidental of the next note.")
            max: 5
        }

        EoNumber {
            sid: "laissezVibBarlineDistance"
            title: qsTrc("notation/engravingoptions", "Laissez vibrer tie to barline")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between the end of a laissez vibrer tie and the barline after it.")
            max: 5
        }

        EoNumber {
            sid: "laissezVibStemDistance"
            title: qsTrc("notation/engravingoptions", "Laissez vibrer tie to stem")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between the end of a laissez vibrer tie and the stem of the next note.")
            max: 5
        }

        EoNumber {
            sid: "parenthesesMinInnerWidth"
            title: qsTrc("notation/engravingoptions", "Parentheses: minimum inner width")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum distance between a left and a right parenthesis; an item narrower than this is centred between them.")
            max: 5
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Tablature and guitar bends")

        EoNumber {
            sid: "tabFretPaddingScale"
            title: qsTrc("notation/engravingoptions", "Fret mark padding scale")
            hint: qsTrc("notation/engravingoptions",
                         "Factor applied to the padding between adjacent fret marks on a tablature staff.")
            min: 0.5
            max: 5
            step: 0.1
        }

        EoNumber {
            sid: "minGuitarBendLength"
            title: qsTrc("notation/engravingoptions", "Minimum guitar bend length")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal space reserved between two notes that are joined by a guitar bend.")
            max: 10
            step: 0.1
        }
    }
}
