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

    code: "sizes"
    legacyCodes: ["sizes"]
    title: qsTrc("EditStyleBase", "Sizes")
    group: qsTrc("notation/engravingoptions", "Layout")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Small and grace sizes")

        EoNumber {
            sid: "smallStaffMag"
            music: "\\smallStaff c''4 d''4"
            title: qsTrc("EditStyleBase", "Small staff size:")
            hint: qsTrc("notation/engravingoptions",
                        "The size of a staff marked as small, such as an ossia or the solo line above a piano part, " +
                        "as a percentage of a normal staff.")
            min: 10
            max: 200
            step: 10
            decimals: 0
            percent: true
        }

        EoNumber {
            sid: "smallClefMag"
            music: "c''4 \\clef bass d4"
            title: qsTrc("EditStyleBase", "Small clef size:")
            hint: qsTrc("notation/engravingoptions",
                        "The size of a clef change in the course of the music, and of a courtesy clef, as a " +
                        "percentage of the clef at the start of a system.")
            min: 10
            max: 200
            step: 10
            decimals: 0
            percent: true
        }

        EoNumber {
            sid: "smallNoteMag"
            music: "\\small c''4 \\small d''4 e''2"
            title: qsTrc("EditStyleBase", "Small note size:")
            hint: qsTrc("notation/engravingoptions",
                        "The size of notes and rests marked as small, such as cue notes, as a percentage of normal " +
                        "notes.")
            min: 10
            max: 200
            step: 10
            decimals: 0
            percent: true
        }

        EoNumber {
            sid: "graceNoteMag"
            music: "\\acciaccatura e''8 d''4"
            title: qsTrc("EditStyleBase", "Grace note size:")
            hint: qsTrc("notation/engravingoptions",
                        "The size of grace notes as a percentage of normal notes; a grace note which is also marked " +
                        "as small is reduced by both.")
            min: 10
            max: 200
            step: 10
            decimals: 0
            percent: true
        }

        EoToggle {
            sid: "scaleRythmicSpacingForSmallNotes"
            music: "\\small c''8 \\small d''8 \\small e''8 \\small f''8 g''2"
            title: qsTrc("EditStyleBase", "Reduce rhythmic spacing for small notes")
            hint: qsTrc("notation/engravingoptions",
                        "Where only small notes move, as in a cue passage, the horizontal space they get is reduced " +
                        "in the same proportion as their size.")
        }
    }
}
