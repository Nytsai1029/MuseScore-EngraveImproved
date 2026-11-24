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
        title: qsTrc("notation/engravingoptions", "Figured bass")
        music: "\\clef bass c4 \\figuredBass \"6+\" b,4 \\figuredBass \"6 5\" c4 \\figuredBass \"6 4 2\" g,4 " +
               "\\figuredBass \"b7\""

        EoChoice {
            sid: "figuredBassStyle"
            title: qsTrc("EditStyleBase", "Style")
            hint: qsTrc("notation/engravingoptions",
                        "The form of the digits drawn with a stroke or a cross through them, for a raised interval; " +
                        "plain digits look the same in both styles.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Modern") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Historic") }
            ]
        }

        EoChoice {
            sid: "figuredBassAlignment"
            title: qsTrc("EditStyleBase", "Alignment")
            hint: qsTrc("notation/engravingoptions",
                        "Top: the first figure of a stack stands at the vertical position below and the others hang " +
                        "under it. Bottom: the last figure stands there and the others pile up above it.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Top") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Bottom") }
            ]
        }

        EoNumber {
            sid: "figuredBassFontSize"
            title: qsTrc("notation/engravingoptions", "Font size")
            hint: qsTrc("notation/engravingoptions",
                        "For a staff of the default size; the figures scale with the staff.")
            min: 5
            step: 1.0
            decimals: 1
            unit: "pt"
        }

        EoNumber {
            sid: "figuredBassYOffset"
            title: qsTrc("EditStyleBase", "Vertical position:")
            hint: qsTrc("notation/engravingoptions",
                        "Distance from the top staff line down to the first row of figures; 6 sp puts the figures " +
                        "under a five-line staff.")
            min: -25
            max: 25
            step: 0.5
            unit: qsTrc("global", "sp")
        }

        EoNumber {
            sid: "figuredBassLineHeight"
            title: qsTrc("EditStyleBase", "Line height:")
            hint: qsTrc("notation/engravingoptions",
                        "Distance from one figure of a stack to the next, as a percentage of the line height of the " +
                        "font.")
            min: 1
            max: 500
            step: 10
            decimals: 0
            percent: true
        }

        EoNumber {
            sid: "figuredBassMinDistance"
            music: "\\clef bass f,,4 \\figuredBass \"6\" g,,4 \\figuredBass \"5\" a,,4 \\figuredBass \"6\" g,,4"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between figured bass and the notes or other signs which lie between it " +
                        "and the staff.")
            min: -100
            step: 0.5
            decimals: 1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Continuation lines")
        music: "\\clef bass c4 \\figuredBass \"6_\" d4 e4 \\figuredBass \"5\" f4"

        EoNumber {
            sid: "figuredBassContLineThickness"
            title: qsTrc("notation/engravingoptions", "Thickness")
            min: 0.01
            max: 0.5
            step: 0.005
            decimals: 5
        }

        EoNumber {
            sid: "figuredBassContLineLeftPadding"
            title: qsTrc("notation/engravingoptions", "Gap between a figure and its line")
            max: 2
            step: 0.0125
            decimals: 4
        }

        EoNumber {
            sid: "figuredBassContLineOverlap"
            music: "\\clef bass c4 \\figuredBass \"6__\" d4 \\figuredBass \"_\" e4 f4 \\figuredBass \"5\""
            title: qsTrc("notation/engravingoptions", "Overlap with the next line")
            hint: qsTrc("notation/engravingoptions",
                        "Where a continuation line carries on into the line of the next figure, it runs this far " +
                        "into it, so that the two join without a gap.")
            max: 1
            step: 0.025
            decimals: 3
        }

        EoNumber {
            sid: "figuredBassContLineHeight"
            title: qsTrc("notation/engravingoptions", "Height within the row of figures")
            hint: qsTrc("notation/engravingoptions",
                        "Vertical position of the line in its row, as a fraction of the row height measured from the " +
                        "top: 0.5 is the middle of the row, 1 its bottom.")
            max: 1.5
            step: 0.025
            decimals: 3
        }

        EoNumber {
            sid: "figuredBassContLineExtension"
            title: qsTrc("notation/engravingoptions", "Length past the start of the last note")
            hint: qsTrc("notation/engravingoptions",
                        "How far a continuation line runs on past the left edge of the last note it covers.")
            max: 5
            step: 0.1
        }
    }
}
