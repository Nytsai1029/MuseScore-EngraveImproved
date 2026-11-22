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

    code: "hammer-ons-pull-offs-and-tapping"
    legacyCodes: ["hammer-ons-pull-offs-and-tapping"]
    title: qsTrc("EditStyleBase", "Hammer-ons, pull-offs & tapping")
    group: qsTrc("notation/engravingoptions", "Fretted instruments")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Tapping")

        EoNumber {
            sid: "lhTappingDotMag"
            title: qsTrc("notation/engravingoptions", "Left-hand tapping dot scale")
            hint: qsTrc("notation/engravingoptions",
                         "Size of the dot used as the left-hand tapping symbol relative to the glyph’s normal size.")
            min: 0.2
            max: 2
        }
    }
}
