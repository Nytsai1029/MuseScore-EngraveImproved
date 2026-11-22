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
        title: qsTrc("EditStyleBase", "Standard staff")

        EoNumber {
            sid: "guitarBendLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            step: 0.01
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Tablature")

        EoNumber {
            sid: "guitarBendLineWidthTab"
            title: qsTrc("EditStyleBase", "Line thickness:")
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
            step: 0.1
        }

        EoNumber {
            sid: "guitarBendPartialBendHeight"
            title: qsTrc("EditStyleBase", "Partial bend height:")
            step: 0.1
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Legacy bends")

        EoNumber {
            sid: "bendArrowWidth"
            title: qsTrc("EditStyleBase", "Arrow width:")
            step: 0.1
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Tablature fret numbers")

        EoToggle {
            sid: "useCueSizeFretForGraceBends"
            title: qsTrc("EditStyleBase", "Use cue sized numbers for grace note bends")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Guitar bends (standard staff)")

        EoNumber {
            sid: "guitarBendNoteVerticalPadding"
            title: qsTrc("notation/engravingoptions", "Vertical distance from noteheads")
            hint: qsTrc("notation/engravingoptions",
                         "On a standard staff, vertical gap between the ends of a bend line and the noteheads.")
            max: 2
        }

        EoNumber {
            sid: "guitarBendNoteHorizontalIndent"
            title: qsTrc("notation/engravingoptions", "Horizontal indent from noteheads")
            hint: qsTrc("notation/engravingoptions",
                         "On a standard staff, how far the ends of a bend line are moved inwards from the notehead centre (from the notehead edge inside chords).")
            max: 2
        }

        EoNumber {
            sid: "guitarBendMinVertexHeight"
            title: qsTrc("notation/engravingoptions", "Minimum peak height")
            hint: qsTrc("notation/engravingoptions",
                         "Height of the peak of an angled bend line above the straight line between its ends, for the shortest bends.")
            max: 5
        }

        EoNumber {
            sid: "guitarBendMaxVertexHeight"
            title: qsTrc("notation/engravingoptions", "Maximum peak height")
            hint: qsTrc("notation/engravingoptions", "Upper limit of the peak height of long angled bend lines.")
            max: 8
        }

        EoNumber {
            sid: "guitarBendVertexHeightGrowth"
            title: qsTrc("notation/engravingoptions", "Peak height growth with length")
            hint: qsTrc("notation/engravingoptions",
                         "How much the peak rises for every staff space of bend length beyond one staff space.")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "guitarBendStaffLineClearance"
            title: qsTrc("notation/engravingoptions", "Clearance from staff lines")
            hint: qsTrc("notation/engravingoptions",
                         "Ends and peak of a bend line inside the staff are moved so that they stay at least this far (plus half the staff line thickness) from the nearest staff line.")
            max: 0.5
            step: 0.01
        }

        EoNumber {
            sid: "guitarBendChordPadding"
            title: qsTrc("notation/engravingoptions", "Padding from stems and other notes")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal clearance kept between the ends of a bend line and the stems, dots or other notes of the chord at that height.")
            max: 2
        }

        EoNumber {
            sid: "guitarBendSlightBendNoteGap"
            title: qsTrc("notation/engravingoptions", "Slight bend: gap after notehead")
            hint: qsTrc("notation/engravingoptions",
                         "On a standard staff, horizontal gap between the notehead and the start of a slight-bend curve.")
            max: 2
        }

        EoNumber {
            sid: "guitarBendSlightBendRaise"
            title: qsTrc("notation/engravingoptions", "Slight bend: start height above note")
            hint: qsTrc("notation/engravingoptions",
                         "On a standard staff, how far above the vertical position of the note a slight-bend curve starts.")
            min: -1
            max: 2
        }

        EoNumber {
            sid: "guitarBendSlightBendWidth"
            title: qsTrc("notation/engravingoptions", "Slight bend: width")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal length of a slight-bend curve, on standard and tablature staves.")
            min: 0.25
            max: 5
        }

        EoNumber {
            sid: "guitarBendSlightBendHeight"
            title: qsTrc("notation/engravingoptions", "Slight bend: height")
            hint: qsTrc("notation/engravingoptions", "On a standard staff, vertical rise of a slight-bend curve.")
            min: 0.25
            max: 5
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Guitar bends (tablature)")

        EoNumber {
            sid: "guitarBendTabNoteVerticalPadding"
            title: qsTrc("notation/engravingoptions", "Vertical distance from fret numbers")
            hint: qsTrc("notation/engravingoptions",
                         "On tablature, vertical gap between a fret number and the start of a pre-bend, or the arrow of a full release.")
            max: 2
        }

        EoNumber {
            sid: "guitarBendTabNoteHorizontalIndent"
            title: qsTrc("notation/engravingoptions", "Horizontal distance from fret numbers")
            hint: qsTrc("notation/engravingoptions",
                         "On tablature, gap between the right edge of the fret number and the start of the bend curve.")
            max: 2
        }

        EoNumber {
            sid: "guitarBendTabTextPadding"
            title: qsTrc("notation/engravingoptions", "Distance between arrow and bend amount")
            hint: qsTrc("notation/engravingoptions",
                         "On tablature, vertical gap between the arrowhead and the bend amount text (‘full’, ‘½’…).")
            max: 2
        }

        EoNumber {
            sid: "guitarBendTabMinWidth"
            title: qsTrc("notation/engravingoptions", "Minimum bend width")
            hint: qsTrc("notation/engravingoptions", "On tablature, a bend curve is never narrower than this.")
            max: 5
        }

        EoNumber {
            sid: "guitarBendHoldLineEndInset"
            title: qsTrc("notation/engravingoptions", "Hold line: distance before end of last note")
            hint: qsTrc("notation/engravingoptions",
                         "On tablature, the dashed hold line stops this far before the end of the space taken by its last note, unless another bend continues from that note.")
            max: 5
        }
    }
}
