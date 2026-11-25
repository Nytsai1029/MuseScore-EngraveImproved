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

    code: "ties"
    legacyCodes: ["slurs-and-ties"]
    title: qsTrc("notation/engravingoptions", "Ties")
    group: qsTrc("notation/engravingoptions", "Notes and rests")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Placement")

        EoChoice {
            sid: "tiePlacementSingleNote"
            title: qsTrc("notation", "Placement on single notes:")
            presentation: "tiles"
            music: "a'2~ a'2 | e''2~ e''2"
            hint: qsTrc("notation/engravingoptions",
                        "‘Outside’: the tie starts and ends above or below the noteheads. ‘Inside’: it runs between " +
                        "the noteheads, beside them.")
            choices: [
                { value: 1, title: qsTrc("notation/engravingoptions", "Inside") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Outside") }
            ]
            tileWidth: 200
            tileHeight: 96
        }

        EoChoice {
            sid: "tiePlacementChord"
            title: qsTrc("notation/engravingoptions", "Placement on chords")
            presentation: "tiles"
            music: "<c'' e'' g''>2~ <c'' e'' g''>2"
            hint: qsTrc("notation/engravingoptions",
                        "The same choice for ties where at least one of the two notes belongs to a chord. With " +
                        "‘Outside’ the top and bottom ties are attached over and under their noteheads; a tie of an " +
                        "inner note whose way is blocked by a neighbouring notehead still starts beside the " +
                        "noteheads.")
            choices: [
                { value: 1, title: qsTrc("notation/engravingoptions", "Inside") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Outside") }
            ]
            tileWidth: 200
            tileHeight: 120
        }

        EoChoice {
            sid: "tieDotsPlacement"
            title: qsTrc("notation", "Placement of inner ties with respect to augmentation dots:")
            presentation: "tiles"
            music: "<b' c''>4.~ <b' c''>4."
            hint: qsTrc("notation/engravingoptions",
                        "For ties which start beside a dotted notehead. ‘Auto’: the tie starts after a dot only when " +
                        "the dot is at its height. ‘Always before dots’: it starts at the notehead. ‘Always after " +
                        "dots’: it starts to the right of all the dots of the chord.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Auto") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Always before dots") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Always after dots") }
            ]
            tileWidth: 200
            tileHeight: 96
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Thickness")
        music: "c''2~ c''2"

        EoNumber {
            sid: "tieEndWidth"
            title: qsTrc("notation/editstyle/slursandties", "Line thickness at end:")
            step: 0.01
        }

        EoNumber {
            sid: "tieMidWidth"
            title: qsTrc("notation/editstyle/slursandties", "Line thickness middle:")
            hint: qsTrc("notation/engravingoptions",
                        "Short ties are thinner than this: see ‘Slurs and ties shorter than this are thinner’ on the " +
                        "Slurs page.")
            step: 0.01
        }

        EoNumber {
            sid: "tieDottedWidth"
            music: "\\tieDotted c''2~ c''2"
            title: qsTrc("notation/engravingoptions", "Thickness of dotted and dashed ties")
            step: 0.01
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Length and height")
        music: "c''2~ c''2 | c'4 \\laissezVibrer"

        EoNumber {
            sid: "minTieLength"
            music: "<c'' e'' g''>16~ <c'' e'' g''>16 r8 r2"
            title: qsTrc("notation/engravingoptions", "Minimum tie length")
            hint: qsTrc("notation/engravingoptions",
                        "Tied notes are spaced far enough apart for the tie between them to be at least this long.")
            step: 0.1
        }

        EoNumber {
            sid: "minHangingTieLength"
            title: qsTrc("notation/editstyle/slursandties", "Minimum hanging tie length:")
            hint: qsTrc("notation/engravingoptions",
                        "The same for a tie which is cut off at one end: the half of a tie before or after a system " +
                        "break, and ties into or out of a repeat or volta.")
            step: 0.1
        }

        EoNumber {
            sid: "tieMinShoulderHeight"
            title: qsTrc("notation/editstyle/slursandties", "Minimum tie height:")
            hint: qsTrc("notation/engravingoptions",
                        "Height of the shortest ties. Tie heights are measured at the control points of the curve; " +
                        "the arc itself rises three quarters as high.")
        }

        EoNumber {
            sid: "tieMaxShoulderHeight"
            title: qsTrc("notation/editstyle/slursandties", "Maximum tie height:")
            hint: qsTrc("notation/engravingoptions", "However long a tie is, it does not arch higher than this.")
        }

        EoNumber {
            sid: "tieShoulderHeightFactor"
            music: "c''2~ c''2"
            title: qsTrc("notation/engravingoptions", "Height gained with length")
            hint: qsTrc("notation/engravingoptions",
                        "Longer ties arch higher: a tie 4 sp longer than the ‘Tie length with the minimum height’ is " +
                        "higher than the minimum by twice this value (in sp), one 9 sp longer by three times, up to " +
                        "the maximum height.")
            max: 2
        }

        EoNumber {
            sid: "tieShoulderHeightReferenceLength"
            music: "a''2~ a''2"
            title: qsTrc("notation/engravingoptions", "Tie length with the minimum height")
            hint: qsTrc("notation/engravingoptions",
                        "A tie of this length gets the ‘Minimum tie height’; from there the height grows with the " +
                        "length.")
            max: 5
            step: 0.1
            decimals: 1
        }

        EoNumber {
            sid: "tieShoulderWidth"
            music: "c''2~ c''2"
            title: qsTrc("notation/engravingoptions", "Shoulder width")
            hint: qsTrc("notation/engravingoptions",
                        "How square the tie is: with a larger value the ends rise more steeply and the middle is " +
                        "flatter; with a smaller value the tie is more pointed. It is the distance between the two " +
                        "control points of the curve, as a fraction of the tie’s length.")
            max: 1
        }

        EoNumber {
            sid: "tieStackedClearance"
            music: "<c'' d'' e'' f''>2~ <c'' d'' e'' f''>2"
            title: qsTrc("notation/engravingoptions", "Gap between stacked ties")
            hint: qsTrc("notation/engravingoptions",
                        "The least white space between two neighbouring ties of a chord which curve the same way; " +
                        "ties which would be closer have their arcs pushed apart.")
            max: 1
            step: 0.025
            decimals: 3
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "End points")
        music: situation.value === "chord" ? "<c'' e'' g''>2~ <c'' e'' g''>2" : "a'2~ a'2 | e''2~ e''2"

        EoContext {
            id: situation

            title: qsTrc("notation/engravingoptions", "Show the option on")
            value: "single"
            choices: [
                { value: "single", title: qsTrc("notation/engravingoptions", "Single notes") },
                { value: "chord", title: qsTrc("notation/engravingoptions", "Chords") }
            ]
        }

        EoNumber {
            sid: "tieEndpointYOffset"
            title: qsTrc("notation/engravingoptions", "Vertical offset from notehead")
            hint: qsTrc("notation/engravingoptions",
                        "For an outside tie, the gap between its ends and the top or bottom of the notehead; for an " +
                        "inside tie, how far its ends sit above or below the middle of the notehead.")
            max: 1
        }

        EoNumber {
            sid: "tieOutsideInset"
            title: qsTrc("notation/engravingoptions", "Outside tie: inward shift from notehead centre")
            hint: qsTrc("notation/engravingoptions",
                        "How far the ends of an outside tie are moved from the middle of the notehead towards the " +
                        "other note; 0 puts them exactly over the middle.")
            max: 1
        }

        EoNumber {
            sid: "tieOutsideInsetChord"
            music: "<c'' e'' g''>2~ <c'' e'' g''>2"
            musicStyle: ({ tiePlacementChord: 2 })
            title: qsTrc("notation/engravingoptions", "Outside tie: inward shift past another chord note")
            hint: qsTrc("notation/engravingoptions",
                        "Used instead when the chord has another note straight above the tied one (for a tie curving " +
                        "up; below for one curving down), so that the tie end clears that notehead.")
            max: 1.5
        }

        EoNumber {
            sid: "tieInsideInset"
            music: "c''2~ c''2"
            musicStyle: ({ tiePlacementSingleNote: 1 })
            title: qsTrc("notation/engravingoptions", "Inside tie: gap to notehead")
            hint: qsTrc("notation/engravingoptions",
                        "The horizontal gap between the notehead and the end of an inside tie; also used for ties on " +
                        "slash noteheads and on hidden fret marks.")
            max: 1
        }

        EoNumber {
            sid: "tieItemPadding"
            music: "<c' e'>4~ <c' e'>4"
            title: qsTrc("notation/engravingoptions", "Gap to dots, accidentals and other items")
            hint: qsTrc("notation/engravingoptions",
                        "The horizontal gap a tie end keeps from an augmentation dot, an accidental, a stem or a " +
                        "neighbouring notehead which is in its way.")
            max: 1
        }

        EoNumber {
            sid: "tieAdjacentTieInset"
            title: qsTrc("notation/engravingoptions", "Extra shift where two ties meet on a note")
            hint: qsTrc("notation/engravingoptions",
                        "When a note is tied on both sides and both ties curve the same way, each of the two tie " +
                        "ends is moved this much further from the middle of the notehead, so that the ties do not " +
                        "touch. For outside ties.")
            max: 1
        }

        EoNumber {
            sid: "tieLedgerLineMargin"
            music: "<a, b, c'>2~ <a, b, c'>2"
            title: qsTrc("notation/engravingoptions", "End point distance from ledger lines")
            hint: qsTrc("notation/engravingoptions",
                        "A tie end which would be closer than this to a ledger line lying on the side it curves to " +
                        "is moved back from that line.")
            max: 1
        }

        EoNumber {
            sid: "tieItemDetectionMarginArcSide"
            music: "<e'~ g'~ b'>2 <e' g' bes'>2"
            musicStyle: ({ tiePlacementChord: 1, tieEndpointYOffset: -0.25 })
            title: qsTrc("notation/engravingoptions", "Items the tie end avoids: reach on the curved side")
            hint: qsTrc("notation/engravingoptions",
                        "A tie end stops short of an accidental, dot, stem or neighbouring notehead when that item " +
                        "lies within this distance above the end point (for a tie curving up; below for one curving " +
                        "down).")
            max: 2
            step: 0.05
        }

        EoNumber {
            sid: "tieItemDetectionMarginOppositeSide"
            music: "<e' g'~ c''>2 <e' g' ces''>2"
            musicStyle: ({ tiePlacementChord: 1, tieEndpointYOffset: -0.5 })
            title: qsTrc("notation/engravingoptions", "Items the tie end avoids: reach on the other side")
            hint: qsTrc("notation/engravingoptions",
                        "The same reach on the side the tie curves away from: below the end point of a tie curving " +
                        "up.")
            max: 2
            step: 0.05
        }

        EoNumber {
            sid: "tieSlashNoteheadCenterOffset"
            music: "\\slash b'4~ \\slash b'4 \\slash b'4~ \\slash b'4"
            title: qsTrc("notation/engravingoptions", "Slash noteheads: distance of the tie from the tip of the slash")
            hint: qsTrc("notation/engravingoptions",
                        "On slash noteheads a tie above is attached near the upper right tip of the slash and a tie " +
                        "below near the lower left tip; this is how far inwards from the tip.")
            min: -1
            max: 1
            step: 0.05
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Ties and staff lines")
        music: "a'8~ a'8 c''8~ c''8"

        EoNumber {
            sid: "tieEndpointStaffLineMargin"
            music: "d''2~ d''2"
            title: qsTrc("notation/engravingoptions", "End point distance from staff lines")
            hint: qsTrc("notation/engravingoptions",
                        "A tie end never stops closer than this to the staff line it curves towards: it is moved to " +
                        "this distance on the far side of that line.")
            max: 0.45
            step: 0.025
            decimals: 3
        }

        EoNumber {
            sid: "tieArcStaffLineMargin"
            music: "c''2~ c''2"
            title: qsTrc("notation/engravingoptions", "Arc distance from staff lines")
            hint: qsTrc("notation/engravingoptions",
                        "The thick middle of a tie inside the staff stays at least this far from the nearest staff " +
                        "line: the tie is made a little higher or flatter until the gap is there.")
            max: 0.45
            step: 0.025
            decimals: 3
        }

        EoNumber {
            sid: "tieArcStaffLineMarginShort"
            music: "a'8~ a'8 c''8~ c''8"
            title: qsTrc("notation/engravingoptions", "Arc distance from staff lines (short ties)")
            hint: qsTrc("notation/engravingoptions",
                        "The same gap for ties shorter than the ‘Length below which a tie counts as short’.")
            max: 0.45
            step: 0.025
            decimals: 3
        }

        EoNumber {
            sid: "tieArcStaffLineShortLength"
            music: "a'8~ a'8 c''8~ c''8"
            title: qsTrc("notation/engravingoptions", "Length below which a tie counts as short")
            hint: qsTrc("notation/engravingoptions",
                        "Ties shorter than this use ‘Arc distance from staff lines (short ties)’.")
            max: 20
            step: 0.5
        }

        EoNumber {
            sid: "tieSmallMaxLength"
            music: "<d' g'~ b'>16 <d' g' b'>16 r8 r4"
            musicStyle: ({ measureSpacing: 0.96 })
            title: qsTrc("notation/engravingoptions", "Small tie: shorter than")
            hint: qsTrc("notation/engravingoptions",
                        "A tie which is both shorter than this and lower than the next value is not reshaped when " +
                        "its arc comes too near a staff line: it is moved as a whole into the staff space, or, where " +
                        "it cannot be moved, made higher but never flatter.")
            max: 6
            step: 0.1
            decimals: 1
        }

        EoNumber {
            sid: "tieSmallMaxHeight"
            music: "<d' g'~ b'>16 <d' g' b'>16 r8 r4"
            musicStyle: ({ measureSpacing: 0.96 })
            title: qsTrc("notation/engravingoptions", "Small tie: lower than")
            hint: qsTrc("notation/engravingoptions",
                        "The height limit of the same rule, measured over the whole tie including its thickness.")
            max: 2
            step: 0.05
        }

        EoNumber {
            sid: "tieMaxArcCorrectionRatio"
            music: "b'2~ b'2"
            musicStyle: ({ tieArcStaffLineMargin: 0.8 })
            title: qsTrc("notation/engravingoptions", "Largest change of height to avoid a staff line")
            hint: qsTrc("notation/engravingoptions",
                        "The arc is made higher or flatter by at most this share of the tie’s height; when it would " +
                        "have to be raised more, the whole tie is moved outwards for the rest.")
            max: 2
            step: 0.05
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Laissez vibrer")
        music: "c''2 \\laissezVibrer"

        EoNumber {
            sid: "minLaissezVibLength"
            music: "c''2 \\laissezVibrer"
            title: qsTrc("notation/engravingoptions", "Minimum length")
            hint: qsTrc("notation/engravingoptions",
                        "A laissez vibrer tie is drawn this long; in a chord all of them end at the same point, so " +
                        "some are longer.")
            step: 0.1
        }

        EoToggle {
            sid: "laissezVibUseSmuflSym"
            music: "c''2 \\laissezVibrer"
            title: qsTrc("notation/editstyle/slursandties", "Use SMuFL symbols")
            hint: qsTrc("notation/engravingoptions",
                        "The laissez vibrer tie is the ready-made sign of the music font, with a fixed length and " +
                        "shape, instead of a drawn tie.")
        }

        EoNumber {
            sid: "laissezVibParenthesisPadding"
            music: "<c'' e''?>2 \\laissezVibrer"
            title: qsTrc("notation/engravingoptions", "Gap to parenthesis")
            hint: qsTrc("notation/engravingoptions",
                        "On a note in parentheses the laissez vibrer tie starts this far above or below the " +
                        "parenthesis, so that it passes clear of it.")
            max: 1
        }

        EoNumber {
            sid: "laissezVibNoteDistance"
            music: "e'16 \\laissezVibrer e'16 r8 r4"
            title: qsTrc("notation/engravingoptions", "Gap to the next note or rest")
            hint: qsTrc("notation/engravingoptions",
                        "The least horizontal space between the end of a laissez vibrer tie and the note or rest " +
                        "which follows; the music is spaced wider if needed.")
            max: 5
        }

        EoNumber {
            sid: "laissezVibAccidentalDistance"
            music: "c''16 \\laissezVibrer cis''16 r8 r4"
            title: qsTrc("notation/engravingoptions", "Gap to the next accidental")
            hint: qsTrc("notation/engravingoptions",
                        "The same when the following note has an accidental in the way of the tie.")
            max: 5
        }

        EoNumber {
            sid: "laissezVibStemDistance"
            music: "c''16 \\laissezVibrer \\stemUp e'16 r8 r4"
            title: qsTrc("notation/engravingoptions", "Gap to the next stem")
            hint: qsTrc("notation/engravingoptions",
                        "The same when the stem of the following note is in the way of the tie.")
            max: 5
        }

        EoNumber {
            sid: "laissezVibBarlineDistance"
            music: "r8. c''16 \\laissezVibrer | c''4"
            title: qsTrc("notation/engravingoptions", "Gap to the barline")
            hint: qsTrc("notation/engravingoptions",
                        "The least horizontal space between the end of a laissez vibrer tie and the barline after " +
                        "it.")
            max: 5
        }
    }
}
