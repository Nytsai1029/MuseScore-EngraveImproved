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

    code: "lyrics"
    legacyCodes: ["lyrics"]
    title: qsTrc("EditStyleBase", "Lyrics")
    group: qsTrc("notation/engravingoptions", "Text")

    EoSection {
        title: qsTrc("EditStyleBase", "Lyrics text")

        EoChoice {
            sid: "lyricsPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Above") },
                { value: 1, title: qsTrc("notation/editstyle", "Below") }
            ]
        }

        EoNumber {
            sid: "lyricsMinTopDistance"
            title: qsTrc("EditStyleBase", "Min. margin to current staff:")
            min: -100
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "lyricsPosAbove"
            title: qsTrc("EditStyleBase", "Offset above:")
            min: -99.99
            step: 0.1
            unit: qsTrc("global", "sp")
            axis: "y"
        }

        EoNumber {
            sid: "lyricsMinBottomDistance"
            title: qsTrc("EditStyleBase", "Min. margin to other staves:")
            min: -100
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "lyricsPosBelow"
            title: qsTrc("EditStyleBase", "Offset below:")
            min: -99.99
            step: 0.1
            unit: qsTrc("global", "sp")
            axis: "y"
        }

        EoNumber {
            sid: "lyricsMinDistance"
            title: qsTrc("EditStyleBase", "Min. space between lyrics:")
            min: -100
        }

        EoNumber {
            sid: "lyricsLineHeight"
            title: qsTrc("EditStyleBase", "Line height (% of text size):")
            min: 50
            max: 1000
            step: 1.0
            percent: true
        }

        EoToggle {
            sid: "lyricsAlignVerseNumber"
            title: qsTrc("EditStyleBase", "Align verse numbers")
        }

        EoToggle {
            sid: "lyricsAvoidBarlines"
            title: qsTrc("EditStyleBase", "Avoid barlines")
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Dashes")

        EoNumber {
            sid: "lyricsDashMinLength"
            title: qsTrc("EditStyleBase", "Min. dash length:")
            max: 9.99
        }

        EoNumber {
            sid: "lyricsDashMaxLength"
            title: qsTrc("EditStyleBase", "Max. dash length:")
            min: 0.05
            max: 9.99
        }

        EoNumber {
            sid: "lyricsDashLineThickness"
            title: qsTrc("EditStyleBase", "Dash thickness:")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "lyricsDashYposRatio"
            title: qsTrc("notation/engravingoptions", "Vertical position (% of text x-height)")
            step: 1.0
        }

        EoNumber {
            sid: "lyricsDashPad"
            title: qsTrc("EditStyleBase", "Min. space between dash and text:")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "lyricsDashMaxDistance"
            title: qsTrc("EditStyleBase", "Max. space between dashes:")
            min: 1
            step: 1
            decimals: 0
        }

        EoToggle {
            sid: "lyricsLimitDashCount"
            title: qsTrc("EditStyleBase", "Limit number of dashes to:")
        }

        EoNumber {
            sid: "lyricsMaxDashCount"
            title: qsTrc("notation/engravingoptions", "Maximum number of dashes")
            min: 1
            max: 99
            step: 1
            decimals: 0
            enabled: Boolean(page.value("lyricsLimitDashCount"))
        }

        EoToggle {
            sid: "lyricsDashForce"
            title: qsTrc("EditStyleBase", "Omit dash when space is limited")
            inverted: true
        }

        EoToggle {
            sid: "lyricsCenterDashedSyllables"
            title: qsTrc("notation/engravingoptions", "Never left-align syllables followed by dashes")
        }

        EoToggle {
            sid: "lyricsShowDashIfSyllableOnFirstNote"
            title: qsTrc("notation/engravingoptions", "Restate dash before syllable on first note of the system")
        }

        EoChoice {
            sid: "lyricsDashPosAtStartOfSystem"
            title: qsTrc("notation/engravingoptions", "Dash at the start of a system")
            presentation: "dropdown"
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Standard") },
                { value: 1, title: qsTrc("notation/editstyle", "Inside the header") },
                { value: 2, title: qsTrc("notation/editstyle", "Under the first note") }
            ]
        }

        EoToggle {
            sid: "lyricsDashFirstAndLastGapAreHalf"
            title: qsTrc("EditStyleBase", "Halve the gap before the first and after the last dash")
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Extension lines")

        EoNumber {
            sid: "lyricsMelismaMinLength"
            title: qsTrc("EditStyleBase", "Min. line length:")
            step: 0.1
        }

        EoNumber {
            sid: "lyricsLineThickness"
            title: qsTrc("EditStyleBase", "Line thickness:")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "lyricsMelismaPad"
            title: qsTrc("EditStyleBase", "Line gap:")
            max: 1
            step: 0.01
        }

        EoToggle {
            sid: "lyricsMelismaForce"
            title: qsTrc("EditStyleBase", "Omit extension line when space is limited")
            inverted: true
        }
    }
}
