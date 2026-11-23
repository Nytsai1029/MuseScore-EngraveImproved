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
import MuseScore.Atelier 1.0

import ".."

EoPage {
    id: page

    code: "score"
    legacyCodes: ["score"]
    title: qsTrc("EditStyleBase", "Score")
    group: qsTrc("notation/engravingoptions", "Layout")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Music fonts")

        EoChoice {
            sid: "musicalSymbolFont"
            music: "c''4 \\fermata d''8[ e''8] \\clef bass e4"
            title: qsTrc("EditStyleBase", "Musical symbols font:")
            hint: qsTrc("notation/engravingoptions",
                        "The font of the notes, rests, clefs and every other music symbol. With ‘Automatically load " +
                        "style settings based on font’ on, the line thicknesses and distances which the font " +
                        "recommends are loaded with it.")
            presentation: "dropdown"
            choices: page.model.choices("musicFonts")
        }

        EoCustom {
            title: qsTrc("EditStyleBase", "Automatically load style settings based on font")
            hint: qsTrc("EditStyleBase", "MuseScore Studio will change the style to suit the font better")
            fullWidth: false

            component: AtSwitch {
                property var optionsModel: null

                checked: Boolean(optionsModel) && optionsModel.loadsFontDefaults

                onToggled: {
                    optionsModel.loadsFontDefaults = !optionsModel.loadsFontDefaults
                }
            }
        }

        EoChoice {
            sid: "musicalTextFont"
            music: "c''4 \\f d''4 \\p"
            title: qsTrc("EditStyleBase", "Musical text font:")
            hint: qsTrc("notation/engravingoptions",
                        "The font of music symbols which stand inside text: the note of a metronome mark, the " +
                        "accidentals of a chord symbol.")
            presentation: "dropdown"
            choices: page.model.choices("musicTextFonts")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "General")

        EoNumber {
            sid: "staffLineWidth"
            music: "c''4 d''4"
            title: qsTrc("EditStyleBase", "Staff line thickness:")
            step: 0.1
        }

        EoToggle {
            sid: "concertPitch"
            music: "c''4 d''4"
            title: qsTrc("EditStyleBase", "Display in concert pitch")
            hint: qsTrc("notation/engravingoptions",
                        "Transposing instruments are written at sounding pitch instead of in their own " +
                        "transposition.")
        }

        EoToggle {
            sid: "crossMeasureValues"
            music: "c''2 c''2~ | c''4 c''2."
            title: qsTrc("notation/engravingoptions", "Write a note tied across a barline as one note value")
            hint: qsTrc("notation/engravingoptions",
                        "Early-music notation (experimental): a note tied over the barline to a single note is drawn " +
                        "before the barline as one longer note when the two add up to one note value; the tie and " +
                        "the second note are not drawn.")
        }

        EoToggle {
            sid: "systemObjectsBelowBottomStaff"
            music: "c''4 d''4"
            title: qsTrc("notation/engravingoptions", "System markings below the bottom staff")
            hint: qsTrc("notation/engravingoptions",
                        "Where the bottom staff of the score also carries the system markings (tempo, rehearsal " +
                        "marks, voltas…), they stand below that staff instead of above it. The Layout panel sets " +
                        "this itself when the system markings are moved under the last instrument.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Superscript and subscript in text")

        EoNumber {
            sid: "textSubSuperscriptSizeRatio"
            music: "a'4^\"1<sup>st</sup>\" a'4^\"H<sub>2</sub>O\""
            title: qsTrc("notation/engravingoptions", "Size")
            hint: qsTrc("notation/engravingoptions",
                        "The size of superscript and subscript characters as a fraction of the text around them: 0.6 " +
                        "is 60%.")
            min: 0.3
            max: 1
            step: 0.05
        }

        EoNumber {
            sid: "textSuperscriptOffsetRatio"
            music: "a'4^\"1<sup>st</sup>\""
            title: qsTrc("notation/engravingoptions", "Superscript position")
            hint: qsTrc("notation/engravingoptions",
                        "Where the baseline of a superscript lies against the baseline of the text, in x-heights of " +
                        "the text; negative is above, so −0.9 lifts it by nine tenths of the height of a lower-case " +
                        "x.")
            min: -2
            max: 1
            step: 0.05
        }

        EoNumber {
            sid: "textSubscriptOffsetRatio"
            music: "a'4^\"H<sub>2</sub>O\""
            title: qsTrc("notation/engravingoptions", "Subscript position")
            hint: qsTrc("notation/engravingoptions",
                        "Where the baseline of a subscript lies against the baseline of the text, in x-heights of " +
                        "the text; positive is below, so 0.5 lowers it by half the height of a lower-case x.")
            min: -1
            max: 2
            step: 0.05
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "First system")

        EoToggle {
            sid: "enableIndentationOnFirstSystem"
            music: "c''4 d''4"
            musicStyle: ({ enableIndentationOnFirstSystem: true })
            title: qsTrc("EditStyleBase", "Enable indentation on first system")
            hint: qsTrc("notation/engravingoptions",
                        "Applies to the first system of the score and to the first system after every section break.")
        }

        EoNumber {
            sid: "firstSystemIndentationValue"
            music: "c''4 d''4"
            musicStyle: ({ enableIndentationOnFirstSystem: true })
            title: qsTrc("EditStyleBase", "First system indentation:")
            hint: qsTrc("notation/engravingoptions",
                        "The staves of the first system start at least this far from the left page margin; wider " +
                        "instrument names push them further.")
            max: 1000
            step: 0.5
            enabled: Boolean(page.value("enableIndentationOnFirstSystem"))
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Empty staves")

        EoToggle {
            sid: "hideEmptyStaves"
            music: "\\staves 2 \\hideStavesIndividually c''1"
            musicStyle: ({ dontHideStavesInFirstSystem: false })
            title: qsTrc("EditStyleBase", "Automatically hide all empty staves")
            hint: qsTrc("notation/engravingoptions",
                        "A staff which has no notes in a system is left out of that system. The staves of a piano or " +
                        "other multi-staff instrument go only when all of them are empty, unless ‘Only hide staves " +
                        "on a system if the entire instrument is empty’ is off in the instrument’s settings, as in " +
                        "the preview.")
        }

        EoToggle {
            sid: "dontHideStavesInFirstSystem"
            music: "\\staves 2 \\hideStavesIndividually c''1"
            musicStyle: ({ hideEmptyStaves: true })
            title: qsTrc("EditStyleBase", "Don’t hide empty staves in first system")
            hint: qsTrc("notation/engravingoptions",
                        "The first system of the score, and of every section, then shows all the instruments.")
            enabled: Boolean(page.value("hideEmptyStaves"))
        }

        EoToggle {
            sid: "alwaysShowBracketsWhenEmptyStavesAreHidden"
            music: "\\staves 2 \\hideStavesIndividually \\group bracket c''1"
            musicStyle: ({ hideEmptyStaves: true, dontHideStavesInFirstSystem: false })
            title: qsTrc("EditStyleBase", "Show brackets when spanning a single staff")
            hint: qsTrc("notation/engravingoptions",
                        "When hiding staves leaves a bracket or brace around a single staff, it is still drawn; off, " +
                        "it is left out.")
            enabled: Boolean(page.value("hideEmptyStaves"))
        }

        EoToggle {
            sid: "alwaysShowSquareBracketsWhenEmptyStavesAreHidden"
            music: "\\staves 2 \\hideStavesIndividually \\group square c''1"
            musicStyle: ({ hideEmptyStaves: true, dontHideStavesInFirstSystem: false })
            title: qsTrc("EditStyleBase", "Show square brackets when spanning a single staff")
            hint: qsTrc("notation/engravingoptions",
                        "When hiding staves leaves a square bracket around a single staff, it is still drawn; off, " +
                        "it is left out.")
            enabled: Boolean(page.value("hideEmptyStaves"))
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Instrument names")

        EoToggle {
            sid: "hideInstrumentNameIfOneInstrument"
            music: "\\name \"Violin I\" \"Vln. I\" c''1 | c''1"
            musicStyle: ({ firstSystemInstNameVisibility: 0 })
            title: qsTrc("EditStyleBase", "Hide if there is only 1 instrument")
        }

        EoChoice {
            sid: "firstSystemInstNameVisibility"
            music: "\\name \"Violin I\" \"Vln. I\" c''1 | c''1"
            musicStyle: ({
                hideInstrumentNameIfOneInstrument: false,
                firstSystemInstNameVisibility: 0,
                subsSystemInstNameVisibility: 1
            })
            title: qsTrc("EditStyleBase", "On first system of sections:")
            hint: qsTrc("notation/engravingoptions",
                        "That is, the first system of the score and the first system after every section break.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Long name") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Short name") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Hide") }
            ]
        }

        EoChoice {
            sid: "subsSystemInstNameVisibility"
            music: "\\name \"Violin I\" \"Vln. I\" c''1 | \\break c''1"
            systemWidth: 20
            musicStyle: ({
                hideInstrumentNameIfOneInstrument: false,
                firstSystemInstNameVisibility: 0,
                subsSystemInstNameVisibility: 1
            })
            title: qsTrc("EditStyleBase", "On subsequent systems:")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Long name") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Short name") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Hide") }
            ]
        }

        EoNumber {
            sid: "instrumentNameOffset"
            music: "\\name \"Violin I\" \"Vln. I\" c''1 | c''1"
            musicStyle: ({
                hideInstrumentNameIfOneInstrument: false,
                firstSystemInstNameVisibility: 0,
                subsSystemInstNameVisibility: 1
            })
            title: qsTrc("EditStyleBase", "Distance from staff:")
            hint: qsTrc("notation/engravingoptions",
                        "The gap between the instrument names and the staves; where there are brackets, it is " +
                        "measured from the outermost bracket.")
            step: 0.1
        }

        EoToggle {
            sid: "instrumentNameOffsetScaleWithFontSize"
            music: "\\name \"Violin I\" \"Vln. I\" c''1 | c''1"
            musicStyle: ({
                hideInstrumentNameIfOneInstrument: false,
                firstSystemInstNameVisibility: 0,
                subsSystemInstNameVisibility: 1,
                longInstrumentFontSize: 16
            })
            title: qsTrc("notation/engravingoptions", "Widen the distance for larger name fonts")
            hint: qsTrc("notation/engravingoptions",
                        "‘Distance from staff’ is multiplied by how much larger the instrument-name font is than its " +
                        "default size; off, the distance stays exactly as set.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Swing")
        hint: qsTrc("notation/engravingoptions", "For playback only; nothing changes on the page.")

        EoChoice {
            sid: "swingUnit"
            music: "c''8 d''8 e''8 f''8"
            title: qsTrc("notation/engravingoptions", "Swung note value")
            hint: qsTrc("notation/engravingoptions",
                        "Pairs of eighth notes, or of sixteenth notes, are played long–short throughout the score; a " +
                        "swing text in the score takes over from the place where it stands.")
            choices: [
                { value: "", title: qsTrc("notation/engravingoptions", "Off") },
                { value: "eighth", title: qsTrc("notation/engravingoptions", "Eighth note") },
                { value: "16th", title: qsTrc("notation/engravingoptions", "Sixteenth note") }
            ]
        }

        EoNumber {
            sid: "swingRatio"
            music: "c''8 d''8 e''8 f''8"
            title: qsTrc("notation/engravingoptions", "Swing ratio")
            hint: qsTrc("notation/engravingoptions",
                        "The share of the beat which the first note of a swung pair takes: 50% is even, 67% a " +
                        "triplet swing. No effect while ‘Swung note value’ is ‘Off’.")
            min: 25
            max: 99
            step: 1
            decimals: 0
            unit: "%"
            enabled: page.value("swingUnit") !== ""
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Automatic placement")

        EoNumber {
            sid: "minVerticalDistance"
            music: "\\staves 2 << { g1 } \\\\ { \\staff 2 a'1 } >>"
            title: qsTrc("EditStyleBase", "Min. vertical distance:")
            hint: qsTrc("notation/engravingoptions",
                        "The least white space between the lowest note or mark of a staff and the highest note or " +
                        "mark of the staff below it; staves and systems are moved apart to keep it.")
            min: -999.99
            step: 0.1
        }

        EoNumber {
            sid: "skylineMinHorizontalClearance"
            music: "\\staves 2 << { g16 c''16 } \\\\ { \\staff 2 c16 a'16 } >>"
            musicStyle: ({ measureSpacing: 1.3 })
            title: qsTrc("EditStyleBase", "Min. horizontal distance:")
            hint: qsTrc("notation/engravingoptions",
                        "Two marks, one above the other, count as colliding and are moved apart vertically when they " +
                        "pass each other sideways by less than this.")
        }

        EoToggle {
            sid: "autoplaceEnabled"
            music: "c'4 \\ff d'4 \\pp"
            title: qsTrc("notation/engravingoptions", "Enable automatic placement")
            hint: qsTrc("notation/engravingoptions",
                        "Off: nothing is moved out of the way of anything else. Every sign stands at its default " +
                        "position or where it was dragged, collisions are left as they are, and notes are spaced by " +
                        "their duration alone, with no extra room for accidentals, dots or lyrics. The same switch " +
                        "as the command ‘Toggle automatic placement for entire score’.")
        }
    }
}
