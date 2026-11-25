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
    previewScope: "page"
    music: "\\instruments 2 \\group bracket c''1 \\break c''1 \\break c''1 \\break c''1 " +
           "\\break c''1 \\break c''1 \\break c''1 \\break c''1"

    EoSection {
        title: qsTrc("notation/engravingoptions", "Music area")

        EoNumber {
            sid: "staffUpperBorder"
            title: qsTrc("EditStyleBase", "Music top margin:")
            hint: qsTrc("notation/engravingoptions",
                        "The distance from the top page margin to the top line of the first staff on the page; notes " +
                        "and marks above that staff can push it further down.")
            min: -1000
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "staffLowerBorder"
            title: qsTrc("EditStyleBase", "Music bottom margin:")
            hint: qsTrc("notation/engravingoptions",
                        "The least distance from the bottom line of the last staff on the page to the bottom page " +
                        "margin.")
            min: -1000
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "staffHeaderFooterPadding"
            music: "\\instruments 2 \\group bracket c''1 | c''1"
            musicStyle: ({
                pageWidth: 4.5,
                pageHeight: 2.8,
                pagePrintableWidth: 3.7,
                pageOddLeftMargin: 0.4,
                pageEvenLeftMargin: 0.4,
                pageOddTopMargin: 0.35,
                pageEvenTopMargin: 0.35,
                pageOddBottomMargin: 0.35,
                pageEvenBottomMargin: 0.35,
                headerFirstPage: true,
                oddHeaderC: "Sonata",
                headerFontSize: 12,
                staffUpperBorder: 0
            })
            title: qsTrc("EditStyleBase", "Min. distance to header/footer:")
            hint: qsTrc("notation/engravingoptions",
                        "The least gap between the music, with everything above and below the staves, and the text " +
                        "of the header or footer.")
            step: 0.5
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Vertical justification")
        music: "\\instruments 2 \\group bracket c''1 \\break c''1 \\pageBreak c''1"

        EoToggle {
            sid: "enableVerticalSpread"
            title: qsTrc("notation/engravingoptions", "Justify staves vertically")
            hint: qsTrc("notation/engravingoptions",
                        "The gaps between staves and between systems are opened up evenly until the music reaches " +
                        "down to the bottom margin. Off, the staves of a system keep fixed distances and only the " +
                        "gaps between systems grow.")
        }

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Distance")
            hint: qsTrc("notation/engravingoptions",
                        "The limits between which the gaps may vary when the page is justified.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Between systems"),
                qsTrc("notation/engravingoptions", "Between staves")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Minimum"),
                qsTrc("notation/engravingoptions", "Maximum")
            ]

            EoNumber {
                sid: "minSystemSpread"
                music: "\\instruments 2 \\group bracket c''1 \\break c''1 \\break c''1"
                musicStyle: ({
                    pageWidth: 4.8,
                    pageHeight: 6.2,
                    pagePrintableWidth: 4.0,
                    pageOddLeftMargin: 0.4,
                    pageEvenLeftMargin: 0.4,
                    pageOddTopMargin: 0.35,
                    pageEvenTopMargin: 0.35,
                    pageOddBottomMargin: 0.35,
                    pageEvenBottomMargin: 0.35,
                    staffUpperBorder: 2.0,
                    maxSystemSpread: 8.5,
                    maxPageFillSpread: 0
                })
                hint: qsTrc("notation/engravingoptions",
                            "Measured from the bottom line of one system to the top line of the next; notes and " +
                            "marks in between can force more.")
                max: 1000
                step: 0.5
                decimals: 1
            }

            EoNumber {
                sid: "maxSystemSpread"
                musicStyle: ({ maxPageFillSpread: 100 })
                hint: qsTrc("notation/engravingoptions",
                            "Justifying never opens the gap between two systems further than this.")
                max: 1000
                step: 0.5
                decimals: 1
            }

            EoNumber {
                sid: "minStaffSpread"
                hint: qsTrc("notation/engravingoptions",
                            "Applies between all staves of a system, also between the two staves of a piano part.")
                max: 1000
                step: 0.5
                decimals: 1
            }

            EoNumber {
                sid: "maxStaffSpread"
                musicStyle: ({ maxPageFillSpread: 100 })
                hint: qsTrc("notation/engravingoptions",
                            "Justifying never opens the gap between two staves of a system further than this; " +
                            "between staves joined by a brace, ‘Max. grand staff distance’ applies instead.")
                max: 1000
                step: 0.5
                decimals: 1
            }
        }

        EoNumber {
            sid: "spreadSystem"
            title: qsTrc("EditStyleBase", "Factor for distance between systems:")
            hint: qsTrc("notation/engravingoptions",
                        "When the page is justified, a gap between two systems is made this many times as large as a " +
                        "gap between ordinary neighbouring staves of a system.")
            min: 1
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "spreadSquareBracket"
            title: qsTrc("EditStyleBase", "Factor for distance above/below bracket:")
            hint: qsTrc("notation/engravingoptions",
                        "When the page is justified, the gap between a bracketed group of staves and the staff above " +
                        "or below it is made this many times as large as a gap between two staves inside the group.")
            min: 1
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "spreadCurlyBracket"
            title: qsTrc("EditStyleBase", "Factor for distance above/below brace:")
            hint: qsTrc("notation/engravingoptions",
                        "When the page is justified, the gap between the staves joined by a brace, such as a piano " +
                        "part, and the staff above or below them is made this many times as large as a gap between " +
                        "ordinary neighbouring staves.")
            min: 1
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "maxAkkoladeDistance"
            music: "\\staves 2 c''1 \\break c''1 \\pageBreak c''1"
            title: qsTrc("EditStyleBase", "Max. grand staff distance:")
            hint: qsTrc("notation/engravingoptions",
                        "Justifying never opens the gap between the staves joined by a brace further than this.")
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "maxPageFillSpread"
            title: qsTrc("EditStyleBase", "Max. page fill distance:")
            hint: qsTrc("notation/engravingoptions",
                        "Once the gaps are even, each may be opened by at most this much more to bring the last " +
                        "staff down to the bottom margin; a page which would need more stays short.")
            step: 0.5
            decimals: 1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Without vertical justification")
        hint: qsTrc("notation/engravingoptions", "These distances apply while ‘Justify staves vertically’ is off.")
        music: "\\instruments 2 \\group bracket c''1 \\break c''1 \\pageBreak c''1"
        musicStyle: ({ enableVerticalSpread: false })

        EoNumber {
            sid: "staffDistance"
            title: qsTrc("EditStyleBase", "Staff distance:")
            hint: qsTrc("notation/engravingoptions",
                        "The distance between the staves of different instruments in a system, from bottom line to " +
                        "top line; notes and marks in between can force more.")
            min: -1000
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "akkoladeDistance"
            music: "\\staves 2 c''1 \\break c''1 \\pageBreak c''1"
            title: qsTrc("EditStyleBase", "Grand staff distance:")
            hint: qsTrc("notation/engravingoptions",
                        "The distance between the staves of one instrument, such as the two staves of a piano part.")
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "minSystemDistance"
            title: qsTrc("EditStyleBase", "Min. system distance:")
            hint: qsTrc("notation/engravingoptions",
                        "The least gap between two systems, from the bottom line of one to the top line of the next.")
            min: -1000
            max: 1000
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "maxSystemDistance"
            title: qsTrc("EditStyleBase", "Max. system distance:")
            hint: qsTrc("notation/engravingoptions",
                        "To fill the page, the gaps between systems are opened up evenly, but never further than " +
                        "this.")
            max: 1000
            step: 0.5
            decimals: 1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Frames")
        music: "\\title \"Sonata\" c''1"

        EoNumber {
            sid: "systemFrameDistance"
            title: qsTrc("notation/engravingoptions", "Gap above a vertical frame")
            hint: qsTrc("notation/engravingoptions",
                        "The distance between a vertical frame and the system above it, for every frame which has " +
                        "not been given its own.")
            min: -100
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "frameSystemDistance"
            title: qsTrc("notation/engravingoptions", "Gap below a vertical frame")
            hint: qsTrc("notation/engravingoptions",
                        "The distance between a vertical frame and the system below it, for every frame which has " +
                        "not been given its own.")
            min: -100
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "verticalFrameAutoSizeMinHeight"
            title: qsTrc("notation/engravingoptions", "Least height of an auto-sized vertical frame")
            hint: qsTrc("notation/engravingoptions",
                        "A vertical frame which takes its height from its content is never lower than this; the same " +
                        "holds for an empty fretboard diagram legend.")
            max: 30
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "paddingToNotationAbove"
            title: qsTrc("notation/engravingoptions", "Clearance for notation above a vertical frame")
            hint: qsTrc("notation/engravingoptions",
                        "Least white space between a vertical frame and the lowest note or sign of the system above " +
                        "it; the frame moves down when ‘Gap above a vertical frame’ would leave less. For every " +
                        "frame which has not been given its own.")
            step: 0.5
            decimals: 1
        }

        EoNumber {
            sid: "paddingToNotationBelow"
            music: "\\title \"Sonata\" c''''1"
            title: qsTrc("notation/engravingoptions", "Clearance for notation below a vertical frame")
            hint: qsTrc("notation/engravingoptions",
                        "Least white space between a vertical frame and the highest note or sign of the system below " +
                        "it; the system moves down when ‘Gap below a vertical frame’ would leave less. For every " +
                        "frame which has not been given its own.")
            step: 0.5
            decimals: 1
        }
    }
}
