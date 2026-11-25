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
        music: "\\staves 2 \\group bracket c''1 | c''1"

        EoGrid {
            rowTitles: [
                qsTrc("notation/engravingoptions", "Bracket"),
                qsTrc("notation/engravingoptions", "Brace")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Thickness"),
                qsTrc("notation/engravingoptions", "Distance from the staves")
            ]

            EoNumber {
                sid: "bracketWidth"
                hint: qsTrc("notation/engravingoptions", "The thickness of the bracket’s heavy vertical line.")
                min: -1000
                max: 1000
                step: 0.1
            }

            EoNumber {
                sid: "bracketDistance"
                hint: qsTrc("notation/engravingoptions",
                            "The gap between a bracket and the line which joins the staves at the start of the " +
                            "system.")
                min: -1000
                max: 1000
                step: 0.1
            }

            EoNumber {
                sid: "akkoladeWidth"
                music: "\\staves 2 \\group brace c''1 | c''1"
                musicStyle: ({ musicalSymbolFont: "Emmentaler" })
                hint: qsTrc("notation/engravingoptions",
                            "Only with the music fonts Emmentaler and Gonville, where it is the width of the whole " +
                            "brace; with every other font the brace is the font’s own symbol, shaped by ‘Brace " +
                            "width’.")
                step: 1.0
            }

            EoNumber {
                sid: "akkoladeBarDistance"
                music: "\\staves 2 \\group brace c''1 | c''1"
                hint: qsTrc("notation/engravingoptions",
                            "The gap between a brace and the line which joins the staves at the start of the system.")
                step: 1.0
            }
        }

        EoToggle {
            sid: "alignSystemToMargin"
            previewScope: "page"
            musicStyle: ({
                pageWidth: 4.5,
                pageHeight: 2.3,
                pagePrintableWidth: 3.3,
                pageOddLeftMargin: 0.6,
                pageEvenLeftMargin: 0.6,
                pageOddTopMargin: 0.35,
                pageEvenTopMargin: 0.35,
                pageOddBottomMargin: 0.35,
                pageEvenBottomMargin: 0.35,
                headerFirstPage: true,
                oddHeaderL: "Sonata",
                headerFontSize: 11,
                enableIndentationOnFirstSystem: false,
                firstSystemInstNameVisibility: 2
            })
            title: qsTrc("EditStyleBase", "Let brackets extend into the left margin when there are no instrument names")
            hint: qsTrc("notation/engravingoptions",
                        "On, the staves start at the left margin and the brackets hang out into it; off, the staves " +
                        "are moved right so that the brackets stay inside the margin.")
        }

        EoNumber {
            sid: "bracketOvershoot"
            musicStyle: ({ musicalSymbolFont: "Emmentaler" })
            title: qsTrc("notation/engravingoptions", "Bracket overshoot beyond the staff")
            hint: qsTrc("notation/engravingoptions",
                        "How far the bracket’s heavy line runs on above the top staff line and below the bottom one, " +
                        "before the curved tips begin. For every music font except Leland.")
            max: 2
        }

        EoNumber {
            sid: "bracketOvershootLeland"
            title: qsTrc("notation/engravingoptions", "Bracket overshoot beyond the staff (Leland)")
            hint: qsTrc("notation/engravingoptions",
                        "How far the bracket’s heavy line runs on above the top staff line and below the bottom one, " +
                        "before the curved tips begin, when the music font is Leland.")
            max: 2
        }

        EoNumber {
            sid: "bracketSquareHookLength"
            music: "\\staves 2 \\group square c''1 | c''1"
            title: qsTrc("notation/engravingoptions", "Square bracket hook length")
            hint: qsTrc("notation/engravingoptions",
                        "The length of the two horizontal strokes of a square bracket; it is also how far the " +
                        "bracket stands out to the left of the staves.")
            max: 3
        }

        EoNumber {
            sid: "bracketLineWidthRatio"
            music: "\\staves 2 \\group line c''1 | c''1"
            title: qsTrc("notation/engravingoptions", "Line bracket thickness")
            hint: qsTrc("notation/engravingoptions",
                        "The thickness of a line bracket, a plain vertical line without tips, as a multiple of the " +
                        "bracket thickness: 0.67 is two thirds of it.")
            min: 0.1
            max: 2
            step: 0.01
        }

        EoNumber {
            sid: "braceWidthGapFactor"
            music: "\\staves 2 \\group brace c''1 | c''1"
            title: qsTrc("notation/engravingoptions", "Brace width")
            hint: qsTrc("notation/engravingoptions",
                        "How broad a brace is drawn: this much is added, in widths of the font’s brace symbol, for " +
                        "every gap between the staves it joins, so a smaller value gives a slimmer brace. Not used " +
                        "with the music fonts Emmentaler and Gonville.")
            max: 5
            step: 0.025
            decimals: 3
        }

        EoNumber {
            sid: "bracketSquareLineWidthRatio"
            music: "\\staves 2 \\group square c''1 | c''1"
            title: qsTrc("notation/engravingoptions", "Square bracket thickness")
            hint: qsTrc("notation/engravingoptions",
                        "The thickness of the lines of a square bracket as a multiple of the staff line thickness.")
            min: 0.5
            max: 20
            step: 0.5
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "System dividers")
        hint: qsTrc("notation/engravingoptions",
                    "The pair of slanting strokes which separates the systems of a large score, at the left or the " +
                    "right margin or both.")
        music: "\\staves 2 c''4 c''4 c''4 c''4 | c''1 \\break c''1 | c''1"
        systemWidth: 30

        EoGrid {
            cornerTitle: qsTrc("notation/engravingoptions", "Divider")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Left"),
                qsTrc("notation/engravingoptions", "Right")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Show"),
                qsTrc("notation/engravingoptions", "Symbol"),
                qsTrc("notation/engravingoptions", "Horizontal offset"),
                qsTrc("notation/engravingoptions", "Vertical offset")
            ]

            EoToggle {
                sid: "dividerLeft"
                hint: qsTrc("notation/engravingoptions",
                            "A divider is drawn at the left margin between every two systems of a page.")
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
                hint: qsTrc("notation/engravingoptions",
                            "Positive values move the left divider to the right, towards the staves.")
                min: -99
                step: 1.0
                unit: qsTrc("global", "sp")
                enabled: Boolean(page.value("dividerLeft"))
            }

            EoNumber {
                sid: "dividerLeftY"
                musicStyle: ({ dividerLeft: true })
                hint: qsTrc("notation/engravingoptions",
                            "Positive values move the left divider down from the middle between the two systems.")
                min: -99
                step: 1.0
                unit: qsTrc("global", "sp")
                enabled: Boolean(page.value("dividerLeft"))
            }

            EoToggle {
                sid: "dividerRight"
                hint: qsTrc("notation/engravingoptions",
                            "A divider is drawn at the right margin between every two systems of a page.")
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
                hint: qsTrc("notation/engravingoptions",
                            "Positive values move the right divider to the right, out into the margin; negative " +
                            "values towards the staves.")
                min: -99
                step: 1.0
                unit: qsTrc("global", "sp")
                enabled: Boolean(page.value("dividerRight"))
            }

            EoNumber {
                sid: "dividerRightY"
                musicStyle: ({ dividerRight: true })
                hint: qsTrc("notation/engravingoptions",
                            "Positive values move the right divider down from the middle between the two systems.")
                min: -99
                step: 1.0
                unit: qsTrc("global", "sp")
                enabled: Boolean(page.value("dividerRight"))
            }
        }
    }
}
