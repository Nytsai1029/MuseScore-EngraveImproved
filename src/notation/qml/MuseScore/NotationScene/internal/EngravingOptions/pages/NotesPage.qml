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
        title: qsTrc("EditStyleBase", "Notes")

        EoToggle {
            sid: "shortenStem"
            title: qsTrc("EditStyleBase", "Shorten stems")
        }

        EoNumber {
            sid: "shortestStem"
            title: qsTrc("EditStyleBase", "Shortest stem:")
            min: 1
            step: 0.25
            unit: qsTrc("global", "sp")
            enabled: Boolean(page.value("shortenStem"))
        }

        EoNumber {
            sid: "shortStemStartLocation"
            title: qsTrc("EditStyleBase", "Shortening starts beyond the staff by (half spaces):")
            max: 8
            step: 1
            decimals: 0
            enabled: Boolean(page.value("shortenStem"))
        }

        EoNumber {
            sid: "accidentalNoteDistance"
            title: qsTrc("EditStyleBase", "Accidental to note distance:")
            step: 0.01
        }

        EoNumber {
            sid: "accidentalDistance"
            title: qsTrc("EditStyleBase", "Accidental to accidental distance:")
            step: 0.01
        }

        EoNumber {
            sid: "dotMag"
            title: qsTrc("EditStyleBase", "Dot size:")
            min: 50
            max: 200
            step: 10
            decimals: 0
            percent: true
        }

        EoNumber {
            sid: "dotNoteDistance"
            title: qsTrc("EditStyleBase", "Note to dot distance:")
            step: 0.1
        }

        EoNumber {
            sid: "dotDotDistance"
            title: qsTrc("EditStyleBase", "Dot to dot distance:")
            step: 0.1
        }

        EoNumber {
            sid: "stemWidth"
            title: qsTrc("EditStyleBase", "Stem thickness:")
            step: 0.1
        }

        EoNumber {
            sid: "ledgerLineWidth"
            title: qsTrc("EditStyleBase", "Ledger line thickness:")
            step: 0.01
        }

        EoNumber {
            sid: "ledgerLineLength"
            title: qsTrc("EditStyleBase", "Ledger line length:")
            step: 0.1
        }

        EoToggle {
            sid: "ledgerLineAutoCut"
            title: qsTrc("EditStyleBase", "Shorten ledger lines that would touch their neighbours")
        }

        EoNumber {
            sid: "stemSlashPosition"
            title: qsTrc("EditStyleBase", "Grace note slash distance from stem tip:")
            step: 0.1
        }

        EoNumber {
            sid: "stemSlashAngle"
            title: qsTrc("EditStyleBase", "Grace note slash angle:")
            max: 90
            step: 1
            decimals: 1
            unit: "°"
        }

        EoNumber {
            sid: "stemSlashThickness"
            title: qsTrc("EditStyleBase", "Grace note slash thickness:")
            step: 0.01
            decimals: 3
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Stem length")

        EoNumber {
            sid: "stemLength"
            title: qsTrc("EditStyleBase", "Standard stem length:")
            min: 1
            max: 10
            step: 0.25
            unit: qsTrc("global", "sp")
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Alignment in chords")

        EoToggle {
            sid: "combineVoice"
            title: qsTrc("EditStyleBase", "Combine voices that share the same stem direction")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Augmentation dots")

        EoToggle {
            sid: "dotBelowLineForLowerVoices"
            title: qsTrc("notation/engravingoptions", "Dots of voices 2 and 4 below the line")
            hint: qsTrc("notation/engravingoptions",
                         "For notes on a staff line, puts the augmentation dots of voices 2 and 4 in the space below the line; when off, the dots of all voices go in the space above.")
        }

        EoNumber {
            sid: "dotFlagVerticalClearance"
            title: qsTrc("notation/engravingoptions", "Dot clearance below a flag")
            hint: qsTrc("notation/engravingoptions",
                         "On stem-up flagged notes, the dots move to the right of the flag when the top dot is higher than the lower end of the flag plus this margin.")
            min: -1
            max: 2
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Ledger lines")

        EoNumber {
            sid: "ledgerLineAutoCutGapRatio"
            title: qsTrc("notation/engravingoptions", "Gap between shortened ledger lines")
            hint: qsTrc("notation/engravingoptions",
                         "Gap left between the ledger lines of neighbouring notes after automatic shortening, as a fraction of the padding that note spacing keeps between ledger lines.")
            max: 2
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Voices: notehead offsets")

        EoNumber {
            sid: "voiceOffsetNoteheadGap"
            title: qsTrc("notation/engravingoptions", "Gap between offset voices")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between the two columns of noteheads when voices are set side by side because of a unison that cannot share a notehead or a second between crossed voices.")
            min: -0.5
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "voiceOffsetReducedNoteheadGap"
            title: qsTrc("notation/engravingoptions", "Gap when the up-stem voice moves right")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between the two columns of noteheads when the up-stem voice is moved to the right of the down-stem voice, either because only it has augmentation dots or to clear a flag.")
            min: -0.5
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "voiceOffsetCrossedSecondOverlap"
            title: qsTrc("notation/engravingoptions", "Overlap of crossed seconds")
            hint: qsTrc("notation/engravingoptions",
                         "When crossed voices form a second, how far the up-stem notehead on the right tucks in towards the down-stem notehead on the left.")
            min: -1
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "voiceOffsetCrossedSecondDottedGap"
            title: qsTrc("notation/engravingoptions", "Gap of crossed seconds with dots")
            hint: qsTrc("notation/engravingoptions",
                         "When crossed voices form a second and only the down-stem voice has augmentation dots, the gap by which the down-stem voice is set to the right of the up-stem voice.")
            min: -0.5
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "voiceOffsetFlagClearance"
            title: qsTrc("notation/engravingoptions", "Extra offset to clear a flag")
            hint: qsTrc("notation/engravingoptions",
                         "Extra horizontal space added between overlapping voices when the down-stem voice carries a flag that the other voice would otherwise collide with.")
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "voiceOffsetStemNoteheadPadding"
            title: qsTrc("notation/engravingoptions", "Stem to notehead of the other voice")
            hint: qsTrc("notation/engravingoptions",
                         "Space between a stem and the noteheads of the other voice when the voices interlock without forming a unison or a second.")
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "voiceOffsetLedgerLineGap"
            title: qsTrc("notation/engravingoptions", "Gap to ledger lines of the other voice")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between the stem or flag of one voice and the ledger lines of the other voice when offset voices lie outside the staff.")
            max: 1
            step: 0.01
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Ambitus")

        EoNumber {
            sid: "ambitusLineOffset"
            title: qsTrc("notation/engravingoptions", "Line to notehead distance")
            hint: qsTrc("notation/engravingoptions",
                         "Distance between the centre of each ambitus notehead and the nearest end of the line that joins them.")
            max: 3
        }

        EoNumber {
            sid: "ambitusAccidentalUndercutRatio"
            title: qsTrc("notation/engravingoptions", "Undercut of the lower accidental")
            hint: qsTrc("notation/engravingoptions",
                         "When the two accidentals of an ambitus collide, a lower flat or natural is moved left by this fraction of its width and tucks under the upper accidental; 1 places it fully to the left.")
            max: 1
        }
    }
}
