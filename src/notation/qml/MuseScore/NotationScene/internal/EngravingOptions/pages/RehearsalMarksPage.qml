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

    code: "rehearsal-marks"
    legacyCodes: ["rehearsal-marks"]
    title: qsTrc("EditStyleBase", "Rehearsal marks")
    group: qsTrc("notation/engravingoptions", "Text")

    EoSection {
        title: qsTrc("EditStyleBase", "Rehearsal marks")

        EoChoice {
            sid: "rehearsalMarkPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Above") },
                { value: 1, title: qsTrc("notation/editstyle", "Below") }
            ]
        }

        EoOffset {
            sid: "rehearsalMarkPosAbove"
            title: qsTrc("EditStyleBase", "Offset above:")
        }

        EoOffset {
            sid: "rehearsalMarkPosBelow"
            title: qsTrc("EditStyleBase", "Offset below:")
        }

        EoNumber {
            sid: "rehearsalMarkMinDistance"
            title: qsTrc("EditStyleBase", "Autoplace min. distance:")
            min: -100
            step: 0.5
            decimals: 1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Jumps, markers, tempo and rehearsal marks")

        EoNumber {
            sid: "tempoRehearsalMarkPadding"
            title: qsTrc("notation/engravingoptions", "Tempo text to rehearsal mark")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between a rehearsal mark and a tempo marking moved to its right to avoid it; scales with the font size.")
            max: 3
        }

        EoNumber {
            sid: "tempoTimeSigAboveStavesDistance"
            title: qsTrc("notation/engravingoptions", "Tempo text to time signature above the staves")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between a time signature placed above the staves and a tempo marking at the start of the measure, which begins to its right.")
            max: 5
            step: 0.1
        }

        EoNumber {
            sid: "rehearsalMarkTimeSigMargin"
            title: qsTrc("notation/engravingoptions", "Rehearsal mark to time signature above the staves")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum gap between a time signature placed above the staves and a rehearsal mark, in x-heights of the rehearsal mark font.")
            max: 5
            step: 0.1
        }
    }
}
