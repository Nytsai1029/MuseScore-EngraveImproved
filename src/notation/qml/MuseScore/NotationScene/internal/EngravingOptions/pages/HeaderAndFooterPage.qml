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

    code: "header-and-footer"
    legacyCodes: ["headerAndFooter"]
    title: qsTrc("EditStyleBase", "Header & footer")
    group: qsTrc("notation/engravingoptions", "Layout")
    //! NOTE: three compact pages (first with title, even, odd) with larger header/footer text so the bands read clearly
    previewScope: "page"
    music: "\\title \"Sonata\" c''1 \\pageBreak c''1 \\pageBreak c''1"
    musicStyle: ({
        pageWidth: 4.5,
        pageHeight: 3.2,
        pagePrintableWidth: 3.7,
        pageOddLeftMargin: 0.4,
        pageEvenLeftMargin: 0.4,
        pageOddTopMargin: 0.35,
        pageEvenTopMargin: 0.35,
        pageOddBottomMargin: 0.35,
        pageEvenBottomMargin: 0.35,
        headerFontSize: 12,
        footerFontSize: 12
    })

    EoSection {
        title: qsTrc("notation/engravingoptions", "Header")

        EoToggle {
            sid: "showHeader"
            title: qsTrc("notation/engravingoptions", "Show header")
        }

        EoToggle {
            sid: "headerFirstPage"
            musicStyle: ({ oddHeaderC: "$:workTitle:" })
            title: qsTrc("EditStyleBase", "Show on first page")
        }

        EoToggle {
            sid: "headerOddEven"
            title: qsTrc("EditStyleBase", "Different odd/even pages")
            hint: qsTrc("notation/engravingoptions", "Off, the texts of the odd pages are used on every page.")
        }

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Header text")
            hint: qsTrc("notation/engravingoptions",
                        "Each field may hold several lines and the variables listed below, such as $p for the page " +
                        "number.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Odd pages"),
                qsTrc("notation/engravingoptions", "Even pages")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Left"),
                qsTrc("notation/engravingoptions", "Middle"),
                qsTrc("notation/engravingoptions", "Right")
            ]

            EoText {
                multiline: true
                sid: "oddHeaderL"
            }

            EoText {
                multiline: true
                sid: "oddHeaderC"
            }

            EoText {
                multiline: true
                sid: "oddHeaderR"
            }

            EoText {
                multiline: true
                sid: "evenHeaderL"
                hint: qsTrc("notation/engravingoptions", "Only with ‘Different odd/even pages’ on.")
                enabled: Boolean(page.value("headerOddEven"))
            }

            EoText {
                multiline: true
                sid: "evenHeaderC"
                hint: qsTrc("notation/engravingoptions", "Only with ‘Different odd/even pages’ on.")
                enabled: Boolean(page.value("headerOddEven"))
            }

            EoText {
                multiline: true
                sid: "evenHeaderR"
                hint: qsTrc("notation/engravingoptions", "Only with ‘Different odd/even pages’ on.")
                enabled: Boolean(page.value("headerOddEven"))
            }
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Footer")

        EoToggle {
            sid: "showFooter"
            musicStyle: ({ oddFooterR: "$p", evenFooterL: "$p" })
            title: qsTrc("notation/engravingoptions", "Show footer")
        }

        EoToggle {
            sid: "footerFirstPage"
            musicStyle: ({ oddFooterC: "$:workTitle:" })
            title: qsTrc("EditStyleBase", "Show on first page")
        }

        EoToggle {
            sid: "footerOddEven"
            musicStyle: ({ oddFooterR: "$p", evenFooterL: "$p" })
            title: qsTrc("EditStyleBase", "Different odd/even pages")
            hint: qsTrc("notation/engravingoptions", "Off, the texts of the odd pages are used on every page.")
        }

        EoGrid {
            title: qsTrc("notation/engravingoptions", "Footer text")
            hint: qsTrc("notation/engravingoptions",
                        "Each field may hold several lines and the variables listed below, such as $p for the page " +
                        "number.")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Odd pages"),
                qsTrc("notation/engravingoptions", "Even pages")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Left"),
                qsTrc("notation/engravingoptions", "Middle"),
                qsTrc("notation/engravingoptions", "Right")
            ]

            EoText {
                multiline: true
                sid: "oddFooterL"
            }

            EoText {
                multiline: true
                sid: "oddFooterC"
            }

            EoText {
                multiline: true
                sid: "oddFooterR"
            }

            EoText {
                multiline: true
                sid: "evenFooterL"
                hint: qsTrc("notation/engravingoptions", "Only with ‘Different odd/even pages’ on.")
                enabled: Boolean(page.value("footerOddEven"))
            }

            EoText {
                multiline: true
                sid: "evenFooterC"
                hint: qsTrc("notation/engravingoptions", "Only with ‘Different odd/even pages’ on.")
                enabled: Boolean(page.value("footerOddEven"))
            }

            EoText {
                multiline: true
                sid: "evenFooterR"
                hint: qsTrc("notation/engravingoptions", "Only with ‘Different odd/even pages’ on.")
                enabled: Boolean(page.value("footerOddEven"))
            }
        }
    }

    EoSection {
        title: qsTrc("notation/editstyle", "Using variables in the header/footer")

        EoCustom {
            component: EoVariablesTable {
            }
        }
    }
}
