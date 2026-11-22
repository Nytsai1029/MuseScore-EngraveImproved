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

    code: "system"
    legacyCodes: ["system"]
    title: qsTrc("EditStyleBase", "System")
    group: qsTrc("notation/engravingoptions", "Layout")

    EoSection {
        title: qsTrc("EditStyleBase", "System brackets")

        EoNumber {
            sid: "bracketWidth"
            title: qsTrc("EditStyleBase", "Bracket thickness:")
            min: -1000
            max: 1000
            step: 0.1
        }

        EoNumber {
            sid: "akkoladeWidth"
            title: qsTrc("EditStyleBase", "Brace thickness:")
            step: 1.0
        }

        EoNumber {
            sid: "bracketDistance"
            title: qsTrc("EditStyleBase", "Bracket distance:")
            min: -1000
            max: 1000
            step: 0.1
        }

        EoNumber {
            sid: "akkoladeBarDistance"
            title: qsTrc("EditStyleBase", "Brace distance:")
            step: 1.0
        }

        EoToggle {
            sid: "alignSystemToMargin"
            title: qsTrc("EditStyleBase",
                          "Let brackets extend into the left margin when there are no instrument names")
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "System dividers")

        EoToggle {
            sid: "dividerLeft"
            title: qsTrc("notation/engravingoptions", "Divider on the left")
        }

        EoChoice {
            sid: "dividerLeftSym"
            title: qsTrc("EditStyleBase", "Symbol:")
            presentation: "dropdown"
            choices: page.model.choices("dividerSymbols")
            enabled: Boolean(page.value("dividerLeft"))
        }

        EoNumber {
            sid: "dividerLeftX"
            title: qsTrc("EditStyleBase", "Horizontal offset:")
            min: -99
            step: 1.0
            unit: qsTrc("global", "sp")
            enabled: Boolean(page.value("dividerLeft"))
        }

        EoNumber {
            sid: "dividerLeftY"
            title: qsTrc("EditStyleBase", "Vertical offset:")
            min: -99
            step: 1.0
            unit: qsTrc("global", "sp")
            enabled: Boolean(page.value("dividerLeft"))
        }

        EoToggle {
            sid: "dividerRight"
            title: qsTrc("notation/engravingoptions", "Divider on the right")
        }

        EoChoice {
            sid: "dividerRightSym"
            title: qsTrc("EditStyleBase", "Symbol:")
            presentation: "dropdown"
            choices: page.model.choices("dividerSymbols")
            enabled: Boolean(page.value("dividerRight"))
        }

        EoNumber {
            sid: "dividerRightX"
            title: qsTrc("EditStyleBase", "Horizontal offset:")
            min: -99
            step: 1.0
            unit: qsTrc("global", "sp")
            enabled: Boolean(page.value("dividerRight"))
        }

        EoNumber {
            sid: "dividerRightY"
            title: qsTrc("EditStyleBase", "Vertical offset:")
            min: -99
            step: 1.0
            unit: qsTrc("global", "sp")
            enabled: Boolean(page.value("dividerRight"))
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Bracket and brace shape")
    }
}
