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

    EoSection {
        title: qsTrc("notation/engravingoptions", "Lines (general)")

        EoNumber {
            sid: "lineEndInset"
            title: qsTrc("notation/engravingoptions", "Line end: distance before end position")
            hint: qsTrc("notation/engravingoptions",
                         "How far before the note, rest or barline at their end position text lines, let-ring and palm-mute lines, gradual tempo changes and similar lines stop.")
            max: 5
        }

        EoNumber {
            sid: "lineAngledHookWidthRatio"
            title: qsTrc("notation/engravingoptions", "Angled hook: width (× hook height)")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal run of an angled (‘45°’) line hook as a multiple of its height; 1 gives a true 45° hook.")
            max: 2
        }

        EoNumber {
            sid: "lineDottedGapLen"
            title: qsTrc("notation/engravingoptions", "Dotted lines: gap between dots (line widths)")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between the dots of dotted lines (text lines, glissandos, hairpins…), in line widths; the distance from dot to dot is this value plus 0.01.")
            min: 0.5
            max: 10
            step: 0.01
        }
    }
}
