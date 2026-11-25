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

    code: "fretboard-diagrams"
    legacyCodes: ["fretboard-diagrams"]
    title: qsTrc("EditStyleBase", "Fretboard diagrams")
    group: qsTrc("notation/engravingoptions", "Text")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Fretboard diagrams")
        music: "c''2 \\fretDiagram \"x32010\" b'2 \\fretDiagram \"x02210\""

        EoNumber {
            sid: "fretY"
            title: qsTrc("notation/engravingoptions", "Distance above the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between the top staff line and the bottom of the diagram, before collision avoidance " +
                        "moves it further up.")
            min: -100
            max: 100
            step: 0.05
        }

        EoNumber {
            sid: "fretMag"
            title: qsTrc("notation", "Scale:")
            hint: qsTrc("notation/engravingoptions",
                        "Size of all fretboard diagrams; string and fret spacing, dots and line thicknesses grow " +
                        "with it.")
            min: 10
            max: 500
            step: 5
            decimals: 0
            percent: true
        }

        EoChoice {
            sid: "fretOrientation"
            title: qsTrc("notation", "Orientation:")
            hint: qsTrc("notation/engravingoptions",
                        "Vertical: the nut is at the top and the strings run downwards. Horizontal: the diagram lies " +
                        "on its side with the nut at the left.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Vertical") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Horizontal") }
            ]
        }

        EoNumber {
            sid: "fretMinDistance"
            music: "e'''2 \\fretDiagram \"x32010\" a'2"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a fretboard diagram and the notes or other signs which lie " +
                        "between it and the staff.")
            min: -100
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "fretDiagramPadding"
            music: "c''4 \\fretDiagram \"x32010\" a'4 \\fretDiagram \"x02210\" f'4 \\fretDiagram \"133211\" g'4 " +
                   "\\fretDiagram \"320003\""
            title: qsTrc("notation/engravingoptions", "Gap between neighbouring diagrams")
            hint: qsTrc("notation/engravingoptions",
                        "Notes are spaced apart far enough to keep this horizontal gap between two fretboard " +
                        "diagrams.")
            max: 5
            step: 0.05
        }

        EoNumber {
            sid: "harmonyFretDiagramPadding"
            music: "c''4 \\chordSymbol \"Cmaj7(#11)\" a'4 \\fretDiagram \"x02210\" r2"
            title: qsTrc("notation/engravingoptions", "Gap between a diagram and a neighbouring chord symbol")
            hint: qsTrc("notation/engravingoptions",
                        "Notes are spaced apart far enough to keep this horizontal gap between a fretboard diagram " +
                        "and a chord symbol on the next or previous note.")
            max: 5
            step: 0.05
        }

        EoChoice {
            sid: "fretPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which fretboard diagrams are placed, unless one is flipped by hand. " +
                        "Below the staff a diagram hangs under the lowest note or sign at ‘Minimum distance from " +
                        "notes and signs’; ‘Distance above the staff’ is not used there.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Strings and frets")
        music: "c''2 \\fretDiagram \"x32010\" b'2 \\fretDiagram \"x02210\""

        EoNumber {
            sid: "fretStringSpacing"
            title: qsTrc("notation", "String spacing:")
            min: 0.1
            max: 5
            step: 0.05
        }

        EoNumber {
            sid: "fretFretSpacing"
            title: qsTrc("notation", "Fret spacing:")
            min: 0.1
            max: 5
            step: 0.05
        }

        EoNumber {
            sid: "fretNutThickness"
            title: qsTrc("notation", "Nut line thickness:")
            hint: qsTrc("notation/engravingoptions",
                        "Thickness of the heavy top line; it is only drawn heavy when the diagram starts at the " +
                        "first fret.")
            min: 0
            max: 2
            step: 0.05
        }

        EoNumber {
            sid: "fretLineThickness"
            title: qsTrc("notation/engravingoptions", "String and fret line thickness")
            min: 0.01
            max: 0.5
            step: 0.01
        }

        EoNumber {
            sid: "fretDotSpatiumSize"
            title: qsTrc("notation/engravingoptions", "Dot size")
            hint: qsTrc("notation/engravingoptions", "Diameter of the dots which show where the fingers go.")
            min: 0.05
            max: 2
            step: 0.05
        }

        EoNumber {
            sid: "fretMarkerSizeRatio"
            title: qsTrc("notation/engravingoptions", "Size of the open and mute markers")
            hint: qsTrc("notation/engravingoptions",
                        "Size of the ○ and × above the nut, as a fraction of the string spacing.")
            min: 0.2
            max: 1.5
        }

        EoNumber {
            sid: "fretMarkerNutDistance"
            title: qsTrc("notation/engravingoptions", "Gap between the markers and the nut")
            max: 2
        }

        EoNumber {
            sid: "fretMarkerLineThicknessRatio"
            title: qsTrc("notation/engravingoptions", "Line thickness of the markers")
            hint: qsTrc("notation/engravingoptions",
                        "Thickness of the ○ and × and of the outlined dot shapes (cross, square, triangle), as a " +
                        "multiple of the string line thickness.")
            min: 0.2
            max: 5
            step: 0.1
            decimals: 1
        }

        EoChoice {
            sid: "fretStyleExtended"
            title: qsTrc("notation/engravingoptions", "Fretboard style")
            hint: qsTrc("notation/engravingoptions",
                        "Trimmed: the strings end at the last fret line. Extended: they run on for half a fret, as " +
                        "if the fretboard continued.")
            choices: [
                { value: false, title: qsTrc("notation/engravingoptions", "Trimmed") },
                { value: true, title: qsTrc("notation/engravingoptions", "Extended") }
            ]
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Fret number")
        music: "c''2 \\fretDiagram \"x35553\" at 3 r2"
        hint: qsTrc("notation/engravingoptions", "The number beside a diagram which does not start at the first fret.")

        EoChoice {
            sid: "fretNumPos"
            title: qsTrc("notation/engravingoptions", "Side of the diagram")
            hint: qsTrc("notation/engravingoptions",
                        "On which side of the diagram the number of its first fret stands; a horizontal diagram has " +
                        "it below or above instead.")
            choices: [
                { value: 0, title: page.value("fretOrientation") === 1 ? qsTrc("notation", "Bottom")
                                                                      : qsTrc("notation", "Left") },
                { value: 1, title: page.value("fretOrientation") === 1 ? qsTrc("notation", "Top")
                                                                      : qsTrc("notation", "Right") }
            ]
        }

        EoNumber {
            sid: "fretNumPadding"
            title: qsTrc("notation/engravingoptions", "Gap between the number and the diagram")
            hint: qsTrc("notation/engravingoptions", "As a fraction of the string spacing.")
            max: 3
        }

        EoChoice {
            sid: "fretUseCustomSuffix"
            title: qsTrc("notation", "Format:")
            choices: [
                { value: false, title: qsTrc("notation/engravingoptions", "Number only") },
                { value: true, title: qsTrc("notation/engravingoptions", "Number with suffix") }
            ]
        }

        EoText {
            sid: "fretCustomSuffix"
            musicStyle: ({ fretUseCustomSuffix: true })
            title: qsTrc("notation", "Custom suffix:")
            hint: qsTrc("notation/engravingoptions",
                        "Text written directly after the fret number, for example ‘fr’ to give ‘3fr’. Only with " +
                        "‘Format’ set to ‘Number with suffix’.")
            enabled: page.value("fretUseCustomSuffix") === true
        }

        EoLink {
            title: qsTrc("notation", "Edit fret number text style")
            target: "fretboard-diagram-fret-number"
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Fingerings and barrés")
        music: "c''2 \\fretDiagram \"x35553\" at 3 barre 3 fingers \"013331\" r2"

        EoToggle {
            sid: "fretShowFingerings"
            title: qsTrc("notation", "Show fingerings")
            hint: qsTrc("notation/engravingoptions",
                        "Shows the finger numbers entered for a diagram, in a row under its strings.")
        }

        EoNumber {
            sid: "fretDiagramFingeringDistance"
            musicStyle: ({ fretShowFingerings: true })
            title: qsTrc("notation/engravingoptions", "Gap between the diagram and its fingerings")
            max: 2
            enabled: page.value("fretShowFingerings") === true
        }

        EoChoice {
            sid: "barreAppearanceSlur"
            title: qsTrc("notation/engravingoptions", "Barré appearance")
            hint: qsTrc("notation/engravingoptions",
                        "Line: a thick bar across the barred strings. Slur: an arc over them, with a dot on every " +
                        "string.")
            choices: [
                { value: false, title: qsTrc("notation/engravingoptions", "Line") },
                { value: true, title: qsTrc("notation/engravingoptions", "Slur") }
            ]
        }

        EoNumber {
            sid: "barreLineWidth"
            title: qsTrc("notation/engravingoptions", "Barré line thickness")
            hint: qsTrc("notation/engravingoptions",
                        "As a percentage of the dot size. Only with ‘Barré appearance’ set to ‘Line’.")
            min: 5
            max: 200
            step: 5
            decimals: 0
            percent: true
            enabled: page.value("barreAppearanceSlur") === false
        }

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Slur-style barrés")
            hint: qsTrc("notation/engravingoptions",
                        "Arc height is given as a fraction of the fret spacing; the thickness for three strings is " +
                        "also used for wider barrés. Only with ‘Barré appearance’ set to ‘Slur’.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Arc height"),
                qsTrc("notation/engravingoptions", "Thickness")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Two strings"),
                qsTrc("notation/engravingoptions", "Three strings"),
                qsTrc("notation/engravingoptions", "Four or more strings")
            ]
            compact: true

            EoNumber {
                sid: "barreSlurHeightTwoStrings"
                music: "c''1 \\fretDiagram \"xx0233\" barre 3"
                musicStyle: ({ barreAppearanceSlur: true, fretMag: 1.8 })
                hint: qsTrc("notation/engravingoptions",
                            "How far the arc of a slur-style barré across two strings bulges above its ends, as a " +
                            "fraction of the fret spacing.")
                max: 1.5
                step: 0.05
            }

            EoNumber {
                sid: "barreSlurHeightThreeStrings"
                music: "c''1 \\fretDiagram \"x02220\" barre 2"
                musicStyle: ({ barreAppearanceSlur: true, fretMag: 1.8 })
                hint: qsTrc("notation/engravingoptions",
                            "How far the arc of a slur-style barré across three strings bulges above its ends, as a " +
                            "fraction of the fret spacing.")
                max: 1.5
                step: 0.05
            }

            EoNumber {
                sid: "barreSlurHeight"
                music: "c''1 \\fretDiagram \"x24432\" barre 2"
                musicStyle: ({ barreAppearanceSlur: true, fretMag: 1.8 })
                hint: qsTrc("notation/engravingoptions",
                            "How far the arc of a slur-style barré across four or more strings bulges above its " +
                            "ends, as a fraction of the fret spacing.")
                max: 1.5
                step: 0.05
            }

            EoNumber {
                sid: "barreSlurThicknessTwoStrings"
                music: "c''1 \\fretDiagram \"xx0233\" barre 3"
                musicStyle: ({ barreAppearanceSlur: true, fretMag: 1.8 })
                hint: qsTrc("notation/engravingoptions",
                            "Thickness of the arc of a slur-style barré across two strings.")
                max: 0.5
                step: 0.01
            }

            EoNumber {
                sid: "barreSlurThickness"
                music: "c''1 \\fretDiagram \"x24432\" barre 2"
                musicStyle: ({ barreAppearanceSlur: true, fretMag: 1.8 })
                hint: qsTrc("notation/engravingoptions",
                            "Thickness of the arc of a slur-style barré across three or more strings.")
                max: 0.5
                step: 0.01
            }

            EoGap {
            }
        }

        EoNumber {
            sid: "barreSlurShoulderOffsetRatio"
            music: "c''1 \\fretDiagram \"x24432\" barre 2"
            musicStyle: ({ barreAppearanceSlur: true, fretMag: 1.8 })
            title: qsTrc("notation/engravingoptions", "Shoulder position of the barré arc")
            hint: qsTrc("notation/engravingoptions",
                        "How far in from each end the arc reaches its full height, as a fraction of its length: a " +
                        "small value gives a flat top with steep ends, 0.5 a pointed arc. Only with ‘Barré " +
                        "appearance’ set to ‘Slur’.")
            max: 0.5
            step: 0.01
            enabled: page.value("barreAppearanceSlur") === true
        }

        EoLink {
            title: qsTrc("notation", "Edit fingering text style")
            target: "fretboard-diagram-fingering"
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Defaults of the diagram properties")
        music: "c''2 \\fretDiagram \"x32010\" b'2 \\fretDiagram \"x02210\""
        hint: qsTrc("notation/engravingoptions",
                    "A fretboard diagram follows these values until it is given its own in the Properties panel. " +
                    "Diagrams taken from the palette or made from a chord symbol bring their own number of strings " +
                    "and frets.")

        EoNumber {
            sid: "fretStrings"
            title: qsTrc("inspector", "Strings")
            hint: qsTrc("notation/engravingoptions", "Number of strings of a diagram which has not been given its own.")
            min: 1
            max: 12
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "fretFrets"
            title: qsTrc("inspector", "Visible frets")
            hint: qsTrc("notation/engravingoptions",
                        "Number of frets drawn in a diagram which has not been given its own.")
            min: 1
            max: 24
            step: 1
            decimals: 0
        }

        EoToggle {
            sid: "fretNut"
            title: qsTrc("inspector", "Show nut")
            hint: qsTrc("notation/engravingoptions",
                        "The thick line of the nut is drawn at the top of every diagram which starts at the first " +
                        "fret, unless ‘Show nut’ was switched off for that diagram.")
        }
    }
}
