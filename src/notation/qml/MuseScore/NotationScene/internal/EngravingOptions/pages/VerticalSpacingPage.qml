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

    code: "vertical-spacing"
    title: qsTrc("EditStyleBase", "Vertical spacing")
    group: qsTrc("notation/engravingoptions", "Layout")

    EoSection {
        title: qsTrc("EditStyleBase", "Vertical spacing")

        EoNumber {
            sid: "staffUpperBorder"
            title: qsTrc("EditStyleBase", "Music top margin:")
            min: -1000
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "staffLowerBorder"
            title: qsTrc("EditStyleBase", "Music bottom margin:")
            min: -1000
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "staffHeaderFooterPadding"
            title: qsTrc("EditStyleBase", "Min. distance to header/footer:")
            step: 0.5
        }

        EoToggle {
            sid: "enableVerticalSpread"
            title: qsTrc("notation/engravingoptions", "Justify staves vertically")
            hint: qsTrc("notation/engravingoptions", "Spreads the staves and systems over the height of the page.")
        }

        EoNumber {
            sid: "spreadSystem"
            title: qsTrc("EditStyleBase", "Factor for distance between systems:")
            min: 1
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "spreadSquareBracket"
            title: qsTrc("EditStyleBase", "Factor for distance above/below bracket:")
            min: 1
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "spreadCurlyBracket"
            title: qsTrc("EditStyleBase", "Factor for distance above/below brace:")
            min: 1
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "minSystemSpread"
            title: qsTrc("EditStyleBase", "Min. system distance:")
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "maxSystemSpread"
            title: qsTrc("EditStyleBase", "Max. system distance:")
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "minStaffSpread"
            title: qsTrc("EditStyleBase", "Min. staff distance:")
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "maxStaffSpread"
            title: qsTrc("EditStyleBase", "Max. staff distance:")
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "maxAkkoladeDistance"
            title: qsTrc("EditStyleBase", "Max. grand staff distance:")
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "maxPageFillSpread"
            title: qsTrc("EditStyleBase", "Max. page fill distance:")
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "staffDistance"
            title: qsTrc("EditStyleBase", "Staff distance:")
            min: -1000
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "minSystemDistance"
            title: qsTrc("EditStyleBase", "Min. system distance:")
            min: -1000
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "akkoladeDistance"
            title: qsTrc("EditStyleBase", "Grand staff distance:")
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "maxSystemDistance"
            title: qsTrc("EditStyleBase", "Max. system distance:")
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "systemFrameDistance"
            title: qsTrc("EditStyleBase", "Vertical frame top margin:")
            min: -100
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "frameSystemDistance"
            title: qsTrc("EditStyleBase", "Vertical frame bottom margin:")
            min: -100
            step: 0.5
            decimals: 1
        }
    }
}
