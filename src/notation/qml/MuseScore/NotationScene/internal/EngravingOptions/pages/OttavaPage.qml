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

    code: "ottava"
    legacyCodes: ["ottava"]
    title: qsTrc("notation/engravingoptions", "Ottavas")
    group: qsTrc("notation/engravingoptions", "Lines")
    music: "d''4 \\startOttava 8va f''4 a''4 f''4 | d''2 \\stopOttava b'2"

    EoSection {
        title: qsTrc("notation/engravingoptions", "Ottavas")

        EoToggle {
            sid: "ottavaNumbersOnly"
            title: qsTrc("EditStyleBase", "Numbers only")
            hint: qsTrc("notation/engravingoptions",
                        "The label is the number alone (8, 15, 22) instead of 8va, 15ma, 22ma and their bassa forms.")
        }

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "Offset: X moves the line to the right, Y moves it down from the top staff line (above the " +
                        "staff) or from the bottom staff line (below the staff), so a Y of −2 above means 2 sp over " +
                        "the staff. Hook height: the hook at the end of the line points towards the staff; a " +
                        "negative value turns it away.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Offset"),
                qsTrc("notation/engravingoptions", "Hook height")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Above the staff"),
                qsTrc("notation/engravingoptions", "Below the staff")
            ]

            EoOffset {
                sid: "ottavaPosAbove"
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and height of an ottava line placed above, measured from the top staff " +
                            "line (negative is up); notes and signs in the way still push it further out.")
            }

            EoOffset {
                sid: "ottavaPosBelow"
                music: "e'4 \\startOttava 8vb c'4 b4 c'4 | e'2 \\stopOttava g'2"
                hint: qsTrc("notation/engravingoptions",
                            "Horizontal shift and depth of an ottava line placed below, measured from the bottom " +
                            "staff line (positive is down); notes and signs in the way still push it further out.")
            }

            EoNumber {
                sid: "ottavaHookAbove"
                hint: qsTrc("notation/engravingoptions",
                            "Length of the hook at the end of an ottava line above the staff; it points down towards " +
                            "the staff, a negative value turns it upwards.")
                min: -99
                step: 1.0
            }

            EoNumber {
                sid: "ottavaHookBelow"
                music: "e'4 \\startOttava 8vb c'4 b4 c'4 | e'2 \\stopOttava g'2"
                hint: qsTrc("notation/engravingoptions",
                            "Length of the hook at the end of an ottava line below the staff; it points up towards " +
                            "the staff, a negative value turns it downwards.")
                min: -99
                step: 1.0
            }
        }

        EoNumber {
            sid: "ottavaLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            step: 0.01
        }

        EoChoice {
            sid: "ottavaLineStyle"
            title: qsTrc("EditStyleBase", "Line style:")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Solid") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Dashed") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Dotted") }
            ]
        }

        EoNumber {
            sid: "ottavaDashLineLen"
            title: qsTrc("notation/engravingoptions", "Dash length")
            hint: qsTrc("notation/engravingoptions",
                        "Length of each dash as a multiple of the line thickness. Only with the line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("ottavaLineStyle") === 1
        }

        EoNumber {
            sid: "ottavaDashGapLen"
            title: qsTrc("notation/engravingoptions", "Gap length")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between two dashes as a multiple of the line thickness; it is evened out slightly so " +
                        "that the line ends with a full dash. Only with the line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("ottavaLineStyle") === 1
        }

        EoToggle {
            sid: "ottavaExtendToEndOfDuration"
            title: qsTrc("notation/engravingoptions", "Extend line to end of last note’s duration")
            hint: qsTrc("notation/engravingoptions",
                        "Off: the line ends just past the last note it covers. On: it runs on to just before the " +
                        "next note or barline.")
        }

        EoNumber {
            sid: "ottavaEndOvershoot"
            title: qsTrc("notation/engravingoptions", "Line extension past last note")
            hint: qsTrc("notation/engravingoptions",
                        "How far the line continues past the right edge of the last note it covers, never beyond the " +
                        "next note. Only with ‘Extend line to end of last note’s duration’ off.")
            max: 5
        }

        EoChoice {
            sid: "ottavaTextAlignBelow"
            music: "e'4 \\startOttava 8vb c'4 b4 c'4 | e'2 \\stopOttava g'2"
            title: qsTrc("notation/engravingoptions", "Text alignment below the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Which part of the label of an ottava below the staff lies on the line: with ‘Baseline’ the " +
                        "label stands on the line and the hook rises beside it. For ottavas above the staff this is " +
                        "the alignment of the text style ‘Ottava’ (‘Top’: the label hangs from the line).")
            choices: [
                { value: "left,top", title: qsTrc("notation/engravingoptions", "Top") },
                { value: "left,center", title: qsTrc("notation/engravingoptions", "Middle") },
                { value: "left,bottom", title: qsTrc("notation/engravingoptions", "Bottom") },
                { value: "left,baseline", title: qsTrc("notation/engravingoptions", "Baseline") }
            ]
        }

        EoNumber {
            sid: "ottavaMinDistance"
            music: "a''4 \\startOttava 8va e'''4 c'''4 e'''4 \\stopOttava | c''1"
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between an ottava line and the notes or other signs which lie between it " +
                        "and the staff.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Texts and placement")

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "For each kind of ottava: its side of the staff, the label at the start of the line and the " +
                        "label repeated at the start of each following system, as they are while ‘Numbers only’ is " +
                        "on (the default). A symbol of the music font is written as <sym>name</sym>, for example " +
                        "<sym>ottava</sym> for 8.")
            cornerTitle: qsTrc("notation/engravingoptions", "Kind")
            rowTitles: [
                qsTrc("notation/engravingoptions", "8va"),
                qsTrc("notation/engravingoptions", "8vb"),
                qsTrc("notation/engravingoptions", "15ma"),
                qsTrc("notation/engravingoptions", "15mb"),
                qsTrc("notation/engravingoptions", "22ma"),
                qsTrc("notation/engravingoptions", "22mb")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Position"),
                qsTrc("notation/engravingoptions", "Text"),
                qsTrc("notation/engravingoptions", "Text on following systems")
            ]

            EoChoice {
                sid: "ottava8VAPlacement"
                music: "d''4 \\startOttava 8va f''4 a''4 f''4 | d''2 \\stopOttava b'2"
                hint: qsTrc("notation/engravingoptions",
                            "Side of the staff on which 8va lines are placed, unless one is flipped by hand. " +
                            "Rasgueado, whammy bar and harmonic mark lines follow this value too.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
                ]
            }

            EoText {
                sid: "ottava8VAnoText"
                music: "d''4 \\startOttava 8va f''4 a''4 f''4 | d''2 \\stopOttava b'2"
                hint: qsTrc("notation/engravingoptions",
                            "Label at the start of an 8va line while ‘Numbers only’ is on.")
            }

            EoText {
                sid: "ottava8VAnoContinueText"
                music: "d''4 \\startOttava 8va f''4 a''4 f''4 \\break d''2 \\stopOttava b'2"
                systemWidth: 16
                hint: qsTrc("notation/engravingoptions",
                            "Label repeated at the start of each following system of an 8va line while ‘Numbers " +
                            "only’ is on.")
            }

            EoChoice {
                sid: "ottava8VBPlacement"
                music: "e'4 \\startOttava 8vb c'4 b4 c'4 | e'2 \\stopOttava g'2"
                hint: qsTrc("notation/engravingoptions",
                            "Side of the staff on which 8vb lines are placed, unless one is flipped by hand.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
                ]
            }

            EoText {
                sid: "ottava8VBnoText"
                music: "e'4 \\startOttava 8vb c'4 b4 c'4 | e'2 \\stopOttava g'2"
                hint: qsTrc("notation/engravingoptions",
                            "Label at the start of an 8vb line while ‘Numbers only’ is on.")
            }

            EoText {
                sid: "ottava8VBnoContinueText"
                music: "e'4 \\startOttava 8vb c'4 b4 c'4 \\break e'2 \\stopOttava g'2"
                systemWidth: 16
                hint: qsTrc("notation/engravingoptions",
                            "Label repeated at the start of each following system of an 8vb line while ‘Numbers " +
                            "only’ is on.")
            }

            EoChoice {
                sid: "ottava15MAPlacement"
                music: "d''4 \\startOttava 15ma f''4 a''4 f''4 | d''2 \\stopOttava b'2"
                hint: qsTrc("notation/engravingoptions",
                            "Side of the staff on which 15ma lines are placed, unless one is flipped by hand.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
                ]
            }

            EoText {
                sid: "ottava15MAnoText"
                music: "d''4 \\startOttava 15ma f''4 a''4 f''4 | d''2 \\stopOttava b'2"
                hint: qsTrc("notation/engravingoptions",
                            "Label at the start of a 15ma line while ‘Numbers only’ is on.")
            }

            EoText {
                sid: "ottava15MAnoContinueText"
                music: "d''4 \\startOttava 15ma f''4 a''4 f''4 \\break d''2 \\stopOttava b'2"
                systemWidth: 16
                hint: qsTrc("notation/engravingoptions",
                            "Label repeated at the start of each following system of a 15ma line while ‘Numbers " +
                            "only’ is on.")
            }

            EoChoice {
                sid: "ottava15MBPlacement"
                music: "e'4 \\startOttava 15mb c'4 b4 c'4 | e'2 \\stopOttava g'2"
                hint: qsTrc("notation/engravingoptions",
                            "Side of the staff on which 15mb lines are placed, unless one is flipped by hand.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
                ]
            }

            EoText {
                sid: "ottava15MBnoText"
                music: "e'4 \\startOttava 15mb c'4 b4 c'4 | e'2 \\stopOttava g'2"
                hint: qsTrc("notation/engravingoptions",
                            "Label at the start of a 15mb line while ‘Numbers only’ is on.")
            }

            EoText {
                sid: "ottava15MBnoContinueText"
                music: "e'4 \\startOttava 15mb c'4 b4 c'4 \\break e'2 \\stopOttava g'2"
                systemWidth: 16
                hint: qsTrc("notation/engravingoptions",
                            "Label repeated at the start of each following system of a 15mb line while ‘Numbers " +
                            "only’ is on.")
            }

            EoChoice {
                sid: "ottava22MAPlacement"
                music: "d''4 \\startOttava 22ma f''4 a''4 f''4 | d''2 \\stopOttava b'2"
                hint: qsTrc("notation/engravingoptions",
                            "Side of the staff on which 22ma lines are placed, unless one is flipped by hand.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
                ]
            }

            EoText {
                sid: "ottava22MAnoText"
                music: "d''4 \\startOttava 22ma f''4 a''4 f''4 | d''2 \\stopOttava b'2"
                hint: qsTrc("notation/engravingoptions",
                            "Label at the start of a 22ma line while ‘Numbers only’ is on.")
            }

            EoText {
                sid: "ottava22MAnoContinueText"
                music: "d''4 \\startOttava 22ma f''4 a''4 f''4 \\break d''2 \\stopOttava b'2"
                systemWidth: 16
                hint: qsTrc("notation/engravingoptions",
                            "Label repeated at the start of each following system of a 22ma line while ‘Numbers " +
                            "only’ is on.")
            }

            EoChoice {
                sid: "ottava22MBPlacement"
                music: "e'4 \\startOttava 22mb c'4 b4 c'4 | e'2 \\stopOttava g'2"
                hint: qsTrc("notation/engravingoptions",
                            "Side of the staff on which 22mb lines are placed, unless one is flipped by hand.")
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
                ]
            }

            EoText {
                sid: "ottava22MBnoText"
                music: "e'4 \\startOttava 22mb c'4 b4 c'4 | e'2 \\stopOttava g'2"
                hint: qsTrc("notation/engravingoptions",
                            "Label at the start of a 22mb line while ‘Numbers only’ is on.")
            }

            EoText {
                sid: "ottava22MBnoContinueText"
                music: "e'4 \\startOttava 22mb c'4 b4 c'4 \\break e'2 \\stopOttava g'2"
                systemWidth: 16
                hint: qsTrc("notation/engravingoptions",
                            "Label repeated at the start of each following system of a 22mb line while ‘Numbers " +
                            "only’ is on.")
            }
        }

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Texts while ‘Numbers only’ is off")
            hint: qsTrc("notation/engravingoptions",
                        "The labels written out in full, as <sym>ottavaAlta</sym> for 8va.")
            cornerTitle: qsTrc("notation/engravingoptions", "Kind")
            rowTitles: [
                qsTrc("notation/engravingoptions", "8va"),
                qsTrc("notation/engravingoptions", "8vb"),
                qsTrc("notation/engravingoptions", "15ma"),
                qsTrc("notation/engravingoptions", "15mb"),
                qsTrc("notation/engravingoptions", "22ma"),
                qsTrc("notation/engravingoptions", "22mb")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Text"),
                qsTrc("notation/engravingoptions", "Text on following systems")
            ]

            EoText {
                sid: "ottava8VAText"
                music: "d''4 \\startOttava 8va f''4 a''4 f''4 | d''2 \\stopOttava b'2"
                musicStyle: ({ ottavaNumbersOnly: false })
                hint: qsTrc("notation/engravingoptions",
                            "Label at the start of an 8va line while ‘Numbers only’ is off.")
            }

            EoText {
                sid: "ottava8VAContinueText"
                music: "d''4 \\startOttava 8va f''4 a''4 f''4 \\break d''2 \\stopOttava b'2"
                systemWidth: 16
                musicStyle: ({ ottavaNumbersOnly: false })
                hint: qsTrc("notation/engravingoptions",
                            "Label repeated at the start of each following system of an 8va line while ‘Numbers " +
                            "only’ is off.")
            }

            EoText {
                sid: "ottava8VBText"
                music: "e'4 \\startOttava 8vb c'4 b4 c'4 | e'2 \\stopOttava g'2"
                musicStyle: ({ ottavaNumbersOnly: false })
                hint: qsTrc("notation/engravingoptions",
                            "Label at the start of an 8vb line while ‘Numbers only’ is off.")
            }

            EoText {
                sid: "ottava8VBContinueText"
                music: "e'4 \\startOttava 8vb c'4 b4 c'4 \\break e'2 \\stopOttava g'2"
                systemWidth: 16
                musicStyle: ({ ottavaNumbersOnly: false })
                hint: qsTrc("notation/engravingoptions",
                            "Label repeated at the start of each following system of an 8vb line while ‘Numbers " +
                            "only’ is off.")
            }

            EoText {
                sid: "ottava15MAText"
                music: "d''4 \\startOttava 15ma f''4 a''4 f''4 | d''2 \\stopOttava b'2"
                musicStyle: ({ ottavaNumbersOnly: false })
                hint: qsTrc("notation/engravingoptions",
                            "Label at the start of a 15ma line while ‘Numbers only’ is off.")
            }

            EoText {
                sid: "ottava15MAContinueText"
                music: "d''4 \\startOttava 15ma f''4 a''4 f''4 \\break d''2 \\stopOttava b'2"
                systemWidth: 16
                musicStyle: ({ ottavaNumbersOnly: false })
                hint: qsTrc("notation/engravingoptions",
                            "Label repeated at the start of each following system of a 15ma line while ‘Numbers " +
                            "only’ is off.")
            }

            EoText {
                sid: "ottava15MBText"
                music: "e'4 \\startOttava 15mb c'4 b4 c'4 | e'2 \\stopOttava g'2"
                musicStyle: ({ ottavaNumbersOnly: false })
                hint: qsTrc("notation/engravingoptions",
                            "Label at the start of a 15mb line while ‘Numbers only’ is off.")
            }

            EoText {
                sid: "ottava15MBContinueText"
                music: "e'4 \\startOttava 15mb c'4 b4 c'4 \\break e'2 \\stopOttava g'2"
                systemWidth: 16
                musicStyle: ({ ottavaNumbersOnly: false })
                hint: qsTrc("notation/engravingoptions",
                            "Label repeated at the start of each following system of a 15mb line while ‘Numbers " +
                            "only’ is off.")
            }

            EoText {
                sid: "ottava22MAText"
                music: "d''4 \\startOttava 22ma f''4 a''4 f''4 | d''2 \\stopOttava b'2"
                musicStyle: ({ ottavaNumbersOnly: false })
                hint: qsTrc("notation/engravingoptions",
                            "Label at the start of a 22ma line while ‘Numbers only’ is off.")
            }

            EoText {
                sid: "ottava22MAContinueText"
                music: "d''4 \\startOttava 22ma f''4 a''4 f''4 \\break d''2 \\stopOttava b'2"
                systemWidth: 16
                musicStyle: ({ ottavaNumbersOnly: false })
                hint: qsTrc("notation/engravingoptions",
                            "Label repeated at the start of each following system of a 22ma line while ‘Numbers " +
                            "only’ is off.")
            }

            EoText {
                sid: "ottava22MBText"
                music: "e'4 \\startOttava 22mb c'4 b4 c'4 | e'2 \\stopOttava g'2"
                musicStyle: ({ ottavaNumbersOnly: false })
                hint: qsTrc("notation/engravingoptions",
                            "Label at the start of a 22mb line while ‘Numbers only’ is off.")
            }

            EoText {
                sid: "ottava22MBContinueText"
                music: "e'4 \\startOttava 22mb c'4 b4 c'4 \\break e'2 \\stopOttava g'2"
                systemWidth: 16
                musicStyle: ({ ottavaNumbersOnly: false })
                hint: qsTrc("notation/engravingoptions",
                            "Label repeated at the start of each following system of a 22mb line while ‘Numbers " +
                            "only’ is off.")
            }
        }
    }
}
