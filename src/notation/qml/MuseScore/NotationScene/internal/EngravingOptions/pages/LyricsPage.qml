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
    group: qsTrc("notation/engravingoptions", "Signs")
    music: "c''4_\"Glo-\" d''4_\"ri-\" e''2_\"a\""

    EoSection {
        title: qsTrc("notation/engravingoptions", "Lyrics")

        EoChoice {
            sid: "lyricsPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "The side of the staff on which lyrics are placed unless a syllable is set otherwise.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoNumber {
            sid: "lyricsLineHeight"
            music: "c''4_\"1. Glo-\"_\"2. Ky-\" d''4_\"ri-\"_\"ri-\" e''2_\"a\"_\"e\""
            title: qsTrc("EditStyleBase", "Line height (% of text size):")
            hint: qsTrc("notation/engravingoptions",
                        "Distance from the baseline of one verse to that of the next; 100 % is the natural line " +
                        "spacing of the lyrics font.")
            min: 50
            max: 1000
            step: 1.0
            percent: true
        }

        EoNumber {
            sid: "lyricsMinDistance"
            music: "c''8_\"Glo-\" d''8_\"ri-\" e''8_\"a\" f''8_\"in\" g''2_\"excelsis\""
            title: qsTrc("notation/engravingoptions", "Minimum space between syllables")
            hint: qsTrc("notation/engravingoptions",
                        "Least horizontal gap between two neighbouring syllables of a verse; the notes are moved " +
                        "apart to keep it.")
            min: -100
        }

        EoToggle {
            sid: "lyricsAlignVerseNumber"
            music: "c''4_\"1. Glo-\"_\"2. Ky-\" d''4_\"ri-\"_\"ri-\" e''2_\"a\"_\"e\""
            title: qsTrc("notation/engravingoptions", "Align verse numbers")
            hint: qsTrc("notation/engravingoptions",
                        "A verse number or punctuation in front of a syllable hangs to the left and the word itself " +
                        "is aligned with the note, so that the first words of all verses line up; off centres number " +
                        "and word together.")
        }

        EoToggle {
            sid: "lyricsAvoidBarlines"
            music: "\\staves 2 \\spanBarlines c''2 c''2_\"Hallelujah\" | c''1_\"Amen\""
            title: qsTrc("EditStyleBase", "Avoid barlines")
            hint: qsTrc("notation/engravingoptions",
                        "Where barlines run between the staves, a syllable is kept clear of the barline: the notes " +
                        "are moved apart rather than letting text and barline overprint.")
        }

        EoNumber {
            sid: "lyricsAvoidBarlinesPadding"
            music: "\\staves 2 \\spanBarlines c''2 c''2_\"Hallelujah\" | c''1_\"Amen\""
            title: qsTrc("notation/engravingoptions", "Distance from barlines")
            hint: qsTrc("notation/engravingoptions",
                        "With ‘Avoid barlines’ on: the gap kept between a syllable and the barline, as a multiple of " +
                        "the height of a small letter x of the lyrics font.")
            max: 3
            step: 0.05
        }

        EoGrid {
            rowTitles: [
                qsTrc("notation/engravingoptions", "Position")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Above the staff"),
                qsTrc("notation/engravingoptions", "Below the staff")
            ]

            EoNumber {
                sid: "lyricsPosAbove"
                musicStyle: ({ lyricsPlacement: 0 })
                hint: qsTrc("notation/engravingoptions",
                            "Height of the baseline of lyrics above the staff, measured from the top staff line " +
                            "(negative is up); notes in the way push the whole line of lyrics further out.")
                min: -99.99
                step: 0.1
                unit: qsTrc("global", "sp")
                axis: "y"
            }

            EoNumber {
                sid: "lyricsPosBelow"
                hint: qsTrc("notation/engravingoptions",
                            "Depth of the baseline of the first verse below the staff, measured from the bottom " +
                            "staff line (positive is down); notes in the way push the whole line of lyrics further " +
                            "out.")
                min: -99.99
                step: 0.1
                unit: qsTrc("global", "sp")
                axis: "y"
            }
        }

        EoNumber {
            sid: "lyricsMinTopDistance"
            music: "a4_\"Glo-\" g4_\"ri-\" f2_\"a\""
            title: qsTrc("notation/engravingoptions", "Minimum distance from the notes of their own staff")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between the lyrics and the notes, stems, slurs and other signs of the " +
                        "staff they belong to; applies above and below alike.")
            min: -100
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "lyricsMinBottomDistance"
            music: "\\staves 2 c''4_\"1. Glo-\"_\"2. Ky-\" d''4_\"ri-\"_\"ri-\" e''2_\"a\"_\"e\""
            title: qsTrc("notation/engravingoptions", "Minimum distance from the neighbouring staff")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between the lyrics and the staff on their far side: the next staff down " +
                        "for lyrics below, the next staff up for lyrics above.")
            min: -100
            step: 0.5
            decimals: 1
        }

        EoLink {
            title: qsTrc("EditStyleBase", "Edit lyrics text style")
            target: "lyrics-odd-lines"
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Dashes")
        music: "c''2_\"Glo-\" d''2_\"ri-\" | e''1_\"a\""

        EoNumber {
            sid: "lyricsDashMinLength"
            music: "c''8_\"Glo-\" d''8_\"ri-\" e''8_\"a\" f''8 g''2"
            title: qsTrc("notation/engravingoptions", "Minimum dash length")
            hint: qsTrc("notation/engravingoptions",
                        "A dash is never drawn shorter than this; where two syllables stand closer, the dash is left " +
                        "out, or the notes are moved apart if ‘Omit dash when space is limited’ is off.")
            max: 9.99
        }

        EoNumber {
            sid: "lyricsDashMaxLength"
            title: qsTrc("notation/engravingoptions", "Maximum dash length")
            hint: qsTrc("notation/engravingoptions",
                        "The length of a dash where there is room for it; in a narrower gap the dash is as long as " +
                        "the gap allows.")
            min: 0.05
            max: 9.99
        }

        EoNumber {
            sid: "lyricsDashLineThickness"
            title: qsTrc("notation/engravingoptions", "Dash thickness")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "lyricsDashYposRatio"
            title: qsTrc("notation/engravingoptions", "Vertical position (% of text x-height)")
            hint: qsTrc("notation/engravingoptions",
                        "Height of the dash above the baseline of the text: 50 % puts it at half the height of a " +
                        "small letter x.")
            max: 2
            step: 5
            decimals: 0
            percent: true
        }

        EoNumber {
            sid: "lyricsDashPad"
            title: qsTrc("notation/engravingoptions", "Minimum gap between dash and syllable")
            hint: qsTrc("notation/engravingoptions",
                        "Least white space between a dash and the syllables on either side of it.")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "lyricsDashMaxDistance"
            music: "c''1_\"Glo-\" | d''1 | e''1_\"ri\""
            title: qsTrc("notation/engravingoptions", "Maximum distance between dashes")
            hint: qsTrc("notation/engravingoptions",
                        "Across a wide gap between two syllables the dash is repeated, so that two dashes are never " +
                        "further apart than this.")
            min: 1
            step: 1
            decimals: 0
        }

        EoToggle {
            sid: "lyricsLimitDashCount"
            musicStyle: ({ lyricsDashMaxDistance: 2 })
            title: qsTrc("notation/engravingoptions", "Limit the number of dashes")
            hint: qsTrc("notation/engravingoptions",
                        "Off, a long gap gets as many dashes as ‘Maximum distance between dashes’ calls for.")
        }

        EoNumber {
            sid: "lyricsMaxDashCount"
            musicStyle: ({ lyricsLimitDashCount: true, lyricsDashMaxDistance: 2 })
            title: qsTrc("notation/engravingoptions", "Maximum number of dashes")
            hint: qsTrc("notation/engravingoptions",
                        "Between two syllables no more dashes than this are drawn, however wide the gap. Only with " +
                        "‘Limit the number of dashes’ on.")
            min: 1
            max: 99
            step: 1
            decimals: 0
            enabled: Boolean(page.value("lyricsLimitDashCount"))
        }

        EoToggle {
            sid: "lyricsDashForce"
            music: "c''8_\"Glo-\" d''8_\"ri-\" e''8_\"a\" f''8 g''2"
            title: qsTrc("notation/engravingoptions", "Omit dash when space is limited")
            hint: qsTrc("notation/engravingoptions",
                        "When two syllables of a word stand too close for a dash of the minimum length, the dash is " +
                        "left out; off moves the notes apart so that every divided word shows its dash.")
            inverted: true
        }

        EoToggle {
            sid: "lyricsCenterDashedSyllables"
            music: "c''4_\"1. Glo-\"_\"2. Ky-\" d''4_\"ri-\"_\"ri-\" e''2_\"a\"_\"e\""
            title: qsTrc("notation/engravingoptions", "Never left-align syllables followed by dashes")
            hint: qsTrc("notation/engravingoptions",
                        "A syllable which is held over several notes and continues with a dash stays centred under " +
                        "its first note; off aligns it like a syllable with an extension line.")
        }

        EoToggle {
            sid: "lyricsShowDashIfSyllableOnFirstNote"
            music: "c''1_\"Glo-\" \\break d''1_\"ri\""
            systemWidth: 16
            title: qsTrc("notation/engravingoptions", "Restate dash before syllable on first note of the system")
            hint: qsTrc("notation/engravingoptions",
                        "When a word is divided at a system break and its next syllable falls on the first note of " +
                        "the new system, a dash is drawn in front of that syllable as well.")
        }

        EoChoice {
            sid: "lyricsDashPosAtStartOfSystem"
            music: "c''1_\"Glo-\" \\break d''2 e''2_\"ri\""
            systemWidth: 16
            title: qsTrc("notation/engravingoptions", "Dash at the start of a system")
            hint: qsTrc("notation/engravingoptions",
                        "Where the dash stands when a word continues from the system before: centred between the " +
                        "start of the music and the next syllable, right after the clef and key signature, or under " +
                        "the first note.")
            presentation: "dropdown"
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Standard") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Inside the header") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Under the first note") }
            ]
        }

        EoToggle {
            sid: "lyricsDashFirstAndLastGapAreHalf"
            musicStyle: ({ lyricsDashMaxDistance: 2 })
            title: qsTrc("notation/engravingoptions", "Halve the gap before the first and after the last dash")
            hint: qsTrc("notation/engravingoptions",
                        "When several dashes fill a gap, the space between a syllable and the dash nearest to it is " +
                        "half the space between two dashes; off makes all the spaces equal.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Extension lines")
        music: "c''4_\"Ah__\" d''4 e''4 f''4 | g''1_\"men\""

        EoNumber {
            sid: "lyricsMelismaMinLength"
            music: "c''16_\"Ah__\" d''16 e''8_\"men\" f''4 g''2"
            musicStyle: ({ lyricsMelismaForce: true })
            title: qsTrc("notation/engravingoptions", "Minimum length")
            hint: qsTrc("notation/engravingoptions",
                        "An extension line which would be shorter than this is left out, or drawn at this length if " +
                        "‘Omit extension line when space is limited’ is off.")
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
            title: qsTrc("notation/engravingoptions", "Gap between syllable and line")
            hint: qsTrc("notation/engravingoptions",
                        "White space between the end of a syllable and the start of its extension line.")
            max: 1
            step: 0.01
        }

        EoToggle {
            sid: "lyricsMelismaForce"
            music: "c''16_\"Ah__\" d''16 e''8_\"men\" f''4 g''2"
            title: qsTrc("EditStyleBase", "Omit extension line when space is limited")
            hint: qsTrc("notation/engravingoptions",
                        "When the notes of a melisma stand too close for a line of the minimum length, the line is " +
                        "left out; off moves the notes apart so that every melisma shows its line.")
            inverted: true
        }

        EoNumber {
            sid: "lyricsMelismaEndNoteheadWidthRatio"
            title: qsTrc("notation/engravingoptions", "End of the line on the last note")
            hint: qsTrc("notation/engravingoptions",
                        "Where the line ends on the last note of the melisma, as a fraction of the notehead width " +
                        "from its left edge: 1 is the right edge of the notehead, 0.5 its middle.")
            max: 2
            step: 0.05
        }

        EoNumber {
            sid: "lyricsMelismaYOffset"
            title: qsTrc("notation/engravingoptions", "Vertical position of the line")
            hint: qsTrc("notation/engravingoptions",
                        "Raises (negative) or lowers (positive) extension lines from their normal place, where the " +
                        "lower edge of the line lies on the baseline of the text. Dashes are not affected.")
            min: -2
            max: 2
            step: 0.05
        }
    }
}
