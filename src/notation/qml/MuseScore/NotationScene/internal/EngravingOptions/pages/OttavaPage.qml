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

    code: "ottava"
    legacyCodes: ["ottava"]
    title: qsTrc("EditStyleBase", "Ottavas")
    group: qsTrc("notation/engravingoptions", "Lines")

    EoSection {
        title: qsTrc("EditStyleBase", "Ottavas")

        EoToggle {
            sid: "ottavaNumbersOnly"
            title: qsTrc("EditStyleBase", "Numbers only")
        }

        EoOffset {
            sid: "ottavaPosAbove"
            title: qsTrc("EditStyleBase", "Offset above:")
        }

        EoNumber {
            sid: "ottavaHookAbove"
            title: qsTrc("EditStyleBase", "Hook height above:")
            min: -99
            step: 1.0
        }

        EoOffset {
            sid: "ottavaPosBelow"
            title: qsTrc("EditStyleBase", "Offset below:")
        }

        EoNumber {
            sid: "ottavaHookBelow"
            title: qsTrc("EditStyleBase", "Hook height below:")
            min: -99
            step: 1.0
        }

        EoNumber {
            sid: "ottavaLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            step: 0.01
        }

        EoChoice {
            sid: "ottavaLineStyle"
            title: qsTrc("EditStyleBase", "Line style:")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Continuous") },
                { value: 1, title: qsTrc("notation/editstyle", "Dashed") },
                { value: 2, title: qsTrc("notation/editstyle", "Dotted") }
            ]
        }

        EoNumber {
            sid: "ottavaDashLineLen"
            title: qsTrc("EditStyleBase", "Dash size:")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("ottavaLineStyle") === 1
        }

        EoNumber {
            sid: "ottavaDashGapLen"
            title: qsTrc("EditStyleBase", "Gap size:")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("ottavaLineStyle") === 1
        }

        EoToggle {
            sid: "ottavaExtendToEndOfDuration"
            title: qsTrc("notation/engravingoptions", "Extend line to end of last note’s duration")
            hint: qsTrc("notation/engravingoptions",
                         "Off: the ottava line ends just past the last note it covers. On: it continues to just before the next note or barline.")
        }

        EoNumber {
            sid: "ottavaEndOvershoot"
            title: qsTrc("notation/engravingoptions", "Line extension past last note")
            hint: qsTrc("notation/engravingoptions",
                         "How far the ottava line continues past the right edge of the last note it covers (never beyond the next note).")
            max: 5
        }
    }
}
