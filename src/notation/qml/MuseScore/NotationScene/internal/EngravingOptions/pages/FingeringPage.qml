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

    code: "fingering"
    title: qsTrc("notation/engravingoptions", "Fingering")
    group: qsTrc("notation/engravingoptions", "Notations")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Fingering")

        EoNumber {
            sid: "fingeringVoicesXOffset"
            title: qsTrc("notation/engravingoptions", "Fingering in voices: horizontal shift away from the stem")
            hint: qsTrc("notation/engravingoptions",
                         "In multi-voice passages, a fingering on a single unbeamed note is shifted sideways by this amount so that it clears the stem.")
            max: 3
        }

        EoNumber {
            sid: "fingeringVoicesYOffset"
            title: qsTrc("notation/engravingoptions", "Fingering in voices: vertical distance from the note")
            hint: qsTrc("notation/engravingoptions",
                         "In multi-voice passages, a fingering on a single unbeamed note is placed this far above or below the note instead of beyond the stem.")
            max: 5
            step: 0.1
        }

        EoNumber {
            sid: "fingeringSkylineExtraHeightRatio"
            title: qsTrc("notation/engravingoptions", "Fingering: extra clearance when avoiding collisions")
            hint: qsTrc("notation/engravingoptions",
                         "Extra distance, as a fraction of the fingering’s own height, added when a fingering has to move away from other objects above or below the chord.")
            max: 2
        }
    }
}
