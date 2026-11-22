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

    code: "pedal"
    legacyCodes: ["pedal"]
    title: qsTrc("EditStyleBase", "Pedal lines")
    group: qsTrc("notation/engravingoptions", "Lines")

    EoSection {
        title: qsTrc("EditStyleBase", "Pedal lines")

        EoChoice {
            sid: "pedalPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Above") },
                { value: 1, title: qsTrc("notation/editstyle", "Below") }
            ]
        }

        EoOffset {
            sid: "pedalPosAbove"
            title: qsTrc("EditStyleBase", "Offset above:")
        }

        EoOffset {
            sid: "pedalPosBelow"
            title: qsTrc("EditStyleBase", "Offset below:")
        }

        EoNumber {
            sid: "pedalLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            step: 0.01
        }

        EoChoice {
            sid: "pedalLineStyle"
            title: qsTrc("EditStyleBase", "Line style:")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Continuous") },
                { value: 1, title: qsTrc("notation/editstyle", "Dashed") },
                { value: 2, title: qsTrc("notation/editstyle", "Dotted") }
            ]
        }

        EoNumber {
            sid: "pedalDashLineLen"
            title: qsTrc("EditStyleBase", "Dash size:")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("pedalLineStyle") === 1
        }

        EoNumber {
            sid: "pedalDashGapLen"
            title: qsTrc("EditStyleBase", "Gap size:")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("pedalLineStyle") === 1
        }

        EoNumber {
            sid: "pedalSymbolLeftShift"
            title: qsTrc("notation/engravingoptions", "Ped. symbol: shift to the left")
            hint: qsTrc("notation/engravingoptions",
                         "How far to the left of its note a pedal line that begins with the Ped. symbol starts.")
            min: -2
            max: 3
        }

        EoNumber {
            sid: "pedalEndInset"
            title: qsTrc("notation/engravingoptions", "Pedal line end: distance before end position")
            hint: qsTrc("notation/engravingoptions",
                         "How far before the note or barline at its end position a pedal line (or its release sign) stops.")
            max: 5
        }

        EoNumber {
            sid: "pedalEndInsetBeforeNextPedal"
            title: qsTrc("notation/engravingoptions", "Pedal line end: distance before a following pedal")
            hint: qsTrc("notation/engravingoptions",
                         "The same distance when the line ends at a note and another pedal line follows on the staff, leaving room for the start of the next one.")
            max: 5
        }
    }
}
