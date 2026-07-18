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
import QtQuick 2.15

import Muse.Ui 1.0
import Muse.Dock 1.0

import MuseScore.FontDesign 1.0

DockPage {
    id: root

    objectName: "FontDesign"
    uri: "musescore://fontdesign"

    panels: [
        DockPanel {
            id: glyphBrowserPanel

            objectName: "fontDesignGlyphBrowserPanel"
            title: qsTrc("appshell", "Glyphs")

            width: 300
            minimumWidth: 220
            maximumWidth: 480

            floatable: true
            closable: true

            GlyphBrowserPanel {}
        },

        DockPanel {
            id: fontDesignInspectorPanel

            objectName: "fontDesignInspectorPanel"
            title: qsTrc("appshell", "Properties")

            width: 320
            minimumWidth: 240
            maximumWidth: 480

            floatable: true
            closable: true

            location: Location.Right

            FontDesignInspectorPanel {}
        }
    ]

    central: fontDesignCentralComp

    Component {
        id: fontDesignCentralComp

        FontDesignCentral {}
    }
}
