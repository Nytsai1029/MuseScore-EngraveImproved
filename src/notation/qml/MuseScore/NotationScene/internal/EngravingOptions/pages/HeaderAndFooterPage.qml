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

//! NOTE: the texts which are printed at the top and the foot of every page of the score.
//! Page texts are not music: the page has no preview pane until the preview can show a page
EoPage {
    id: page

    code: "header-and-footer"
    legacyCodes: ["headerAndFooter"]
    title: qsTrc("EditStyleBase", "Header & footer")
    group: qsTrc("notation/engravingoptions", "Layout")

    hasPreview: false

    EoSection {
        title: qsTrc("EditStyleBase", "Header")

        EoToggle {
            sid: "showHeader"
            title: qsTrc("notation/engravingoptions", "Show header")
        }

        EoToggle {
            sid: "headerFirstPage"
            title: qsTrc("EditStyleBase", "Show on first page")
        }

        EoToggle {
            sid: "headerOddEven"
            title: qsTrc("EditStyleBase", "Different odd/even pages")
        }

        EoGrid {
            cornerTitle: qsTrc("EditStyleBase", "Header")
            rowTitles: [qsTrc("EditStyleBase", "Odd"), qsTrc("EditStyleBase", "Even")]
            columnTitles: [
                qsTrc("EditStyleBase", "Left"), qsTrc("EditStyleBase", "Middle"), qsTrc("EditStyleBase", "Right")
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
                enabled: Boolean(page.value("headerOddEven"))
            }
            EoText {
                multiline: true
                sid: "evenHeaderC"
                enabled: Boolean(page.value("headerOddEven"))
            }
            EoText {
                multiline: true
                sid: "evenHeaderR"
                enabled: Boolean(page.value("headerOddEven"))
            }
        }

    }

    EoSection {
        title: qsTrc("EditStyleBase", "Footer")

        EoToggle {
            sid: "showFooter"
            title: qsTrc("notation/engravingoptions", "Show footer")
        }

        EoToggle {
            sid: "footerFirstPage"
            title: qsTrc("EditStyleBase", "Show on first page")
        }

        EoToggle {
            sid: "footerOddEven"
            title: qsTrc("EditStyleBase", "Different odd/even pages")
        }

        EoGrid {
            cornerTitle: qsTrc("EditStyleBase", "Footer")
            rowTitles: [qsTrc("EditStyleBase", "Odd"), qsTrc("EditStyleBase", "Even")]
            columnTitles: [
                qsTrc("EditStyleBase", "Left"), qsTrc("EditStyleBase", "Middle"), qsTrc("EditStyleBase", "Right")
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
                enabled: Boolean(page.value("footerOddEven"))
            }
            EoText {
                multiline: true
                sid: "evenFooterC"
                enabled: Boolean(page.value("footerOddEven"))
            }
            EoText {
                multiline: true
                sid: "evenFooterR"
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
