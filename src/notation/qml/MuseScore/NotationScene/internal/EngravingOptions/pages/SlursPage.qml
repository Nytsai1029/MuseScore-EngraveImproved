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

    code: "slurs"
    title: qsTrc("notation/engravingoptions", "Slurs")
    group: qsTrc("notation/engravingoptions", "Notes and rests")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Thickness")

        EoPreview {
            snippet: "slurs"
            zoom: 1.5
        }

        EoNumber {
            sid: "slurEndWidth"
            title: qsTrc("notation/editstyle/slursandties", "Line thickness at end:")
            step: 0.01
        }

        EoNumber {
            sid: "slurMidWidth"
            title: qsTrc("notation/editstyle/slursandties", "Line thickness middle:")
            step: 0.01
        }

        EoNumber {
            sid: "slurDottedWidth"
            title: qsTrc("notation/editstyle/slursandties", "Dotted line thickness:")
            step: 0.01
        }

        EoNumber {
            sid: "slurTieShortLengthLimit"
            title: qsTrc("notation/engravingoptions", "Thinning starts below length")
            hint: qsTrc("notation/engravingoptions",
                         "Slurs and ties shorter than this are drawn progressively thinner, down to the thickness for the shortest curves at the minimum tie length.")
            min: 1
            max: 20
            step: 0.5
        }

        EoNumber {
            sid: "slurTieShortMidWidth"
            title: qsTrc("notation/engravingoptions", "Middle thickness of the shortest curves")
            hint: qsTrc("notation/engravingoptions",
                         "Middle thickness of a slur or tie whose length equals the minimum tie length.")
            min: 0.05
            max: 0.5
            step: 0.01
        }

        EoNumber {
            sid: "slurTieDottedGap"
            title: qsTrc("notation/engravingoptions", "Dotted: gap")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between the dots of dotted slurs and ties, in multiples of the dotted line thickness.")
            min: 0.5
            max: 10
            step: 0.1
        }

        EoNumber {
            sid: "slurTieDashLength"
            title: qsTrc("notation/engravingoptions", "Dashed: dash length")
            hint: qsTrc("notation/engravingoptions",
                         "Length of the dashes of dashed slurs and ties, in multiples of the dotted line thickness.")
            min: 0.5
            max: 20
            step: 0.25
        }

        EoNumber {
            sid: "slurTieDashGap"
            title: qsTrc("notation/engravingoptions", "Dashed: gap")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between the dashes of dashed slurs and ties, in multiples of the dotted line thickness.")
            min: 0.5
            max: 20
            step: 0.25
        }

        EoNumber {
            sid: "slurTieWideDashLength"
            title: qsTrc("notation/engravingoptions", "Wide dashed: dash length")
            hint: qsTrc("notation/engravingoptions",
                         "Length of the dashes of wide-dashed slurs and ties, in multiples of the dotted line thickness.")
            min: 0.5
            max: 30
            step: 0.25
        }

        EoNumber {
            sid: "slurTieWideDashGap"
            title: qsTrc("notation/engravingoptions", "Wide dashed: gap")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between the dashes of wide-dashed slurs and ties, in multiples of the dotted line thickness.")
            min: 0.5
            max: 30
            step: 0.25
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Placement")

        EoNumber {
            sid: "slurMinDistance"
            title: qsTrc("notation/editstyle/slursandties", "Autoplace min. distance:")
            step: 0.1
        }

        EoToggle {
            sid: "angleHangingSlursAwayFromStaff"
            title: qsTrc("notation/editstyle/slursandties", "Angle away from staff")
            hint: qsTrc("notation/engravingoptions", "For slurs which hang over the end of a system.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Slur end points")

        EoNumber {
            sid: "slurEndpointNoteDistance"
            title: qsTrc("notation/engravingoptions", "Distance from notehead centre")
            hint: qsTrc("notation/engravingoptions",
                         "Vertical distance from the centre of the notehead to a slur end that attaches on the notehead side.")
            max: 3
        }

        EoNumber {
            sid: "slurEndpointNoteDistanceTab"
            title: qsTrc("notation/engravingoptions", "Distance from fret mark centre (tablature)")
            hint: qsTrc("notation/engravingoptions",
                         "The same distance on tablature staves, measured from the centre of the fret mark.")
            max: 3
        }

        EoNumber {
            sid: "slurStemSideInset"
            title: qsTrc("notation/engravingoptions", "Inset from stem tip")
            hint: qsTrc("notation/engravingoptions",
                         "How far a slur end that attaches to a stem sits back from the stem tip towards the notehead.")
            max: 2
        }

        EoNumber {
            sid: "slurStemOffsetX"
            title: qsTrc("notation/engravingoptions", "Horizontal gap to stem")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal distance between a stem and a slur end that starts or ends beside it.")
            max: 2
        }

        EoNumber {
            sid: "slurBeamClearance"
            title: qsTrc("notation/engravingoptions", "Gap to beam")
            hint: qsTrc("notation/engravingoptions",
                         "Vertical distance between the outer edge of a beam and a slur end placed over it.")
            max: 2
        }

        EoNumber {
            sid: "slurBeamAnchorInset"
            title: qsTrc("notation/engravingoptions", "Inset from stem over beams")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal distance by which a slur end placed over a beam moves inwards from the stem.")
            max: 1
        }

        EoNumber {
            sid: "slurMinStemBeamOffset"
            title: qsTrc("notation/engravingoptions", "Minimum gap to stems and beams (small notes)")
            hint: qsTrc("notation/engravingoptions",
                         "Lower limit of the stem and beam gaps after they are scaled down for grace notes and cue-size chords.")
            max: 1
        }

        EoNumber {
            sid: "slurStemFloatRatio"
            title: qsTrc("notation/engravingoptions", "End point float along stem")
            hint: qsTrc("notation/engravingoptions",
                         "When the first and last chord have opposite stem directions, the stem-side slur end moves along the stem by this share of the pitch difference.")
            max: 1
        }

        EoNumber {
            sid: "slurArticulationDistance"
            title: qsTrc("notation/engravingoptions", "Distance beyond articulation")
            hint: qsTrc("notation/engravingoptions",
                         "Vertical distance between an articulation next to the notehead (staccato, tenuto) and the slur end placed outside it.")
            max: 2
        }

        EoNumber {
            sid: "slurArticulationInset"
            title: qsTrc("notation/engravingoptions", "Inward shift at articulation")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal shift towards the inside of the slur of an end point centred on such an articulation.")
            max: 1
        }

        EoNumber {
            sid: "slurEndpointStaffLineMargin"
            title: qsTrc("notation/engravingoptions", "End point distance from staff lines")
            hint: qsTrc("notation/engravingoptions",
                         "Slur ends inside the staff keep at least this distance (plus half the staff line thickness) from a staff line.")
            max: 0.45
            step: 0.025
            decimals: 3
        }

        EoNumber {
            sid: "slurEndpointTieClearance"
            title: qsTrc("notation/engravingoptions", "Vertical gap to tie on same note")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum vertical distance between a slur end and the end of a tie that curves the same way on the same note.")
            max: 2
        }

        EoNumber {
            sid: "slurEndpointTieShiftX"
            title: qsTrc("notation/engravingoptions", "Horizontal shift next to tie end")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal shift of a slur end when a tie arriving at (or leaving from) the same note ends at about the same height.")
            max: 2
        }

        EoNumber {
            sid: "slurOverlapOffsetY"
            title: qsTrc("notation/engravingoptions", "Nested slur end offset")
            hint: qsTrc("notation/engravingoptions",
                         "When two slurs on the same side share a start or end position, the end of the outer slur moves away vertically by this amount.")
            max: 2
        }

        EoNumber {
            sid: "slurOverlapOffsetX"
            title: qsTrc("notation/engravingoptions", "Pull-back between consecutive slurs")
            hint: qsTrc("notation/engravingoptions",
                         "When one slur ends on the chord where the next begins, each end point is pulled back horizontally by this amount.")
            max: 1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Slur shape")

        EoNumber {
            sid: "slurShoulderLengthShort"
            title: qsTrc("notation/engravingoptions", "Short slur limit")
            hint: qsTrc("notation/engravingoptions",
                         "Slurs shorter than this use the shoulder width for short slurs.")
            max: 50
            step: 0.5
        }

        EoNumber {
            sid: "slurShoulderLengthMedium"
            title: qsTrc("notation/engravingoptions", "Medium slur limit")
            hint: qsTrc("notation/engravingoptions",
                         "Slurs shorter than this (but not short) use the shoulder width for medium slurs.")
            max: 100
            step: 0.5
        }

        EoNumber {
            sid: "slurShoulderLengthLong"
            title: qsTrc("notation/engravingoptions", "Long slur limit")
            hint: qsTrc("notation/engravingoptions",
                         "Slurs shorter than this use the shoulder width for long slurs; longer ones use the width for very long slurs.")
            max: 200
            step: 0.5
        }

        EoNumber {
            sid: "slurShoulderWidthShort"
            title: qsTrc("notation/engravingoptions", "Shoulder width, short slurs")
            hint: qsTrc("notation/engravingoptions",
                         "Distance between the two curve control points as a fraction of the slur length; larger values give a flatter middle and steeper ends.")
            max: 1
        }

        EoNumber {
            sid: "slurShoulderWidthMedium"
            title: qsTrc("notation/engravingoptions", "Shoulder width, medium slurs")
            hint: qsTrc("notation/engravingoptions", "The same fraction for medium slurs.")
            max: 1
        }

        EoNumber {
            sid: "slurShoulderWidthLong"
            title: qsTrc("notation/engravingoptions", "Shoulder width, long slurs")
            hint: qsTrc("notation/engravingoptions", "The same fraction for long slurs.")
            max: 1
        }

        EoNumber {
            sid: "slurShoulderWidthVeryLong"
            title: qsTrc("notation/engravingoptions", "Shoulder width, very long slurs")
            hint: qsTrc("notation/engravingoptions", "The same fraction for very long slurs.")
            max: 1
        }

        EoNumber {
            sid: "slurShoulderHeightFactor"
            title: qsTrc("notation/engravingoptions", "Height factor")
            hint: qsTrc("notation/engravingoptions",
                         "Default slur height in sp = factor × √(length in sp); larger values give more arched slurs.")
            min: 0.05
            max: 2
        }

        EoNumber {
            sid: "slurOverBeamsHeightFactor"
            title: qsTrc("notation/engravingoptions", "Height factor over beams")
            hint: qsTrc("notation/engravingoptions",
                         "Slurs whose notes are all beamed on the slur side have their height multiplied by this factor.")
            min: 0.1
            max: 1.5
        }

        EoNumber {
            sid: "slurFlatCurveHandleRatio"
            title: qsTrc("notation/engravingoptions", "Flat-middle slur handle length")
            hint: qsTrc("notation/engravingoptions",
                         "Length of the curve handles of a flat-middle slur as a fraction of one third of its length; larger values give rounder, more gradual corners.")
            min: 0.05
            max: 1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Slur collision avoidance")

        EoNumber {
            sid: "slurNoteClearance"
            title: qsTrc("notation/engravingoptions", "Clearance from noteheads")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum vertical distance between a slur and the noteheads it passes over (other than those of its first and last chord).")
            max: 2
        }

        EoNumber {
            sid: "slurArticulationClearance"
            title: qsTrc("notation/engravingoptions", "Clearance from articulations")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum vertical distance between a slur and the articulations, ornaments and tapping marks it passes over.")
            max: 2
        }

        EoNumber {
            sid: "slurItemClearance"
            title: qsTrc("notation/engravingoptions", "Clearance from other items")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum vertical distance between a slur and anything else it passes over (stems, beams, accidentals, rests…).")
            max: 2
        }

        EoNumber {
            sid: "slurArcClearanceLengthRatio"
            title: qsTrc("notation/engravingoptions", "Mid-arc clearance per sp of length")
            hint: qsTrc("notation/engravingoptions",
                         "Extra clearance under the middle of the slur, in sp per sp of slur length (reduced for slanted slurs).")
            max: 0.2
            step: 0.005
            decimals: 3
        }

        EoNumber {
            sid: "slurArcClearanceMin"
            title: qsTrc("notation/engravingoptions", "Minimum mid-arc clearance")
            hint: qsTrc("notation/engravingoptions",
                         "Lower limit of the extra clearance under the middle of the slur.")
            max: 2
        }

        EoNumber {
            sid: "slurArcClearanceMax"
            title: qsTrc("notation/engravingoptions", "Maximum mid-arc clearance")
            hint: qsTrc("notation/engravingoptions",
                         "Upper limit of the extra clearance under the middle of the slur.")
            max: 3
        }

        EoNumber {
            sid: "slurCollisionBalanceConstrained"
            title: qsTrc("notation/engravingoptions", "End point share: constrained end")
            hint: qsTrc("notation/engravingoptions",
                         "Share of a collision correction made by moving the end point instead of reshaping the curve (0 = curve only, 1 = end point only), for ends with an articulation or ornament above, cross-staff slurs and the attached end of a slur broken across systems.")
            max: 1
        }

        EoNumber {
            sid: "slurCollisionBalanceNormal"
            title: qsTrc("notation/engravingoptions", "End point share: ordinary end")
            hint: qsTrc("notation/engravingoptions", "The same share for an ordinary slur end attached to a note.")
            max: 1
        }

        EoNumber {
            sid: "slurCollisionBalanceFree"
            title: qsTrc("notation/engravingoptions", "End point share: free end")
            hint: qsTrc("notation/engravingoptions",
                         "The same share for open ends at a system break and for ends floated along a stem.")
            max: 1
        }

        EoNumber {
            sid: "slurCollisionStep"
            title: qsTrc("notation/engravingoptions", "Adjustment step")
            hint: qsTrc("notation/engravingoptions",
                         "Distance the curve or an end point moves in each collision-avoidance pass; smaller values fit the music more tightly but may leave collisions in crowded passages.")
            min: 0.05
            max: 1
        }

        EoNumber {
            sid: "slurCollisionLongSlurLength"
            title: qsTrc("notation/engravingoptions", "Long slur length for larger steps")
            hint: qsTrc("notation/engravingoptions",
                         "Slurs longer than this use a proportionally larger adjustment step.")
            min: 4
            max: 100
            step: 1
        }

        EoNumber {
            sid: "slurCollisionMaxStep"
            title: qsTrc("notation/engravingoptions", "Maximum adjustment step")
            hint: qsTrc("notation/engravingoptions", "Upper limit of the enlarged adjustment step of long slurs.")
            min: 0.1
            max: 5
            step: 0.1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Slurs and ties across systems")

        EoNumber {
            sid: "slurContinuedEndOffset"
            title: qsTrc("notation/engravingoptions", "Open end distance from chord")
            hint: qsTrc("notation/engravingoptions",
                         "Vertical distance between the open end of a slur broken across systems and the first or last chord of that system.")
            max: 2
        }

        EoNumber {
            sid: "slurContinuedMaxHeightDiff"
            title: qsTrc("notation/engravingoptions", "Maximum drop of the open end at system end")
            hint: qsTrc("notation/engravingoptions",
                         "Limits how far the open end at the end of a system may move towards the staff relative to the slur’s start.")
            max: 10
            step: 0.25
        }

        EoNumber {
            sid: "slurContinuedSingleChordRise"
            title: qsTrc("notation/engravingoptions", "Open end rise over a single chord")
            hint: qsTrc("notation/engravingoptions",
                         "When a slur covers only one chord before or after a system break, its open end is placed this much further out than the attached end.")
            max: 2
        }

        EoNumber {
            sid: "slurHangingEndHeight"
            title: qsTrc("notation/engravingoptions", "Open end rise when angled away from staff")
            hint: qsTrc("notation/engravingoptions",
                         "With ‘Angle away from staff’ chosen for partial slurs across repeats and breaks, the open end lies at least this much further from the staff than the attached end.")
            max: 4
            step: 0.1
        }

        EoToggle {
            sid: "slurTieExtendToSystemEndBarline"
            title: qsTrc("notation/engravingoptions", "Extend to the last barline before a system break")
            hint: qsTrc("notation/engravingoptions",
                         "Slurs and ties that continue on the next system end flush with the last barline of the system instead of stopping short of it.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Tapping half slurs")

        EoNumber {
            sid: "tappingHalfSlurLength"
            title: qsTrc("notation/engravingoptions", "Length")
            hint: qsTrc("notation/engravingoptions", "Horizontal length of the half slur drawn before a tapped note.")
            min: 0.5
            max: 10
            step: 0.1
        }

        EoNumber {
            sid: "tappingHalfSlurHeight"
            title: qsTrc("notation/engravingoptions", "Height")
            hint: qsTrc("notation/engravingoptions", "Shoulder height of the tapping half slur on standard staves.")
            min: 0.1
            max: 3
        }

        EoNumber {
            sid: "tappingHalfSlurHeightTab"
            title: qsTrc("notation/engravingoptions", "Height (tablature)")
            hint: qsTrc("notation/engravingoptions", "Shoulder height of the tapping half slur on tablature staves.")
            min: 0.1
            max: 3
        }
    }
}
