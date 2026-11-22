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

    code: "tuplets"
    legacyCodes: ["tuplets"]
    title: qsTrc("EditStyleBase", "Tuplets")
    group: qsTrc("notation/engravingoptions", "Notes and rests")

    EoSection {
        title: qsTrc("EditStyleBase", "Properties")

        EoChoice {
            sid: "tupletDirection"
            title: qsTrc("EditStyleBase", "Direction:")
            choices: [
                { value: 0, title: qsTrc("engraving", "Auto") },
                { value: 1, title: qsTrc("engraving", "Up") },
                { value: 2, title: qsTrc("engraving", "Down") }
            ]
        }

        EoChoice {
            sid: "tupletNumberType"
            title: qsTrc("EditStyleBase", "Number type:")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Number") },
                { value: 1, title: qsTrc("notation/editstyle", "Ratio") },
                { value: 2, title: qsTrc("notation/editstyle", "None", "no tuplet number type") }
            ]
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Brackets")

        EoChoice {
            sid: "tupletBracketType"
            title: qsTrc("EditStyleBase", "Bracket type:")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Automatic") },
                { value: 1, title: qsTrc("notation/editstyle", "Bracket") },
                { value: 2, title: qsTrc("notation/editstyle", "None", "no tuplet bracket type") }
            ]
        }

        EoNumber {
            sid: "tupletBracketWidth"
            title: qsTrc("EditStyleBase", "Bracket thickness:")
            min: 0.05
            max: 1
        }

        EoNumber {
            sid: "tupletBracketHookHeight"
            title: qsTrc("EditStyleBase", "Bracket hook height:")
            min: 0.05
            max: 10
        }

        EoToggle {
            sid: "tupletExtendToEndOfDuration"
            title: qsTrc("EditStyleBase", "Extend bracket to end of duration")
        }

        EoToggle {
            sid: "tupletBracketFixedSlant"
            title: qsTrc("EditStyleBase", "Fixed slant (follow beam/stems)")
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Vertical distance from notes")

        EoNumber {
            sid: "tupletMaxSlope"
            title: qsTrc("EditStyleBase", "Maximum slope:")
            max: 1
            unit: qsTrc("global", "sp")
        }

        EoNumber {
            sid: "tupletVStemDistance"
            title: qsTrc("EditStyleBase", "Vertical distance from stem:")
            min: -5
            max: 5
        }

        EoNumber {
            sid: "tupletVHeadDistance"
            title: qsTrc("EditStyleBase", "Vertical distance from notehead:")
            min: -5
            max: 5
        }

        EoToggle {
            sid: "tupletOutOfStaff"
            title: qsTrc("EditStyleBase", "Avoid staves")
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Horizontal distance from notes")

        EoNumber {
            sid: "tupletStemLeftDistance"
            title: qsTrc("EditStyleBase", "Distance before stem of first note:")
            min: -5
            max: 5
        }

        EoNumber {
            sid: "tupletNoteLeftDistance"
            title: qsTrc("EditStyleBase", "Distance before head of first note:")
            min: -5
            max: 5
        }

        EoNumber {
            sid: "tupletStemRightDistance"
            title: qsTrc("EditStyleBase", "Distance after stem of last note:")
            min: -5
            max: 5
        }

        EoNumber {
            sid: "tupletNoteRightDistance"
            title: qsTrc("EditStyleBase", "Distance after head of last note:")
            min: -5
            max: 5
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Number and bracket details")

        EoNumber {
            sid: "tupletBracketNumberGap"
            title: qsTrc("notation/engravingoptions", "Gap between bracket and number")
            hint: qsTrc("notation/engravingoptions",
                         "Clear space left on each side of the tuplet number where the bracket is interrupted.")
            max: 2
        }

        EoNumber {
            sid: "tupletBracketMinArmLengthRatio"
            title: qsTrc("notation/engravingoptions", "Minimum bracket arm length (× hook height)")
            hint: qsTrc("notation/engravingoptions",
                         "Shortest horizontal bracket stroke kept on each side of the number, as a multiple of the hook height; a bracket that is too narrow is widened.")
            max: 3
        }

        EoToggle {
            sid: "tupletBracketOnNoteheadSide"
            title: qsTrc("notation/engravingoptions", "Bracket beamed tuplets on the notehead side")
            hint: qsTrc("notation/engravingoptions",
                         "A beamed tuplet whose number sits on the notehead side gets a bracket; turn off to show the number alone, as on the beam side.")
        }

        EoToggle {
            sid: "tupletBracketOmitOnBeamSubgroup"
            title: qsTrc("notation/engravingoptions", "Omit bracket when beam subdivisions outline the tuplet")
            hint: qsTrc("notation/engravingoptions",
                         "A tuplet inside a longer beam gets no bracket when secondary-beam breaks (or neighbours with fewer beams) already delimit it; turn off to bracket every tuplet that does not fill the whole beam.")
        }

        EoNumber {
            sid: "tupletNumberRhythmicCenterStemShift"
            title: qsTrc("notation/engravingoptions", "Rhythmic-centre number: shift towards stem (× notehead width)")
            hint: qsTrc("notation/engravingoptions",
                         "With ‘number at rhythmic centre’ on and the number on the stem side, how far it moves from the centre of the middle notehead towards the stem.")
            min: -1
            max: 1
        }

        EoNumber {
            sid: "tupletExtendToEndOfDurationPadding"
            title: qsTrc("notation/engravingoptions", "Extended bracket: gap before next element")
            hint: qsTrc("notation/engravingoptions",
                         "With ‘extend bracket to end of duration’ on, the minimum gap between the right end of the bracket and the following note, clef, barline or next tuplet bracket.")
            max: 3
        }
    }
}
