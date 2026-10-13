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
import Muse.UiComponents 1.0
import MuseScore.AppShell 1.0
import MuseScore.Atelier 1.0

import MuseScore.FontDesign 1.0

//! NOTE: the font editor: the glyphs on the left, the glyph in the middle, its properties on the right
Item {
    id: root

    required property AtelierShellView shell

    readonly property string pageUri: "musescore://fontdesign"

    //! NOTE: the names which the font design module toggles
    readonly property string leftPaneName: "fontDesignGlyphBrowserPanel"
    readonly property string leftPaneTitle: qsTrc("appshell", "Glyphs")
    readonly property string rightPaneName: "fontDesignInspectorPanel"
    readonly property string rightPaneTitle: qsTrc("appshell", "Properties")

    Component.onCompleted: {
        root.shell.declarePanel(root.pageUri, root.leftPaneName, true)
        root.shell.declarePanel(root.pageUri, root.rightPaneName, true)
    }

    Item {
        id: leftPane

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        width: visible ? 300 : 0
        visible: root.shell.isPanelOpen(root.pageUri, root.leftPaneName, root.shell.panelsRevision)

        GlyphBrowserPanel {
            anchors.fill: parent
            anchors.rightMargin: 1
        }

        Rectangle {
            anchors.right: parent.right
            width: 1
            height: parent.height
            color: AtStyle.stroke
        }
    }

    Item {
        id: rightPane

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        width: visible ? 320 : 0
        visible: root.shell.isPanelOpen(root.pageUri, root.rightPaneName, root.shell.panelsRevision)

        Rectangle {
            width: 1
            height: parent.height
            color: AtStyle.stroke
        }

        FontDesignInspectorPanel {
            anchors.fill: parent
            anchors.leftMargin: 1
        }
    }

    Loader {
        anchors.left: leftPane.right
        anchors.right: rightPane.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        active: root.visible

        sourceComponent: FontDesignCentral {}
    }
}
