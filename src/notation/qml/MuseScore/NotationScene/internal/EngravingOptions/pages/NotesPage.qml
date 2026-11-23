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

    code: "notes"
    legacyCodes: ["notes"]
    title: qsTrc("EditStyleBase", "Notes")
    group: qsTrc("notation/engravingoptions", "Notes and rests")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Ledger lines")
        music: "c'''4"

        EoNumber {
            sid: "ledgerLineWidth"
            music: "c'''4"
            title: qsTrc("EditStyleBase", "Ledger line thickness:")
            step: 0.01
        }

        EoNumber {
            sid: "ledgerLineLength"
            music: "c'''4"
            title: qsTrc("notation/engravingoptions", "Extension past the notehead")
            hint: qsTrc("notation/engravingoptions", "How far a ledger line sticks out on each side of the notehead.")
            step: 0.1
        }

        EoNumber {
            sid: "ledgerLineLedgerLineDistance"
            music: "a''16 b'' c''' d''' e''' f''' g''' a'''"
            musicStyle: ({ ledgerLineAutoCut: true })
            title: qsTrc("notation/engravingoptions", "Standard gap between neighbouring ledger lines")
            hint: qsTrc("notation/engravingoptions",
                        "The gap which ‘Gap between shortened ledger lines’ is a fraction of. Notes are spaced " +
                        "without regard to their ledger lines, so this value alone does not move them apart.")
            max: 2
        }

        EoToggle {
            sid: "ledgerLineAutoCut"
            music: "a''16 b'' c''' d''' e''' f''' g''' a'''"
            title: qsTrc("EditStyleBase", "Shorten ledger lines that would touch their neighbours")
            hint: qsTrc("notation/engravingoptions",
                        "In tightly spaced music the ledger lines of neighbouring notes are shortened on the sides " +
                        "which face each other; only the part that sticks out past the notehead is taken away.")
        }

        EoNumber {
            sid: "ledgerLineAutoCutGapRatio"
            music: "a''16 b'' c''' d''' e''' f''' g''' a'''"
            musicStyle: ({ ledgerLineAutoCut: true })
            title: qsTrc("notation/engravingoptions", "Gap between shortened ledger lines")
            hint: qsTrc("notation/engravingoptions",
                        "As a fraction of ‘Standard gap between neighbouring ledger lines’: 0.5 leaves half of that " +
                        "gap between two shortened lines. Only with ‘Shorten ledger lines that would touch their " +
                        "neighbours’ on.")
            max: 2
        }

        EoNumber {
            sid: "ledgerLineAutoCutMinGap"
            music: "a''16 b'' c''' d''' e''' f''' g''' a'''"
            musicStyle: ({ ledgerLineAutoCut: true })
            title: qsTrc("notation/engravingoptions", "Smallest gap between shortened ledger lines")
            hint: qsTrc("notation/engravingoptions",
                        "The gap between shortened ledger lines is never smaller than this, whatever the fraction " +
                        "above gives; 0 leaves it to the fraction alone.")
            max: 2
            step: 0.05
        }

        EoNumber {
            sid: "ledgerLineAutoCutMinExtension"
            music: "a''16 b'' c''' d''' e''' f''' g''' a'''"
            musicStyle: ({ ledgerLineAutoCut: true, ledgerLineAutoCutMinGap: 1.5 })
            title: qsTrc("notation/engravingoptions", "Smallest extension left on a shortened ledger line")
            hint: qsTrc("notation/engravingoptions",
                        "A shortened ledger line still sticks out past the notehead by at least this much, even if " +
                        "the lines then come closer than the gap; 0 lets a line be cut back to the width of the " +
                        "notehead.")
            max: 1
            step: 0.01
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Augmentation dots")
        music: "c'4."

        EoNumber {
            sid: "dotMag"
            music: "c'4."
            title: qsTrc("EditStyleBase", "Dot size:")
            hint: qsTrc("notation/engravingoptions", "Relative to the size of the note the dot belongs to.")
            min: 50
            max: 200
            step: 10
            decimals: 0
            percent: true
        }

        EoNumber {
            sid: "dotNoteDistance"
            music: "c'4."
            title: qsTrc("EditStyleBase", "Note to dot distance:")
            hint: qsTrc("notation/engravingoptions", "Gap between the notehead, or a rest, and its first dot.")
            step: 0.1
        }

        EoNumber {
            sid: "dotDotDistance"
            music: "c'4.."
            title: qsTrc("EditStyleBase", "Dot to dot distance:")
            hint: qsTrc("notation/engravingoptions",
                        "Measured from the left edge of one dot to the left edge of the next, so it includes the " +
                        "width of a dot.")
            step: 0.1
        }

        EoToggle {
            sid: "dotBelowLineForLowerVoices"
            music: "<< { b'4. r8 r2 } \\\\ { g'4. r8 r2 } >>"
            title: qsTrc("notation/engravingoptions", "Dots of voices 2 and 4 below the line")
            hint: qsTrc("notation/engravingoptions",
                        "For a note on a staff line, the dots of voices 2 and 4 go into the space below the line; " +
                        "when off, the dots of all voices go into the space above.")
        }

        EoNumber {
            sid: "dotFlagVerticalClearance"
            music: "\\stemUp g'8. r16 r4 r2"
            title: qsTrc("notation/engravingoptions", "Clearance between a flag and the dot below it")
            hint: qsTrc("notation/engravingoptions",
                        "On a stem-up note with a flag, the dots stay beside the notehead only if the flag ends at " +
                        "least this far above the top dot; otherwise they move to the right of the flag.")
            min: -1
            max: 2
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Voices sharing a staff")

        EoToggle {
            sid: "combineVoice"
            music: "<< { c''2 d''2 } \\\\ { c''2 d''2 } >>"
            title: qsTrc("EditStyleBase", "Combine voices that share the same stem direction")
            hint: qsTrc("notation/engravingoptions",
                        "Voices which sound together with their stems in the same direction are set like one chord: " +
                        "a second between them is written on both sides of the stem and their dots line up in one " +
                        "column. When off, each voice is placed on its own.")
        }

        EoToggle {
            sid: "voiceUnisonShareHeadsDifferentDots"
            music: "<< { g'4. g'8 } \\\\ { g'8 g'4. } >>"
            title: qsTrc("notation/engravingoptions", "Dotted and undotted unisons share one notehead")
            hint: qsTrc("notation/engravingoptions",
                        "Two voices on the same pitch with the same kind of notehead share it even when they have a " +
                        "different number of dots; when off, they are set side by side.")
        }

        EoNumber {
            sid: "voiceOffsetNoteheadGap"
            music: "<< { c''2 r2 } \\\\ { c''4 r4 r2 } >>"
            title: qsTrc("notation/engravingoptions", "Gap between voices set side by side")
            hint: qsTrc("notation/engravingoptions",
                        "Space between the two columns of noteheads when the voices cannot overlap: a unison which " +
                        "cannot share a notehead, or interlocking voices with a second between them.")
            min: -0.5
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "voiceOffsetReducedNoteheadGap"
            music: "<< { d''4. r8 r2 } \\\\ { c''4 r4 r2 } >>"
            title: qsTrc("notation/engravingoptions", "Gap when the up-stem voice is set to the right")
            hint: qsTrc("notation/engravingoptions",
                        "Used when only the up-stem voice is dotted in a unison or second, or when its noteheads " +
                        "must stay clear of a flag of the down-stem voice: the up-stem voice then stands to the " +
                        "right, this far from the other noteheads.")
            min: -0.5
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "voiceOffsetCrossedSecondOverlap"
            music: "<< { c''4 r4 r2 } \\\\ { d''4 r4 r2 } >>"
            title: qsTrc("notation/engravingoptions", "Overlap of the noteheads in a crossed second")
            hint: qsTrc("notation/engravingoptions",
                        "When the down-stem voice lies a second above the up-stem voice, the up-stem notehead stands " +
                        "to the right and is pushed into the other notehead by this much; 0 sets them edge to edge.")
            min: -1
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "voiceOffsetCrossedSecondDottedGap"
            music: "<< { c''4 r4 r2 } \\\\ { d''4. r8 r2 } >>"
            title: qsTrc("notation/engravingoptions", "Gap in a crossed second with dots")
            hint: qsTrc("notation/engravingoptions",
                        "When the down-stem voice lies a second above the up-stem voice and only it is dotted, it is " +
                        "set to the right, this far from the other notehead.")
            min: -0.5
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "voiceOffsetFlagClearance"
            music: "<< { c''4 r4 r2 } \\\\ { e''8 r8 r4 r2 } >>"
            title: qsTrc("notation/engravingoptions", "Gap beside a flagged down-stem voice")
            hint: qsTrc("notation/engravingoptions",
                        "When the voices overlap and only the down-stem voice has a flag, that voice is set to the " +
                        "right, with its stem this far from the noteheads of the up-stem voice, so that the flag " +
                        "does not run into them.")
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "voiceOffsetStemNoteheadPadding"
            music: "<< { c''4 r4 r2 } \\\\ { e''4 r4 r2 } >>"
            title: qsTrc("notation/engravingoptions", "Stem to noteheads of the other voice")
            hint: qsTrc("notation/engravingoptions",
                        "When the voices interlock without a unison or a second, the up-stem voice is moved right " +
                        "until every stem is this far from the noteheads of the other voice.")
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "voiceOffsetLedgerLineGap"
            music: "<< { a4 r4 r2 } \\\\ { c'4 r4 r2 } >>"
            title: qsTrc("notation/engravingoptions", "Stem or flag to ledger lines of the other voice")
            hint: qsTrc("notation/engravingoptions",
                        "Outside the staff, where the stem of one voice passes the ledger lines of the other, the " +
                        "voices are moved apart until the stem or flag is this far from the end of those ledger " +
                        "lines.")
            max: 1
            step: 0.01
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Grace notes")
        music: "\\acciaccatura c''8 e''4"

        EoToggle {
            sid: "graceNoteStemAlwaysUp"
            music: "\\acciaccatura e''8 d''4"
            title: qsTrc("notation/engravingoptions", "Grace note stems always up")
            hint: qsTrc("notation/engravingoptions",
                        "In a single voice, grace notes have their stems up whatever their pitch; when off, their " +
                        "stems follow the staff position like those of normal notes. With several voices the stem " +
                        "direction follows the voice in both cases.")
        }

        EoNumber {
            sid: "graceToMainNoteDist"
            music: "\\appoggiatura a'4 a'4"
            title: qsTrc("EditStyleBase", "Grace note to main note:")
            hint: qsTrc("notation/engravingoptions",
                        "Smallest gap between a grace note and the normal note or rest next to it.")
            step: 0.1
        }

        EoNumber {
            sid: "graceToGraceNoteDist"
            music: "\\acciaccatura { c''16 c'' c'' } c''4"
            title: qsTrc("EditStyleBase", "Grace note to grace note:")
            hint: qsTrc("notation/engravingoptions", "Smallest gap between two successive grace notes.")
            step: 0.1
        }

        EoNumber {
            sid: "stemSlashPosition"
            music: "\\acciaccatura c''8 e''4"
            title: qsTrc("EditStyleBase", "Grace note slash distance from stem tip:")
            hint: qsTrc("notation/engravingoptions",
                        "How far from the end of the stem the slash starts, on a grace note with a curved flag.")
            step: 0.1
        }

        EoNumber {
            sid: "stemSlashAngle"
            music: "\\acciaccatura c''8 e''4"
            title: qsTrc("EditStyleBase", "Grace note slash angle:")
            hint: qsTrc("notation/engravingoptions",
                        "Measured from the horizontal: a larger angle makes the slash steeper.")
            max: 90
            step: 1
            decimals: 1
            unit: "°"
        }

        EoNumber {
            sid: "stemSlashThickness"
            music: "\\acciaccatura c''8 e''4"
            title: qsTrc("EditStyleBase", "Grace note slash thickness:")
            step: 0.01
            decimals: 3
        }

        EoNumber {
            sid: "stemSlashPositionReduction"
            music: "\\acciaccatura { c''8 d''8 } e''4"
            title: qsTrc("notation/engravingoptions", "Slash distance from stem tip without a curved flag")
            hint: qsTrc("notation/engravingoptions",
                        "As a multiple of ‘Grace note slash distance from stem tip’: on beamed grace notes, on stems " +
                        "without a flag and with straight flags the slash sits nearer the end of the stem.")
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "stemSlashMultiFlagAngleFactor"
            music: "\\acciaccatura c''16 e''4"
            title: qsTrc("notation/engravingoptions", "Slash angle with two or more curved flags")
            hint: qsTrc("notation/engravingoptions",
                        "As a multiple of ‘Grace note slash angle’, so that the slash crosses the taller double " +
                        "flags of Bravura, Finale Maestro and Gonville; other music fonts are not affected.")
            min: 0.5
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "stemSlashBeamedLength"
            music: "\\acciaccatura { c''8 d''8 } e''4"
            title: qsTrc("notation/engravingoptions", "Slash length on beamed grace notes")
            hint: qsTrc("notation/engravingoptions",
                        "Length of the slash drawn through the first stem and the beam of a group of beamed grace " +
                        "notes.")
            min: 0.5
            max: 5
        }

        EoNumber {
            sid: "stemSlashBeamedObtuseLengthFactor"
            music: "\\stemUp \\acciaccatura { c''8 g''8 } e''4"
            title: qsTrc("notation/engravingoptions", "Slash lengthening when the beam slants away")
            hint: qsTrc("notation/engravingoptions",
                        "On beamed grace notes the slash is made this many times longer when the beam slants away " +
                        "from it (a rising beam with stems up, a falling beam with stems down), so that it still " +
                        "crosses the beam.")
            min: 0.5
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "stemSlashOverhangRatio"
            music: "\\acciaccatura c''8 e''4"
            title: qsTrc("notation/engravingoptions", "Slash start to the left of the stem")
            hint: qsTrc("notation/engravingoptions",
                        "As a fraction of the notehead width: how far to the left of the stem the slash begins. On a " +
                        "grace note with neither flag nor beam it ends equally far to the right.")
            max: 1.5
            step: 0.05
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Ambitus")

        EoNumber {
            sid: "ambitusMargin"
            music: "\\ambitus c''4 e''4 g'4"
            title: qsTrc("EditStyleBase", "Ambitus padding:")
            hint: qsTrc("notation/engravingoptions",
                        "Smallest gap between an ambitus and whatever stands on either side of it: clef, key " +
                        "signature, time signature or the first note.")
            step: 0.1
        }

        EoNumber {
            sid: "ambitusLineOffset"
            music: "\\ambitus c'4 e''4"
            title: qsTrc("notation/engravingoptions", "Line to notehead distance")
            hint: qsTrc("notation/engravingoptions",
                        "The line which joins the two noteheads stops this far from the centre of each of them.")
            max: 3
        }

        EoNumber {
            sid: "ambitusAccidentalUndercutRatio"
            music: "\\ambitus bes'2 des''2"
            title: qsTrc("notation/engravingoptions", "Lower accidental tucked under the upper one")
            hint: qsTrc("notation/engravingoptions",
                        "When the two accidentals of an ambitus would collide, a lower flat or natural moves left by " +
                        "this fraction of its own width only and tucks under the upper accidental; 1 sets it fully " +
                        "to the left.")
            max: 1
        }
    }
}
