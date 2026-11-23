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
import MuseScore.Atelier 1.0

//! NOTE: the variables which a header or footer can use, as the table of the classic dialog:
//! the codes with what they mean, then the metadata tags of the score with their values.
//! Made by an EoCustom of the header-and-footer page
Column {
    id: root

    //! NOTE: the EngravingOptionsModel, for the tags of the score
    property var optionsModel: null

    //! NOTE: the variables of a header or footer: the code and what it means,
    //! the rows and the texts of the classic dialog (editstyle.cpp)
    readonly property var variables: [
        { code: "$p", meaning: qsTrc("notation/editstyle", "Page number, except on first page") },
        { code: "$N", meaning: qsTrc("notation/editstyle", "Page number, if there is more than one page") },
        { code: "$P", meaning: qsTrc("notation/editstyle", "Page number, on all pages") },
        { code: "$n", meaning: qsTrc("notation/editstyle", "Number of pages") },
        { code: "$f", meaning: qsTrc("notation/editstyle", "File name") },
        { code: "$F", meaning: qsTrc("notation/editstyle", "File path+name") },
        { code: "$i", meaning: qsTrc("notation/editstyle", "Part name, except on first page") },
        { code: "$I", meaning: qsTrc("notation/editstyle", "Part name, on all pages") },
        { code: "$d", meaning: qsTrc("notation/editstyle", "Current date") },
        { code: "$D", meaning: qsTrc("notation/editstyle", "Creation date") },
        { code: "$m", meaning: qsTrc("notation/editstyle", "Last modification time") },
        { code: "$M", meaning: qsTrc("notation/editstyle", "Last modification date") },
        { code: "$c", meaning: qsTrc("notation/editstyle", "Copyright, on first page only") },
        { code: "$C", meaning: qsTrc("notation/editstyle", "Copyright, on all pages") },
        { code: "$v", meaning: qsTrc("notation/editstyle",
                                     "MuseScore Studio version this score was last saved with") },
        { code: "$r", meaning: qsTrc("notation/editstyle",
                                     "MuseScore Studio revision this score was last saved with") },
        { code: "$$", meaning: qsTrc("notation/editstyle", "The $ sign itself") }
    ]

    readonly property var tags: Boolean(optionsModel) ? optionsModel.metaTags() : []

    //! NOTE: the rows are in two columns when there is the room for them
    readonly property bool twoColumns: width >= 520
    readonly property int rowWidth: twoColumns ? (width - 24) / 2 : width

    spacing: 14

    StyledTextLabel {
        width: parent.width
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap

        text: qsTrc("notation/editstyle",
                    "Type these special character combinations in the fields above to reference document data in the header/footer.")
        font: AtStyle.font(12)
        color: AtStyle.text2
    }

    Flow {
        width: parent.width
        spacing: 8

        Repeater {
            model: root.variables

            delegate: Row {
                width: root.rowWidth
                spacing: 10

                StyledTextLabel {
                    width: 22

                    text: modelData.code
                    font: AtStyle.font(12, Font.Medium)
                    color: AtStyle.text
                }

                StyledTextLabel {
                    width: parent.width - 32
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.WordWrap

                    text: modelData.meaning
                    font: AtStyle.font(12)
                    color: AtStyle.text3
                }
            }
        }
    }

    Column {
        width: parent.width
        spacing: 8

        StyledTextLabel {
            width: parent.width
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
            textFormat: Text.StyledText

            text: qsTrc("notation/editstyle",
                        "<b>Metadata tags and current values</b> (configured in <b>File > Project properties…</b>)")
            font: AtStyle.font(12)
            color: AtStyle.text
        }

        StyledTextLabel {
            width: parent.width
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap

            text: qsTrc("notation/editstyle",
                        "Type $:tag: into a field above, replacing the word ‘tag’ with one of the labels below. Its associated metadata will be shown on your score.")
            font: AtStyle.font(12)
            color: AtStyle.text2
        }
    }

    Flow {
        width: parent.width
        spacing: 8

        Repeater {
            model: root.tags

            delegate: Row {
                width: root.rowWidth
                spacing: 10

                StyledTextLabel {
                    id: codeLabel

                    text: "$:" + modelData.tag + ":"
                    font: AtStyle.font(12, Font.Medium)
                    color: AtStyle.text
                }

                StyledTextLabel {
                    width: parent.width - (codeLabel.width + 10)
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.WrapAnywhere

                    text: modelData.value
                    font: AtStyle.font(12)
                    color: AtStyle.text3
                }
            }
        }
    }
}
