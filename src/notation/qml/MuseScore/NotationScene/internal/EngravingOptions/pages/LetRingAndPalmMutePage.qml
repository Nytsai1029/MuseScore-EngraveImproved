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

    code: "let-ring-and-palm-mute"
    title: qsTrc("notation/engravingoptions", "Let ring and palm mute")
    group: qsTrc("notation/engravingoptions", "Lines")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Lines")
        music: "c'4 \\startPalmMute e'4 g'4 c''4 | e''2 \\stopPalmMute c''2"

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "The let ring values, except the position, also shape rasgueado, whammy bar and harmonic " +
                        "mark lines; the palm mute values also shape pick scrape lines.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Position"),
                qsTrc("notation/engravingoptions", "Offset above the staff"),
                qsTrc("notation/engravingoptions", "Offset below the staff"),
                qsTrc("notation/engravingoptions", "Line thickness"),
                qsTrc("notation/engravingoptions", "Line style"),
                qsTrc("notation/engravingoptions", "Dash length"),
                qsTrc("notation/engravingoptions", "Gap length"),
                qsTrc("notation/engravingoptions", "Hook height"),
                qsTrc("notation/engravingoptions", "End hook"),
                qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Let ring"),
                qsTrc("notation/engravingoptions", "Palm mute")
            ]

            EoChoice {
                sid: "letRingPlacement"
                music: "c'4 \\startLetRing e'4 g'4 c''4 | e''2 \\stopLetRing c''2"
                hint: qsTrc("notation/engravingoptions",
                            "Side of the staff on which let ring lines are placed, unless one is flipped by hand.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
                ]
            }

            EoChoice {
                sid: "palmMutePlacement"
                hint: qsTrc("notation/engravingoptions",
                            "Side of the staff on which palm mute lines are placed, unless one is flipped by hand. " +
                            "Pick scrape lines follow it too.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
                ]
            }

            EoGap {
            }

            EoOffset {
                sid: "palmMutePosAbove"
                musicStyle: ({ palmMutePlacement: 0 })
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and height of a palm mute line placed above, measured from the top " +
                            "staff line (negative is up); notes and signs in the way still push it further out.")
            }

            EoGap {
            }

            EoOffset {
                sid: "palmMutePosBelow"
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and depth of a palm mute line placed below, measured from the bottom " +
                            "staff line (positive is down); notes and signs in the way still push it further out.")
            }

            EoNumber {
                sid: "letRingLineWidth"
                music: "c'4 \\startLetRing e'4 g'4 c''4 | e''2 \\stopLetRing c''2"
                hint: qsTrc("notation/engravingoptions", "Thickness of the line of a let ring marking.")
                step: 0.01
            }

            EoNumber {
                sid: "palmMuteLineWidth"
                hint: qsTrc("notation/engravingoptions", "Thickness of the line of a palm mute marking.")
                step: 0.01
            }

            EoChoice {
                sid: "letRingLineStyle"
                music: "c'4 \\startLetRing e'4 g'4 c''4 | e''2 \\stopLetRing c''2"
                hint: qsTrc("notation/engravingoptions", "The line after the text: dashed by default.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Solid") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Dashed") },
                    { value: 2, title: qsTrc("notation/engravingoptions", "Dotted") }
                ]
            }

            EoChoice {
                sid: "palmMuteLineStyle"
                hint: qsTrc("notation/engravingoptions", "The line after the text: dashed by default.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Solid") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Dashed") },
                    { value: 2, title: qsTrc("notation/engravingoptions", "Dotted") }
                ]
            }

            EoNumber {
                sid: "letRingDashLineLen"
                music: "c'4 \\startLetRing e'4 g'4 c''4 | e''2 \\stopLetRing c''2"
                hint: qsTrc("notation/engravingoptions",
                            "Length of each dash as a multiple of the line thickness. Only with the line style " +
                            "‘Dashed’.")
                min: 0.1
                max: 100
                step: 0.1
                enabled: page.value("letRingLineStyle") === 1
            }

            EoNumber {
                sid: "palmMuteDashLineLen"
                hint: qsTrc("notation/engravingoptions",
                            "Length of each dash as a multiple of the line thickness. Only with the line style " +
                            "‘Dashed’.")
                min: 0.1
                max: 100
                step: 0.1
                enabled: page.value("palmMuteLineStyle") === 1
            }

            EoNumber {
                sid: "letRingDashGapLen"
                music: "c'4 \\startLetRing e'4 g'4 c''4 | e''2 \\stopLetRing c''2"
                hint: qsTrc("notation/engravingoptions",
                            "Gap between two dashes as a multiple of the line thickness; it is evened out slightly " +
                            "so that the line ends with a full dash. Only with the line style ‘Dashed’.")
                min: 0.1
                max: 100
                step: 0.1
                enabled: page.value("letRingLineStyle") === 1
            }

            EoNumber {
                sid: "palmMuteDashGapLen"
                hint: qsTrc("notation/engravingoptions",
                            "Gap between two dashes as a multiple of the line thickness; it is evened out slightly " +
                            "so that the line ends with a full dash. Only with the line style ‘Dashed’.")
                min: 0.1
                max: 100
                step: 0.1
                enabled: page.value("palmMuteLineStyle") === 1
            }

            EoNumber {
                sid: "letRingHookHeight"
                music: "c'4 \\startLetRing e'4 g'4 c''4 | e''2 \\stopLetRing c''2"
                hint: qsTrc("notation/engravingoptions",
                            "Length of the stroke which closes the line: a T-shaped end reaches this far to both " +
                            "sides of the line, a hook this far towards the staff.")
            }

            EoNumber {
                sid: "palmMuteHookHeight"
                hint: qsTrc("notation/engravingoptions",
                            "Length of the stroke which closes the line: a T-shaped end reaches this far to both " +
                            "sides of the line, a hook this far towards the staff.")
            }

            EoChoice {
                sid: "letRingEndHookType"
                music: "c'4 \\startLetRing e'4 g'4 c''4 | e''2 \\stopLetRing c''2"
                hint: qsTrc("notation/engravingoptions",
                            "How the line ends: without a hook, with a hook at a right angle or at 45°, or with a " +
                            "short stroke across the line (T-style, the default).")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "No hook") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Hooked 90°") },
                    { value: 2, title: qsTrc("notation/engravingoptions", "Hooked 45°") },
                    { value: 3, title: qsTrc("notation/engravingoptions", "Hooked 90° T-style") }
                ]
            }

            EoChoice {
                sid: "palmMuteEndHookType"
                hint: qsTrc("notation/engravingoptions",
                            "How the line ends: without a hook, with a hook at a right angle or at 45°, or with a " +
                            "short stroke across the line (T-style, the default).")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "No hook") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Hooked 90°") },
                    { value: 2, title: qsTrc("notation/engravingoptions", "Hooked 45°") },
                    { value: 3, title: qsTrc("notation/engravingoptions", "Hooked 90° T-style") }
                ]
            }

            EoNumber {
                sid: "letRingMinDistance"
                music: "c'4 \\startLetRing a4 f4 a4 \\stopLetRing | c'1"
                hint: qsTrc("notation/engravingoptions",
                            "Least vertical gap between a let ring line and the notes or other signs which lie " +
                            "between it and the staff.")
            }

            EoNumber {
                sid: "palmMuteMinDistance"
                music: "c'4 \\startPalmMute a4 f4 a4 \\stopPalmMute | c'1"
                hint: qsTrc("notation/engravingoptions",
                            "Least vertical gap between a palm mute line and the notes or other signs which lie " +
                            "between it and the staff.")
            }
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Texts")

        EoText {
            sid: "letRingText"
            music: "c'4 \\startLetRing e'4 g'4 c''4 | e''2 \\stopLetRing c''2"
            title: qsTrc("notation/engravingoptions", "Let ring text")
            hint: qsTrc("notation/engravingoptions",
                        "The text with which let ring lines begin and which they repeat on each following system; a " +
                        "line whose text was changed by hand keeps its own.")
        }

        EoText {
            sid: "palmMuteText"
            music: "c'4 \\startPalmMute e'4 g'4 c''4 | e''2 \\stopPalmMute c''2"
            title: qsTrc("notation/engravingoptions", "Palm mute text")
            hint: qsTrc("notation/engravingoptions",
                        "The text with which palm mute lines begin and which they repeat on each following system; a " +
                        "line whose text was changed by hand keeps its own.")
        }
    }
}
