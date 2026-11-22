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

    code: "chord-symbols"
    legacyCodes: ["chord-symbols"]
    title: qsTrc("EditStyleBase", "Chord symbols")
    group: qsTrc("notation/engravingoptions", "Text")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Details")

        EoNumber {
            sid: "chordBassNoteStackedOffset"
            title: qsTrc("notation/engravingoptions", "Bass note: extra offset after stacked modifiers")
            hint: qsTrc("notation/engravingoptions",
                         "Extra horizontal shift of the slash and bass note, in capital heights, when the chord symbol has vertically stacked modifiers or a 6/9.")
            min: -0.5
            max: 1
            step: 0.01
        }
    }
}
