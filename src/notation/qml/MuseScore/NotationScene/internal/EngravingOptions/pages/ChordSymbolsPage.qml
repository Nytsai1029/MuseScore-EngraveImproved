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
import Muse.UiComponents 1.0

import ".."

EoPage {
    id: page

    code: "chord-symbols"
    legacyCodes: ["chord-symbols"]
    title: qsTrc("EditStyleBase", "Chord symbols")
    group: qsTrc("notation/engravingoptions", "Text")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Appearance")
        music: "c''2 \\chordSymbol \"C7(b9)\" b'2 \\chordSymbol \"Bm7/D\" | f'2 \\chordSymbol \"Fmaj7\" g'2 " +
               "\\chordSymbol \"G7sus4\""

        EoChoice {
            sid: "chordStyle"
            title: qsTrc("notation/editstyle/chordsymbols", "Preset")
            hint: qsTrc("notation/engravingoptions",
                        "‘Standard’ sets chord symbols in the text font, ‘Jazz’ in the handwritten MuseJazz font " +
                        "with small raised modifiers, ‘Legacy MuseScore’ as versions before 4.6 drew them; ‘Custom " +
                        "file’ uses a style file of your own.")
            presentation: "list"
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Standard") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Jazz") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Legacy MuseScore") },
                { value: 3, title: qsTrc("notation/engravingoptions", "Custom file") }
            ]
        }

        EoCustom {
            sid: "chordDescriptionFile"
            title: qsTrc("notation/engravingoptions", "Chord symbol style file")
            hint: qsTrc("notation/engravingoptions",
                        "The XML file of your own which describes how chord symbols are drawn, as a copy of a " +
                        "built-in one (chords_std.xml, chords_jazz.xml) changed to your liking.")
            visible: page.value("chordStyle") === 3
            fullWidth: false

            component: FilePicker {
                property var optionsModel: null
                property NavigationPanel navigationPanel: null
                readonly property var styleItem: optionsModel ? optionsModel.item("chordDescriptionFile") : null

                implicitWidth: 280
                path: styleItem ? String(styleItem.value) : ""
                dir: optionsModel ? optionsModel.userStylesPath : ""
                filter: [ qsTrc("notation", "MuseScore chord symbol style files") + " (*.xml)" ]
                dialogTitle: qsTrc("notation", "Load style")
                navigation: navigationPanel

                onPathEdited: function(newPath) {
                    if (styleItem) {
                        styleItem.value = newPath
                    }
                }
            }
        }

        EoChoice {
            sid: "chordSymbolSpelling"
            title: qsTrc("notation/editstyle/chordsymbols", "Spelling")
            hint: qsTrc("notation/engravingoptions",
                        "How note names are written. Standard: B♭ and B. German: B♭ and H. Full German: B and H, " +
                        "with Fis, Es and the like. Solfeggio: Do Re Mi. French: Do Ré Mi.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Standard") },
                { value: 1, title: qsTrc("notation/engravingoptions", "German") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Full German") },
                { value: 3, title: qsTrc("notation/engravingoptions", "Solfeggio") },
                { value: 4, title: qsTrc("notation/engravingoptions", "French") }
            ]
        }

        EoToggle {
            sid: "automaticCapitalization"
            musicStyle: ({ lowerCaseMinorChords: true })
            title: qsTrc("notation/editstyle/chordsymbols", "Automatically capitalize note names")
            hint: qsTrc("notation/engravingoptions",
                        "Off: note names stay in upper or lower case exactly as they were typed.")
        }

        EoToggle {
            sid: "lowerCaseMinorChords"
            title: qsTrc("notation/engravingoptions", "Lowercase minor chords")
            hint: qsTrc("notation/engravingoptions",
                        "The root of minor, diminished and half-diminished chords is written in lower case (c for C " +
                        "minor). Only with ‘Automatically capitalize note names’ on.")
            enabled: page.value("automaticCapitalization") === true
        }

        EoToggle {
            sid: "lowerCaseBassNotes"
            title: qsTrc("notation/editstyle/chordsymbols", "Lowercase bass notes")
            hint: qsTrc("notation/engravingoptions",
                        "The bass note after the slash is written in lower case (C/e). Only with ‘Automatically " +
                        "capitalize note names’ on.")
            enabled: page.value("automaticCapitalization") === true
        }

        EoToggle {
            sid: "allCapsNoteNames"
            musicStyle: ({ chordSymbolSpelling: 3 })
            title: qsTrc("notation/editstyle/chordsymbols", "All caps note names")
            hint: qsTrc("notation/engravingoptions",
                        "Note names of more than one letter are written entirely in capitals (DO, FIS instead of Do, " +
                        "Fis). Only with ‘Automatically capitalize note names’ on.")
            enabled: page.value("automaticCapitalization") === true
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Extensions and modifiers")
        music: "c''2 \\chordSymbol \"C13\" g'2 \\chordSymbol \"G7sus4\""

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "Scale is the size relative to the root letter. The vertical offset is a percentage of the " +
                        "height of a capital letter; negative values raise, positive values lower.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Scale"),
                qsTrc("notation/engravingoptions", "Vertical offset")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Extensions (e.g. 7, 11)"),
                qsTrc("notation/engravingoptions", "Modifiers (e.g. sus4, no 3)")
            ]

            EoNumber {
                sid: "chordExtensionMag"
                hint: qsTrc("notation/engravingoptions",
                            "Size of extensions (7, 9, 11, 13…) relative to the root letter.")
                min: 0
                max: 999
                step: 5
                decimals: 0
                percent: true
            }

            EoNumber {
                sid: "chordModifierMag"
                hint: qsTrc("notation/engravingoptions",
                            "Size of modifiers (sus4, add9, ♭5…) relative to the root letter.")
                min: 0
                max: 999
                step: 5
                decimals: 0
                percent: true
            }

            EoNumber {
                sid: "chordExtensionAdjust"
                hint: qsTrc("notation/engravingoptions",
                            "Vertical shift of extensions as a percentage of the height of a capital letter; " +
                            "negative values raise, positive values lower.")
                min: -999
                max: 999
                step: 5
                decimals: 0
                percent: true
            }

            EoNumber {
                sid: "chordModifierAdjust"
                hint: qsTrc("notation/engravingoptions",
                            "Vertical shift of modifiers as a percentage of the height of a capital letter; negative " +
                            "values raise, positive values lower.")
                min: -999
                max: 999
                step: 5
                decimals: 0
                percent: true
            }
        }

        EoToggle {
            sid: "verticallyStackModifiers"
            music: "c''2 \\chordSymbol \"C7b9#11\" r2"
            title: qsTrc("notation/editstyle/chordsymbols", "Vertically stack modifiers")
            hint: qsTrc("notation/engravingoptions",
                        "Two or more modifiers are set one above the other after the chord symbol instead of in a " +
                        "row.")
        }

        EoNumber {
            sid: "chordStackedModifierMag"
            music: "c''2 \\chordSymbol \"C7b9#11\" r2"
            musicStyle: ({ verticallyStackModifiers: true })
            title: qsTrc("notation/engravingoptions", "Scale of stacked modifiers")
            hint: qsTrc("notation/engravingoptions",
                        "Size of modifiers which are stacked, relative to the modifier size set above.")
            min: 10
            max: 999
            step: 5
            decimals: 0
            percent: true
            enabled: page.value("verticallyStackModifiers") === true
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Bass notes and polychords")
        music: "c''2 \\chordSymbol \"A7/G\" f'2 \\chordSymbol \"D|C7\""

        EoChoice {
            sid: "chordBassNoteStagger"
            title: qsTrc("notation/engravingoptions", "Bass note of slash chords")
            hint: qsTrc("notation/engravingoptions",
                        "Whether the bass note after the slash stays on the line of the chord symbol (A7/G) or is " +
                        "set lower, diagonally below the slash.")
            choices: [
                { value: false, title: qsTrc("notation/engravingoptions", "On the same line") },
                { value: true, title: qsTrc("notation/engravingoptions", "Lowered diagonally") }
            ]
            enabled: page.value("chordStyle") !== 2
        }

        EoNumber {
            sid: "chordBassNoteScale"
            title: qsTrc("notation", "Bass note scale:")
            hint: qsTrc("notation/engravingoptions",
                        "Size of the bass note after the slash relative to the root letter.")
            min: 0
            max: 999
            step: 5
            decimals: 0
            percent: true
        }

        EoNumber {
            sid: "chordBassNoteStackedOffset"
            music: "c''2 \\chordSymbol \"C7b9#11/E\" r2"
            musicStyle: ({ verticallyStackModifiers: true })
            title: qsTrc("notation/engravingoptions", "Extra gap before the bass note after stacked modifiers")
            hint: qsTrc("notation/engravingoptions",
                        "When the chord symbol has stacked modifiers, or is a 6/9 chord, the slash and bass note are " +
                        "moved to the right by this much, as a multiple of the height of a capital letter.")
            min: -0.5
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "polychordDividerThickness"
            title: qsTrc("notation/engravingoptions", "Polychord divider thickness")
            hint: qsTrc("notation/engravingoptions",
                        "Thickness of the horizontal line between the upper and the lower chord of a polychord.")
            min: 0
            max: 99
            step: 0.01
            decimals: 2
        }

        EoNumber {
            sid: "polychordDividerSpacing"
            title: qsTrc("notation/engravingoptions", "Polychord divider spacing")
            hint: qsTrc("notation/engravingoptions", "White space left above and below the line.")
            min: 0
            max: 99
            step: 0.05
            decimals: 2
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Position")
        music: "c''4 \\chordSymbol \"C\" e''4 \\chordSymbol \"Am7\" f''4 \\chordSymbol \"Dm7\" g''4 \\chordSymbol " +
               "\"G7\""

        EoChoice {
            sid: "chordSymPosition"
            title: qsTrc("notation", "Alignment to notehead:")
            hint: qsTrc("notation/engravingoptions",
                        "Which part of a chord symbol is lined up with its note: left edges together, centred over " +
                        "the notehead, or right edges together.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Left") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Centre") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Right") }
            ]
        }

        EoToggle {
            sid: "verticallyAlignChordSymbols"
            music: "c''2 \\chordSymbol \"C\" c'''2 \\chordSymbol \"Am7\""
            title: qsTrc("notation/engravingoptions",
                         "Vertically align chord symbols and fretboard diagrams on the same system")
            hint: qsTrc("notation/engravingoptions",
                        "All chord symbols of a system stand at one height, that of the one which had to be placed " +
                        "furthest from the staff, instead of each as close as its own notes allow; fretboard " +
                        "diagrams are lined up in the same way.")
        }

        EoChoice {
            sid: "chordAlignmentToFretboard"
            music: "c''2 \\fretDiagram \"x32010\" \\chordSymbol \"Cmaj7(#11)\" r2"
            title: qsTrc("notation", "Alignment to fretboard diagram:")
            hint: qsTrc("notation/engravingoptions",
                        "For a chord symbol which stands on a fretboard diagram: lined up with the left edge of the " +
                        "diagram, centred over it, or lined up with its right edge.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Left") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Centre") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Right") }
            ]
        }

        EoToggle {
            sid: "chordAlignmentExcludeModifiers"
            music: "c''2 \\chordSymbol \"C7sus4\" g'2 \\chordSymbol \"G\""
            title: qsTrc("notation/editstyle/chordsymbols", "Exclude modifiers from horizontal alignment")
            hint: qsTrc("notation/engravingoptions",
                        "Only the part of the chord symbol before its first modifier is centred or right-aligned; " +
                        "the modifiers hang out to the right.")
        }

        EoNumber {
            sid: "minHarmonyDistance"
            music: "e'''4 \\chordSymbol \"C7\" c'''4 a''4 c'''4"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a chord symbol and the notes or other signs which lie between it " +
                        "and the staff.")
            min: -100
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "harmonyHarmonyDistance"
            music: "c''8 \\chordSymbol \"Cmaj7\" d''8 \\chordSymbol \"Dm7\" e''8 \\chordSymbol \"Em7\" f''8 " +
                   "\\chordSymbol \"Fmaj7\" g''2"
            title: qsTrc("notation", "Min. space between chord symbols:")
            hint: qsTrc("notation/engravingoptions",
                        "Notes are spaced apart far enough to keep this horizontal gap between neighbouring chord " +
                        "symbols; half of it is kept between a chord symbol and a barline.")
            min: 0
            max: 99
            step: 0.1
            decimals: 2
        }

        EoNumber {
            sid: "harmonyFretDist"
            music: "c''2 \\fretDiagram \"x32010\" \\chordSymbol \"Cmaj7(#11)\" r2"
            musicStyle: ({ minHarmonyDistance: 0 })
            title: qsTrc("notation", "Min. space to fretboard diagram:")
            hint: qsTrc("notation/engravingoptions",
                        "Height of a chord symbol above the nut of the fretboard diagram it stands on. It also keeps " +
                        "‘Minimum distance from notes and signs’ from the top of the diagram, where the ○ and × " +
                        "markers are; with the default values that distance is the larger one.")
            min: 0
            max: 99
            step: 0.05
            decimals: 2
        }

        EoChoice {
            sid: "harmonyPlacement"
            title: qsTrc("notation/engravingoptions", "Above or below the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which chord symbols are placed, unless one is flipped by hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoOffset {
            sid: "chordSymbolAPosBelow"
            musicStyle: ({ harmonyPlacement: 1 })
            title: qsTrc("notation/engravingoptions", "Offset below the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal shift and depth of a chord symbol placed below, measured from the bottom staff " +
                        "line (positive is down); notes and signs in the way still push it further out. The offset " +
                        "above the staff is the ‘Offset’ of the text style ‘Chord symbol’.")
        }

        EoOffset {
            sid: "chordSymbolBPosBelow"
            music: "c''2 \\chordSymbol \"C7\" alternate g'2 \\chordSymbol \"G7\" alternate"
            musicStyle: ({ harmonyPlacement: 1 })
            title: qsTrc("notation/engravingoptions", "Offset below the staff, alternate text style")
            hint: qsTrc("notation/engravingoptions",
                        "The same offset for chord symbols which use the text style ‘Chord symbol (alternate)’.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Playback")
        music: "c''2 \\chordSymbol \"C7\" g'2 \\chordSymbol \"G7\""
        hint: qsTrc("notation/engravingoptions", "How chord symbols sound in playback; nothing changes on the page.")

        EoChoice {
            sid: "harmonyVoiceLiteral"
            title: qsTrc("notation", "Interpretation:")
            hint: qsTrc("notation/engravingoptions",
                        "‘Literal’ plays exactly the notes the chord symbol names; ‘Jazz’ voices it as a jazz player " +
                        "would, adding or leaving out notes.")
            choices: [
                { value: false, title: qsTrc("notation/engravingoptions", "Jazz") },
                { value: true, title: qsTrc("notation/engravingoptions", "Literal") }
            ]
        }

        EoChoice {
            sid: "harmonyVoicing"
            title: qsTrc("notation", "Voicing:")
            hint: qsTrc("notation/engravingoptions", "How the notes of the chord are spread when it is played.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Automatic") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Root only") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Close") },
                { value: 3, title: qsTrc("notation/engravingoptions", "Drop two") },
                { value: 4, title: qsTrc("notation/engravingoptions", "Six note") },
                { value: 5, title: qsTrc("notation/engravingoptions", "Four note") },
                { value: 6, title: qsTrc("notation/engravingoptions", "Three note") }
            ]
        }

        EoChoice {
            sid: "harmonyDuration"
            title: qsTrc("notation", "Duration:")
            hint: qsTrc("notation/engravingoptions", "How long the chord sounds when it is played.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Until next chord symbol") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Until end of measure") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Chord/rest duration") }
            ]
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Capo")
        music: "c''2 \\chordSymbol \"Eb\" g'2 \\chordSymbol \"Bb7\""

        EoNumber {
            sid: "capoPosition"
            title: qsTrc("notation", "Capo fret position:")
            hint: qsTrc("notation/engravingoptions",
                        "With a capo at this fret, every chord symbol is followed in parentheses by the chord " +
                        "actually fingered, e.g. E♭ (D) with the capo at fret 1; 0 shows nothing.")
            min: 0
            max: 11
            step: 1
            decimals: 0
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Roman numerals and Nashville numbers")
        music: "c'4 \\romanNumeral \"I\" f'4 \\romanNumeral \"IV\" g'4 \\romanNumeral \"V7\" c'4 \\romanNumeral \"I\""
        hint: qsTrc("notation/engravingoptions",
                    "The two other kinds of harmony text: Roman numeral analysis (‘V7/IV’) and Nashville numbers.")

        EoChoice {
            sid: "romanNumeralPlacement"
            title: qsTrc("notation/engravingoptions", "Roman numerals: position")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which Roman numerals are placed, unless one is flipped by hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoOffset {
            sid: "romanNumeralPosBelow"
            title: qsTrc("notation/engravingoptions", "Roman numerals: offset below the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal shift and depth of a Roman numeral placed below, measured from the bottom staff " +
                        "line (positive is down); notes and signs in the way still push it further out. The offset " +
                        "above the staff is the ‘Offset’ of the text style ‘Roman numeral analysis’.")
        }

        EoChoice {
            sid: "nashvilleNumberPlacement"
            music: "c''4 \\nashville \"1\" f''4 \\nashville \"4\" g''4 \\nashville \"5\" c''4 \\nashville \"1\""
            title: qsTrc("notation/engravingoptions", "Nashville numbers: position")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which Nashville numbers are placed, unless one is flipped by hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoOffset {
            sid: "nashvilleNumberPosBelow"
            music: "c''4 \\nashville \"1\" f''4 \\nashville \"4\" g''4 \\nashville \"5\" c''4 \\nashville \"1\""
            musicStyle: ({ nashvilleNumberPlacement: 1 })
            title: qsTrc("notation/engravingoptions", "Nashville numbers: offset below the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal shift and depth of a Nashville number placed below, measured from the bottom " +
                        "staff line (positive is down); notes and signs in the way still push it further out. The " +
                        "offset above the staff is the ‘Offset’ of the text style ‘Nashville number’.")
        }
    }
}
