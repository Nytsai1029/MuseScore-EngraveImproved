/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
pragma Singleton

import QtQuick 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

//! NOTE: the colors and fonts of the Atelier skin, which the theme does not have a value for.
//! The names are the ones of the design
QtObject {
    readonly property bool dark: ui.theme.isDark

    readonly property color island: ui.theme.backgroundPrimaryColor
    readonly property color island2: ui.theme.backgroundSecondaryColor
    readonly property color raised: ui.theme.popupBackgroundColor
    readonly property color field: ui.theme.textFieldColor

    readonly property color text: ui.theme.fontPrimaryColor
    readonly property color text2: dark ? "#BDBDC3" : "#55555A"
    readonly property color text3: dark ? "#9C9CA3" : "#6B6B70"

    readonly property color stroke: ui.theme.strokeColor
    readonly property color stroke2: ui.theme.controlBorderColor.valid ? ui.theme.controlBorderColor : ui.theme.strokeColor

    readonly property color button: Utils.colorWithAlpha(ui.theme.buttonColor, ui.theme.buttonOpacityNormal)
    readonly property color hover: Utils.colorWithAlpha(ui.theme.buttonColor, ui.theme.buttonOpacityHover)
    readonly property color pressed: Utils.colorWithAlpha(ui.theme.buttonColor, ui.theme.buttonOpacityHit)
    readonly property color cell: dark ? Qt.rgba(1, 1, 1, 0.05) : Qt.rgba(0, 0, 0, 0.035)

    //! NOTE: what is behind the paper
    readonly property color canvas: dark ? "#17181B" : "#E2E3E8"

    readonly property color accent: ui.theme.accentColor
    readonly property color ink: dark ? ui.theme.linkColor : ui.theme.accentColor
    readonly property color soft: Utils.colorWithAlpha(ui.theme.accentColor, ui.theme.accentOpacityNormal)

    //! NOTE: vermilion, only for alignment guides and anchors
    readonly property color guide: "#D9381E"

    readonly property int toolBarHeight: 52
    readonly property int statusBarHeight: 26
    readonly property int libraryWidth: 296
    readonly property int railWidth: 48
    readonly property int propertiesWidth: 280

    //! NOTE: the first shortcut of an action, written as on the keys (e.g. "Ctrl+Alt+1" is "⌥⌘1" on a Mac)
    function keyHint(shortcuts) {
        if (!Boolean(shortcuts)) {
            return ""
        }

        var isMac = Qt.platform.os === "osx"
        var first = String(shortcuts).split(/[;,] ?/)[0]

        return first.replace(/Ctrl\+/g, isMac ? "⌘" : "Ctrl+")
                    .replace(/Meta\+/g, isMac ? "⌃" : "Meta+")
                    .replace(/Alt\+/g, isMac ? "⌥" : "Alt+")
                    .replace(/Shift\+/g, isMac ? "⇧" : "Shift+")
    }

    //! NOTE: the title of an action for a button. In some languages the access key of the menu item
    //! is written after the title, in brackets (e.g. "样式… (S)"), which a button does not need
    function plainTitle(title) {
        return String(title).replace(/\s*[(（][A-Za-z0-9][)）]\s*(…|\.\.\.)?\s*$/, "$1")
    }

    function font(pixelSize, weight) {
        return Qt.font({
            family: ui.theme.bodyFont.family,
            pixelSize: pixelSize,
            weight: weight === undefined ? Font.Normal : weight
        })
    }
}
