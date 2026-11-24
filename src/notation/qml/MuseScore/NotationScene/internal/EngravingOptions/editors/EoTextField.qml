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

import Muse.UiComponents 1.0

//! NOTE: a field for a text value which, while it is not being edited, shows the beginning of a long text: the
//! beginning says what the text is (as <sym>ottavaAlta</sym>), its end often does not
TextInputField {
    id: root

    function showBeginning() {
        if (!inputField.activeFocus) {
            inputField.cursorPosition = 0
        }
    }

    onCurrentTextChanged: Qt.callLater(showBeginning)
    Component.onCompleted: Qt.callLater(showBeginning)

    Connections {
        target: root.inputField

        function onActiveFocusChanged() {
            root.showBeginning()
        }
    }
}
