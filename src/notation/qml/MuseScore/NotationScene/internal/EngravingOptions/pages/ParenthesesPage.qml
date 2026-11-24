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

    code: "parentheses"
    title: qsTrc("notation/engravingoptions", "Parentheses")
    group: qsTrc("notation/engravingoptions", "Signs")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Shape")
        music: "\\parenthesize c''2 \\parenthesizeChord <e' g' c''>2"

        EoNumber {
            sid: "parenthesisMaxMidThickness"
            music: "\\parenthesizeChord <c' e' g' c'' e''>1"
            musicStyle: ({ parenthesisChordHeightToThicknessRatio: 20 })
            title: qsTrc("notation/engravingoptions", "Maximum thickness in the middle")
            hint: qsTrc("notation/engravingoptions",
                        "However tall a parenthesis is, it does not become thicker than this in the middle.")
            min: 0.02
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "parenthesisShoulderHeightRatio"
            title: qsTrc("notation/engravingoptions", "Shoulder position")
            hint: qsTrc("notation/engravingoptions",
                        "Where the curve turns, as a fraction of the height measured from each end: smaller values " +
                        "give round ends and a flat middle, larger values a more pointed curve.")
            max: 0.5
            step: 0.01
        }

        EoNumber {
            sid: "parenthesisShoulderWidthFactor"
            title: qsTrc("notation/engravingoptions", "Sideways bulge")
            hint: qsTrc("notation/engravingoptions",
                        "How far a parenthesis bulges sideways, as a fraction of its height; larger values give a " +
                        "more curved parenthesis. Parentheses in chord symbols have their own value below.")
            min: 0.05
            max: 0.6
            step: 0.01
        }

        EoNumber {
            sid: "parenthesisMinShoulderWidth"
            title: qsTrc("notation/engravingoptions", "Minimum sideways bulge")
            hint: qsTrc("notation/engravingoptions",
                        "Short parentheses bulge at least this far, so that they keep a visible curve.")
            max: 1
        }

        EoNumber {
            sid: "parenthesesMinInnerWidth"
            music: "\\key -1 f'2 | \\key 0 c''2 \\bar \":|.\" c''2"
            musicStyle: ({ smallParens: false })
            title: qsTrc("notation/engravingoptions", "Minimum width between the two parentheses")
            hint: qsTrc("notation/engravingoptions",
                        "The opening and the closing parenthesis stay at least this far apart; an item narrower than " +
                        "this is centred between them.")
            max: 5
            step: 0.05
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Around notes, chords and chord symbols")
        music: "c''1 \\chordSymbol \"(C7)\""

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "‘Notes’ are parentheses around a single notehead, ‘Chords’ those around a whole chord, " +
                        "‘Chord symbols’ those inside or around a chord symbol.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Extension above and below"),
                qsTrc("notation/engravingoptions", "Height ÷ thickness in the middle"),
                qsTrc("notation/engravingoptions", "Thickness at the ends")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Notes"),
                qsTrc("notation/engravingoptions", "Chords"),
                qsTrc("notation/engravingoptions", "Chord symbols")
            ]
            compact: true

            EoNumber {
                sid: "parenthesisNoteExtension"
                music: "\\parenthesize c''2 \\parenthesize e''2"
                hint: qsTrc("notation/engravingoptions",
                            "How far the parentheses around a single note reach beyond the top and the bottom of the " +
                            "notehead.")
                max: 2
            }

            EoNumber {
                sid: "parenthesisChordExtension"
                music: "\\parenthesizeChord <e' g' c''>2 \\parenthesizeChord <f' a' d''>2"
                hint: qsTrc("notation/engravingoptions",
                            "How far the parentheses around a whole chord reach beyond its top and bottom noteheads.")
                max: 2
            }

            EoNumber {
                sid: "parenthesisChordSymbolExtension"
                hint: qsTrc("notation/engravingoptions",
                            "How far parentheses in chord symbols reach above the capital letters and below the " +
                            "baseline; given for 10 pt text and growing with the text size.")
                max: 1
                step: 0.01
            }

            EoNumber {
                sid: "parenthesisNoteHeightToThicknessRatio"
                music: "\\parenthesize c''2 \\parenthesize e''2"
                hint: qsTrc("notation/engravingoptions",
                            "The thickness in the middle is the height divided by this number: larger numbers give " +
                            "thinner parentheses.")
                min: 5
                max: 200
                step: 1
            }

            EoNumber {
                sid: "parenthesisChordHeightToThicknessRatio"
                music: "\\parenthesizeChord <e' g' c''>2 \\parenthesizeChord <f' a' d''>2"
                hint: qsTrc("notation/engravingoptions",
                            "The thickness in the middle is the height divided by this number: larger numbers give " +
                            "thinner parentheses.")
                min: 5
                max: 200
                step: 1
            }

            EoNumber {
                sid: "parenthesisChordSymbolHeightToThicknessRatio"
                hint: qsTrc("notation/engravingoptions",
                            "The thickness in the middle is the height divided by this number: larger numbers give " +
                            "thinner parentheses.")
                min: 5
                max: 200
                step: 1
            }

            EoNumber {
                sid: "parenthesisNoteEndThickness"
                music: "\\parenthesize c''2 \\parenthesize e''2"
                hint: qsTrc("notation/engravingoptions",
                            "Thickness of the two tips of the parentheses around notes and around whole chords.")
                max: 0.5
                step: 0.01
            }

            EoGap {
            }

            EoNumber {
                sid: "parenthesisChordSymbolEndThickness"
                hint: qsTrc("notation/engravingoptions",
                            "Thickness of the two tips of the parentheses in chord symbols.")
                max: 0.5
                step: 0.01
            }
        }

        EoNumber {
            sid: "parenthesisChordPadding"
            music: "\\parenthesizeChord <e' g' c''>2 \\parenthesizeChord <f' a' d''>2"
            title: qsTrc("notation/engravingoptions", "Chords: distance from noteheads and accidentals")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal gap between the parentheses around a whole chord and its outermost noteheads or " +
                        "accidentals.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisChordSymbolShoulderWidthFactor"
            title: qsTrc("notation/engravingoptions", "Chord symbols: sideways bulge")
            hint: qsTrc("notation/engravingoptions",
                        "How far the parentheses in chord symbols bulge sideways, as a fraction of their height.")
            min: 0.05
            max: 0.6
            step: 0.01
        }

        EoNumber {
            sid: "parenthesisChordSymbolInnerPadding"
            music: "c''1 \\chordSymbol \"C7(b9)\""
            title: qsTrc("notation/engravingoptions", "Chord symbols: gap to the text inside")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between a parenthesis in a chord symbol and the text it encloses, as a fraction of the " +
                        "height of the two.")
            max: 0.5
            step: 0.01
        }

        EoNumber {
            sid: "parenthesisChordSymbolOuterPadding"
            music: "c''1 \\chordSymbol \"C7(b9)/E\""
            title: qsTrc("notation/engravingoptions", "Chord symbols: gap to the text outside")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between a parenthesis in a chord symbol and the text before an opening or after a " +
                        "closing parenthesis, as a fraction of the height of the two.")
            max: 0.5
            step: 0.01
        }

        EoNumber {
            sid: "parenthesisChordSymbolTallOpeningWidthScale"
            music: "c''1 \\chordSymbol \"C7(b9#11)\""
            musicStyle: ({ verticallyStackModifiers: true })
            title: qsTrc("notation/engravingoptions", "Chord symbols: room for a tall opening parenthesis")
            hint: qsTrc("notation/engravingoptions",
                        "An opening parenthesis which is taller than the root letter, as around stacked alterations, " +
                        "takes up less room than its width so that the text after it tucks into its curve; smaller " +
                        "values tuck it in further.")
            max: 2
            step: 0.05
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Around courtesy signatures at repeats")
        hint: qsTrc("notation/engravingoptions",
                    "Parentheses of the courtesies switched on under Clefs, key and time signatures > Courtesies at " +
                    "repeats and jumps; ‘Small parentheses’ there chooses between the small and the tall form.")
        music: "\\time 2/4 c''2 | \\time 3/4 c''2. \\bar \":|.\" c''2."

        EoNumber {
            sid: "parenthesisCourtesySmallInset"
            title: qsTrc("notation/engravingoptions", "Small parentheses: shorter than the signatures by")
            hint: qsTrc("notation/engravingoptions",
                        "How far each end of the parentheses stops short of the top and the bottom of the clef, key " +
                        "or time signature they enclose; negative values make them taller than the signatures.")
            min: -2
            max: 2
            step: 0.05
        }

        EoNumber {
            sid: "parenthesisCourtesyOvershoot"
            musicStyle: ({ smallParens: false })
            title: qsTrc("notation/engravingoptions", "Tall parentheses: reach beyond the staff")
            hint: qsTrc("notation/engravingoptions",
                        "How far the parentheses reach above the top staff line and below the bottom one.")
            max: 4
            step: 0.05
        }

        EoNumber {
            sid: "parenthesisCourtesyBigTimeSigExtension"
            musicStyle: ({ smallParens: false, timeSigPlacement: 1 })
            title: qsTrc("notation/engravingoptions", "Tall parentheses around large time signatures: extension")
            hint: qsTrc("notation/engravingoptions",
                        "For time signatures above or across the staves: how far the parentheses reach beyond the " +
                        "top and the bottom of the numerals.")
            max: 5
            step: 0.05
        }

        EoNumber {
            sid: "parenthesisCourtesyHeightToThicknessRatio"
            title: qsTrc("notation/engravingoptions", "Height ÷ thickness in the middle")
            hint: qsTrc("notation/engravingoptions",
                        "The thickness in the middle is the height divided by this number: larger numbers give " +
                        "thinner parentheses.")
            min: 5
            max: 200
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "parenthesisCourtesyEndThickness"
            title: qsTrc("notation/engravingoptions", "Thickness at the ends")
            hint: qsTrc("notation/engravingoptions", "Thickness of the two tips of these parentheses.")
            max: 0.5
            step: 0.01
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Padding")
        hint: qsTrc("notation/engravingoptions", "The white space between a parenthesis and what stands next to it.")
        music: "\\key 2 a'2 | \\key 0 c''2 \\bar \":|.\" c''2"

        EoNumber {
            sid: "parenthesisDefaultPadding"
            music: "\\parenthesize c''2 \\handBracket left \\parenthesize e''2"
            title: qsTrc("notation/engravingoptions", "Parenthesis to other objects")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between a parenthesis and anything of the same chord which has no value of its own " +
                        "below.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisNotePadding"
            music: "\\parenthesize c''2 \\parenthesize e''2"
            title: qsTrc("notation/engravingoptions", "Parenthesis to notehead")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal gap between a parenthesis and the notehead it encloses; tablature has its own " +
                        "value.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisAccidentalPadding"
            music: "\\parenthesize cis''2 \\parenthesize ees''2"
            title: qsTrc("notation/engravingoptions", "Parenthesis to accidental")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal gap between the parenthesis of a note and an accidental of the same chord.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisDotPadding"
            music: "\\parenthesize c''2. \\parenthesize e''4"
            title: qsTrc("notation/engravingoptions", "Parenthesis to augmentation dot")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal gap between the parenthesis of a note and its augmentation dot.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisLedgerLinePadding"
            music: "\\parenthesize c'''2 \\parenthesize a2"
            title: qsTrc("notation/engravingoptions", "Parenthesis to ledger line")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal gap between the parenthesis of a note and the ledger lines of the same chord.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisChordSymbolPadding"
            music: "c''1 \\chordSymbol \"(C7)\""
            title: qsTrc("notation/engravingoptions", "Parenthesis to chord symbol")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between a chord symbol and the parentheses which enclose the whole symbol.")
            max: 2
        }

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Signatures in parentheses")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between a clef, key or time signature and the parenthesis on each side of it.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Key signature"),
                qsTrc("notation/engravingoptions", "Time signature"),
                qsTrc("notation/engravingoptions", "Clef")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "On the left"),
                qsTrc("notation/engravingoptions", "On the right")
            ]

            EoNumber {
                sid: "parenthesisKeySigPaddingLeft"
                hint: qsTrc("notation/engravingoptions",
                            "Gap between the opening parenthesis and the key signature it encloses.")
                max: 2
            }

            EoNumber {
                sid: "parenthesisKeySigPaddingRight"
                hint: qsTrc("notation/engravingoptions", "Gap between the key signature and the closing parenthesis.")
                max: 2
            }

            EoNumber {
                sid: "parenthesisTimeSigPaddingLeft"
                music: "\\time 2/4 c''2 | \\time 3/4 c''2. \\bar \":|.\" c''2."
                hint: qsTrc("notation/engravingoptions",
                            "Gap between the opening parenthesis and the time signature it encloses.")
                max: 2
            }

            EoNumber {
                sid: "parenthesisTimeSigPaddingRight"
                music: "\\time 2/4 c''2 | \\time 3/4 c''2. \\bar \":|.\" c''2."
                hint: qsTrc("notation/engravingoptions", "Gap between the time signature and the closing parenthesis.")
                max: 2
            }

            EoNumber {
                sid: "parenthesisClefPaddingLeft"
                music: "c''2 | \\clef bass c2 \\bar \":|.\" c2"
                hint: qsTrc("notation/engravingoptions",
                            "Gap between the opening parenthesis and the clef it encloses.")
                max: 2
            }

            EoNumber {
                sid: "parenthesisClefPaddingRight"
                music: "c''2 | \\clef bass c2 \\bar \":|.\" c2"
                hint: qsTrc("notation/engravingoptions", "Gap between the clef and the closing parenthesis.")
                max: 2
            }
        }
    }
}
