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
        title: qsTrc("notation/engravingoptions", "Brackets and braces")

        EoGrid {
            cornerTitle: qsTrc("notation/engravingoptions", "Brackets and braces")
            rowTitles: [qsTrc("notation/engravingoptions", "Bracket"), qsTrc("notation/engravingoptions", "Brace")]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Thickness"),
                qsTrc("notation/engravingoptions", "Distance from the staves")
            ]

            EoNumber {
                sid: "bracketWidth"
                music: "\\staves 2 \\group bracket c''1 | c''1"
                min: -1000
                max: 1000
                step: 0.1
            }
            EoNumber {
                sid: "bracketDistance"
                music: "\\staves 2 \\group bracket c''1 | c''1"
                min: -1000
                max: 1000
                step: 0.1
            }
            EoNumber {
                sid: "akkoladeWidth"
                music: "\\staves 2 \\group brace c''1 | c''1"
                musicStyle: ({ musicalSymbolFont: "Emmentaler" })
                step: 1.0
            }
            EoNumber {
                sid: "akkoladeBarDistance"
                music: "\\staves 2 \\group brace c''1 | c''1"
                step: 1.0
            }
        }

        EoToggle {
            sid: "alignSystemToMargin"
            music: "\\staves 2 \\group bracket c''1 | c''1"
            title: qsTrc("EditStyleBase",
                          "Let brackets extend into the left margin when there are no instrument names")
        }

        EoNumber {
            sid: "bracketOvershoot"
            music: "\\staves 2 \\group bracket c''1 | c''1"
            musicStyle: ({ musicalSymbolFont: "Emmentaler" })
            title: qsTrc("notation/engravingoptions", "Bracket overshoot beyond the staff")
            hint: qsTrc("notation/engravingoptions",
                         "How far the thick line of a bracket extends above the top staff and below the bottom staff before its tips, with music fonts other than Leland.")
            max: 2
        }

        EoNumber {
            sid: "bracketOvershootLeland"
            music: "\\staves 2 \\group bracket c''1 | c''1"
            title: qsTrc("notation/engravingoptions", "Bracket overshoot beyond the staff (Leland)")
            hint: qsTrc("notation/engravingoptions",
                         "How far the thick line of a bracket extends above the top staff and below the bottom staff before its tips, with the music font Leland.")
            max: 2
        }

        EoNumber {
            sid: "bracketSquareHookLength"
            music: "\\staves 2 \\group square c''1 | c''1"
            title: qsTrc("notation/engravingoptions", "Square bracket hook length")
            hint: qsTrc("notation/engravingoptions",
                         "Length of the horizontal hooks of a square (sub-)bracket, which is also the room the bracket takes up left of the staves.")
            max: 3
        }

        EoNumber {
            sid: "bracketSquareLineWidthRatio"
            music: "\\staves 2 \\group square c''1 | c''1"
            title: qsTrc("notation/engravingoptions", "Square bracket thickness (× staff line thickness)")
            hint: qsTrc("notation/engravingoptions",
                         "Thickness of a square (sub-)bracket as a multiple of the thickness of a staff line.")
            min: 0.5
            max: 20
            step: 0.5
        }

        EoNumber {
            sid: "bracketLineWidthRatio"
            music: "\\staves 2 \\group line c''1 | c''1"
            title: qsTrc("notation/engravingoptions", "Line bracket thickness (× bracket thickness)")
            hint: qsTrc("notation/engravingoptions",
                         "Thickness of a line bracket as a multiple of the thickness of a regular bracket.")
            min: 0.1
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "braceWidthGapFactor"
            music: "\\staves 2 \\group brace c''1 | c''1"
            title: qsTrc("notation/engravingoptions", "Brace width growth per staff gap")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal stretch of a brace is one glyph width per staff it spans plus this much per gap between those staves; lower values give a slimmer brace.")
            max: 5
            step: 0.025
            decimals: 3
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "System dividers")
        music: "\\staves 2 c''4 c''4 c''4 c''4 | c''1 \\break c''1 | c''1"
        systemWidth: 30

        EoGrid {
            cornerTitle: qsTrc("notation/engravingoptions", "Divider")
            rowTitles: [qsTrc("notation/engravingoptions", "Left"), qsTrc("notation/engravingoptions", "Right")]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Show"), qsTrc("notation/engravingoptions", "Symbol"),
                qsTrc("notation/engravingoptions", "Horizontal offset"),
                qsTrc("notation/engravingoptions", "Vertical offset")
            ]

            EoToggle {
                sid: "dividerLeft"
            }
            EoChoice {
                sid: "dividerLeftSym"
                musicStyle: ({ dividerLeft: true })
                presentation: "dropdown"
                choices: page.model.choices("dividerSymbols")
                enabled: Boolean(page.value("dividerLeft"))
            }
            EoNumber {
                sid: "dividerLeftX"
                musicStyle: ({ dividerLeft: true })
                min: -99
                step: 1.0
                unit: qsTrc("global", "sp")
                enabled: Boolean(page.value("dividerLeft"))
            }
            EoNumber {
                sid: "dividerLeftY"
                musicStyle: ({ dividerLeft: true })
                min: -99
                step: 1.0
                unit: qsTrc("global", "sp")
                enabled: Boolean(page.value("dividerLeft"))
            }

            EoToggle {
                sid: "dividerRight"
            }
            EoChoice {
                sid: "dividerRightSym"
                musicStyle: ({ dividerRight: true })
                presentation: "dropdown"
                choices: page.model.choices("dividerSymbols")
                enabled: Boolean(page.value("dividerRight"))
            }
            EoNumber {
                sid: "dividerRightX"
                musicStyle: ({ dividerRight: true })
                min: -99
                step: 1.0
                unit: qsTrc("global", "sp")
                enabled: Boolean(page.value("dividerRight"))
            }
            EoNumber {
                sid: "dividerRightY"
                musicStyle: ({ dividerRight: true })
                min: -99
                step: 1.0
                unit: qsTrc("global", "sp")
                enabled: Boolean(page.value("dividerRight"))
            }
        }
    }
}
