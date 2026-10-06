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
import QtQuick.Controls 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

BaseSection {
    id: root

    title: qsTrc("appshell/preferences", "Interface skin")
    navigation.direction: NavigationPanel.Horizontal

    property alias skins: radioButtonList.model
    property string currentSkinCode: ""

    signal skinChangeRequested(string newSkinCode)

    RadioButtonGroup {
        id: radioButtonList

        width: parent.width
        height: implicitHeight

        spacing: root.columnSpacing
        orientation: ListView.Horizontal

        delegate: RoundedRadioButton {
            width: root.columnWidth

            leftPadding: 0
            spacing: 6

            property string title: modelData["title"]

            checked: root.currentSkinCode === modelData["code"]

            navigation.name: "SkinButton_" + modelData["code"]
            navigation.panel: root.navigation
            navigation.column: model.index
            navigation.accessible.name: title

            StyledTextLabel {
                text: title
                horizontalAlignment: Text.AlignLeft
            }

            onToggled: {
                root.skinChangeRequested(modelData["code"])
            }
        }
    }
}
