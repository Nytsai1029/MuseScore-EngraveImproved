/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

ExportSettingsPage {
    id: root

    StyledTextLabel {
        width: parent.width

        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap

        //: Explains the export of a score to a file for the official MuseScore Studio application
        text: qsTrc("project/export", "Saves the whole score, parts included, as a file for the official MuseScore Studio. The layout shown here is written into the file as manual adjustments, so that it looks the same there.")
    }

    StyledTextLabel {
        width: parent.width

        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap
        opacity: 0.7

        text: qsTrc("project/export", "Such a file no longer spaces itself when it is edited: keep working in the score you have open.")
    }
}
