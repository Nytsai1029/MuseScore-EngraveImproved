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

    code: "spacing"
    legacyCodes: ["spacing"]
    title: qsTrc("EditStyleBase", "Horizontal spacing")
    group: qsTrc("notation/engravingoptions", "Layout")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Note spacing")

        EoNumber {
            sid: "spacingDensity"
            music: "c''2 c''4 c''8 c''16 c''16"
            title: qsTrc("EditStyleBase", "System density:")
            hint: qsTrc("notation/engravingoptions",
                        "How tightly the notes are packed before each system is stretched out to the right margin: " +
                        "higher values put more measures on a system, lower values fewer.")
            min: 10
            max: 1000
            step: 1.0
            decimals: 1
            percent: true
        }

        EoNumber {
            sid: "measureSpacing"
            music: "c''2 c''4 c''8 c''16 c''16"
            title: qsTrc("EditStyleBase", "Spacing ratio:")
            hint: qsTrc("notation/engravingoptions",
                        "How much more room a note gets when its value doubles: with 1.5 a half note takes one and a " +
                        "half times the space of a quarter note, with 2 twice as much.")
            min: 1
            max: 2
            step: 0.1
            decimals: 3
        }

        EoNumber {
            sid: "spacingQuarterNoteSpace"
            music: "c''4 d'' e'' f''"
            title: qsTrc("notation/engravingoptions", "Space for a quarter note")
            hint: qsTrc("notation/engravingoptions",
                        "The width a quarter note is given before the system is stretched or compressed to the line; " +
                        "the other note values follow from it by the spacing ratio.")
            min: 0.5
            max: 10
            step: 0.1
        }

        EoNumber {
            sid: "minNoteDistance"
            music: "c''32[ c'' c'' c''] c''32[ c'' c'' c'']"
            title: qsTrc("EditStyleBase", "Note to note:")
            hint: qsTrc("notation/engravingoptions",
                        "The least gap between consecutive notes, and between a note and the stem of the next; it is " +
                        "what keeps very short notes apart in a crowded system.")
            step: 0.1
        }

        EoNumber {
            sid: "minMeasureWidth"
            music: "\\time 1/4 c''4 | c''4"
            title: qsTrc("EditStyleBase", "Minimum measure width:")
            hint: qsTrc("notation/engravingoptions",
                        "No full measure is narrower than this, however little it holds; a shorter measure, such as " +
                        "a pickup, gets a proportional part of it.")
            min: 2
            step: 1.0
        }

        EoNumber {
            sid: "lastSystemFillLimit"
            music: "c''4 d''4 e''4"
            systemWidth: 38
            title: qsTrc("EditStyleBase", "Last system fill threshold:")
            hint: qsTrc("notation/engravingoptions",
                        "The last system of the score, or of a section, is stretched to the right margin only if its " +
                        "music already fills at least this much of the width; otherwise it ends where the music " +
                        "ends.")
            max: 100
            step: 1
            decimals: 0
            percent: true
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Japanese-style spacing")

        EoToggle {
            sid: "useAdaptiveSpacingRatio"
            music: "c''1 | c''2 c''4 c''4"
            title: qsTrc("EditStyleBase", "Vary spacing ratio by note density of each system")
            hint: qsTrc("notation/engravingoptions",
                        "Every system gets its own spacing ratio, between the two set below: a system crowded with " +
                        "notes the dense-line ratio, which leaves more room for short notes, an open system the " +
                        "sparse-line ratio.")
        }

        EoNumber {
            sid: "spacingRatioDense"
            music: "c''16 c'' c'' c'' c''8 c'' c''4 c''2"
            systemWidth: 30
            musicStyle: ({ useAdaptiveSpacingRatio: true })
            title: qsTrc("EditStyleBase", "Dense-line spacing ratio:")
            hint: qsTrc("notation/engravingoptions",
                        "The spacing ratio of a system crowded with notes; a smaller ratio gives the short notes a " +
                        "larger share of the line.")
            min: 1
            max: 2
            step: 0.1
            decimals: 3
        }

        EoNumber {
            sid: "spacingRatioDenseThreshold"
            music: "c''2 c''4 c''8 c''8"
            systemWidth: 24
            musicStyle: ({ useAdaptiveSpacingRatio: true })
            title: qsTrc("notation/engravingoptions", "Note density of a dense line")
            hint: qsTrc("notation/engravingoptions",
                        "From this many notes for every staff space of its width, a system gets the full dense-line " +
                        "ratio: 0.4 is a note every 2.5 staff spaces (a line of sixteenth notes is about 0.5). Notes " +
                        "struck together count once; keep the value above that of a sparse line.")
            min: 0.31
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "spacingRatioSparse"
            music: "c''1 | c''2 c''4 c''4"
            musicStyle: ({ useAdaptiveSpacingRatio: true })
            title: qsTrc("EditStyleBase", "Sparse-line spacing ratio:")
            hint: qsTrc("notation/engravingoptions",
                        "The spacing ratio of a system with few notes; a larger ratio spreads the long notes more " +
                        "nearly in proportion to their value.")
            min: 1
            max: 2
            step: 0.1
            decimals: 3
        }

        EoNumber {
            sid: "spacingRatioSparseThreshold"
            music: "c''1 | c''2 c''2"
            musicStyle: ({ useAdaptiveSpacingRatio: true })
            title: qsTrc("notation/engravingoptions", "Note density of a sparse line")
            hint: qsTrc("notation/engravingoptions",
                        "Up to this many notes for every staff space of its width, a system gets the full " +
                        "sparse-line ratio: 0.1 is a note every ten staff spaces (a line of whole notes is about " +
                        "0.08). Between the two values the ratio changes gradually.")
            min: 0.01
            max: 0.3
            step: 0.01
        }

        EoToggle {
            sid: "keepEqualDurationSpacing"
            music: "<< { c''4. d''8 c''4. d''8 } \\\\ { e'4 f'4 e'4 f'4 } >>"
            title: qsTrc("EditStyleBase", "Keep equal spacing for equal note values (per system)")
            hint: qsTrc("notation/engravingoptions",
                        "Notes of the same value get the same width everywhere in a system, also where the voices " +
                        "move in different rhythms, such as triplets against eighth notes.")
        }

        EoToggle {
            sid: "crossStaffBeamSpacingOffset"
            music: "\\staves 2 c''8[ \\crossStaff down e'8 \\crossStaff off c''8 \\crossStaff down e'8]"
            title: qsTrc("EditStyleBase", "Offset spacing of cross-staff beamed notes")
            hint: qsTrc("notation/engravingoptions",
                        "Under a beam which crosses between staves, a stem-up note followed by a stem-down note is " +
                        "set a notehead’s width further apart, and the opposite pair as much closer, so that the " +
                        "stems rather than the noteheads are evenly spaced.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Start of the system")

        EoNumber {
            sid: "systemHeaderDistance"
            music: "c''4 d''4"
            title: qsTrc("EditStyleBase", "Clef/key signature to first note:")
            hint: qsTrc("notation/engravingoptions",
                        "The distance from the clef or key signature which begins the system to the first notehead; " +
                        "an accidental before that note reaches into this space.")
            step: 0.1
        }

        EoNumber {
            sid: "systemHeaderTimeSigDistance"
            music: "\\time 2/4 c''4 d''4"
            title: qsTrc("EditStyleBase", "Time signature to first note:")
            hint: qsTrc("notation/engravingoptions",
                        "The distance from the time signature which begins the system to the first notehead; an " +
                        "accidental before that note reaches into this space.")
            step: 0.1
        }

        EoNumber {
            sid: "systemHeaderMinStartOfSystemDistance"
            music: "c''1 \\break c''4 d''4 e''4 f''4"
            systemWidth: 16
            musicStyle: ({ genClef: false })
            title: qsTrc("EditStyleBase", "Start of the system to first note:")
            hint: qsTrc("notation/engravingoptions",
                        "The first note stands at least this far from the left end of the staff; it shows where a " +
                        "system begins without clef or key signature.")
            step: 1.0
        }

        EoNumber {
            sid: "systemHeaderMinClearDistance"
            music: "\\key 3 <ceses'' eses''>4 d''4"
            title: qsTrc("notation/engravingoptions", "Free space after the clef, key or time signature")
            hint: qsTrc("notation/engravingoptions",
                        "At least this much stays empty between the clef, key or time signature which begins the " +
                        "system and whatever comes first after it: the accidentals of the first note, or a " +
                        "start-repeat barline.")
            max: 5
        }

        EoNumber {
            sid: "headerToLineStartDistance"
            music: "c''2~ | \\break c''2"
            systemWidth: 16
            title: qsTrc("EditStyleBase", "System header to continued tie or line:")
            hint: qsTrc("notation/engravingoptions",
                        "Where a tie, slur or other line carried over from the previous system begins: this far " +
                        "after the clef, key or time signature.")
            step: 0.1
        }

        EoNumber {
            sid: "barlineToLineStartDistance"
            music: "c''2~ | \\volta \"1.\" 1 c''2 \\bar \":|.\" \\volta \"2.\" 1 c''2 \\partialTie"
            title: qsTrc("EditStyleBase", "Barline to continued tie or line:")
            hint: qsTrc("notation/engravingoptions",
                        "Where a tie or line begins which comes out of a barline inside a system, as after a repeat " +
                        "barline or at the start of a second ending.")
            step: 0.1
        }

        EoNumber {
            sid: "clefLeftMargin"
            music: "c''4 d''4"
            title: qsTrc("EditStyleBase", "Barline to clef:")
            hint: qsTrc("notation/engravingoptions",
                        "The distance from the left end of the staff to the clef at the start of every system; also " +
                        "the gap before a clef which stands after a barline.")
            step: 0.1
        }

        EoNumber {
            sid: "keysigLeftMargin"
            music: "c''4 | \\key 2 d''4"
            title: qsTrc("EditStyleBase", "Barline to key signature:")
            hint: qsTrc("notation/engravingoptions",
                        "The gap before a key signature which follows a barline: a change of key, or the courtesy " +
                        "key signature at the end of a system.")
            step: 0.1
        }

        EoNumber {
            sid: "timesigLeftMargin"
            music: "c''4 | \\time 3/4 d''4"
            title: qsTrc("EditStyleBase", "Barline to time signature:")
            hint: qsTrc("notation/engravingoptions",
                        "The gap before a time signature which follows a barline: a change of time, or the courtesy " +
                        "time signature at the end of a system.")
            step: 0.1
        }

        EoNumber {
            sid: "clefKeyRightMargin"
            music: "c''8 \\clef bass e8 d8 c8"
            title: qsTrc("EditStyleBase", "Clef to note:")
            hint: qsTrc("notation/engravingoptions",
                        "The least gap between any clef and the note or rest after it. At the start of a system the " +
                        "larger ‘Clef/key signature to first note’ rules, so this shows at clef changes.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "End of the system")

        EoNumber {
            sid: "lineEndToBarlineDistance"
            music: "c''2 \\startOttava 8va c''2 | \\break c''2 \\stopOttava c''2"
            systemWidth: 16
            title: qsTrc("EditStyleBase", "Continued tie or line to barline:")
            hint: qsTrc("notation/engravingoptions",
                        "Where a tie, slur or other line which goes on in the next system ends: this far before the " +
                        "last barline.")
            step: 0.1
        }

        EoNumber {
            sid: "systemTrailerRightMargin"
            music: "c''2 | \\break \\key 2 d''2"
            systemWidth: 16
            title: qsTrc("EditStyleBase", "Right margin after courtesy signatures:")
            hint: qsTrc("notation/engravingoptions",
                        "The space left after a courtesy clef, key or time signature at the end of a system, before " +
                        "the right end of the staff.")
            step: 0.1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Padding between items")
        hint: qsTrc("notation/engravingoptions",
                    "The smallest gap between an item (row) and the item which follows it on its right (column). " +
                    "Small notes get proportionally less, and so does everything in a system which is compressed to " +
                    "fit the line.")

        EoNote {
            title: qsTrc("notation/engravingoptions",
                         "The smallest gap between an item (row) and the item which follows it (column), in staff " +
                         "spaces.")
        }

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "The ‘Accidental’ column also applies to a fingering which stands before its note.")
            cornerTitle: qsTrc("notation/engravingoptions", "From ↓ to →")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Note"),
                qsTrc("notation/engravingoptions", "Flag"),
                qsTrc("notation/engravingoptions", "Dot"),
                qsTrc("notation/engravingoptions", "Rest"),
                qsTrc("notation/engravingoptions", "Stem"),
                qsTrc("notation/engravingoptions", "Clef"),
                qsTrc("notation/engravingoptions", "Barline"),
                qsTrc("notation/engravingoptions", "Key signature"),
                qsTrc("notation/engravingoptions", "Time signature")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Note"),
                qsTrc("notation/engravingoptions", "Accidental"),
                qsTrc("notation/engravingoptions", "Rest"),
                qsTrc("notation/engravingoptions", "Stem"),
                qsTrc("notation/engravingoptions", "Clef"),
                qsTrc("notation/engravingoptions", "Key signature"),
                qsTrc("notation/engravingoptions", "Time signature"),
                qsTrc("notation/engravingoptions", "Barline"),
                qsTrc("notation/engravingoptions", "Arpeggio")
            ]
            compact: true

            EoGap {
            }

            EoNumber {
                sid: "noteAccidentalDistance"
                music: "\\stemDown c''16 cis''16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Note to accidental")
                hint: qsTrc("notation/engravingoptions",
                            "The gap between a note and the accidental of the next note; if the distance set between " +
                            "an accidental and its own notehead is larger, that is used.")
                max: 5
            }

            EoNumber {
                sid: "noteRestDistance"
                music: "\\stemDown c''16 r16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Note to rest")
                hint: qsTrc("notation/engravingoptions", "Also used from a rest to a note and between two rests.")
                max: 5
            }

            EoGap {
            }

            EoNumber {
                sid: "noteClefDistance"
                music: "\\stemDown c''16 \\clef bass e16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Note to clef")
                hint: qsTrc("notation/engravingoptions", "Also used from a rest to a clef.")
                max: 5
            }

            EoNumber {
                sid: "noteKeyDistance"
                music: "\\key 2 a'4 | \\key 0 c''16 c''16 \\bar \":|.\" c''4"
                musicStyle: ({ useParensRepeatCourtesies: false, showCourtesiesAfterCancellingRepeats: false })
                title: qsTrc("notation/engravingoptions", "Note to key signature")
                hint: qsTrc("notation/engravingoptions",
                            "Seen where a key signature follows a note with no barline between them, as a courtesy " +
                            "key signature before a repeat barline does.")
                max: 5
            }

            EoNumber {
                sid: "noteTimesigDistance"
                music: "\\time 3/4 c''2. | \\time 1/4 c''16[ c''16 c''16 c''16] \\bar \":|.\" c''4"
                musicStyle: ({ useParensRepeatCourtesies: false, showCourtesiesAfterCancellingRepeats: false })
                title: qsTrc("notation/engravingoptions", "Note to time signature")
                hint: qsTrc("notation/engravingoptions",
                            "Seen where a time signature follows a note with no barline between them, as a courtesy " +
                            "time signature before a repeat barline does.")
                max: 5
            }

            EoNumber {
                sid: "noteBarDistance"
                music: "\\stemDown c''16 | c''4"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("EditStyleBase", "Note to barline:")
                hint: qsTrc("notation/engravingoptions", "The room left after the last note of a measure.")
                step: 0.1
            }

            EoNumber {
                sid: "noteArpeggioDistance"
                music: "\\stemDown c''16 <c'' e''>16 \\arpeggio"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Note to arpeggio")
                max: 5
            }

            EoNumber {
                sid: "flagNoteDistance"
                music: "\\stemUp \\noBeam g'8 c''16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Flag to note")
                hint: qsTrc("notation/engravingoptions", "Also used from a flag to a rest.")
                max: 5
            }

            EoNumber {
                sid: "flagAccidentalDistance"
                music: "\\stemUp \\noBeam g'8 cis''16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Flag to accidental")
                max: 5
            }

            EoGap {
            }

            EoGap {
            }

            EoNumber {
                sid: "flagClefDistance"
                music: "\\stemUp \\noBeam g'8 \\clef bass e16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Flag to clef")
                max: 5
            }

            EoNumber {
                sid: "flagKeyDistance"
                music: "\\stemUp \\noBeam g'8 | \\key 2 c''16"
                musicStyle: ({ flagBarlineDistance: 0.0, keysigLeftMargin: 0.0, spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Flag to key signature")
                max: 5
            }

            EoNumber {
                sid: "flagTimesigDistance"
                music: "\\stemUp \\noBeam g'8 | \\time 3/4 c''16"
                musicStyle: ({ flagBarlineDistance: 0.0, timesigLeftMargin: 0.0, spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Flag to time signature")
                max: 5
            }

            EoNumber {
                sid: "flagBarlineDistance"
                music: "\\stemUp \\noBeam g'8 | c''4"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Flag to barline")
                max: 5
            }

            EoNumber {
                sid: "flagArpeggioDistance"
                music: "\\stemUp \\noBeam g'8 <c'' e''>16 \\arpeggio"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Flag to arpeggio")
                max: 5
            }

            EoGap {
            }

            EoNumber {
                sid: "dotAccidentalDistance"
                music: "c''8. cis''16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Augmentation dot to accidental")
                max: 5
            }

            EoGap {
            }

            EoGap {
            }

            EoNumber {
                sid: "dotClefDistance"
                music: "c''8. \\clef bass e16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Augmentation dot to clef")
                max: 5
            }

            EoNumber {
                sid: "dotKeyDistance"
                music: "c''8. | \\key 2 c''16"
                musicStyle: ({ dotBarlineDistance: 0.0, keysigLeftMargin: 0.0, spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Augmentation dot to key signature")
                max: 5
            }

            EoNumber {
                sid: "dotTimesigDistance"
                music: "c''8. | \\time 3/4 c''16"
                musicStyle: ({ dotBarlineDistance: 0.0, timesigLeftMargin: 0.0, spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Augmentation dot to time signature")
                max: 5
            }

            EoNumber {
                sid: "dotBarlineDistance"
                music: "c''8. | c''4"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Augmentation dot to barline")
                max: 5
            }

            EoNumber {
                sid: "dotArpeggioDistance"
                music: "c''8. <c'' e''>16 \\arpeggio"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Augmentation dot to arpeggio")
                max: 5
            }

            EoGap {
            }

            EoNumber {
                sid: "restAccidentalDistance"
                music: "r16 cis''16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Rest to accidental")
                max: 5
            }

            EoGap {
            }

            EoGap {
            }

            EoGap {
            }

            EoNumber {
                sid: "restKeyDistance"
                music: "r16 | \\key 2 c''16"
                musicStyle: ({ restBarlineDistance: 0.0, keysigLeftMargin: 0.0, spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Rest to key signature")
                max: 5
            }

            EoNumber {
                sid: "restTimesigDistance"
                music: "r16 | \\time 3/4 c''16"
                musicStyle: ({ restBarlineDistance: 0.0, timesigLeftMargin: 0.0, spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Rest to time signature")
                max: 5
            }

            EoNumber {
                sid: "restBarlineDistance"
                music: "r16 | c''4"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Rest to barline")
                max: 5
            }

            EoGap {
            }

            EoGap {
            }

            EoNumber {
                sid: "stemAccidentalDistance"
                music: "\\stemUp a'16[ cis''16]"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Stem to accidental")
                max: 5
            }

            EoGap {
            }

            EoNumber {
                sid: "stemStemDistance"
                music: "\\stemUp e'4 \\stemDown f''4 \\stemUp e'4 \\stemDown f''4"
                musicStyle: ({ spacingQuarterNoteSpace: 1.0 })
                title: qsTrc("notation/engravingoptions", "Stem to stem")
                hint: qsTrc("notation/engravingoptions",
                            "The gap between the stems of neighbouring chords; it shows above all under beams which " +
                            "cross between staves.")
                max: 5
            }

            EoGap {
            }

            EoGap {
            }

            EoGap {
            }

            EoGap {
            }

            EoGap {
            }

            EoGap {
            }

            EoNumber {
                sid: "clefAccidentalDistance"
                music: "c''16 \\clef bass fis16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Clef to accidental")
                max: 5
            }

            EoGap {
            }

            EoNumber {
                sid: "clefStemDistance"
                music: "c''16 \\clef bass \\stemDown g16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Clef to stem")
                max: 5
            }

            EoNumber {
                sid: "clefClefDistance"
                music: "\\clef bass e2 | \\clef treble c''2 \\jump \"D.C.\" | c''2"
                musicStyle: ({
                    useParensOtherJumpCourtesies: false,
                    useParensOtherJumpCourtesiesAfterCancelling: false,
                    clefBarlineDistance: 0.0,
                    clefLeftMargin: 0.0
                })
                title: qsTrc("notation/engravingoptions", "Clef to clef")
                max: 5
            }

            EoNumber {
                sid: "clefKeyDistance"
                music: "c''4 | \\clef bass \\key 2 e4"
                musicStyle: ({ clefBarlineDistance: 0.0, keysigLeftMargin: 0.0 })
                title: qsTrc("EditStyleBase", "Clef to key signature:")
                step: 0.01
            }

            EoNumber {
                sid: "clefTimesigDistance"
                music: "c''4 | \\clef bass \\time 3/4 e4"
                musicStyle: ({ clefBarlineDistance: 0.0, timesigLeftMargin: 0.0 })
                title: qsTrc("EditStyleBase", "Clef to time signature:")
                step: 0.01
            }

            EoNumber {
                sid: "clefBarlineDistance"
                music: "c''16 \\clef bass | e4"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("EditStyleBase", "Clef to barline:")
                hint: qsTrc("notation/engravingoptions",
                            "The gap after a clef change which stands at the end of a measure.")
                step: 0.01
            }

            EoNumber {
                sid: "clefArpeggioDistance"
                music: "c''4 \\clef bass <e g>4 \\arpeggio"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Clef to arpeggio")
                max: 5
            }

            EoNumber {
                sid: "barNoteDistance"
                music: "c''16 | c''16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("EditStyleBase", "Barline to note:")
                hint: qsTrc("notation/engravingoptions",
                            "The room left before the first note of a measure; also used from a barline to a rest.")
                step: 0.1
            }

            EoNumber {
                sid: "barAccidentalDistance"
                music: "c''16 | cis''16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("EditStyleBase", "Barline to accidental:")
                hint: qsTrc("notation/engravingoptions",
                            "The gap between a barline and the accidental of the first note after it.")
                step: 0.1
            }

            EoGap {
            }

            EoGap {
            }

            EoGap {
            }

            EoGap {
            }

            EoGap {
            }

            EoNumber {
                sid: "barBarlineDistance"
                music: "c''4 | \\small r32 | c''4"
                musicStyle: ({
                    minMeasureWidth: 0.5,
                    barNoteDistance: 0.0,
                    restBarlineDistance: 0.0,
                    spacingQuarterNoteSpace: 0.5
                })
                title: qsTrc("notation/engravingoptions", "Barline to barline")
                hint: qsTrc("notation/engravingoptions",
                            "The least inner width of a measure whose content takes up no room.")
                max: 5
            }

            EoNumber {
                sid: "barArpeggioDistance"
                music: "c''16 | <c'' e''>16 \\arpeggio"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Barline to arpeggio")
                max: 5
            }

            EoNumber {
                sid: "keyNoteDistance"
                music: "c''16 | \\key 2 e''16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Key signature to note")
                hint: qsTrc("notation/engravingoptions",
                            "Also used from a key signature to a rest. At the start of a system the larger ‘Clef/key " +
                            "signature to first note’ rules.")
                max: 5
            }

            EoNumber {
                sid: "keyNoteAccidentalDistance"
                music: "c''16 | \\key 2 bes'16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Key signature to accidental")
                hint: qsTrc("notation/engravingoptions",
                            "The gap between a key signature and the accidental of the first note after it.")
                max: 5
            }

            EoGap {
            }

            EoGap {
            }

            EoNumber {
                sid: "keyClefDistance"
                music: "c''4 \\bar \":|.\" \\key 2 \\clef bass e4"
                musicStyle: ({ placeClefsBeforeRepeats: false })
                title: qsTrc("notation/engravingoptions", "Key signature to clef")
                max: 5
            }

            EoNumber {
                sid: "keyKeyDistance"
                music: "\\key 2 a'2 | \\key 0 c''2 \\jump \"D.C.\" | c''2"
                musicStyle: ({
                    useParensOtherJumpCourtesies: false,
                    useParensOtherJumpCourtesiesAfterCancelling: false,
                    keyBarlineDistance: 0.0,
                    keysigLeftMargin: 0.0
                })
                title: qsTrc("notation/engravingoptions", "Key signature to key signature")
                max: 5
            }

            EoNumber {
                sid: "keyTimesigDistance"
                music: "c''4 | \\key 2 \\time 3/4 c''4"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("EditStyleBase", "Key signature to time signature:")
                hint: qsTrc("notation/engravingoptions", "Also used from a time signature to a key signature.")
                step: 0.01
            }

            EoNumber {
                sid: "keyBarlineDistance"
                music: "\\key 2 a'2 | \\key 0 c''2 \\bar \":|.\" c''2"
                musicStyle: ({ useParensRepeatCourtesies: false, showCourtesiesAfterCancellingRepeats: false })
                title: qsTrc("EditStyleBase", "Key signature to barline:")
                hint: qsTrc("notation/engravingoptions",
                            "Seen where a barline follows a key signature: a start-repeat barline after a change of " +
                            "key, or the barline after a courtesy key signature at a repeat.")
                step: 0.01
            }

            EoNumber {
                sid: "keyArpeggioDistance"
                music: "c''16 | \\key 2 <c'' e''>16 \\arpeggio"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Key signature to arpeggio")
                max: 5
            }

            EoNumber {
                sid: "timesigNoteDistance"
                music: "c''16 | \\time 3/4 c''16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Time signature to note")
                hint: qsTrc("notation/engravingoptions",
                            "Also used from a time signature to a rest. At the start of a system the larger ‘Time " +
                            "signature to first note’ rules.")
                max: 5
            }

            EoNumber {
                sid: "timesigAccidentalDistance"
                music: "c''16 | \\time 3/4 cis''16"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Time signature to accidental")
                max: 5
            }

            EoGap {
            }

            EoGap {
            }

            EoNumber {
                sid: "timesigClefDistance"
                music: "c''4 \\bar \":|.\" \\time 3/4 \\clef bass e2."
                musicStyle: ({ placeClefsBeforeRepeats: false })
                title: qsTrc("notation/engravingoptions", "Time signature to clef")
                max: 5
            }

            EoGap {
            }

            EoNumber {
                sid: "timesigTimesigDistance"
                music: "\\time 3/4 c''2. | \\time 2/4 c''2 \\jump \"D.C.\" | c''2"
                musicStyle: ({
                    useParensOtherJumpCourtesies: false,
                    useParensOtherJumpCourtesiesAfterCancelling: false,
                    timesigBarlineDistance: 0.0,
                    timesigLeftMargin: 0.0
                })
                title: qsTrc("notation/engravingoptions", "Time signature to time signature")
                max: 5
            }

            EoNumber {
                sid: "timesigBarlineDistance"
                music: "\\time 3/4 c''2. | \\time 2/4 c''2 \\bar \":|.\" c''2"
                musicStyle: ({ useParensRepeatCourtesies: false, showCourtesiesAfterCancellingRepeats: false })
                title: qsTrc("EditStyleBase", "Time signature to barline:")
                hint: qsTrc("notation/engravingoptions",
                            "Seen where a barline follows a time signature: a start-repeat barline after a change of " +
                            "time, or the barline after a courtesy time signature at a repeat.")
                step: 0.1
            }

            EoNumber {
                sid: "timesigArpeggioDistance"
                music: "c''16 | \\time 3/4 <c'' e''>16 \\arpeggio"
                musicStyle: ({ spacingDensity: 1.0 })
                title: qsTrc("notation/engravingoptions", "Time signature to arpeggio")
                max: 5
            }
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Padding limits and kerning")

        EoNumber {
            sid: "spacingDefaultPadding"
            music: "\\stemDown e'4 \\stemUp f''4 \\stemDown e'4 \\stemUp f''4"
            musicStyle: ({ spacingQuarterNoteSpace: 1.0 })
            title: qsTrc("notation/engravingoptions", "Default padding between items")
            hint: qsTrc("notation/engravingoptions",
                        "The gap between two neighbouring items of kinds which have no value of their own in the " +
                        "table above; also between notes of different voices, which need not keep ‘Note to note’.")
            max: 2
        }

        EoNumber {
            sid: "spacingAbsoluteMinPadding"
            music: "a'16 r16 a'16 r16"
            musicStyle: ({ noteRestDistance: 0.0, spacingDensity: 1.0 })
            title: qsTrc("notation/engravingoptions", "Least padding between items")
            hint: qsTrc("notation/engravingoptions",
                        "No two neighbouring items come closer than this, whatever their own value says; like all " +
                        "padding it shrinks when a crowded system is compressed.")
            max: 2
        }

        EoNumber {
            sid: "kerningVerticalClearance"
            music: "\\stemUp \\noBeam g'8 \\stemDown f'16 \\stemUp \\noBeam g'8 \\stemDown f'16"
            musicStyle: ({ spacingDensity: 3.0 })
            title: qsTrc("notation/engravingoptions", "Vertical gap needed for items to tuck past each other")
            hint: qsTrc("notation/engravingoptions",
                        "Two neighbouring items at different heights, such as a low note after the flag of a high " +
                        "one, may overlap sideways only while this much vertical space stays between them; with " +
                        "less, they are set side by side.")
            max: 2
        }

        EoNumber {
            sid: "kerningVerticalClearanceAccidental"
            music: "\\stemUp c''16 \\stemDown ees'16 \\stemUp c''16 \\stemDown ees'16"
            title: qsTrc("notation/engravingoptions", "Vertical gap needed for an accidental to tuck in")
            hint: qsTrc("notation/engravingoptions",
                        "An accidental may slide above or below the item before it, a note or a flag, only while " +
                        "this much vertical space stays between the two.")
            max: 2
        }

        EoNumber {
            sid: "kerningVerticalClearanceParenthesis"
            music: "\\stemUp c''16 \\parenthesize \\stemDown a'16 \\stemUp c''16 \\parenthesize \\stemDown a'16"
            musicStyle: ({ spacingDensity: 2.0 })
            title: qsTrc("notation/engravingoptions", "Vertical gap needed for a parenthesis to tuck in")
            hint: qsTrc("notation/engravingoptions",
                        "A parenthesis and its neighbour may overlap sideways only while this much vertical space " +
                        "stays between them.")
            max: 2
        }
    }
}
