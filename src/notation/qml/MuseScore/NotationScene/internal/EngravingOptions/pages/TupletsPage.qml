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
        title: qsTrc("notation/engravingoptions", "Number")
        music: "\\tuplet 3/2 { c''4 d'' e'' }"

        EoChoice {
            sid: "tupletDirection"
            title: qsTrc("EditStyleBase", "Direction:")
            hint: qsTrc("notation/engravingoptions",
                        "On which side of the notes the number and the bracket stand; ‘Auto’ puts them on the side " +
                        "of the stems.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Auto") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Up") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Down") }
            ]
        }

        EoChoice {
            sid: "tupletNumberType"
            title: qsTrc("EditStyleBase", "Number type:")
            hint: qsTrc("notation/engravingoptions", "‘Number’ prints 3, ‘Ratio’ prints 3:2, ‘None’ prints no number.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Number") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Ratio") },
                { value: 2, title: qsTrc("notation/editstyle", "None", "no tuplet number type") }
            ]
        }

        EoToggle {
            sid: "tupletUseSymbols"
            title: qsTrc("EditStyleBase", "Use musical symbols font for tuplet numbers")
            hint: qsTrc("notation/engravingoptions",
                        "The digits are the tuplet numerals of the music font instead of the text font of the tuplet " +
                        "text style.")
        }

        EoChoice {
            sid: "tupletNumberRythmicCenter"
            music: "\\tuplet 5/4 { c''16 d''8 e''8 }"
            title: qsTrc("notation/editstyle/tupletCentering", "Number alignment")
            hint: qsTrc("notation/engravingoptions",
                        "‘Graphical center’ puts the number in the middle of the bracket; ‘Rhythmic center’ puts it " +
                        "over the middle unit of the tuplet (the second eighth of an eighth-note triplet), which is " +
                        "somewhere else when the notes are of unequal length.")
            presentation: "tiles"
            choices: [
                { value: false, title: qsTrc("notation/engravingoptions", "Graphical center") },
                { value: true, title: qsTrc("notation/engravingoptions", "Rhythmic center") }
            ]
        }

        EoNumber {
            sid: "tupletNumberRhythmicCenterStemShift"
            music: "\\stemUp \\tuplet 5/4 { c''8 d''16 e''8 f''16 g''8 }"
            musicStyle: ({ tupletNumberRythmicCenter: true })
            title: qsTrc("notation/engravingoptions", "Number at the rhythmic centre: shift towards the stem")
            hint: qsTrc("notation/engravingoptions",
                        "When the rhythmic centre falls on a note and the number is on the stem side, the number " +
                        "moves this far from the middle of the notehead towards the stem, as a fraction of the " +
                        "notehead width (0.5 = exactly over the stem). Only with ‘Number alignment’ set to ‘Rhythmic " +
                        "center’.")
            min: -1
            max: 1
        }

        EoNumber {
            sid: "tupletMinDistance"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a tuplet’s number or bracket and whatever lies on its side: " +
                        "notes, stems, beams, articulations, slurs and, with ‘Avoid staves’ on, the staff lines. " +
                        "‘Vertical distance from stem’ and ‘Vertical distance from notehead’ cannot bring the " +
                        "bracket closer than this.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Bracket")
        music: "\\tuplet 3/2 { c''4 d'' e'' }"

        EoChoice {
            sid: "tupletBracketType"
            music: "\\tuplet 3/2 { c''8[ d'' e''] }"
            title: qsTrc("EditStyleBase", "Bracket type:")
            hint: qsTrc("notation/engravingoptions",
                        "‘Automatic’ leaves the bracket out where a beam already shows which notes belong to the " +
                        "tuplet.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Automatic") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Bracket") },
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
            hint: qsTrc("notation/engravingoptions",
                        "Length of the short vertical strokes at the two ends of the bracket. A number without a " +
                        "bracket stands this much further from the notes as well.")
            min: 0.05
            max: 10
        }

        EoNumber {
            sid: "tupletBracketNumberGap"
            title: qsTrc("notation/engravingoptions", "Gap between bracket and number")
            hint: qsTrc("notation/engravingoptions",
                        "The clear space on each side of the number where the bracket is interrupted.")
            max: 2
        }

        EoNumber {
            sid: "tupletBracketMinArmLengthRatio"
            music: "\\tuplet 3/2 { c''16 d''16 e''16 }"
            musicStyle: ({ tupletNumberType: 1, tupletBracketType: 1 })
            title: qsTrc("notation/engravingoptions", "Shortest bracket stroke beside the number")
            hint: qsTrc("notation/engravingoptions",
                        "The horizontal stroke on each side of the number is at least this long, as a multiple of " +
                        "the ‘Bracket hook height’; a bracket over very close notes is widened to make room for it.")
            max: 3
        }

        EoToggle {
            sid: "tupletBracketOnNoteheadSide"
            music: "\\stemUp \\tuplet 3/2 { c''8[ d''8 e''8] }"
            musicStyle: ({ tupletDirection: 2 })
            title: qsTrc("notation/engravingoptions", "Bracket beamed tuplets on the notehead side")
            hint: qsTrc("notation/engravingoptions",
                        "A beamed tuplet whose number is on the notehead side gets a bracket; turn off to print the " +
                        "number alone there too, as on the beam side. Only with ‘Bracket type’ set to ‘Automatic’.")
        }

        EoToggle {
            sid: "tupletBracketOmitOnBeamSubgroup"
            music: "c''8[ d''8 \\tuplet 3/2 { e''16 f''16 g''16] }"
            title: qsTrc("notation/engravingoptions", "No bracket when the secondary beams mark off the tuplet")
            hint: qsTrc("notation/engravingoptions",
                        "A tuplet inside a longer beam gets no bracket when a break in the secondary beams, or " +
                        "neighbouring notes with fewer beams, already shows where it begins and ends; turn off to " +
                        "bracket every tuplet which does not fill its whole beam. Only with ‘Bracket type’ set to " +
                        "‘Automatic’.")
        }

        EoToggle {
            sid: "tupletExtendToEndOfDuration"
            music: "\\tuplet 3/2 { c''4 d''2 } \\tuplet 3/2 { c''4 d''2 }"
            title: qsTrc("notation/engravingoptions", "Extend bracket to end of duration")
            hint: qsTrc("notation/engravingoptions",
                        "When the last note of a tuplet is a long one, the bracket runs on to the right over the " +
                        "time that note lasts instead of stopping at the note.")
        }

        EoNumber {
            sid: "tupletExtendToEndOfDurationPadding"
            music: "\\tuplet 3/2 { c''4 d''2 } \\tuplet 3/2 { c''4 d''2 }"
            title: qsTrc("notation/engravingoptions", "Extended bracket: gap before the next note or sign")
            hint: qsTrc("notation/engravingoptions",
                        "The right end of an extended bracket stops at least this far before the following note, " +
                        "clef, barline or tuplet bracket. Only with ‘Extend bracket to end of duration’ on.")
            max: 3
        }

        EoToggle {
            sid: "tupletBracketFixedSlant"
            title: qsTrc("EditStyleBase", "Fixed slant (follow beam/stems)")
            hint: qsTrc("notation/engravingoptions",
                        "The bracket runs parallel to the beam, or to the line through the tips of the first and " +
                        "last stems, however steep that is; ‘Maximum slope’ is then not applied.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Vertical distance from the notes")

        EoNumber {
            sid: "tupletMaxSlope"
            music: "\\tuplet 3/2 { c'4 c''4 c'''4 }"
            title: qsTrc("EditStyleBase", "Maximum slope:")
            hint: qsTrc("notation/engravingoptions",
                        "The steepest a bracket may be, as rise over length: 0.5 lets it climb one staff space for " +
                        "every two it runs. Steeper brackets are flattened by moving one end away from the notes.")
            max: 1
            unit: qsTrc("global", "sp")
        }

        EoNumber {
            sid: "tupletVStemDistance"
            music: "\\tuplet 3/2 { c''8 d'' e'' }"
            title: qsTrc("EditStyleBase", "Vertical distance from stem:")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between the ends of the bracket hooks and the stem tips, where the tuplet is on the " +
                        "stem side; also the gap to the staff when ‘Avoid staves’ has moved the bracket out.")
            min: -5
            max: 5
        }

        EoNumber {
            sid: "tupletVHeadDistance"
            music: "\\stemDown \\tuplet 3/2 { c''4 d'' e'' }"
            musicStyle: ({ tupletDirection: 1 })
            title: qsTrc("EditStyleBase", "Vertical distance from notehead:")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between the ends of the bracket hooks and the noteheads, where the tuplet is on the " +
                        "notehead side.")
            min: -5
            max: 5
        }

        EoToggle {
            sid: "tupletOutOfStaff"
            music: "\\tuplet 3/2 { c'''4 d''' e''' }"
            title: qsTrc("EditStyleBase", "Avoid staves")
            hint: qsTrc("notation/engravingoptions",
                        "A bracket which would lie inside the staff is moved out, to just above the top line or " +
                        "below the bottom line.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Horizontal distance from the notes")
        music: "\\stemDown \\tuplet 3/2 { c'4 d' e' }"

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "How far the bracket reaches beyond the first and the last note: measured from the stem " +
                        "where the bracket is on the stem side, from the outer edge of the notehead where it is on " +
                        "the notehead side. Negative values pull the end inwards.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Before the first note"),
                qsTrc("notation/engravingoptions", "After the last note")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "From the stem"),
                qsTrc("notation/engravingoptions", "From the notehead")
            ]

            EoNumber {
                sid: "tupletStemLeftDistance"
                music: "\\tuplet 3/2 { c''8 d'' e'' }"
                min: -5
                max: 5
            }

            EoNumber {
                sid: "tupletNoteLeftDistance"
                music: "\\stemDown \\tuplet 3/2 { c'4 d' e' }"
                musicStyle: ({ tupletDirection: 1 })
                min: -5
                max: 5
            }

            EoNumber {
                sid: "tupletStemRightDistance"
                music: "\\stemUp \\tuplet 3/2 { e'8 g' c'' }"
                min: -5
                max: 5
            }

            EoNumber {
                sid: "tupletNoteRightDistance"
                music: "\\stemDown \\tuplet 3/2 { c'4 d' e' }"
                musicStyle: ({ tupletDirection: 1 })
                min: -5
                max: 5
            }
        }
    }
}
