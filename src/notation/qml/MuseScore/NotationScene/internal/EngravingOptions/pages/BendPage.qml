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

    code: "bend"
    legacyCodes: ["bend"]
    title: qsTrc("EditStyleBase", "Bends")
    group: qsTrc("notation/engravingoptions", "Lines")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Guitar bends")

        EoChoice {
            sid: "guitarBendUseFull"
            music: "\\tab a'4 \\bend b'4 r2"
            title: qsTrc("notation/engravingoptions", "Label for full bends")
            hint: qsTrc("notation/engravingoptions",
                        "How a bend of a whole tone is labelled above its arrow on tablature staves.")
            choices: [
                { value: false, title: qsTrc("notation/engravingoptions", "1") },
                { value: true, title: qsTrc("notation/engravingoptions", "full") }
            ]
        }

        EoToggle {
            sid: "useCueSizeFretForGraceBends"
            music: "\\tab \\grace g'8 \\bend a'4 r4 r2"
            title: qsTrc("notation/engravingoptions", "Use cue sized numbers for grace note bends")
            hint: qsTrc("notation/engravingoptions",
                        "Off: on tablature the fret number of a grace note which starts a bend or pre-bend stays " +
                        "full size. On: it is as small as any other grace note.")
        }

        EoNumber {
            sid: "minGuitarBendLength"
            music: "a'16 \\bend b'16 r8 r4 r2"
            title: qsTrc("notation/engravingoptions", "Minimum length")
            hint: qsTrc("notation/engravingoptions",
                        "The two notes of a bend are spaced apart far enough for a bend of at least this length.")
            max: 10
            step: 0.1
            decimals: 1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "On standard staves")
        music: "a'4 \\bend b'4 r2"

        EoNumber {
            sid: "guitarBendLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            step: 0.01
        }

        EoNumber {
            sid: "guitarBendNoteVerticalPadding"
            title: qsTrc("notation/engravingoptions", "Vertical gap to the noteheads")
            hint: qsTrc("notation/engravingoptions", "Vertical gap between each end of the bend line and its notehead.")
            max: 2
        }

        EoNumber {
            sid: "guitarBendNoteHorizontalIndent"
            title: qsTrc("notation/engravingoptions", "Inset of the ends from the notehead centres")
            hint: qsTrc("notation/engravingoptions",
                        "How far the start of the bend line lies to the right of the centre of its notehead, and its " +
                        "end to the left; inside a chord it is measured from the edge of the notehead instead.")
            max: 2
        }

        EoNumber {
            sid: "guitarBendMinVertexHeight"
            title: qsTrc("notation/engravingoptions", "Peak height of the shortest bends")
            hint: qsTrc("notation/engravingoptions",
                        "How far the corner of the angled bend line stands off the straight line between its two " +
                        "ends, for a bend one staff space long.")
            max: 5
        }

        EoNumber {
            sid: "guitarBendMaxVertexHeight"
            music: "a'1 \\bend b'1"
            musicStyle: ({ measureSpacing: 2.2 })
            title: qsTrc("notation/engravingoptions", "Greatest peak height")
            hint: qsTrc("notation/engravingoptions", "The peak of a long bend never stands higher than this.")
            max: 8
        }

        EoNumber {
            sid: "guitarBendVertexHeightGrowth"
            title: qsTrc("notation/engravingoptions", "Peak height gained with length")
            hint: qsTrc("notation/engravingoptions",
                        "How much higher the peak stands for every staff space of bend length beyond the first.")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "guitarBendStaffLineClearance"
            title: qsTrc("notation/engravingoptions", "Gap to staff lines")
            hint: qsTrc("notation/engravingoptions",
                        "Ends and peak of a bend line inside the staff are moved until they are at least this far " +
                        "from the nearest staff line, so that they do not merge with it.")
            max: 0.5
            step: 0.01
        }

        EoNumber {
            sid: "guitarBendChordPadding"
            music: "<a' e''>4 \\bend <b' e''>4 r2"
            title: qsTrc("notation/engravingoptions", "Gap to stems and other notes of the chord")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal gap kept between an end of the bend line and a stem, a dot or another note of " +
                        "the chord which stands at the same height.")
            max: 2
        }

        EoNumber {
            sid: "guitarBendSlightBendNoteGap"
            music: "a'4 \\slightBend r4 r2"
            title: qsTrc("notation/engravingoptions", "Slight bend: gap after the notehead")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal gap between the notehead and the start of the little curve of a quarter-tone " +
                        "bend.")
            max: 2
        }

        EoNumber {
            sid: "guitarBendSlightBendRaise"
            music: "a'4 \\slightBend r4 r2"
            title: qsTrc("notation/engravingoptions", "Slight bend: start height above the note")
            hint: qsTrc("notation/engravingoptions", "How far above the middle of the notehead the curve starts.")
            min: -1
            max: 2
        }

        EoNumber {
            sid: "guitarBendSlightBendWidth"
            music: "a'4 \\slightBend r4 r2"
            title: qsTrc("notation/engravingoptions", "Slight bend: width")
            hint: qsTrc("notation/engravingoptions", "Horizontal length of the curve; also used on tablature staves.")
            min: 0.25
            max: 5
        }

        EoNumber {
            sid: "guitarBendSlightBendHeight"
            music: "a'4 \\slightBend r4 r2"
            title: qsTrc("notation/engravingoptions", "Slight bend: height")
            hint: qsTrc("notation/engravingoptions", "How far the curve rises from its start to its tip.")
            min: 0.25
            max: 5
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "On tablature staves")
        music: "\\tab a'4 \\bend b'4 r2"

        EoNumber {
            sid: "guitarBendLineWidthTab"
            title: qsTrc("EditStyleBase", "Line thickness:")
            hint: qsTrc("notation/engravingoptions", "Also the thickness of the dashed hold line.")
            max: 10
            step: 0.01
        }

        EoNumber {
            sid: "guitarBendArrowWidth"
            title: qsTrc("EditStyleBase", "Arrow width:")
            max: 10
            step: 0.1
        }

        EoNumber {
            sid: "guitarBendArrowHeight"
            title: qsTrc("EditStyleBase", "Arrow height:")
            step: 0.1
        }

        EoNumber {
            sid: "guitarBendHeightAboveTABStaff"
            title: qsTrc("EditStyleBase", "Height above staff (line distances):")
            hint: qsTrc("notation/engravingoptions",
                        "How high above the top line of the tablature staff a bend curve rises before its arrowhead, " +
                        "as a multiple of the distance between two string lines.")
            step: 0.1
        }

        EoNumber {
            sid: "guitarBendPartialBendHeight"
            music: "\\tab a'4 \\bend b'4 \\bend cis''4 r4"
            title: qsTrc("notation/engravingoptions", "Partial bend height")
            hint: qsTrc("notation/engravingoptions",
                        "How far a bend which continues from another bend rises above it, and how far a release " +
                        "which does not return to the unbent note drops.")
            step: 0.1
        }

        EoNumber {
            sid: "guitarBendTabNoteVerticalPadding"
            music: "\\tab \\grace a'8 \\preBend b'4 r4 r2"
            title: qsTrc("notation/engravingoptions", "Vertical gap to the fret numbers")
            hint: qsTrc("notation/engravingoptions",
                        "Vertical gap between a fret number and the start of a pre-bend above it, or the arrowhead " +
                        "of a full release coming down to it.")
            max: 2
        }

        EoNumber {
            sid: "guitarBendTabNoteHorizontalIndent"
            title: qsTrc("notation/engravingoptions", "Horizontal gap to the fret number")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between the right edge of the fret number and the start of the bend curve.")
            max: 2
        }

        EoNumber {
            sid: "guitarBendTabTextPadding"
            title: qsTrc("notation/engravingoptions", "Gap between arrow and bend amount")
            hint: qsTrc("notation/engravingoptions",
                        "Vertical gap between the tip of the arrow and the label of the bend (‘full’, ‘½’…).")
            max: 2
        }

        EoNumber {
            sid: "guitarBendTabMinWidth"
            music: "\\tab a'16 \\bend b'16 r8 r4 r2"
            title: qsTrc("notation/engravingoptions", "Minimum width of a bend")
            hint: qsTrc("notation/engravingoptions",
                        "A bend curve is never drawn narrower than this, however close its notes are.")
            max: 5
        }

        EoNumber {
            sid: "guitarBendHoldLineEndInset"
            music: "\\tab a'4 \\bend b'4~ b'4 r4"
            title: qsTrc("notation/engravingoptions", "Hold line: gap before the next note")
            hint: qsTrc("notation/engravingoptions",
                        "The dashed hold line runs on past its last note and stops this far before the following " +
                        "note or barline, unless another bend starts on that last note.")
            max: 5
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Legacy bends")
        music: "a'4 \\oldBend r4 r2"
        hint: qsTrc("notation/engravingoptions",
                    "Bends of the old kind, as found in scores from earlier versions and in some imported Guitar Pro " +
                    "files.")

        EoNumber {
            sid: "bendArrowWidth"
            title: qsTrc("EditStyleBase", "Arrow width:")
            hint: qsTrc("notation/engravingoptions", "Width of the arrowhead, which is as high as it is wide.")
            step: 0.1
        }

        EoNumber {
            sid: "bendLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            hint: qsTrc("notation/engravingoptions", "Thickness of the curve and of the shaft of the arrow.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Tremolo bar")
        music: "a'4 \\tremoloBar r4 r2"

        EoNumber {
            sid: "tremoloBarYOffset"
            title: qsTrc("notation/engravingoptions", "Height above the staff")
            hint: qsTrc("notation/engravingoptions", "How far above the top staff line the curve starts.")
            max: 8
            step: 0.1
            decimals: 1
        }

        EoNumber {
            sid: "tremoloBarHeightScale"
            title: qsTrc("notation/engravingoptions", "Height of the curve")
            hint: qsTrc("notation/engravingoptions",
                        "Height in sp drawn for each hundredth of a semitone of pitch change: at 0.02 a dip of a " +
                        "semitone is 2 sp deep.")
            min: 0.005
            max: 0.1
            step: 0.005
            decimals: 3
        }

        EoNumber {
            sid: "tremoloBarLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
        }
    }
}
