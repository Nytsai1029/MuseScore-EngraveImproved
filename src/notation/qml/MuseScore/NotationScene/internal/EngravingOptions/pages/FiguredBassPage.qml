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

    code: "figured-bass"
    legacyCodes: ["figured-bass"]
    title: qsTrc("EditStyleBase", "Figured bass")
    group: qsTrc("notation/engravingoptions", "Text")

    EoSection {
        title: qsTrc("EditStyleBase", "Figured bass")

        EoNumber {
            sid: "figuredBassFontSize"
            title: qsTrc("EditStyleBase", "Size:")
            min: 5
            step: 1.0
            decimals: 1
            unit: "pt"
        }

        EoNumber {
            sid: "figuredBassYOffset"
            title: qsTrc("EditStyleBase", "Vertical position:")
            min: -25
            max: 25
            step: 0.5
            unit: qsTrc("global", "sp")
        }

        EoNumber {
            sid: "figuredBassLineHeight"
            title: qsTrc("EditStyleBase", "Line height:")
            min: 1
            max: 500
            step: 10
            decimals: 0
            percent: true
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Details")

        EoNumber {
            sid: "figuredBassContLineThickness"
            title: qsTrc("notation/engravingoptions", "Figured bass: continuation line thickness")
            hint: qsTrc("notation/engravingoptions", "Thickness of figured bass continuation lines.")
            min: 0.01
            max: 0.5
            step: 0.005
            decimals: 5
        }

        EoNumber {
            sid: "figuredBassContLineLeftPadding"
            title: qsTrc("notation/engravingoptions", "Figured bass: gap between figure and continuation line")
            hint: qsTrc("notation/engravingoptions",
                         "Blank space between a figure and the continuation line that follows it.")
            max: 2
            step: 0.0125
            decimals: 4
        }

        EoNumber {
            sid: "figuredBassContLineOverlap"
            title: qsTrc("notation/engravingoptions", "Figured bass: overlap of joined continuation lines")
            hint: qsTrc("notation/engravingoptions",
                         "How far an extended continuation line runs into the continuation line of the next figure so that the two join without a gap.")
            max: 1
            step: 0.025
            decimals: 3
        }

        EoNumber {
            sid: "figuredBassContLineHeight"
            title: qsTrc("notation/engravingoptions", "Figured bass: continuation line height")
            hint: qsTrc("notation/engravingoptions",
                         "Vertical position of a continuation line within its row of figures, as a fraction of the row height measured from the top.")
            max: 1.5
            step: 0.025
            decimals: 3
        }

        EoNumber {
            sid: "figuredBassContLineExtension"
            title: qsTrc("notation/engravingoptions", "Figured bass: continuation line reach into the last note")
            hint: qsTrc("notation/engravingoptions",
                         "How far a continuation line runs past the left edge of the last note it covers.")
            max: 5
            step: 0.1
        }
    }
}
