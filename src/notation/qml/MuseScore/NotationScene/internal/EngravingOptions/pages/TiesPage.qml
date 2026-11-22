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
            snippet: "tie-single"
            choices: [
                { value: 1, title: qsTrc("inspector", "Inside") },
                { value: 2, title: qsTrc("inspector", "Outside") }
            ]
            tileZoom: 1.4
            tileWidth: 200
            tileHeight: 96
        }

        EoChoice {
            sid: "tiePlacementChord"
            title: qsTrc("notation", "Placement on chords:")
            presentation: "tiles"
            snippet: "tie-chord"
            choices: [
                { value: 1, title: qsTrc("inspector", "Inside") },
                { value: 2, title: qsTrc("inspector", "Outside") }
            ]
            tileZoom: 1.4
            tileWidth: 200
            tileHeight: 120
        }

        EoChoice {
            sid: "tieDotsPlacement"
            title: qsTrc("notation", "Placement of inner ties with respect to augmentation dots:")
            presentation: "tiles"
            snippet: "tie-dots"
            choices: [
                { value: 0, title: qsTrc("notation", "Auto") },
                { value: 1, title: qsTrc("notation", "Always before dots") },
                { value: 2, title: qsTrc("notation", "Always after dots") }
            ]
            tileZoom: 1.4
            tileWidth: 200
            tileHeight: 96
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Thickness")

        EoPreview {
            snippet: "ties"
            zoom: 1.75
        }

        EoNumber {
            sid: "tieEndWidth"
            title: qsTrc("notation/editstyle/slursandties", "Line thickness at end:")
            step: 0.01
        }

        EoNumber {
            sid: "tieMidWidth"
            title: qsTrc("notation/editstyle/slursandties", "Line thickness middle:")
            step: 0.01
        }

        EoNumber {
            sid: "tieDottedWidth"
            title: qsTrc("notation/editstyle/slursandties", "Dotted line thickness:")
            step: 0.01
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Length and height")

        EoNumber {
            sid: "minTieLength"
            title: qsTrc("notation/editstyle/slursandties", "Minimum tie length:")
            step: 0.1
        }

        EoNumber {
            sid: "minHangingTieLength"
            title: qsTrc("notation/editstyle/slursandties", "Minimum hanging tie length:")
            step: 0.1
        }

        EoNumber {
            sid: "tieMinShoulderHeight"
            title: qsTrc("notation/editstyle/slursandties", "Minimum tie height:")
        }

        EoNumber {
            sid: "tieMaxShoulderHeight"
            title: qsTrc("notation/editstyle/slursandties", "Maximum tie height:")
        }

        EoNumber {
            sid: "tieMinDistance"
            title: qsTrc("notation/editstyle/slursandties", "Autoplace min. distance:")
            step: 0.1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Tie end points")

        EoNumber {
            sid: "tieEndpointYOffset"
            title: qsTrc("notation/engravingoptions", "Vertical offset from notehead")
            hint: qsTrc("notation/engravingoptions",
                         "Vertical distance of tie ends from the top or bottom of the notehead (outside ties) or from its centre (inside ties).")
            max: 1
        }

        EoNumber {
            sid: "tieOutsideInset"
            title: qsTrc("notation/engravingoptions", "Outside tie: inward shift from notehead centre")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal shift of the end of an outside tie from the optical centre of the notehead towards the other note.")
            max: 1
        }

        EoNumber {
            sid: "tieOutsideInsetChord"
            title: qsTrc("notation/engravingoptions", "Outside tie: inward shift past another chord note")
            hint: qsTrc("notation/engravingoptions",
                         "The same shift when another note of the chord lies directly above or below on the side the tie curves towards.")
            max: 1.5
        }

        EoNumber {
            sid: "tieInsideInset"
            title: qsTrc("notation/engravingoptions", "Inside tie: gap to notehead")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal gap between the notehead and the end of an inside tie (also used for slash noteheads and hidden fret marks).")
            max: 1
        }

        EoNumber {
            sid: "tieItemPadding"
            title: qsTrc("notation/engravingoptions", "Gap to dots, accidentals and other items")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal gap kept between a tie end and the augmentation dots, accidentals, stems or noteheads it has to clear.")
            max: 1
        }

        EoNumber {
            sid: "tieAdjacentTieInset"
            title: qsTrc("notation/engravingoptions", "Shift between consecutive ties")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal shift of a tie end when the same note also carries a tie on its other side curving the same way, so that the two do not touch.")
            max: 1
        }

        EoNumber {
            sid: "tieLedgerLineMargin"
            title: qsTrc("notation/engravingoptions", "End point distance from ledger lines")
            hint: qsTrc("notation/engravingoptions",
                         "Tie ends closer than this to a ledger line on the side they curve towards are moved away from it.")
            max: 1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Tie shape and staff lines")

        EoNumber {
            sid: "tieShoulderHeightFactor"
            title: qsTrc("notation/engravingoptions", "Height growth with length")
            hint: qsTrc("notation/engravingoptions",
                         "Tie height in sp = minimum height + factor × √|length − 1| (length in sp), limited by the maximum tie height.")
            max: 2
        }

        EoNumber {
            sid: "tieShoulderWidth"
            title: qsTrc("notation/engravingoptions", "Shoulder width")
            hint: qsTrc("notation/engravingoptions",
                         "Distance between the two curve control points as a fraction of the tie length; larger values give a flatter middle and steeper ends.")
            max: 1
        }

        EoNumber {
            sid: "tieEndpointStaffLineMargin"
            title: qsTrc("notation/engravingoptions", "End point distance from staff lines")
            hint: qsTrc("notation/engravingoptions",
                         "Tie ends that would lie closer than this to a staff line are moved away from it.")
            max: 0.45
            step: 0.025
            decimals: 3
        }

        EoNumber {
            sid: "tieArcStaffLineMargin"
            title: qsTrc("notation/engravingoptions", "Arc distance from staff lines")
            hint: qsTrc("notation/engravingoptions",
                         "The apex of a tie inside the staff keeps at least this distance from a staff line.")
            max: 0.45
            step: 0.025
            decimals: 3
        }

        EoNumber {
            sid: "tieArcStaffLineMarginShort"
            title: qsTrc("notation/engravingoptions", "Arc distance from staff lines (short ties)")
            hint: qsTrc("notation/engravingoptions", "The same distance for ties shorter than the short-tie length.")
            max: 0.45
            step: 0.025
            decimals: 3
        }

        EoNumber {
            sid: "tieArcStaffLineShortLength"
            title: qsTrc("notation/engravingoptions", "Short-tie length for staff line distance")
            hint: qsTrc("notation/engravingoptions", "Ties shorter than this use the arc distance for short ties.")
            max: 20
            step: 0.5
        }

        EoNumber {
            sid: "tieStackedClearance"
            title: qsTrc("notation/engravingoptions", "Gap between stacked ties")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum vertical distance between neighbouring ties of a chord that curve the same way.")
            max: 1
            step: 0.025
            decimals: 3
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Laissez vibrer")

        EoNumber {
            sid: "minLaissezVibLength"
            title: qsTrc("notation/editstyle/slursandties", "Minimum laissez vibrer length:")
            step: 0.1
        }

        EoToggle {
            sid: "laissezVibUseSmuflSym"
            title: qsTrc("notation/editstyle/slursandties", "Use SMuFL symbols")
        }

        EoNumber {
            sid: "laissezVibParenthesisPadding"
            title: qsTrc("notation/engravingoptions", "Gap to parenthesis")
            hint: qsTrc("notation/engravingoptions",
                         "Vertical gap between a laissez vibrer tie and the parenthesis of the notehead that it has to avoid.")
            max: 1
        }
    }
}
