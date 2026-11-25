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
        music: "c''4( d'' e'' f'')"

        EoNumber {
            sid: "slurEndWidth"
            title: qsTrc("notation/editstyle/slursandties", "Line thickness at end:")
            step: 0.01
        }

        EoNumber {
            sid: "slurMidWidth"
            title: qsTrc("notation/editstyle/slursandties", "Line thickness middle:")
            hint: qsTrc("notation/engravingoptions",
                        "Short slurs are thinner than this: see ‘Slurs and ties shorter than this are thinner’.")
            step: 0.01
        }

        EoNumber {
            sid: "slurDottedWidth"
            music: "\\slurDotted c''4( d'' e'' f'')"
            title: qsTrc("notation/engravingoptions", "Thickness of dotted and dashed slurs")
            step: 0.01
        }

        EoNumber {
            sid: "slurTieShortLengthLimit"
            music: "c''16~ c''16 c''2~ c''2"
            title: qsTrc("notation/engravingoptions", "Slurs and ties shorter than this are thinner")
            hint: qsTrc("notation/engravingoptions",
                        "Below this length the middle of a slur or tie becomes gradually thinner, down to the " +
                        "‘Middle thickness of the shortest slurs and ties’ at the minimum tie length.")
            min: 1
            max: 20
            step: 0.5
        }

        EoNumber {
            sid: "slurTieShortMidWidth"
            music: "c''16~ c''16"
            title: qsTrc("notation/engravingoptions", "Middle thickness of the shortest slurs and ties")
            hint: qsTrc("notation/engravingoptions",
                        "The thickness in the middle of a slur or tie which is only as long as the ‘Minimum tie " +
                        "length’ of the Ties page.")
            min: 0.05
            max: 0.5
            step: 0.01
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Dashed and dotted slurs and ties")
        hint: qsTrc("notation/engravingoptions",
                    "All lengths are multiples of the thickness of dotted and dashed slurs or ties, so the pattern " +
                    "keeps its look when the line is made thicker.")
        music: "\\slurDashed c''4( d'' e'' f'')"

        EoNumber {
            sid: "slurTieDottedGap"
            music: "\\slurDotted c''4( d'' e'' f'')"
            title: qsTrc("notation/engravingoptions", "Dotted: distance from dot to dot")
            hint: qsTrc("notation/engravingoptions",
                        "From the middle of one dot to the middle of the next, as a multiple of the line thickness: " +
                        "at 2 the white space between two dots is as wide as a dot.")
            min: 0.5
            max: 10
            step: 0.1
        }

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "Length of the dashes and of the gaps between them. The dashes have square ends, which make " +
                        "each printed dash one line thickness longer and each gap one thickness shorter than the " +
                        "numbers say.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Dashed"),
                qsTrc("notation/engravingoptions", "Wide dashed")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Dash"),
                qsTrc("notation/engravingoptions", "Gap")
            ]

            EoNumber {
                sid: "slurTieDashLength"
                music: "\\slurDashed c''4( d'' e'' f'')"
                min: 0.5
                max: 20
                step: 0.25
            }

            EoNumber {
                sid: "slurTieDashGap"
                music: "\\slurDashed c''4( d'' e'' f'')"
                min: 0.5
                max: 20
                step: 0.25
            }

            EoNumber {
                sid: "slurTieWideDashLength"
                music: "\\slurWideDashed c''4( d'' e'' f'')"
                min: 0.5
                max: 30
                step: 0.25
            }

            EoNumber {
                sid: "slurTieWideDashGap"
                music: "\\slurWideDashed c''4( d'' e'' f'')"
                min: 0.5
                max: 30
                step: 0.25
            }
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Placement")

        EoToggle {
            sid: "angleHangingSlursAwayFromStaff"
            music: "c'4( d'4 \\break e'4) f'4"
            systemWidth: 26
            title: qsTrc("notation/engravingoptions", "Slurs cut off at a break angle away from the staff")
            hint: qsTrc("notation/engravingoptions",
                        "A slur which goes on in the next system, or into or out of a repeat or volta, has its open " +
                        "end drawn further from the staff than its end on the note, so that the half slur visibly " +
                        "points outwards.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "End points")
        music: "\\slurUp \\stemUp c'8[( e'8 g'8 c''8])"

        EoNumber {
            sid: "slurEndpointNoteDistance"
            music: "c''4( d'' e'' f'')"
            title: qsTrc("notation/engravingoptions", "Distance from notehead centre")
            hint: qsTrc("notation/engravingoptions",
                        "Vertical distance between the middle of the notehead and a slur end on the notehead side; " +
                        "the end is centred over the notehead.")
            max: 3
        }

        EoNumber {
            sid: "slurEndpointNoteDistanceTab"
            music: "\\tab c'4( d'4)"
            title: qsTrc("notation/engravingoptions", "Distance from fret mark centre (tablature)")
            hint: qsTrc("notation/engravingoptions",
                        "The same distance on tablature staves, measured from the middle of the fret mark.")
            max: 3
        }

        EoNumber {
            sid: "slurStemSideInset"
            music: "\\slurUp \\stemUp c'4( d'4)"
            title: qsTrc("notation/engravingoptions", "Inset from stem tip")
            hint: qsTrc("notation/engravingoptions",
                        "A slur end on the stem side sits beside the stem, this far back from the stem tip towards " +
                        "the notehead; 0 puts it level with the tip.")
            max: 2
        }

        EoNumber {
            sid: "slurStemOffsetX"
            music: "\\slurUp \\stemUp c'4( d'4)"
            title: qsTrc("notation/engravingoptions", "Horizontal gap to stem")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal distance between a stem and a slur end which starts or stops beside it.")
            max: 2
        }

        EoNumber {
            sid: "slurBeamClearance"
            music: "\\slurUp \\stemUp c'8[( e'8 g'8 c''8])"
            title: qsTrc("notation/engravingoptions", "Gap to beam")
            hint: qsTrc("notation/engravingoptions",
                        "Vertical distance between the outer edge of a beam and a slur end placed over it.")
            max: 2
        }

        EoNumber {
            sid: "slurBeamAnchorInset"
            music: "\\slurUp \\stemUp c'8[( e'8 g'8 c''8])"
            title: qsTrc("notation/engravingoptions", "Over a beam: shift inwards from the stem")
            hint: qsTrc("notation/engravingoptions",
                        "A slur end placed over a beam is moved this far from the stem towards the middle of the " +
                        "slur.")
            max: 1
        }

        EoNumber {
            sid: "slurMinStemBeamOffset"
            music: "\\slurUp \\stemUp c'8[( e'8 g'8 c''8])"
            title: qsTrc("notation/engravingoptions", "Minimum gap to stems and beams (small notes)")
            hint: qsTrc("notation/engravingoptions",
                        "On grace notes and small notes ‘Horizontal gap to stem’ and ‘Gap to beam’ shrink with the " +
                        "size of the note, but not below this.")
            max: 1
        }

        EoNumber {
            sid: "slurStemFloatRatio"
            music: "\\stemUp e'4( \\stemDown b''4)"
            title: qsTrc("notation/engravingoptions", "Stems in opposite directions: slide along the stem")
            hint: qsTrc("notation/engravingoptions",
                        "When the first and last notes have stems in opposite directions, the slur end on a stem " +
                        "side slides along that stem towards the other note, by this share of the height difference " +
                        "between the two notes (0 = it stays by the notehead).")
            max: 1
        }

        EoNumber {
            sid: "slurStemFloatMaxOvershoot"
            music: "\\stemUp c'4( \\stemDown c''''4)"
            title: qsTrc("notation/engravingoptions",
                         "Stems in opposite directions: furthest point beyond the stem tip")
            hint: qsTrc("notation/engravingoptions",
                        "The sliding slur end stops at the latest this far beyond the tip of the stem.")
            max: 4
            step: 0.05
        }

        EoNumber {
            sid: "slurArticulationDistance"
            music: "\\stemUp c'4-.( d'4-.)"
            title: qsTrc("notation/engravingoptions", "Distance beyond articulation")
            hint: qsTrc("notation/engravingoptions",
                        "When a staccato dot, tenuto line or similar mark sits next to the notehead on the slur’s " +
                        "side, the slur end is placed outside it at this vertical distance.")
            max: 2
        }

        EoNumber {
            sid: "slurArticulationInset"
            music: "\\stemUp c''4-.( d''4)"
            title: qsTrc("notation/engravingoptions", "Inward shift at articulation")
            hint: qsTrc("notation/engravingoptions",
                        "Such a slur end is centred on the articulation and then moved this far towards the middle " +
                        "of the slur.")
            max: 1
        }

        EoNumber {
            sid: "slurEndpointStaffLineMargin"
            music: "c''4( d'' e'' f'')"
            title: qsTrc("notation/engravingoptions", "End point distance from staff lines")
            hint: qsTrc("notation/engravingoptions",
                        "A slur end inside the staff which would be closer than this to a staff line is moved " +
                        "outwards, in the direction the slur curves, until it is this far from the line.")
            max: 0.45
            step: 0.025
            decimals: 3
        }

        EoNumber {
            sid: "slurEndpointTieClearance"
            music: "c'4~( c'4 d'4 e'4)"
            title: qsTrc("notation/engravingoptions", "Vertical gap to tie on same note")
            hint: qsTrc("notation/engravingoptions",
                        "When the note at a slur end also has a tie on the same side, the slur end is placed at " +
                        "least this far outside the end of the tie.")
            max: 2
        }

        EoNumber {
            sid: "slurEndpointTieShiftX"
            music: "c''4~ c''4( d''4)"
            title: qsTrc("notation/engravingoptions", "Horizontal shift next to tie end")
            hint: qsTrc("notation/engravingoptions",
                        "When a tie arrives at the note where a slur begins, or leaves the note where it ends, and " +
                        "its tip is at about the same height, the slur end is moved this far along, away from the " +
                        "tie.")
            max: 2
        }

        EoNumber {
            sid: "slurOverlapOffsetY"
            music: "c''4(( d''4) e''4 f''4)"
            title: qsTrc("notation/engravingoptions", "Slurs sharing an end point: lift of the outer slur")
            hint: qsTrc("notation/engravingoptions",
                        "When two slurs on the same side begin or end at the same place, the end of the longer, " +
                        "outer slur is moved this far out so that the two do not touch.")
            max: 2
        }

        EoNumber {
            sid: "slurOverlapOffsetX"
            music: "c''4( d''4)( e''4)"
            title: qsTrc("notation/engravingoptions", "Slurs meeting on a note: pull-back of each end")
            hint: qsTrc("notation/engravingoptions",
                        "When one slur ends on the note where the next begins, each of the two ends is drawn back by " +
                        "this amount, leaving a gap between them.")
            max: 1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Shape")
        music: "c''4( d'' e'' f'')"

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "The shoulder width says how square a slur is: with a larger value its ends rise more " +
                        "steeply and its middle is flatter, with a smaller value it is more pointed (it is the " +
                        "distance between the two control points of the curve, as a fraction of the slur’s length). " +
                        "It is set separately for four classes of length.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Short slurs"),
                qsTrc("notation/engravingoptions", "Medium slurs"),
                qsTrc("notation/engravingoptions", "Long slurs"),
                qsTrc("notation/engravingoptions", "Very long slurs")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Up to a length of"),
                qsTrc("notation/engravingoptions", "Shoulder width")
            ]

            EoNumber {
                sid: "slurShoulderLengthShort"
                music: "c''8( d''8)"
                max: 50
                step: 0.5
            }

            EoNumber {
                sid: "slurShoulderWidthShort"
                music: "c''16( d''16)"
                max: 1
            }

            EoNumber {
                sid: "slurShoulderLengthMedium"
                music: "c''4( d'' e'' f'')"
                max: 100
                step: 0.5
            }

            EoNumber {
                sid: "slurShoulderWidthMedium"
                music: "c''8( e''8)"
                max: 1
            }

            EoNumber {
                sid: "slurShoulderLengthLong"
                music: "c''4( d'' e'' f'' | g''4 a'' b'' c''')"
                max: 200
                step: 0.5
            }

            EoNumber {
                sid: "slurShoulderWidthLong"
                music: "c''4( d'' e'' f'')"
                max: 1
            }

            EoGap {
            }

            EoNumber {
                sid: "slurShoulderWidthVeryLong"
                music: "c''1( | d''1 | e''1 | f''1)"
                max: 1
            }
        }

        EoNumber {
            sid: "slurShoulderHeightFactor"
            music: "c''4( d'' e'' f'')"
            title: qsTrc("notation/engravingoptions", "Height in relation to length")
            hint: qsTrc("notation/engravingoptions",
                        "A slur is as high as this value times the square root of its length, both in sp: at 0.5 a " +
                        "slur 16 sp long is 2 sp high. The height is that of the control points of the curve; the " +
                        "arc itself rises three quarters as high.")
            min: 0.05
            max: 2
        }

        EoNumber {
            sid: "slurOverBeamsHeightFactor"
            music: "\\slurUp \\stemUp c'8[( e'8 g'8 c''8])"
            title: qsTrc("notation/engravingoptions", "Height of slurs along a beam")
            hint: qsTrc("notation/engravingoptions",
                        "A slur whose notes are all beamed, with the beams on the slur’s side, has its height " +
                        "multiplied by this; under 1 it is flatter than other slurs.")
            min: 0.1
            max: 1.5
        }

        EoNumber {
            sid: "slurFlatCurveHandleRatio"
            music: "\\slurFlat c''8( c'' c'' c'' c'' c'' c'' c'')"
            title: qsTrc("notation/engravingoptions", "Flat-middle slurs: roundness of the corners")
            hint: qsTrc("notation/engravingoptions",
                        "For slurs of the ‘Flat middle’ type: with a larger value the two corners between the rising " +
                        "ends and the flat middle are rounder and more gradual, with a smaller value they are " +
                        "tighter.")
            min: 0.05
            max: 1
        }

        EoNumber {
            sid: "slurMaxHeightToLengthRatio"
            music: "g'16( g''' g') r16"
            musicStyle: ({
                slurCollisionBalanceFree: 0.0,
                slurCollisionBalanceConstrained: 0.0,
                slurCollisionStep: 1.0,
                measureSpacing: 1.2
            })
            title: qsTrc("notation/engravingoptions", "Greatest height in relation to length")
            hint: qsTrc("notation/engravingoptions",
                        "However much a slur is raised to clear the notes under it, its height stays under this " +
                        "multiple of its length: at 1 a slur is never higher than it is long.")
            min: 0.1
            max: 3
            step: 0.05
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Collision avoidance")

        EoNumber {
            sid: "slurNoteClearance"
            music: "c''4( e''4 g''4 c''4)"
            title: qsTrc("notation/engravingoptions", "Clearance from noteheads")
            hint: qsTrc("notation/engravingoptions",
                        "The least vertical distance between a slur and the noteheads it passes over, other than " +
                        "those of its first and last chord.")
            max: 2
        }

        EoNumber {
            sid: "slurArticulationClearance"
            music: "\\stemUp c''4( d''4-. e''4)"
            title: qsTrc("notation/engravingoptions", "Clearance from articulations")
            hint: qsTrc("notation/engravingoptions",
                        "The least vertical distance between a slur and the articulations, ornaments and tapping " +
                        "marks it passes over.")
            max: 2
        }

        EoNumber {
            sid: "slurItemClearance"
            music: "\\stemUp c''4( g''4 c''4)"
            title: qsTrc("notation/engravingoptions", "Clearance from other items")
            hint: qsTrc("notation/engravingoptions",
                        "The least vertical distance between a slur and anything else it passes over: stems, beams, " +
                        "flags, accidentals, rests.")
            max: 2
        }

        EoNumber {
            sid: "slurArcClearanceLengthRatio"
            music: "c''4( d'' e'' f'' | g''' f'' e'' d'')"
            title: qsTrc("notation/engravingoptions", "Extra clearance in the middle: growth with length")
            hint: qsTrc("notation/engravingoptions",
                        "On top of the clearances above, the middle of a slur keeps an extra distance from the music " +
                        "under it which grows with its length: this many sp for every sp of length, less for slanted " +
                        "slurs. Nothing is added at the ends.")
            max: 0.2
            step: 0.005
            decimals: 3
        }

        EoNumber {
            sid: "slurArcClearanceMin"
            music: "c''4( g'''4 c''4)"
            title: qsTrc("notation/engravingoptions", "Extra clearance in the middle: at least")
            hint: qsTrc("notation/engravingoptions", "Short slurs get this much extra room under their middle.")
            max: 2
        }

        EoNumber {
            sid: "slurArcClearanceMax"
            music: "c''1( | d''1 | g'''1 | d''1)"
            title: qsTrc("notation/engravingoptions", "Extra clearance in the middle: at most")
            hint: qsTrc("notation/engravingoptions",
                        "However long the slur, the extra room under its middle is not more than this.")
            max: 3
        }

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Moving the end point or reshaping the curve")
            hint: qsTrc("notation/engravingoptions",
                        "A slur which runs into notes can lift its end points or bend its curve further out. 0 = the " +
                        "end point stays and only the curve changes; 1 = only the end point moves. Constrained: an " +
                        "end with an accent, ornament or similar mark outside it, the end on the note of a slur " +
                        "which goes on to another system, and the ends of a slur between two staves. Free: the open " +
                        "end at a system break and an end which was slid along a stem. Normal: every other end.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Constrained end"),
                qsTrc("notation/engravingoptions", "Normal end"),
                qsTrc("notation/engravingoptions", "Free end")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Share of end-point movement")
            ]

            EoNumber {
                sid: "slurCollisionBalanceConstrained"
                music: "c''4( \\fermata d''4 g'''4 d''4 c''4)"
                max: 1
            }

            EoNumber {
                sid: "slurCollisionBalanceNormal"
                music: "c''4( g'''4 c''4 c''4)"
                max: 1
            }

            EoNumber {
                sid: "slurCollisionBalanceFree"
                music: "\\stemUp c''4-.( g''4 c''4)"
                max: 1
            }
        }

        EoNumber {
            sid: "slurCollisionStep"
            music: "c''8( c'' c'' g'''8 c''8 c''8 c''8 c''8)"
            title: qsTrc("notation/engravingoptions", "Size of one avoidance step")
            hint: qsTrc("notation/engravingoptions",
                        "A slur is moved off the music in steps of this size, a limited number of times: small steps " +
                        "fit it closely over the notes but may not get it clear of a tall obstacle, large steps " +
                        "clear more but may leave it further away than needed.")
            min: 0.05
            max: 1
        }

        EoNumber {
            sid: "slurCollisionLongSlurLength"
            music: "c''1( | c''1 | g'''1 | c''1)"
            title: qsTrc("notation/engravingoptions", "Slurs longer than this take larger steps")
            hint: qsTrc("notation/engravingoptions",
                        "Beyond this length the step grows in proportion to the length of the slur, up to the " +
                        "‘Largest avoidance step’.")
            min: 4
            max: 100
            step: 1
        }

        EoNumber {
            sid: "slurCollisionMaxStep"
            music: "c''1( | g'''1 | c''1)"
            title: qsTrc("notation/engravingoptions", "Largest avoidance step")
            hint: qsTrc("notation/engravingoptions", "The upper limit of the enlarged step of long slurs.")
            min: 0.1
            max: 5
            step: 0.1
        }

        EoNumber {
            sid: "slurSteepAngleLimit"
            music: "\\stemDown c'16[( d' e' b'' c''')] r16 r4"
            musicStyle: ({ measureSpacing: 1.45 })
            title: qsTrc("notation/engravingoptions", "Steep slurs: angle above which the far end is moved too")
            hint: qsTrc("notation/engravingoptions",
                        "While a slur is being moved off the notes under it, a slur steeper than this also has its " +
                        "left end (rising slur) or right end (falling slur) moved outwards, even if nothing touches " +
                        "it there; for a slur above the notes this makes it less steep.")
            max: 90
            step: 1
            decimals: 0
            unit: "°"
        }

        EoNumber {
            sid: "slurSlantReductionAngle"
            music: "\\stemDown c''4( d'' e''' e'' f'' g'')"
            title: qsTrc("notation/engravingoptions", "Collision in the middle: slant above which the slur is levelled")
            hint: qsTrc("notation/engravingoptions",
                        "When a slur touches the music in its middle part only and slants by more than this, its " +
                        "lower end (the higher end of a slur below the notes) is moved out more than the other end, " +
                        "so that the slur also becomes less steep. Only for slurs longer than the next value.")
            max: 45
            step: 0.5
            decimals: 1
            unit: "°"
        }

        EoNumber {
            sid: "slurSlantReductionMinLength"
            music: "\\stemDown c''4( d'' a''' e'' f'')"
            title: qsTrc("notation/engravingoptions", "Collision in the middle: levelling only for slurs longer than")
            hint: qsTrc("notation/engravingoptions",
                        "Shorter slurs keep their slant and move both ends by their normal shares.")
            max: 40
            step: 0.5
            decimals: 1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Across systems")
        hint: qsTrc("notation/engravingoptions",
                    "The open end is the end of a slur which is cut off at a system break; the slur goes on in the " +
                    "next system.")
        music: "c'4( d'4 \\break e'4) f'4"
        systemWidth: 26

        EoNumber {
            sid: "slurContinuedEndOffset"
            music: "c'4( d'4 \\break e'4) f'4"
            systemWidth: 26
            title: qsTrc("notation/engravingoptions", "Open end: distance from the chord next to it")
            hint: qsTrc("notation/engravingoptions",
                        "The open end is level with the top (slur above) or bottom of the last chord before the " +
                        "break, or the first chord after it; this distance is added when the slur is on the notehead " +
                        "side of that chord.")
            max: 2
        }

        EoNumber {
            sid: "slurContinuedMaxHeightDiff"
            music: "c'''4( d'4 \\break e'4) f'4"
            systemWidth: 26
            title: qsTrc("notation/engravingoptions",
                         "Open end at the end of a system: greatest drop towards the staff")
            hint: qsTrc("notation/engravingoptions",
                        "At the end of a system the open end follows the last chord, but it ends at most this much " +
                        "nearer the staff than the slur began.")
            max: 10
            step: 0.25
        }

        EoNumber {
            sid: "slurContinuedSingleChordRise"
            music: "c'4( \\break e'4) f'4 r4"
            systemWidth: 26
            title: qsTrc("notation/engravingoptions", "Open end: rise when only one chord is under the slur")
            hint: qsTrc("notation/engravingoptions",
                        "When a slur begins on the last chord of a system, or ends on the first chord of the next, " +
                        "its open end lies this much further from the staff than its end on the note.")
            max: 2
        }

        EoNumber {
            sid: "slurHangingEndHeight"
            title: qsTrc("notation/engravingoptions", "Open end: rise when angled away from the staff")
            hint: qsTrc("notation/engravingoptions",
                        "With ‘Slurs cut off at a break angle away from the staff’ on, the open end lies at least " +
                        "this much further from the staff than the end on the note.")
            max: 4
            step: 0.1
        }

        EoToggle {
            sid: "slurTieExtendToSystemEndBarline"
            music: "c'4( d'4 \\break e'4 f'4)"
            systemWidth: 26
            title: qsTrc("notation/engravingoptions", "Extend to the last barline before a system break")
            hint: qsTrc("notation/engravingoptions",
                        "Slurs and ties which go on in the next system run right up to the last barline of the " +
                        "system instead of stopping a little before it.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Tapping half slurs")
        music: "c'4 \\tapping left"

        EoNumber {
            sid: "tappingHalfSlurLength"
            music: "c'4 \\tapping left"
            title: qsTrc("notation/engravingoptions", "Length")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal length of the half slur drawn before a note tapped with the left hand.")
            min: 0.5
            max: 10
            step: 0.1
        }

        EoNumber {
            sid: "tappingHalfSlurHeight"
            music: "c'4 \\tapping left"
            title: qsTrc("notation/engravingoptions", "Height")
            hint: qsTrc("notation/engravingoptions",
                        "Height of the half slur on standard staves, measured at the control points of the curve.")
            min: 0.1
            max: 3
        }

        EoNumber {
            sid: "tappingHalfSlurHeightTab"
            music: "\\tab c'4 \\tapping left"
            title: qsTrc("notation/engravingoptions", "Height (tablature)")
            hint: qsTrc("notation/engravingoptions", "The same height on tablature staves.")
            min: 0.1
            max: 3
        }
    }
}
