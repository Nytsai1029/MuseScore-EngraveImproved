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

    code: "lines"
    title: qsTrc("notation/engravingoptions", "Lines in general")
    group: qsTrc("notation/engravingoptions", "Lines")
    music: "c''4 \\startTextLine \"sul pont.\" \\hooks none 45 d''4 e''4 f''4 | e''4 d''4 \\stopTextLine c''2"

    EoSection {
        title: qsTrc("notation/engravingoptions", "All lines")

        EoNumber {
            sid: "lineEndInset"
            title: qsTrc("notation/engravingoptions", "Gap between a line’s end and the next note")
            hint: qsTrc("notation/engravingoptions",
                        "How far text lines, let-ring, palm-mute, tempo-change and similar lines stop before the " +
                        "note, rest or barline at which they end. Ottavas, pedal lines, trills, hairpins and voltas " +
                        "have their own values.")
            max: 5
        }

        EoNumber {
            sid: "lineAngledHookWidthRatio"
            title: qsTrc("notation/engravingoptions", "Width of an angled hook")
            hint: qsTrc("notation/engravingoptions",
                        "As a multiple of the hook’s height: 1 gives a true 45° hook, smaller values a steeper one.")
            max: 2
        }

        EoNumber {
            sid: "lineDottedGapLen"
            title: qsTrc("notation/engravingoptions", "Spacing of the dots of a dotted line")
            hint: qsTrc("notation/engravingoptions",
                        "Distance from dot to dot as a multiple of the line thickness; 2 leaves one dot’s width of " +
                        "white between neighbouring dots.")
            min: 0.5
            max: 10
            step: 0.01
            musicStyle: ({ textLineLineStyle: 2 })
        }
    }
}
