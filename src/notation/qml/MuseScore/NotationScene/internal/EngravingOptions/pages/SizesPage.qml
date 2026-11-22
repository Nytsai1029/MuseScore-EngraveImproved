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

    code: "sizes"
    legacyCodes: ["sizes"]
    title: qsTrc("EditStyleBase", "Sizes")
    group: qsTrc("notation/engravingoptions", "Layout")

    EoSection {
        title: qsTrc("EditStyleBase", "Cue sizes")

        EoNumber {
            sid: "smallStaffMag"
            title: qsTrc("EditStyleBase", "Small staff size:")
            min: 10
            max: 200
            step: 10
            decimals: 0
            percent: true
        }

        EoNumber {
            sid: "smallClefMag"
            title: qsTrc("EditStyleBase", "Small clef size:")
            min: 10
            max: 200
            step: 10
            decimals: 0
            percent: true
        }

        EoNumber {
            sid: "graceNoteMag"
            title: qsTrc("EditStyleBase", "Grace note size:")
            min: 10
            max: 200
            step: 10
            decimals: 0
            percent: true
        }

        EoNumber {
            sid: "smallNoteMag"
            title: qsTrc("EditStyleBase", "Small note size:")
            min: 10
            max: 200
            step: 10
            decimals: 0
            percent: true
        }

        EoToggle {
            sid: "scaleRythmicSpacingForSmallNotes"
            title: qsTrc("EditStyleBase", "Reduce rhythmic spacing for small notes")
        }
    }
}
