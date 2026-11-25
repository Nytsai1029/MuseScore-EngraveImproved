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

    code: "clefs-key-and-time-signatures"
    legacyCodes: ["clefs-key-and-time-signatures"]
    title: qsTrc("EditStyleBase", "Clefs, key & time signatures")
    group: qsTrc("notation/engravingoptions", "Signs")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Clefs")

        EoToggle {
            sid: "genClef"
            music: "c''1 \\break c''1"
            systemWidth: 16
            title: qsTrc("notation/editstyle/timesignatures", "Show all clefs on every system")
            hint: qsTrc("notation/engravingoptions",
                        "Off, a clef is shown only on the first system after it is set or changed, as in many jazz " +
                        "and studio parts.")
        }

        EoToggle {
            sid: "hideTabClefAfterFirst"
            music: "\\tab c'1 \\break c'1"
            systemWidth: 16
            title: qsTrc("notation/editstyle/timesignatures", "Hide tab clefs after the first system where they appear")
            hint: qsTrc("notation/engravingoptions",
                        "Tablature staves follow only this switch, whatever ‘Show all clefs on every system’ says.")
        }

        EoToggle {
            sid: "genCourtesyClef"
            music: "c''1 \\break \\clef bass c1"
            systemWidth: 16
            title: qsTrc("notation/editstyle/timesignatures", "Show courtesy clefs")
            hint: qsTrc("notation/engravingoptions",
                        "A change of clef at the start of a system is announced by a small clef at the end of the " +
                        "system before. Off also removes the courtesy clefs at repeats and jumps.")
        }

        EoChoice {
            sid: "placeClefsBeforeRepeats"
            music: "c''1 \\bar \":|.\" \\clef bass c1"
            title: qsTrc("notation/editstyle/timesignatures", "Place clefs before repeats")
            hint: qsTrc("notation/engravingoptions",
                        "Where a change of clef falls on an end repeat barline: whether the new clef stands before " +
                        "that barline or after it.")
            choices: [
                { value: true, title: qsTrc("notation/engravingoptions", "Before the repeat barline") },
                { value: false, title: qsTrc("notation/engravingoptions", "After the repeat barline") }
            ]
        }

        EoChoice {
            sid: "tabClef"
            music: "\\tab c'1"
            title: qsTrc("notation/editstyle/timesignatures", "Default TAB clef")
            hint: qsTrc("notation/engravingoptions", "The clef which tablature staves get by default.")
            presentation: "tiles"
            choices: [
                { value: 31, title: qsTrc("notation/engravingoptions", "Standard TAB clef") },
                { value: 33, title: qsTrc("notation/engravingoptions", "Serif TAB clef") }
            ]
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Key signatures")

        EoToggle {
            sid: "genKeysig"
            music: "\\key 3 a'1 \\break a'1"
            systemWidth: 18
            title: qsTrc("notation/editstyle/timesignatures", "Show on every system")
            hint: qsTrc("notation/engravingoptions",
                        "Off, the key signature is shown only on the first system after it is set or changed.")
        }

        EoToggle {
            sid: "genCourtesyKeysig"
            music: "c''1 \\break \\key 3 a'1"
            systemWidth: 18
            title: qsTrc("notation/editstyle/timesignatures", "Show courtesy key signatures")
            hint: qsTrc("notation/engravingoptions",
                        "A change of key at the start of a system is announced by a key signature at the end of the " +
                        "system before. Off also removes the courtesy key signatures at repeats and jumps.")
        }

        EoChoice {
            sid: "keySigCourtesyBarlineMode"
            music: "c''1 \\break \\key 3 a'1"
            systemWidth: 18
            title: qsTrc("EditStyleBase", "Use double barlines before key signatures")
            hint: qsTrc("notation/engravingoptions",
                        "Whether the barline in front of a change of key becomes a double barline.")
            presentation: "dropdown"
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Never") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Always") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Only before courtesy key signatures") }
            ]
        }

        EoChoice {
            sid: "keySigNaturals"
            music: "\\key 3 a'1 | \\key 1 b'1"
            title: qsTrc("notation/engravingoptions", "Naturals at a change of key")
            hint: qsTrc("notation/engravingoptions",
                        "The naturals which cancel sharps or flats of the old key: whether they are shown, and on " +
                        "which side of the new key signature.")
            presentation: "list"
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Only for a change to C major / A minor") },
                { value: 1, title: qsTrc("notation/engravingoptions",
                                         "Before the new key signature when it has fewer ♯ or ♭") },
                { value: 2, title: qsTrc("notation/engravingoptions",
                                         "After the new key signature when it has fewer ♯ or ♭; before it when " +
                                         "changing between ♯ and ♭") }
            ]
        }

        EoGrid {
            rowTitles: [
                qsTrc("notation/engravingoptions", "Distance between")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Sharps"),
                qsTrc("notation/engravingoptions", "Flats"),
                qsTrc("notation/engravingoptions", "Naturals")
            ]
            compact: true

            EoNumber {
                sid: "keysigSharpAccidentalDistance"
                music: "\\key 4 b'1"
                title: qsTrc("notation/editstyle/accidentals", "Distance between sharps:")
                hint: qsTrc("notation/engravingoptions",
                            "Gap between two neighbouring sharps of a key signature; a negative value pushes them " +
                            "together.")
                min: -10
                max: 10
                step: 0.1
                decimals: 3
            }

            EoNumber {
                sid: "keysigFlatAccidentalDistance"
                music: "\\key -4 c''1"
                title: qsTrc("notation/editstyle/accidentals", "Distance between flats:")
                hint: qsTrc("notation/engravingoptions",
                            "Gap between two neighbouring flats of a key signature; a negative value pushes them " +
                            "together.")
                min: -10
                max: 10
                step: 0.1
                decimals: 3
            }

            EoNumber {
                sid: "keysigNaturalDistance"
                music: "\\key 4 b'1 | \\key 0 b'1"
                title: qsTrc("notation/editstyle/accidentals", "Distance between naturals:")
                hint: qsTrc("notation/engravingoptions",
                            "Gap between two neighbouring naturals where a key signature is cancelled.")
                min: -10
                max: 10
                step: 0.1
                decimals: 3
            }
        }

        EoNumber {
            sid: "keysigMixedAccidentalDistanceScale"
            music: "\\key 4 b'1 | \\key 1 b'1"
            musicStyle: ({ keySigNaturals: 1 })
            title: qsTrc("notation/engravingoptions", "Distance where different accidentals meet")
            hint: qsTrc("notation/engravingoptions",
                        "Where the cancelling naturals meet the new sharps or flats, the distance is multiplied by " +
                        "this; 1 keeps the same distance as between like accidentals.")
            max: 5
            step: 0.1
        }

        EoToggle {
            sid: "keysigAccidentalsUseCutouts"
            music: "\\key 4 b'1"
            title: qsTrc("notation/engravingoptions", "Interlock the accidentals")
            hint: qsTrc("notation/engravingoptions",
                        "Sharps and naturals tuck into the notch of their neighbour where the music font provides " +
                        "one; off sets all accidentals side by side at their full width.")
        }

        EoNumber {
            sid: "keysigAccidentalDistance"
            music: "\\key 4 b'1 | \\key 1 b'1"
            musicStyle: ({ keySigNaturals: 2 })
            title: qsTrc("notation/engravingoptions", "Distance before a natural which follows sharps or flats")
            hint: qsTrc("notation/engravingoptions",
                        "Also the distance before an accidental of a custom key signature which is neither a sharp, " +
                        "a flat nor a natural. Where the two accidentals differ it is multiplied by ‘Distance where " +
                        "different accidentals meet’.")
            min: -10
            max: 10
            step: 0.1
            decimals: 3
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Time signatures")
        music: "\\time 3/4 c''2."

        EoToggle {
            sid: "genCourtesyTimesig"
            music: "\\time 4/4 c''1 \\break \\time 3/4 c''2."
            systemWidth: 16
            title: qsTrc("notation/editstyle/timesignatures", "Show courtesy time signatures")
            hint: qsTrc("notation/engravingoptions",
                        "A change of time signature at the start of a system is announced at the end of the system " +
                        "before. Off also removes the courtesy time signatures at repeats and jumps.")
        }

        EoChoice {
            sid: "timeSigCourtesyBarlineMode"
            music: "\\time 4/4 c''1 \\break \\time 3/4 c''2."
            systemWidth: 16
            title: qsTrc("EditStyleBase", "Use double barlines before time signatures")
            hint: qsTrc("notation/engravingoptions",
                        "Whether the barline in front of a change of time signature becomes a double barline.")
            presentation: "dropdown"
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Never") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Always") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Only before courtesy time signatures") }
            ]
        }

        EoChoice {
            sid: "timeSigPlacement"
            music: "\\instruments 2 \\time 3/4 c''2. | c''2."
            title: qsTrc("notation/editstyle/timesignatures", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "Above or across the staves, one large time signature serves a whole group of staves; the " +
                        "staves at which it appears are chosen in the Layout panel.")
            presentation: "tiles"
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "On all staves") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Above the staves") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Across the staves") }
            ]
        }

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Style and size")
            hint: qsTrc("notation/engravingoptions",
                        "Each position has its own settings; only the column of the chosen ‘Position’ shows in the " +
                        "score.")
            cornerTitle: qsTrc("notation/engravingoptions", "Position")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Numeral style"),
                qsTrc("notation/engravingoptions", "Scale"),
                qsTrc("notation/engravingoptions", "Gap between the numbers"),
                qsTrc("notation/engravingoptions", "Vertical position")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "On all staves"),
                qsTrc("notation/engravingoptions", "Above the staves"),
                qsTrc("notation/engravingoptions", "Across the staves")
            ]

            EoChoice {
                sid: "timeSigNormalStyle"
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Standard") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Narrow") },
                    { value: 2, title: qsTrc("notation/engravingoptions", "Large sans-serif") }
                ]
            }

            EoChoice {
                sid: "timeSigAboveStyle"
                musicStyle: ({ timeSigPlacement: 1 })
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Standard") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Narrow") },
                    { value: 2, title: qsTrc("notation/engravingoptions", "Large sans-serif") }
                ]
            }

            EoChoice {
                sid: "timeSigAcrossStyle"
                music: "\\instruments 2 \\time 3/4 c''2."
                musicStyle: ({ timeSigPlacement: 2 })
                choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Standard") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Narrow") },
                    { value: 2, title: qsTrc("notation/engravingoptions", "Large sans-serif") }
                ]
            }

            EoScale {
                sid: "timeSigNormalScale"
                lockSid: "timeSigNormalScaleLock"
                hint: qsTrc("notation/engravingoptions",
                            "Width and height of the numerals as multiples of their normal size; the lock between " +
                            "them keeps the two the same.")
            }

            EoScale {
                sid: "timeSigAboveScale"
                musicStyle: ({ timeSigPlacement: 1 })
                lockSid: "timeSigAboveScaleLock"
                hint: qsTrc("notation/engravingoptions",
                            "Width and height of the numerals as multiples of their normal size; the lock between " +
                            "them keeps the two the same.")
            }

            EoScale {
                sid: "timeSigAcrossScale"
                music: "\\instruments 2 \\time 3/4 c''2."
                musicStyle: ({ timeSigPlacement: 2 })
                lockSid: "timeSigAcrossScaleLock"
                hint: qsTrc("notation/engravingoptions",
                            "Width and height of the numerals as multiples of their normal size; the lock between " +
                            "them keeps the two the same.")
            }

            EoNumber {
                sid: "timeSigNormalNumDist"
                hint: qsTrc("notation/engravingoptions",
                            "Extra space between the upper and the lower number; it grows with the vertical scale.")
                min: -10
                max: 10
                step: 0.1
            }

            EoNumber {
                sid: "timeSigAboveNumDist"
                musicStyle: ({ timeSigPlacement: 1 })
                hint: qsTrc("notation/engravingoptions",
                            "Extra space between the upper and the lower number; it grows with the vertical scale.")
                min: -10
                max: 10
                step: 0.1
            }

            EoNumber {
                sid: "timeSigAcrossNumDist"
                music: "\\instruments 2 \\time 3/4 c''2."
                musicStyle: ({ timeSigPlacement: 2 })
                hint: qsTrc("notation/engravingoptions",
                            "Extra space between the upper and the lower number; it grows with the vertical scale.")
                min: -10
                max: 10
                step: 0.1
            }

            EoNumber {
                sid: "timeSigNormalY"
                hint: qsTrc("notation/engravingoptions",
                            "Positive values move the time signature down from the middle of the staff, negative " +
                            "values up.")
                min: -20
                max: 20
                step: 0.25
            }

            EoNumber {
                sid: "timeSigAboveY"
                musicStyle: ({ timeSigPlacement: 1 })
                hint: qsTrc("notation/engravingoptions",
                            "Negative values lift the time signature further away from the staff it stands over; 0 " +
                            "lets it sit on the staff line.")
                min: -20
                max: 20
                step: 0.25
            }

            EoNumber {
                sid: "timeSigAcrossY"
                music: "\\instruments 2 \\time 3/4 c''2."
                musicStyle: ({ timeSigPlacement: 2 })
                hint: qsTrc("notation/engravingoptions",
                            "Positive values move the time signature down from the top line of the uppermost staff " +
                            "of its group; not used while the time signature is centred on the group.")
                min: -20
                max: 20
                step: 0.25
                enabled: !page.value("timeSigCenterAcrossStaveGroup")
            }
        }

        EoChoice {
            sid: "timeSigCenterOnBarline"
            music: "\\time 4/4 c''1 | \\time 3/4 c''2."
            musicStyle: ({ timeSigPlacement: 1 })
            title: qsTrc("notation/engravingoptions", "Above the staves: centre on the barline")
            hint: qsTrc("notation/engravingoptions",
                        "Where a time signature above the staves stands at a barline: centred on the barline, or " +
                        "starting just after it.")
            choices: [
                { value: false, title: qsTrc("notation/engravingoptions", "Starts after the barline") },
                { value: true, title: qsTrc("notation/engravingoptions", "Centred on the barline") }
            ]
            enabled: page.value("timeSigPlacement") === 1
        }

        EoChoice {
            sid: "timeSigCenterAcrossStaveGroup"
            music: "\\instruments 2 \\group bracket \\time 3/4 c''2."
            musicStyle: ({ timeSigPlacement: 2 })
            title: qsTrc("notation/engravingoptions", "Across the staves: centre on the group of staves")
            hint: qsTrc("notation/engravingoptions",
                        "A time signature across the staves either hangs down from the top line of the uppermost " +
                        "staff of its group, or is centred on the whole group.")
            choices: [
                { value: false, title: qsTrc("notation/engravingoptions", "From the top staff down") },
                { value: true, title: qsTrc("notation/engravingoptions", "Centred on the group") }
            ]
            enabled: page.value("timeSigPlacement") === 2
        }

        EoChoice {
            sid: "timeSigVSMarginCentered"
            music: "\\time 4/4 c''1 \\break \\time 3/4 c''2."
            systemWidth: 16
            musicStyle: ({ timeSigPlacement: 1 })
            title: qsTrc("notation/engravingoptions", "Above the staves: courtesy at the end of the system, centred")
            hint: qsTrc("notation/engravingoptions",
                        "For a time signature above the staves which is centred on the barline: the courtesy time " +
                        "signature may hang into the margin, end at the margin beside the last barline, or stay " +
                        "centred on a last barline which is moved in.")
            presentation: "dropdown"
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Hang into the page margin") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Inset the time signature") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Inset the barline") }
            ]
            enabled: page.value("timeSigPlacement") === 1 && Boolean(page.value("genCourtesyTimesig"))
                     && Boolean(page.value("timeSigCenterOnBarline"))
        }

        EoChoice {
            sid: "timeSigVSMarginNonCentered"
            music: "\\time 4/4 c''1 \\break \\time 3/4 c''2."
            systemWidth: 22
            musicStyle: ({ timeSigPlacement: 1, timeSigCenterOnBarline: false })
            title: qsTrc("notation/engravingoptions",
                         "Above the staves: courtesy at the end of the system, not centred")
            hint: qsTrc("notation/engravingoptions",
                        "For a time signature above the staves which starts after the barline: the courtesy time " +
                        "signature either ends at the margin beside the last barline, or the last barline is moved " +
                        "in to leave room for it.")
            presentation: "dropdown"
            choices: [
                { value: 1, title: qsTrc("notation/engravingoptions", "Inset the time signature") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Inset the barline") }
            ]
            enabled: page.value("timeSigPlacement") === 1 && Boolean(page.value("genCourtesyTimesig"))
                     && !page.value("timeSigCenterOnBarline")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Courtesies at repeats and jumps")
        music: "\\key 2 a'2 | \\key 0 c''2 \\bar \":|.\" c''2"
        hint: qsTrc("notation/engravingoptions",
                    "What is shown where the music goes back or jumps to a passage whose clef, key or time signature " +
                    "is different. A clef, key or time signature gets these courtesies only while its own ‘Show " +
                    "courtesy …’ switch above is on.")

        EoGrid {
            rowTitles: [
                qsTrc("notation/engravingoptions", "At repeats"),
                qsTrc("notation/engravingoptions", "At all other jumps")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Show a courtesy"),
                qsTrc("notation/engravingoptions", "Cancel the courtesy after the barline"),
                qsTrc("notation/engravingoptions", "Courtesy in parentheses"),
                qsTrc("notation/engravingoptions", "Cancellation in parentheses"),
                qsTrc("notation/engravingoptions", "Only show changes before the barline")
            ]

            EoToggle {
                sid: "showCourtesiesRepeats"
                hint: qsTrc("notation/engravingoptions",
                            "At an end repeat: when the passage the music goes back to has another clef, key or time " +
                            "signature than the one in force, a courtesy before the repeat barline announces it.")
            }

            EoToggle {
                sid: "showCourtesiesAfterCancellingRepeats"
                hint: qsTrc("notation/engravingoptions",
                            "After such a courtesy, the clef, key or time signature which holds for the music going " +
                            "on is restated after the barline. Only with ‘Show a courtesy’ on.")
                enabled: Boolean(page.value("showCourtesiesRepeats"))
            }

            EoToggle {
                sid: "useParensRepeatCourtesies"
                hint: qsTrc("notation/engravingoptions", "The courtesy before the barline is enclosed in parentheses.")
                enabled: Boolean(page.value("showCourtesiesRepeats"))
            }

            EoToggle {
                sid: "useParensRepeatCourtesiesAfterCancelling"
                hint: qsTrc("notation/engravingoptions",
                            "The signature restated after the barline is enclosed in parentheses.")
                enabled: Boolean(page.value("showCourtesiesRepeats"))
                         && Boolean(page.value("showCourtesiesAfterCancellingRepeats"))
            }

            EoToggle {
                sid: "changesBeforeBarlineRepeats"
                music: "\\key 2 a'2 | \\key 0 c''2 \\bar \":|.\" \\key 2 a'2"
                hint: qsTrc("notation/engravingoptions",
                            "A change which holds both for the passage the music goes back to and for the measure " +
                            "after the repeat barline is written once, before the barline, instead of after it.")
            }

            EoToggle {
                sid: "showCourtesiesOtherJumps"
                music: "\\key 2 a'2 | \\key 0 c''2 \\jump \"D.C.\" | c''2"
                hint: qsTrc("notation/engravingoptions",
                            "At a D.C., D.S. or other jump: when the passage the music jumps to has another clef, " +
                            "key or time signature than the one in force, a courtesy before the barline announces " +
                            "it.")
            }

            EoToggle {
                sid: "showCourtesiesAfterCancellingOtherJumps"
                music: "\\key 2 a'2 | \\key 0 c''2 \\jump \"D.C.\" | c''2"
                hint: qsTrc("notation/engravingoptions",
                            "After such a courtesy, the clef, key or time signature which holds for the music going " +
                            "on is restated after the barline. Only with ‘Show a courtesy’ on.")
                enabled: Boolean(page.value("showCourtesiesOtherJumps"))
            }

            EoToggle {
                sid: "useParensOtherJumpCourtesies"
                music: "\\key 2 a'2 | \\key 0 c''2 \\jump \"D.C.\" | c''2"
                hint: qsTrc("notation/engravingoptions", "The courtesy before the barline is enclosed in parentheses.")
                enabled: Boolean(page.value("showCourtesiesOtherJumps"))
            }

            EoToggle {
                sid: "useParensOtherJumpCourtesiesAfterCancelling"
                music: "\\key 2 a'2 | \\key 0 c''2 \\jump \"D.C.\" | c''2"
                hint: qsTrc("notation/engravingoptions",
                            "The signature restated after the barline is enclosed in parentheses.")
                enabled: Boolean(page.value("showCourtesiesOtherJumps"))
                         && Boolean(page.value("showCourtesiesAfterCancellingOtherJumps"))
            }

            EoToggle {
                sid: "changesBeforeBarlineOtherJumps"
                music: "\\key 2 a'2 | \\key 0 c''2 \\jump \"D.C.\" | \\key 2 a'2"
                hint: qsTrc("notation/engravingoptions",
                            "A change which holds both for the passage the music jumps to and for the measure after " +
                            "the barline is written once, before the barline, instead of after it.")
            }
        }

        EoToggle {
            sid: "changesBetweenEndStartRepeat"
            music: "c''2 \\bar \":|.|:\" \\key 2 a'2 \\bar \":|.\""
            title: qsTrc("notation/engravingoptions", "Place changes between an end repeat and a start repeat")
            hint: qsTrc("notation/engravingoptions",
                        "Where an end repeat is followed at once by a start repeat, a change of clef, key or time " +
                        "signature stands between the two barlines; off puts it after the start repeat.")
        }

        EoToggle {
            sid: "smallParens"
            title: qsTrc("notation/engravingoptions", "Small parentheses")
            hint: qsTrc("notation/engravingoptions",
                        "The parentheses are only as tall as the signatures they enclose; off makes them as tall as " +
                        "the staff.")
        }
    }
}
