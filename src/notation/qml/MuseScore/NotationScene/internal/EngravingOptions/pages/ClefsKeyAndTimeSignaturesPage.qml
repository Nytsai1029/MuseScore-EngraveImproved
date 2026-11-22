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

    code: "clefs-key-and-time-signatures"
    legacyCodes: ["clefs-key-and-time-signatures"]
    title: qsTrc("EditStyleBase", "Clefs, key & time signatures")
    group: qsTrc("notation/engravingoptions", "Notations")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Key signatures")

        EoNumber {
            sid: "keysigMixedAccidentalDistanceScale"
            title: qsTrc("notation/engravingoptions", "Distance scale between different accidentals")
            hint: qsTrc("notation/engravingoptions",
                         "Multiplies the accidental distance where two different accidentals meet in a key signature, for example between the cancelling naturals and the new sharps or flats.")
            max: 5
            step: 0.1
        }
    }
}
