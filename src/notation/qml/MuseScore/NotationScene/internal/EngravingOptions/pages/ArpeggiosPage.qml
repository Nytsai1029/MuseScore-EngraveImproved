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

    code: "arpeggios"
    legacyCodes: ["arpeggios"]
    title: qsTrc("EditStyleBase", "Arpeggios")
    group: qsTrc("notation/engravingoptions", "Notes and rests")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Arpeggios")
        music: "c''4 <f' a' c''>4 \\arpeggio"

        EoNumber {
            sid: "arpeggioNoteDistance"
            music: "c''4 <f' a' c''>4 \\arpeggio"
            title: qsTrc("EditStyleBase", "Distance to note:")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between the arpeggio and the noteheads to its right, for a chord without accidentals.")
            max: 99999
            step: 0.2
        }

        EoNumber {
            sid: "arpeggioAccidentalDistance"
            music: "<c' e' gis'>4 \\arpeggio r2."
            title: qsTrc("notation/engravingoptions", "Distance to accidental")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between the arpeggio and the accidentals of its chord. Under an arrowhead or inside a " +
                        "bracket the accidentals may slip in beside the line, so the gap is measured from the line " +
                        "itself.")
            step: 0.1
        }

        EoNumber {
            sid: "arpeggioAccidentalDistanceMin"
            music: "<cis' eis' gis'>2 \\arpeggio"
            title: qsTrc("notation/engravingoptions", "Distance from arrowhead or hook to accidental")
            hint: qsTrc("notation/engravingoptions",
                        "Used instead of ‘Distance to accidental’ when the outermost accidental is level with the " +
                        "arrowhead of an arpeggio, or with a hook of a bracket arpeggio, so that it cannot slip in " +
                        "beside the line.")
            step: 0.1
        }

        EoNumber {
            sid: "arpeggioLedgerLineDistance"
            music: "<a c' e' g'>4 \\arpeggio"
            title: qsTrc("notation/engravingoptions", "Distance to ledger line")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between the arpeggio and the left ends of the ledger lines of its chord.")
            max: 2
        }

        EoNumber {
            sid: "arpeggioLineWidth"
            music: "c''4 <f' a' c''>4 \\arpeggio"
            title: qsTrc("EditStyleBase", "Line thickness:")
            hint: qsTrc("notation/engravingoptions",
                        "For bracket arpeggios and straight arpeggios with an arrow; the wavy line is a shape of the " +
                        "music font.")
            max: 99999
            step: 0.01
        }

        EoNumber {
            sid: "arpeggioHookLen"
            music: "c''4 <f' a' c''>4 \\arpeggioBracket"
            title: qsTrc("EditStyleBase", "Hook length:")
            hint: qsTrc("notation/engravingoptions",
                        "Length of the two short horizontal strokes at the ends of a bracket arpeggio.")
            max: 99999
            step: 0.1
        }

        EoToggle {
            sid: "arpeggioHiddenInStdIfTab"
            music: "\\tab <c' e' g'>4 \\arpeggio r2."
            title: qsTrc("EditStyleBase", "Do not show arpeggios in standard notation when displayed in tablature")
            hint: qsTrc("notation/engravingoptions",
                        "On a staff which is linked to a visible tablature staff the arpeggio is printed in the " +
                        "tablature only.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Extent")
        hint: qsTrc("notation/engravingoptions", "How far an arpeggio reaches beyond the outer notes of its chord.")

        EoNumber {
            sid: "arpeggioTopExtensionInSpace"
            music: "<b' e''>2 \\arpeggioUp"
            title: qsTrc("notation/engravingoptions", "Wavy arpeggio: rise above a top note in a space")
            hint: qsTrc("notation/engravingoptions",
                        "How far the wavy line rises above the top notehead when that note is in a space inside the " +
                        "staff. Over a top note on a line or outside the staff it ends level with the notehead.")
            max: 1
        }

        EoNumber {
            sid: "arpeggioTopExtensionBothInSpaces"
            music: "<f' a' c''>2 \\arpeggio"
            title: qsTrc("notation/engravingoptions", "Wavy arpeggio: rise when both outer notes are in spaces")
            hint: qsTrc("notation/engravingoptions",
                        "Used instead of the value above when the bottom note of the chord is in a space inside the " +
                        "staff as well.")
            max: 1
        }

        EoNumber {
            sid: "arpeggioStraightExtension"
            music: "<f' a' c''>2 \\arpeggioUpStraight"
            title: qsTrc("notation/engravingoptions", "Straight arpeggio: reach beyond the outer notes")
            hint: qsTrc("notation/engravingoptions",
                        "How far a straight arpeggio with an arrow reaches above the top notehead and below the " +
                        "bottom notehead.")
            max: 2
        }
    }
}
