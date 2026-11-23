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

import "EoMusic.js" as EoMusic
import "EoTitles.js" as EoTitles

//! NOTE: an option of a page as the dialog shows it: the title on the left, the editor which the kind
//! of the option asks for on the right (or under the title, if it needs the width), and a button
//! which sets the option back to its default while it differs from it.
//! The editors themselves are in editors/Eo*Editor.qml, chosen by EoEditor
FocusScope {
    id: root

    //! NOTE: an EoOption, the EoSection and the EoPage it is in
    required property var option
    property var section: null
    property var page: null

    //! NOTE: the EoSession: the option under the pointer or with the focus is the one which the preview pane shows
    property var session: null

    readonly property bool isActive: Boolean(session) && session.activeOption === option

    readonly property var musicInfo: EoMusic.musicInfoOf(option, section, page)
    readonly property var musicStyle: EoMusic.musicStyleOf(option, section, page)

    //! NOTE: the EngravingOptionsModel
    required property var optionsModel

    property NavigationPanel navigationPanel: null
    property int navigationRow: 0

    readonly property var sids: {
        var result = Boolean(option.sid) ? [option.sid] : []
        return result.concat(option.extraSids)
    }

    readonly property bool isModified: {
        //! NOTE: the count is asked again when any value changes
        optionsModel.revision
        return sids.length > 0 && optionsModel.modifiedCount(sids) > 0
    }

    //! NOTE: the kinds of which the editor takes the width of the page, under the title:
    //! the ones the editor itself calls wide, and the ones it does not edit (a note, a full-width
    //! custom view)
    readonly property bool isWide: option.kind === "note"
                                   || (option.kind === "custom" && Boolean(option.fullWidth))
                                   || narrowEditor.isWide

    readonly property bool hasTitleRow: option.kind !== "note"
                                        && (Boolean(option.fullTitle) || Boolean(option.hint) || !isWide)

    //! NOTE: a title which is taken from a form ends with a colon, which a line of this list does not need
    readonly property string plainTitle: EoTitles.plain(option.fullTitle)

    implicitHeight: content.implicitHeight
    height: implicitHeight

    enabled: option.enabled
    opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled

    function resetToDefault() {
        optionsModel.resetValues(sids)
    }

    function activate() {
        if (Boolean(session)) {
            session.activate(option, section, page)
        }
    }

    onActiveFocusChanged: {
        if (activeFocus) {
            activate()
        }
    }

    HoverHandler {
        onHoveredChanged: {
            if (hovered) {
                root.activate()
            }
        }
    }

    //! NOTE: the option which the preview pane shows is marked quietly
    Rectangle {
        anchors.fill: parent
        anchors.leftMargin: -8
        anchors.rightMargin: -8
        anchors.topMargin: -2
        anchors.bottomMargin: -2

        radius: 7
        color: AtStyle.cell
        visible: root.isActive
    }

    Column {
        id: content

        width: parent.width
        spacing: 8

        // ----- the title, and the editor if it has room beside it -----

        Item {
            id: titleRow

            width: parent.width
            height: Math.max(30, titleColumn.implicitHeight + 8, trailing.height)

            visible: root.hasTitleRow

            Column {
                id: titleColumn

                anchors.left: parent.left
                anchors.right: trailing.left
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter

                spacing: 2

                StyledTextLabel {
                    width: parent.width
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.WordWrap

                    text: root.plainTitle
                    font: AtStyle.font(13)
                }

                StyledTextLabel {
                    width: parent.width
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.WordWrap

                    visible: Boolean(root.option.hint)
                    text: root.option.hint
                    font: AtStyle.font(11.5)
                    color: AtStyle.text3
                }
            }

            Row {
                id: trailing

                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter

                height: Math.max(narrowEditor.height, resetButton.height)
                spacing: 6

                EoEditor {
                    id: narrowEditor

                    anchors.verticalCenter: parent.verticalCenter

                    active: !root.isWide && root.option.kind !== "custom"

                    option: root.option
                    optionsModel: root.optionsModel
                    session: root.session
                    navigationPanel: root.navigationPanel
                    navigationRow: root.navigationRow
                    navigationColumn: 1

                    musicInfo: root.musicInfo
                    musicStyle: root.musicStyle
                }

                Loader {
                    anchors.verticalCenter: parent.verticalCenter

                    active: !root.isWide && root.option.kind === "custom"
                    sourceComponent: customComp
                }

                //! NOTE: the room of the button is kept, so that the controls of the options are in a line
                Item {
                    anchors.verticalCenter: parent.verticalCenter

                    width: resetButton.width
                    height: resetButton.height

                    AtToolButton {
                        id: resetButton

                        width: 26
                        height: 26

                        shape: "reset"
                        visible: root.isModified

                        toolTipTitle: qsTrc("notation/engravingoptions", "Reset to default")

                        navigation.panel: root.navigationPanel
                        navigation.row: root.navigationRow
                        navigation.column: 100
                        navigation.name: "Reset_" + root.option.sid
                        navigation.accessible.name: toolTipTitle + ": " + root.plainTitle

                        onClicked: {
                            root.resetToDefault()
                        }
                    }
                }
            }
        }

        // ----- the editor which needs the width -----

        Loader {
            id: wideLoader

            width: parent.width

            active: root.isWide
            visible: active

            sourceComponent: {
                switch (root.option.kind) {
                case "note": return noteComp
                case "custom": return customComp
                }
                return wideEditorComp
            }
        }
    }

    Component {
        id: wideEditorComp

        EoEditor {
            option: root.option
            optionsModel: root.optionsModel
            session: root.session
            navigationPanel: root.navigationPanel
            navigationRow: root.navigationRow
            navigationColumn: 1

            musicInfo: root.musicInfo
            musicStyle: root.musicStyle
        }
    }

    Component {
        id: noteComp

        StyledTextLabel {
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap

            text: root.option.title
            font: AtStyle.font(12)
            color: AtStyle.text2
        }
    }

    Component {
        id: customComp

        Loader {
            sourceComponent: root.option.component

            onLoaded: {
                if (item.hasOwnProperty("option")) {
                    item.option = root.option
                }

                if (item.hasOwnProperty("optionsModel")) {
                    item.optionsModel = root.optionsModel
                }

                if (item.hasOwnProperty("navigationPanel")) {
                    item.navigationPanel = Qt.binding(function() { return root.navigationPanel })
                }
            }
        }
    }
}
