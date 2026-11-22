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

    code: "articulations-and-ornaments"
    legacyCodes: ["articulations-and-ornaments"]
    title: qsTrc("EditStyleBase", "Articulations & ornaments")
    group: qsTrc("notation/engravingoptions", "Notations")

    EoSection {
        title: qsTrc("EditStyleBase", "Articulations & ornaments")

        EoNumber {
            sid: "articulationMinDistance"
            title: qsTrc("EditStyleBase", "Minimum vertical distance:")
            step: 0.1
        }

        EoNumber {
            sid: "propertyDistanceHead"
            title: qsTrc("EditStyleBase", "Distance from notehead:")
            step: 0.1
        }

        EoNumber {
            sid: "propertyDistanceStem"
            title: qsTrc("EditStyleBase", "Distance from stem:")
            step: 0.1
        }

        EoNumber {
            sid: "propertyDistance"
            title: qsTrc("EditStyleBase", "Distance from staff:")
            step: 0.1
        }

        EoNumber {
            sid: "articulationMag"
            title: qsTrc("EditStyleBase", "Articulation size:")
            max: 300
            step: 10
            decimals: 0
            percent: true
        }

        EoNumber {
            sid: "chordlineThickness"
            title: qsTrc("EditStyleBase", "Fall, doit, plop and scoop line thickness:")
            step: 0.01
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Articulation details")

        EoNumber {
            sid: "articulationTenutoTieClearance"
            title: qsTrc("notation/engravingoptions", "Tenuto: extra distance from notehead next to a tie")
            hint: qsTrc("notation/engravingoptions",
                         "Extra distance between a tenuto mark and the notehead when the mark sits outside the staff and an outer tie starts or ends on the same side of the note.")
            max: 3
        }

        EoNumber {
            sid: "articulationStaccatoTieClearance"
            title: qsTrc("notation/engravingoptions", "Staccato: extra distance from notehead next to a tie")
            hint: qsTrc("notation/engravingoptions",
                         "Extra distance between a staccato dot and the notehead when the dot sits outside the staff and an outer tie starts or ends on the same side of the note.")
            max: 3
        }

        EoNumber {
            sid: "articulationStaccatoAccentKern"
            title: qsTrc("notation/engravingoptions", "Staccato–accent kerning")
            hint: qsTrc("notation/engravingoptions",
                         "How far an accent is pulled towards a staccato dot stacked on the same side outside the staff.")
            max: 1
        }

        EoToggle {
            sid: "articulationMarcatoAbove"
            title: qsTrc("notation/engravingoptions", "Marcato always above the staff")
            hint: qsTrc("notation/engravingoptions",
                         "With automatic placement, marcato marks go above the staff whatever the stem direction; when off they are placed on the notehead side like other articulations.")
        }

        EoToggle {
            sid: "articulationStaccatoCloseToNote"
            title: qsTrc("notation/engravingoptions", "Staccato next to the notehead (inside staff and slurs)")
            hint: qsTrc("notation/engravingoptions",
                         "Staccato dots are placed right next to the note, in a staff space and inside slurs; when off they are stacked outside the staff with the other articulations.")
        }

        EoToggle {
            sid: "articulationTenutoCloseToNote"
            title: qsTrc("notation/engravingoptions", "Tenuto next to the notehead (inside staff and slurs)")
            hint: qsTrc("notation/engravingoptions",
                         "Tenuto marks are placed right next to the note, in a staff space and inside slurs; when off they are stacked outside the staff with the other articulations.")
        }

        EoNumber {
            sid: "articulationSkylineMinHorizontalClearance"
            title: qsTrc("notation/engravingoptions", "Articulations, ornaments and fermatas: horizontal clearance")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal margin within which neighbouring objects count as colliding when articulations, ornaments and fermatas are moved away from the staff automatically.")
            max: 2
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Ornaments")

        EoNumber {
            sid: "ornamentAccidentalDistance"
            title: qsTrc("notation/engravingoptions", "Ornament to accidental distance")
            hint: qsTrc("notation/engravingoptions",
                         "Vertical gap between an ornament and the accidental shown above or below it.")
            max: 2
        }

        EoNumber {
            sid: "ornamentAccidentalMag"
            title: qsTrc("notation/engravingoptions", "Ornament accidental scale")
            hint: qsTrc("notation/engravingoptions",
                         "Size of the accidentals above and below ornaments relative to normal accidentals.")
            min: 0.3
            max: 1.5
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Breath marks and caesuras")

        EoNumber {
            sid: "breathMarkYOffset"
            title: qsTrc("notation/engravingoptions", "Breath mark vertical position")
            hint: qsTrc("notation/engravingoptions",
                         "Vertical position of breath marks relative to the top staff line (the bottom line when placed below); negative values are above that line.")
            min: -5
            max: 5
            step: 0.1
        }

        EoNumber {
            sid: "caesuraYOffset"
            title: qsTrc("notation/engravingoptions", "Caesura vertical position")
            hint: qsTrc("notation/engravingoptions",
                         "Vertical position of caesuras relative to the top staff line (the bottom line when placed below); positive values are below that line.")
            min: -5
            max: 5
            step: 0.1
        }
    }
}
