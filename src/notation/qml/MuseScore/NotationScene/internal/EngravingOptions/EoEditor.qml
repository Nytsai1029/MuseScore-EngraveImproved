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

import "editors"

//! NOTE: the control which edits the value of an option, chosen by the kind of the option
//! (and by its presentation, for a choice). The options in a list and the cells of a grid
//! both use it — the cells with `compact`
Loader {
    id: root

    required property var option
    required property var optionsModel

    property var session: null

    property NavigationPanel navigationPanel: null
    property int navigationRow: 0
    property int navigationColumn: 1
    property bool compact: false

    //! NOTE: the music of the option, for the control of a choice shown as pictures
    property var musicInfo: null
    property var musicStyle: ({})

    //! NOTE: whether the control takes the width of the page, under the title
    readonly property bool isWide: {
        if (!Boolean(option)) {
            return false
        }
        if (compact) {
            return false
        }
        switch (option.kind) {
        case "choice":
            return option.presentation === "tiles" || option.presentation === "list"
        case "text":
            return option.multiline
        }
        return false
    }

    sourceComponent: {
        if (!Boolean(option)) {
            return null
        }
        switch (option.kind) {
        case "number": return numberComp
        case "offset": return offsetComp
        case "scale": return scaleComp
        case "toggle": return toggleComp
        case "text": return textComp
        case "color": return colorComp
        case "font": return fontComp
        case "align": return alignComp
        case "fontStyle": return fontStyleComp
        case "choice":
            switch (option.presentation) {
            case "tiles": return root.compact ? choiceComp : tilesComp
            case "list": return root.compact ? choiceComp : listComp
            }
            return choiceComp
        }
        return null
    }

    Component {
        id: numberComp
        EoNumberEditor {
            option: root.option
            optionsModel: root.optionsModel
            session: root.session
            navigationPanel: root.navigationPanel
            navigationRow: root.navigationRow
            navigationColumn: root.navigationColumn
            compact: root.compact
        }
    }
    Component {
        id: offsetComp
        EoOffsetEditor {
            option: root.option
            optionsModel: root.optionsModel
            session: root.session
            navigationPanel: root.navigationPanel
            navigationRow: root.navigationRow
            navigationColumn: root.navigationColumn
            compact: root.compact
        }
    }
    Component {
        id: scaleComp
        EoScaleEditor {
            option: root.option
            optionsModel: root.optionsModel
            session: root.session
            navigationPanel: root.navigationPanel
            navigationRow: root.navigationRow
            navigationColumn: root.navigationColumn
            compact: root.compact
        }
    }
    Component {
        id: toggleComp
        EoToggleEditor {
            option: root.option
            optionsModel: root.optionsModel
            session: root.session
            navigationPanel: root.navigationPanel
            navigationRow: root.navigationRow
            navigationColumn: root.navigationColumn
            compact: root.compact
        }
    }
    Component {
        id: textComp
        EoTextEditor {
            option: root.option
            optionsModel: root.optionsModel
            session: root.session
            navigationPanel: root.navigationPanel
            navigationRow: root.navigationRow
            navigationColumn: root.navigationColumn
            compact: root.compact
        }
    }
    Component {
        id: colorComp
        EoColorEditor {
            option: root.option
            optionsModel: root.optionsModel
            session: root.session
            navigationPanel: root.navigationPanel
            navigationRow: root.navigationRow
            navigationColumn: root.navigationColumn
            compact: root.compact
        }
    }
    Component {
        id: alignComp
        EoAlignEditor {
            option: root.option
            optionsModel: root.optionsModel
            session: root.session
            navigationPanel: root.navigationPanel
            navigationRow: root.navigationRow
            navigationColumn: root.navigationColumn
            compact: root.compact
        }
    }
    Component {
        id: fontStyleComp
        EoFontStyleEditor {
            option: root.option
            optionsModel: root.optionsModel
            session: root.session
            navigationPanel: root.navigationPanel
            navigationRow: root.navigationRow
            navigationColumn: root.navigationColumn
            compact: root.compact
        }
    }
    Component {
        id: fontComp
        EoFontEditor {
            option: root.option
            optionsModel: root.optionsModel
            session: root.session
            navigationPanel: root.navigationPanel
            navigationRow: root.navigationRow
            navigationColumn: root.navigationColumn
            compact: root.compact
        }
    }
    Component {
        id: choiceComp
        EoChoiceEditor {
            option: root.option
            optionsModel: root.optionsModel
            session: root.session
            navigationPanel: root.navigationPanel
            navigationRow: root.navigationRow
            navigationColumn: root.navigationColumn
            compact: root.compact
        }
    }
    Component {
        id: listComp
        EoListEditor {
            option: root.option
            optionsModel: root.optionsModel
            session: root.session
            navigationPanel: root.navigationPanel
            navigationRow: root.navigationRow
            navigationColumn: root.navigationColumn
            compact: root.compact
        }
    }
    Component {
        id: tilesComp
        EoTilesEditor {
            option: root.option
            optionsModel: root.optionsModel
            session: root.session
            navigationPanel: root.navigationPanel
            navigationRow: root.navigationRow
            navigationColumn: root.navigationColumn
            compact: root.compact
            musicInfo: root.musicInfo
            musicStyle: root.musicStyle
        }
    }
}
