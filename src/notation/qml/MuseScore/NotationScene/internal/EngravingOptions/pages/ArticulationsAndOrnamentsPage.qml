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

    code: "articulations-and-ornaments"
    legacyCodes: ["articulations-and-ornaments"]
    title: qsTrc("EditStyleBase", "Articulations & ornaments")
    group: qsTrc("notation/engravingoptions", "Signs")
    music: "b'4-. e'4-> a'4-- c''4-^"

    EoSection {
        title: qsTrc("notation/engravingoptions", "Articulations")

        EoNumber {
            sid: "articulationMag"
            title: qsTrc("EditStyleBase", "Articulation size:")
            hint: qsTrc("notation/engravingoptions",
                        "Size of articulations, ornaments and fermatas relative to their normal size.")
            max: 300
            step: 10
            decimals: 0
            percent: true
        }

        EoNumber {
            sid: "articulationMinDistance"
            music: "a'4->-! b'4->-!"
            title: qsTrc("notation/engravingoptions", "Minimum vertical distance")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between two articulations stacked on one note, and between an " +
                        "articulation and a slur or another sign which it must clear.")
            step: 0.1
        }

        EoNumber {
            sid: "propertyDistanceHead"
            music: "a''4-> b''4->"
            title: qsTrc("EditStyleBase", "Distance from notehead:")
            hint: qsTrc("notation/engravingoptions",
                        "Vertical gap between the notehead and the nearest articulation on the notehead side when " +
                        "that mark stands outside the staff; inside the staff, staccato and tenuto sit in the middle " +
                        "of the next space.")
            step: 0.1
        }

        EoNumber {
            sid: "propertyDistanceStem"
            music: "<< { e''2-> c''2-> } \\\\ { g'2 e'2 } >>"
            title: qsTrc("notation/engravingoptions", "Distance from stem")
            hint: qsTrc("notation/engravingoptions",
                        "Vertical gap between the tip of the stem or flag and the nearest articulation on the stem " +
                        "side.")
            step: 0.1
        }

        EoNumber {
            sid: "propertyDistance"
            music: "b'4-> c''4->"
            title: qsTrc("EditStyleBase", "Distance from staff:")
            hint: qsTrc("notation/engravingoptions",
                        "Accents and other articulations which are kept outside the staff stay at least this far " +
                        "from the outer staff line.")
            step: 0.1
        }

        EoChoice {
            sid: "articulationStemHAlign"
            music: "<< { e''2-. c''2-. } \\\\ { g'2 e'2 } >>"
            title: qsTrc("EditStyleBase", "Articulation alignment when positioned at the stem")
            hint: qsTrc("notation/engravingoptions",
                        "Whether a staccato dot, tenuto or similar mark at the stem end of a note is centred on the " +
                        "stem, on the notehead, or halfway between the two.")
            presentation: "list"
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Align with stem") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Align with notehead") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Center between stem and notehead") }
            ]
        }

        EoChoice {
            sid: "articulationKeepTogether"
            music: "e''8-.->[ e'8] d''8-.->[ f'8] c''2"
            title: qsTrc("notation/engravingoptions", "Keep combined articulations together")
            hint: qsTrc("notation/engravingoptions",
                        "‘Always keep together’: a staccato or tenuto on a note which also has an accent or another " +
                        "mark outside the staff moves out of the staff to join it. ‘Prefer closeness to notehead’ " +
                        "leaves it next to the notehead, so that the two may be far apart.")
            presentation: "list"
            choices: [
                { value: true, title: qsTrc("notation/engravingoptions", "Always keep together") },
                { value: false, title: qsTrc("notation/engravingoptions", "Prefer closeness to notehead") }
            ]
        }

        EoNumber {
            sid: "articulationHorizontalPadding"
            music: "c''16-> d''16-> e''16-> f''16-> c''4"
            title: qsTrc("notation/engravingoptions", "Horizontal padding")
            hint: qsTrc("notation/engravingoptions",
                        "Least horizontal gap which note spacing keeps between an articulation or fermata and the " +
                        "items next to it in the staff.")
            max: 5
            step: 0.05
        }

        EoNumber {
            sid: "articulationSkylineMinHorizontalClearance"
            music: "c''16->[ a''16-> c''16-> a''16->] c''4"
            title: qsTrc("notation/engravingoptions", "Sideways clearance when avoiding collisions")
            hint: qsTrc("notation/engravingoptions",
                        "An articulation, ornament or fermata is pushed away from the staff by anything above or " +
                        "below it, and also by anything closer than this to its left or right; 0 lets it stand right " +
                        "beside its neighbours.")
            max: 2
        }

        EoChoice {
            sid: "articulationAnchorDefault"
            music: "e'4-. g'4-> a'4 \\turn"
            title: qsTrc("notation/engravingoptions", "Position of articulations and ornaments")
            hint: qsTrc("notation/engravingoptions",
                        "‘Auto’ puts staccato, tenuto, accents and the like on the notehead side, or on the stem " +
                        "side where the staff has several voices; ornaments and trill lines go above for voices 1 " +
                        "and 3 and below for voices 2 and 4. ‘Above’ and ‘Below’ put them all on one side of the " +
                        "notes.")
            choices: [
                { value: 2, title: qsTrc("notation/engravingoptions", "Auto") },
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoChoice {
            sid: "articulationAnchorLuteFingering"
            music: "c''4 \\luteFinger thumb b'4 \\luteFinger 1 d''4 \\luteFinger 2 c''4 \\luteFinger 3"
            musicStyle: ({ articulationMag: 1.4 })
            title: qsTrc("notation/engravingoptions", "Position of lute right-hand fingering")
            hint: qsTrc("notation/engravingoptions",
                        "The dots and strokes which name the fingers of the right hand in lute music: below the " +
                        "notes (default), above them, or ‘Auto’ as for articulations.")
            choices: [
                { value: 2, title: qsTrc("notation/engravingoptions", "Auto") },
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoChoice {
            sid: "articulationAnchorOther"
            music: "c''4 \\downbow d''4 \\upbow"
            title: qsTrc("notation/engravingoptions", "Position of bowings and other playing marks")
            hint: qsTrc("notation/engravingoptions",
                        "Up bow and down bow, harmonics, open and stopped signs, snap pizzicato and the other signs " +
                        "which are not articulations in the narrow sense: above the notes (default), below them, or " +
                        "‘Auto’ as for articulations.")
            choices: [
                { value: 2, title: qsTrc("notation/engravingoptions", "Auto") },
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Kinds of articulation")

        EoToggle {
            sid: "articulationMarcatoAbove"
            music: "e'4-^ g'4-^"
            title: qsTrc("notation/engravingoptions", "Marcato always above the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Marcato marks stand above the staff whatever the stem direction; off puts them on the " +
                        "notehead side like other articulations.")
        }

        EoToggle {
            sid: "articulationStaccatoCloseToNote"
            music: "b'4-. a'4-."
            title: qsTrc("notation/engravingoptions", "Staccato next to the notehead")
            hint: qsTrc("notation/engravingoptions",
                        "The dot sits right at the notehead, in a staff space if need be and inside slurs; off " +
                        "stacks it outside the staff and outside slurs, with accents and the other articulations.")
        }

        EoToggle {
            sid: "articulationTenutoCloseToNote"
            music: "b'4-- a'4--"
            title: qsTrc("notation/engravingoptions", "Tenuto next to the notehead")
            hint: qsTrc("notation/engravingoptions",
                        "The dash sits right at the notehead, in a staff space if need be and inside slurs; off " +
                        "stacks it outside the staff and outside slurs, with accents and the other articulations.")
        }

        EoNumber {
            sid: "articulationStaccatoAccentKern"
            music: "a''4-.-> b''4-.->"
            title: qsTrc("notation/engravingoptions", "Accent drawn towards a staccato dot")
            hint: qsTrc("notation/engravingoptions",
                        "Outside the staff, an accent stacked on a staccato dot is moved towards the dot by this " +
                        "much, so that the two read as one sign.")
            max: 1
        }

        EoNumber {
            sid: "articulationTenutoTieClearance"
            music: "a''2--~ a''2"
            title: qsTrc("notation/engravingoptions", "Tenuto: extra distance from notehead next to a tie")
            hint: qsTrc("notation/engravingoptions",
                        "Added to ‘Distance from notehead’ when the tenuto stands outside the staff on the side " +
                        "where a tie starts or ends, so that the tie passes between notehead and mark.")
            max: 3
        }

        EoNumber {
            sid: "articulationStaccatoTieClearance"
            music: "a''2-.~ a''2"
            title: qsTrc("notation/engravingoptions", "Staccato: extra distance from notehead next to a tie")
            hint: qsTrc("notation/engravingoptions",
                        "Added to ‘Distance from notehead’ when the staccato dot stands outside the staff on the " +
                        "side where a tie starts or ends, so that the tie passes between notehead and dot.")
            max: 3
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Ornaments")
        music: "\\key 2 b'2 \\trill! e''2 \\turn!"

        EoNumber {
            sid: "ornamentAccidentalDistance"
            title: qsTrc("notation/engravingoptions", "Accidental: distance from the ornament")
            hint: qsTrc("notation/engravingoptions",
                        "Vertical gap between a trill, turn or other ornament and the small accidental above or " +
                        "below it.")
            max: 2
        }

        EoNumber {
            sid: "ornamentAccidentalMag"
            title: qsTrc("notation/engravingoptions", "Accidental: size")
            hint: qsTrc("notation/engravingoptions",
                        "Size of the accidentals above and below ornaments as a multiple of a normal accidental; 0.6 " +
                        "is 60 %.")
            min: 0.3
            max: 1.5
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Falls, doits, plops and scoops")
        music: "c''2 \\fall c''2 \\doit"

        EoNumber {
            sid: "chordlineThickness"
            title: qsTrc("notation", "Line thickness:")
            hint: qsTrc("notation/engravingoptions",
                        "Thickness of the straight and curved lines; the wavy forms are symbols of the music font " +
                        "and keep their own weight.")
            step: 0.01
        }

        EoNumber {
            sid: "chordlineHeight"
            title: qsTrc("notation/engravingoptions", "Height")
            hint: qsTrc("notation/engravingoptions",
                        "How far a straight or curved line rises or falls, as long as it has not been reshaped by " +
                        "hand.")
            min: 0.25
            max: 4
            step: 0.05
        }

        EoNumber {
            sid: "chordlineWidthRatio"
            title: qsTrc("notation/engravingoptions", "Width")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal extent as a multiple of the height; 1.2 makes the line a little wider than it is " +
                        "tall.")
            min: 0.25
            max: 4
            step: 0.05
        }

        EoNumber {
            sid: "chordlineNoteDistance"
            title: qsTrc("notation/engravingoptions", "Distance from the note")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal gap between the line and whatever lies at its height on that side of the chord: " +
                        "the notehead, an augmentation dot or an accidental.")
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "chordlineVerticalOffset"
            title: qsTrc("notation/engravingoptions", "Start height above or below the notehead centre")
            hint: qsTrc("notation/engravingoptions",
                        "Falls and scoops meet the note this far below the middle of the notehead, doits and plops " +
                        "this far above it.")
            max: 1
            step: 0.05
        }

        EoNumber {
            sid: "chordlineWavyOffsetAbove"
            music: "c''2 \\doitWavy c''2 \\plopWavy"
            title: qsTrc("notation/engravingoptions", "Wavy doits and plops: shift downwards")
            hint: qsTrc("notation/engravingoptions",
                        "A fine adjustment which lowers the wavy symbol so that it meets the note at the right " +
                        "height in the chosen music font.")
            min: -1
            max: 1
            step: 0.005
            decimals: 3
        }

        EoNumber {
            sid: "chordlineWavyOffsetBelow"
            music: "c''2 \\fallWavy c''2 \\scoopWavy"
            title: qsTrc("notation/engravingoptions", "Wavy falls and scoops: shift upwards")
            hint: qsTrc("notation/engravingoptions",
                        "A fine adjustment which raises the wavy symbol, which otherwise hangs below its start " +
                        "height by its full height.")
            min: -1
            max: 1
            step: 0.005
            decimals: 3
        }

        EoNumber {
            sid: "chordlineHorizontalPadding"
            music: "c''8[ c''8] \\fall c''8[ c''8] \\fall c''2"
            title: qsTrc("notation/engravingoptions", "Horizontal padding")
            hint: qsTrc("notation/engravingoptions",
                        "Least horizontal gap which note spacing keeps between a fall, doit, plop or scoop and the " +
                        "items before and after it.")
            max: 5
            step: 0.05
        }

        EoNumber {
            sid: "chordlineBarlinePadding"
            music: "c''2 c''2 \\fall | c''2 \\scoop c''2"
            title: qsTrc("notation/engravingoptions", "Distance from a barline")
            hint: qsTrc("notation/engravingoptions",
                        "Least horizontal gap between a fall, doit, plop or scoop and a barline.")
            max: 5
            step: 0.05
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Breath marks and caesuras")

        EoNumber {
            sid: "breathMarkYOffset"
            music: "c''2 \\breathe c''2"
            title: qsTrc("notation/engravingoptions", "Breath mark: vertical position")
            hint: qsTrc("notation/engravingoptions",
                        "Height of breath marks relative to the top staff line, or the bottom line for a mark placed " +
                        "below; negative values are above that line.")
            min: -5
            max: 5
            step: 0.1
        }

        EoNumber {
            sid: "caesuraYOffset"
            music: "c''2 \\caesura c''2"
            title: qsTrc("notation/engravingoptions", "Caesura: vertical position")
            hint: qsTrc("notation/engravingoptions",
                        "Height of the foot of a caesura relative to the top staff line, or the bottom line for a " +
                        "mark placed below; positive values are below that line, so 1 sp lets the strokes cut " +
                        "through the line.")
            min: -5
            max: 5
            step: 0.1
        }

        EoNumber {
            sid: "breathHorizontalPadding"
            music: "c''8[ c''8] \\breathe c''8[ c''8] \\breathe c''2"
            title: qsTrc("notation/engravingoptions", "Horizontal padding")
            hint: qsTrc("notation/engravingoptions",
                        "Least horizontal gap which note spacing keeps between a breath mark or caesura and the " +
                        "items before and after it.")
            max: 5
            step: 0.05
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Keyboard hand brackets")
        music: "<e' g' c''>2 \\handBracket right <e' g' c''>2 \\handBracket left"
        hint: qsTrc("notation/engravingoptions",
                    "The angled brackets which mark notes to be played with the left or the right hand.")

        EoNumber {
            sid: "keyboardHandBracketShortSide"
            title: qsTrc("inspector", "Short side")
            hint: qsTrc("notation/engravingoptions", "Length of the horizontal stroke of the bracket.")
        }

        EoNumber {
            sid: "keyboardHandBracketLongSide"
            title: qsTrc("inspector", "Long side")
            hint: qsTrc("notation/engravingoptions", "Length of the vertical stroke of the bracket.")
        }

        EoNumber {
            sid: "keyboardHandBracketLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
        }
    }
}
