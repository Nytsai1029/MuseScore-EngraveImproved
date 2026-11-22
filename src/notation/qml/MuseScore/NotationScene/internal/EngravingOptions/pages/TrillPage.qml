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

    code: "trill"
    legacyCodes: ["trill", "vibrato"]
    title: qsTrc("notation/engravingoptions", "Trill and vibrato lines")
    group: qsTrc("notation/engravingoptions", "Lines")

    EoSection {
        title: qsTrc("EditStyleBase", "Trill lines")

        EoChoice {
            sid: "trillPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Above") },
                { value: 1, title: qsTrc("notation/editstyle", "Below") }
            ]
        }

        EoOffset {
            sid: "trillPosAbove"
            title: qsTrc("EditStyleBase", "Offset above:")
        }

        EoOffset {
            sid: "trillPosBelow"
            title: qsTrc("EditStyleBase", "Offset below:")
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Vibrato lines")

        EoChoice {
            sid: "vibratoPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Above") },
                { value: 1, title: qsTrc("notation/editstyle", "Below") }
            ]
        }

        EoOffset {
            sid: "vibratoPosAbove"
            title: qsTrc("EditStyleBase", "Offset above:")
        }

        EoOffset {
            sid: "vibratoPosBelow"
            title: qsTrc("EditStyleBase", "Offset below:")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Details")

        EoNumber {
            sid: "trillLineEndInset"
            title: qsTrc("notation/engravingoptions", "Trill / vibrato line end: distance before end position")
            hint: qsTrc("notation/engravingoptions",
                         "How far before the next note, grace notes, clef or barline the wavy line of a trill or vibrato stops.")
            max: 5
        }

        EoNumber {
            sid: "trillAccidentalMargin"
            title: qsTrc("notation/engravingoptions", "Trill line accidental: vertical gap")
            hint: qsTrc("notation/engravingoptions",
                         "Vertical gap between the first symbol of a trill line and the accidental placed above (or below) it.")
            max: 2
        }
    }
}
