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

    code: "fermatas"
    legacyCodes: ["fermatas"]
    title: qsTrc("EditStyleBase", "Fermatas")
    group: qsTrc("notation/engravingoptions", "Notations")

    EoSection {
        title: qsTrc("EditStyleBase", "Fermatas")

        EoOffset {
            sid: "fermataPosAbove"
            title: qsTrc("EditStyleBase", "Offset above:")
        }

        EoOffset {
            sid: "fermataPosBelow"
            title: qsTrc("EditStyleBase", "Offset below:")
        }

        EoNumber {
            sid: "fermataMinDistance"
            title: qsTrc("EditStyleBase", "Autoplace min. distance:")
            max: 1000
            step: 0.2
        }
    }
}
