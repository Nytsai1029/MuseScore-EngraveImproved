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
    legacyCodes: ["header-and-footer"]
    title: qsTrc("EditStyleBase", "Header & footer")
    group: qsTrc("notation/engravingoptions", "Layout")

    EoSection {
        title: qsTrc("EditStyleBase", "Header & footer")

        EoToggle {
            sid: "showHeader"
            title: qsTrc("notation/engravingoptions", "Show header")
        }

        EoToggle {
            sid: "headerFirstPage"
            title: qsTrc("EditStyleBase", "Show on first page")
            enabled: Boolean(page.value("showHeader"))
        }

        EoToggle {
            sid: "headerOddEven"
            title: qsTrc("EditStyleBase", "Different odd/even pages")
            enabled: Boolean(page.value("showHeader"))
        }

        EoText {
            sid: "oddHeaderL"
            title: qsTrc("notation/engravingoptions", "Odd pages, left")
            multiline: true
            enabled: Boolean(page.value("showHeader"))
        }

        EoText {
            sid: "oddHeaderC"
            title: qsTrc("notation/engravingoptions", "Odd pages, middle")
            multiline: true
            enabled: Boolean(page.value("showHeader"))
        }

        EoText {
            sid: "oddHeaderR"
            title: qsTrc("notation/engravingoptions", "Odd pages, right")
            multiline: true
            enabled: Boolean(page.value("showHeader"))
        }

        EoText {
            sid: "evenHeaderL"
            title: qsTrc("notation/engravingoptions", "Even pages, left")
            multiline: true
            enabled: Boolean(page.value("showHeader"))
        }

        EoText {
            sid: "evenHeaderC"
            title: qsTrc("notation/engravingoptions", "Even pages, middle")
            multiline: true
            enabled: Boolean(page.value("showHeader"))
        }

        EoText {
            sid: "evenHeaderR"
            title: qsTrc("notation/engravingoptions", "Even pages, right")
            multiline: true
            enabled: Boolean(page.value("showHeader"))
        }

        EoToggle {
            sid: "showFooter"
            title: qsTrc("notation/engravingoptions", "Show footer")
        }

        EoToggle {
            sid: "footerFirstPage"
            title: qsTrc("EditStyleBase", "Show on first page")
            enabled: Boolean(page.value("showFooter"))
        }

        EoToggle {
            sid: "footerOddEven"
            title: qsTrc("EditStyleBase", "Different odd/even pages")
            enabled: Boolean(page.value("showFooter"))
        }

        EoText {
            sid: "oddFooterL"
            title: qsTrc("notation/engravingoptions", "Odd pages, left")
            multiline: true
            enabled: Boolean(page.value("showFooter"))
        }

        EoText {
            sid: "oddFooterC"
            title: qsTrc("notation/engravingoptions", "Odd pages, middle")
            multiline: true
            enabled: Boolean(page.value("showFooter"))
        }

        EoText {
            sid: "oddFooterR"
            title: qsTrc("notation/engravingoptions", "Odd pages, right")
            multiline: true
            enabled: Boolean(page.value("showFooter"))
        }

        EoText {
            sid: "evenFooterL"
            title: qsTrc("notation/engravingoptions", "Even pages, left")
            multiline: true
            enabled: Boolean(page.value("showFooter"))
        }

        EoText {
            sid: "evenFooterC"
            title: qsTrc("notation/engravingoptions", "Even pages, middle")
            multiline: true
            enabled: Boolean(page.value("showFooter"))
        }

        EoText {
            sid: "evenFooterR"
            title: qsTrc("notation/engravingoptions", "Even pages, right")
            multiline: true
            enabled: Boolean(page.value("showFooter"))
        }
    }
}
