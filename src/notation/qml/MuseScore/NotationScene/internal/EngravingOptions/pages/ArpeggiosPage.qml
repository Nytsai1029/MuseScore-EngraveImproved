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
        title: qsTrc("EditStyleBase", "Arpeggios")

        EoNumber {
            sid: "arpeggioNoteDistance"
            title: qsTrc("EditStyleBase", "Distance to note:")
            max: 99999
            step: 0.2
        }

        EoNumber {
            sid: "arpeggioLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            max: 99999
            step: 0.01
        }

        EoNumber {
            sid: "arpeggioHookLen"
            title: qsTrc("EditStyleBase", "Hook length:")
            max: 99999
            step: 0.1
        }

        EoNumber {
            sid: "arpeggioAccidentalDistance"
            title: qsTrc("EditStyleBase", "Distance to accidental:")
            step: 0.1
        }

        EoNumber {
            sid: "arpeggioAccidentalDistanceMin"
            title: qsTrc("EditStyleBase", "Min. distance to accidental:")
            step: 0.1
        }

        EoToggle {
            sid: "arpeggioHiddenInStdIfTab"
            title: qsTrc("EditStyleBase", "Do not show arpeggios in standard notation when displayed in tablature")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Details")

        EoNumber {
            sid: "arpeggioTopExtensionInSpace"
            title: qsTrc("notation/engravingoptions", "Arpeggio: extension above a top note in a space")
            hint: qsTrc("notation/engravingoptions",
                         "How far a wavy arpeggio extends above the top notehead when that note sits in a staff space and the bottom note sits on a line.")
            max: 1
        }

        EoNumber {
            sid: "arpeggioTopExtensionBothInSpaces"
            title: qsTrc("notation/engravingoptions", "Arpeggio: extension above when both outer notes are in spaces")
            hint: qsTrc("notation/engravingoptions",
                         "How far a wavy arpeggio extends above the top notehead when both the top and the bottom note sit in staff spaces.")
            max: 1
        }

        EoNumber {
            sid: "arpeggioStraightExtension"
            title: qsTrc("notation/engravingoptions", "Straight arpeggio: extension beyond the outer notes")
            hint: qsTrc("notation/engravingoptions",
                         "How far straight arpeggio lines with an arrow extend beyond the top and the bottom notehead.")
            max: 2
        }
    }
}
