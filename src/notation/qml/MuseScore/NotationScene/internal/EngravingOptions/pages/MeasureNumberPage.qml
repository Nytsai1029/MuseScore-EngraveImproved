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

    code: "measure-number"
    legacyCodes: ["measure-number"]
    title: qsTrc("EditStyleBase", "Measure numbers")
    group: qsTrc("notation/engravingoptions", "Layout")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Position")

        EoNumber {
            sid: "measureNumberOneLineStaffOffset"
            title: qsTrc("notation/engravingoptions", "Measure numbers on one-line staves: extra distance")
            hint: qsTrc("notation/engravingoptions",
                         "Extra vertical distance of measure numbers and multimeasure-rest ranges from a one-line staff, where the barline extends beyond the staff line.")
            max: 6
            step: 0.1
        }

        EoNumber {
            sid: "measureNumberMinBarlineDistance"
            title: qsTrc("notation/engravingoptions", "Measure number to barline clearance")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum horizontal gap between a left- or right-aligned measure number and a barline that runs through its position.")
            max: 2
        }
    }
}
