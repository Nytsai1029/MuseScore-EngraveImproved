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

    code: "fingering"
    title: qsTrc("notation/engravingoptions", "Fingering")
    group: qsTrc("notation/engravingoptions", "Signs")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Fingering")

        EoToggle {
            sid: "fingeringBelowOnBottomStaff"
            music: "\\staves 2 \\staff 2 c4-5 e4-3 g4-2 c'4-1"
            title: qsTrc("notation/engravingoptions", "Below the notes on the bottom staff of an instrument")
            hint: qsTrc("notation/engravingoptions",
                        "On an instrument written on several staves, such as the piano, fingering of the bottom " +
                        "staff goes below the notes and that of the other staves above; off puts all fingering " +
                        "above. Where a staff has several voices, the stem direction decides instead.")
        }

        EoToggle {
            sid: "fingeringVoicesTightPlacement"
            music: "<< { e''4-4 d''4-3 c''4-2 b'4-1 } \\\\ { g'4-1 f'4-2 e'4-3 d'4-4 } >>"
            title: qsTrc("notation/engravingoptions", "In voices: fingering next to the note")
            hint: qsTrc("notation/engravingoptions",
                        "Where a staff has several voices, the fingering of a single note without a beam stands " +
                        "right by its notehead, at the two distances below; off places it beyond the stem and " +
                        "outside the staff like other fingering.")
        }

        EoNumber {
            sid: "fingeringVoicesXOffset"
            music: "<< { e''4-4 d''4-3 c''4-2 b'4-1 } \\\\ { g'4-1 f'4-2 e'4-3 d'4-4 } >>"
            title: qsTrc("notation/engravingoptions", "In voices: sideways shift away from the stem")
            hint: qsTrc("notation/engravingoptions",
                        "How far such a fingering is moved sideways from the middle of the notehead so that it " +
                        "clears the stem. Only with ‘In voices: fingering next to the note’ on.")
            max: 3
        }

        EoNumber {
            sid: "fingeringVoicesYOffset"
            music: "<< { e''4-4 d''4-3 c''4-2 b'4-1 } \\\\ { g'4-1 f'4-2 e'4-3 d'4-4 } >>"
            title: qsTrc("notation/engravingoptions", "In voices: distance above or below the note")
            hint: qsTrc("notation/engravingoptions",
                        "How far such a fingering stands above or below the middle of the notehead. Only with ‘In " +
                        "voices: fingering next to the note’ on.")
            max: 5
            step: 0.1
        }

        EoNumber {
            sid: "fingeringSkylineExtraHeightRatio"
            music: "c''4-.-3( d''4-4 e''2-5)"
            title: qsTrc("notation/engravingoptions", "Extra gap when moved out of the way")
            hint: qsTrc("notation/engravingoptions",
                        "A fingering which has to move away from a slur, articulation or other sign moves a little " +
                        "further, by this fraction of its own height; 0.25 is a quarter of the height of the digit.")
            max: 2
        }

        EoNumber {
            sid: "fingeringMinDistance"
            music: "c''4-1 e''4-2 g''4-3 b''4-5"
            title: qsTrc("notation/engravingoptions", "Minimum distance from the staff and the notes")
            hint: qsTrc("notation/engravingoptions",
                        "Fingering placed above or below a chord stays at least this far outside the staff, and this " +
                        "far beyond the outermost notehead or the end of a beamed stem.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Sticking")
        music: "c''4 \\sticking \"R\" c''4 \\sticking \"L\" c''4 \\sticking \"R\" c''4 \\sticking \"R\""
        hint: qsTrc("notation/engravingoptions",
                    "The letters under or over percussion notes which name the hand (L, R).")

        EoChoice {
            sid: "stickingPlacement"
            title: qsTrc("EditStyleBase", "Position:")
            hint: qsTrc("notation/engravingoptions",
                        "Side of the staff on which sticking letters are placed, unless one is flipped by hand.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Above") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Below") }
            ]
        }

        EoOffset {
            sid: "stickingPosAbove"
            musicStyle: ({ stickingPlacement: 0 })
            title: qsTrc("notation/engravingoptions", "Offset above the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal shift and height of sticking placed above, measured from the top staff line " +
                        "(negative is up); notes and signs in the way still push it further out.")
        }

        EoOffset {
            sid: "stickingPosBelow"
            title: qsTrc("notation/engravingoptions", "Offset below the staff")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal shift and depth of sticking placed below, measured from the bottom staff line " +
                        "(positive is down); notes and signs in the way still push it further out.")
        }

        EoNumber {
            sid: "stickingMinDistance"
            music: "c'4 \\sticking \"R\" a4 \\sticking \"L\" c'4 \\sticking \"R\" a4 \\sticking \"R\""
            title: qsTrc("notation/engravingoptions", "Minimum distance from notes and signs")
            hint: qsTrc("notation/engravingoptions",
                        "Least vertical gap between a sticking letter and the notes or other signs which lie between " +
                        "it and the staff.")
        }
    }
}
