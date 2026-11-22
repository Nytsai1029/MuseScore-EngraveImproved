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

    code: "text-line"
    legacyCodes: ["text-line", "system-text-line"]
    title: qsTrc("EditStyleBase", "Text lines")
    group: qsTrc("notation/engravingoptions", "Lines")

    EoSection {
        title: qsTrc("EditStyleBase", "Text lines")

        EoChoice {
            sid: "textLinePlacement"
            title: qsTrc("EditStyleBase", "Position:")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Above") },
                { value: 1, title: qsTrc("notation/editstyle", "Below") }
            ]
        }

        EoOffset {
            sid: "textLinePosAbove"
            title: qsTrc("EditStyleBase", "Offset above:")
        }

        EoOffset {
            sid: "textLinePosBelow"
            title: qsTrc("EditStyleBase", "Offset below:")
        }

        EoNumber {
            sid: "textLineLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            step: 0.1
        }

        EoChoice {
            sid: "textLineLineStyle"
            title: qsTrc("EditStyleBase", "Line style:")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Continuous") },
                { value: 1, title: qsTrc("notation/editstyle", "Dashed") },
                { value: 2, title: qsTrc("notation/editstyle", "Dotted") }
            ]
        }

        EoNumber {
            sid: "textLineDashLineLen"
            title: qsTrc("EditStyleBase", "Dash size:")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("textLineLineStyle") === 1
        }

        EoNumber {
            sid: "textLineDashGapLen"
            title: qsTrc("EditStyleBase", "Gap size:")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("textLineLineStyle") === 1
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "System text lines")

        EoChoice {
            sid: "systemTextLinePlacement"
            title: qsTrc("EditStyleBase", "Position:")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Above") },
                { value: 1, title: qsTrc("notation/editstyle", "Below") }
            ]
        }

        EoOffset {
            sid: "systemTextLinePosAbove"
            title: qsTrc("EditStyleBase", "Offset above:")
        }

        EoOffset {
            sid: "systemTextLinePosBelow"
            title: qsTrc("EditStyleBase", "Offset below:")
        }

        EoNumber {
            sid: "systemTextLineLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            step: 0.1
        }

        EoChoice {
            sid: "systemTextLineLineStyle"
            title: qsTrc("EditStyleBase", "Line style:")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Continuous") },
                { value: 1, title: qsTrc("notation/editstyle", "Dashed") },
                { value: 2, title: qsTrc("notation/editstyle", "Dotted") }
            ]
        }

        EoNumber {
            sid: "systemTextLineDashLineLen"
            title: qsTrc("EditStyleBase", "Dash size:")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("systemTextLineLineStyle") === 1
        }

        EoNumber {
            sid: "systemTextLineDashGapLen"
            title: qsTrc("EditStyleBase", "Gap size:")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("systemTextLineLineStyle") === 1
        }
    }
}
