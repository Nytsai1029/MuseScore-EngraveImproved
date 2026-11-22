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
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0
import MuseScore.Atelier 1.0

//! NOTE: a page of the dialog as it is shown: all its sections, which are scrolled
StyledFlickable {
    id: root

    //! NOTE: an EoPage
    required property var page

    //! NOTE: the EngravingOptionsModel
    required property var optionsModel

    property NavigationPanel navigationPanel: null

    //! NOTE: the section at the top of what is seen
    readonly property int currentSectionIndex: {
        content.height
        return content.sectionIndexAt(contentY + 24)
    }

    readonly property int sideMargin: 28

    //! NOTE: the options are not stretched over a wide dialog: the title and the control stay near each other
    readonly property int maxContentWidth: 780

    function scrollToSection(sectionIndex) {
        var maxContentY = Math.max(0, contentHeight - height)
        contentY = Math.min(content.sectionY(sectionIndex), maxContentY)
    }

    contentWidth: width
    contentHeight: content.y + content.height + 32

    onPageChanged: {
        contentY = 0
    }

    EoPageContent {
        id: content

        x: root.sideMargin
        y: 8
        width: Math.min(root.width - 2 * root.sideMargin, root.maxContentWidth)

        page: root.page
        optionsModel: root.optionsModel

        navigationPanel: root.navigationPanel
    }
}
