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
    group: qsTrc("notation/engravingoptions", "Notations")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Parentheses: shape")

        EoNumber {
            sid: "parenthesisMaxMidThickness"
            title: qsTrc("notation/engravingoptions", "Parenthesis: maximum thickness in the middle")
            hint: qsTrc("notation/engravingoptions",
                         "Limits how thick a drawn parenthesis can become in the middle, however tall it is.")
            min: 0.02
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "parenthesisShoulderHeightRatio"
            title: qsTrc("notation/engravingoptions", "Parenthesis: shoulder position")
            hint: qsTrc("notation/engravingoptions",
                         "Where the curve of a drawn parenthesis turns, as a fraction of its height from each end; smaller values give rounder ends and a flatter middle, larger values a more pointed curve.")
            max: 0.5
            step: 0.01
        }

        EoNumber {
            sid: "parenthesisShoulderWidthFactor"
            title: qsTrc("notation/engravingoptions", "Parenthesis: curvature")
            hint: qsTrc("notation/engravingoptions",
                         "How far a drawn parenthesis bulges sideways in proportion to its height; larger values give a more curved parenthesis.")
            min: 0.05
            max: 0.6
            step: 0.01
        }

        EoNumber {
            sid: "parenthesisMinShoulderWidth"
            title: qsTrc("notation/engravingoptions", "Parenthesis: minimum bulge")
            hint: qsTrc("notation/engravingoptions",
                         "Smallest sideways bulge of a drawn parenthesis, so that short parentheses keep a visible curve.")
            max: 1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Parentheses around notes and chords")

        EoNumber {
            sid: "parenthesisNoteExtension"
            title: qsTrc("notation/engravingoptions", "Note parentheses: extension above and below the notehead")
            hint: qsTrc("notation/engravingoptions",
                         "How far the parentheses around a single note extend beyond the top and the bottom of the notehead.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisNoteHeightToThicknessRatio"
            title: qsTrc("notation/engravingoptions", "Note parentheses: height-to-thickness ratio")
            hint: qsTrc("notation/engravingoptions",
                         "The thickness of note parentheses in the middle is their height divided by this value; smaller values give bolder parentheses.")
            min: 5
            max: 200
            step: 1
        }

        EoNumber {
            sid: "parenthesisNoteEndThickness"
            title: qsTrc("notation/engravingoptions", "Note and chord parentheses: tip thickness")
            hint: qsTrc("notation/engravingoptions",
                         "Thickness of the two tips of the parentheses around notes and chords.")
            max: 0.5
            step: 0.01
        }

        EoNumber {
            sid: "parenthesisChordExtension"
            title: qsTrc("notation/engravingoptions", "Chord parentheses: extension above and below the chord")
            hint: qsTrc("notation/engravingoptions",
                         "How far the parentheses around a whole chord extend beyond its top and bottom noteheads.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisChordHeightToThicknessRatio"
            title: qsTrc("notation/engravingoptions", "Chord parentheses: height-to-thickness ratio")
            hint: qsTrc("notation/engravingoptions",
                         "The thickness of chord parentheses in the middle is their height divided by this value; smaller values give bolder parentheses.")
            min: 5
            max: 200
            step: 1
        }

        EoNumber {
            sid: "parenthesisChordPadding"
            title: qsTrc("notation/engravingoptions", "Chord parentheses: distance from noteheads and accidentals")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal gap between the parentheses around a whole chord and its outermost noteheads or accidentals.")
            max: 2
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Parentheses: padding")

        EoNumber {
            sid: "parenthesisNotePadding"
            title: qsTrc("notation/engravingoptions", "Parenthesis to notehead")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal gap between a parenthesis and the notehead it encloses on standard staves.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisArticulationPadding"
            title: qsTrc("notation/engravingoptions", "Parenthesis to articulation")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal gap between a note’s parenthesis and an articulation of the same chord.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisAccidentalPadding"
            title: qsTrc("notation/engravingoptions", "Parenthesis to accidental")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal gap between a note’s parenthesis and an accidental of the same chord.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisStemPadding"
            title: qsTrc("notation/engravingoptions", "Parenthesis to stem or flag")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal gap between a note’s parenthesis and the stem or flag of the same chord.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisDotPadding"
            title: qsTrc("notation/engravingoptions", "Parenthesis to augmentation dot")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal gap between a note’s parenthesis and its augmentation dot.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisLedgerLinePadding"
            title: qsTrc("notation/engravingoptions", "Parenthesis to ledger line")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal gap between a note’s parenthesis and the ledger lines of the same chord.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisKeySigPaddingLeft"
            title: qsTrc("notation/engravingoptions", "Opening parenthesis to key signature")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between the opening parenthesis and a key signature shown in parentheses.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisKeySigPaddingRight"
            title: qsTrc("notation/engravingoptions", "Key signature to closing parenthesis")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between a key signature shown in parentheses and the closing parenthesis.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisTimeSigPaddingLeft"
            title: qsTrc("notation/engravingoptions", "Opening parenthesis to time signature")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between the opening parenthesis and a time signature shown in parentheses.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisTimeSigPaddingRight"
            title: qsTrc("notation/engravingoptions", "Time signature to closing parenthesis")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between a time signature shown in parentheses and the closing parenthesis.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisClefPaddingLeft"
            title: qsTrc("notation/engravingoptions", "Opening parenthesis to clef")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between the opening parenthesis and a clef shown in parentheses.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisClefPaddingRight"
            title: qsTrc("notation/engravingoptions", "Clef to closing parenthesis")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between a clef shown in parentheses and the closing parenthesis.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisChordSymbolPadding"
            title: qsTrc("notation/engravingoptions", "Parenthesis to chord symbol")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between a chord symbol and the parentheses that enclose the whole symbol.")
            max: 2
        }

        EoNumber {
            sid: "parenthesisDefaultPadding"
            title: qsTrc("notation/engravingoptions", "Parenthesis to other objects")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between a parenthesis and any other object of the same chord that has no setting of its own.")
            max: 2
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Parentheses in chord symbols")

        EoNumber {
            sid: "parenthesisChordSymbolExtension"
            title: qsTrc("notation/engravingoptions", "Chord-symbol parentheses: extension above and below the text")
            hint: qsTrc("notation/engravingoptions",
                         "How far parentheses in chord symbols extend above the capital height and below the baseline; scales with the font size.")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "parenthesisChordSymbolHeightToThicknessRatio"
            title: qsTrc("notation/engravingoptions", "Chord-symbol parentheses: height-to-thickness ratio")
            hint: qsTrc("notation/engravingoptions",
                         "The thickness of chord-symbol parentheses in the middle is their height divided by this value; smaller values give bolder parentheses.")
            min: 5
            max: 200
            step: 1
        }

        EoNumber {
            sid: "parenthesisChordSymbolEndThickness"
            title: qsTrc("notation/engravingoptions", "Chord-symbol parentheses: tip thickness")
            hint: qsTrc("notation/engravingoptions", "Thickness of the two tips of the parentheses in chord symbols.")
            max: 0.5
            step: 0.01
        }

        EoNumber {
            sid: "parenthesisChordSymbolShoulderWidthFactor"
            title: qsTrc("notation/engravingoptions", "Chord-symbol parentheses: curvature")
            hint: qsTrc("notation/engravingoptions",
                         "How far the parentheses in chord symbols bulge sideways in proportion to their height.")
            min: 0.05
            max: 0.6
            step: 0.01
        }
    }
}
