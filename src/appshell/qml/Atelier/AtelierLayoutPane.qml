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
import MuseScore.NotationScene 1.0
import MuseScore.Atelier 1.0

//! NOTE: the left pane of the Engrave mode: what shapes the systems and pages
//! (fitting the music, breaks, spacers and frames, resets), and at the bottom the style and the page settings
Item {
    id: root

    property NavigationSection navigationSection: null
    property int navigationOrderStart: 1

    EngraveModeModel {
        id: engraveModel

        Component.onCompleted: {
            load()
        }
    }

    AtelierActionsModel {
        id: actions

        actionCodes: [
            "fit-music-reflow", "measures-per-system",
            "system-break", "page-break", "section-break", "toggle-system-lock",
            "insert-vbox", "insert-hbox", "insert-textframe",
            "reset", "reset-beammode", "reset-text-style-overrides", "reset-stretch",
            "edit-style", "page-settings"
        ]

        Component.onCompleted: {
            load()
        }
    }

    //! NOTE: the actions by their codes
    readonly property var items: {
        var result = {}
        var list = actions.items

        for (var i = 0; i < list.length; ++i) {
            result[list[i].code] = list[i]
        }

        return result
    }

    //! NOTE: a line is a title of a group (section), an action (code, with its own title if `title` is not given),
    //! or a staff spacer (spacer)
    readonly property var lines: [
        { code: "fit-music-reflow" },
        { code: "measures-per-system" },

        { section: qsTrc("appshell", "Breaks") },
        { code: "system-break", title: qsTrc("engraving/layoutbreaktype", "System break") },
        { code: "page-break", title: qsTrc("engraving/layoutbreaktype", "Page break") },
        { code: "section-break", title: qsTrc("engraving/layoutbreaktype", "Section break") },
        { code: "toggle-system-lock" },

        { section: qsTrc("appshell", "Spacers and frames") },
        { spacer: EngraveModeModel.SpacerDown, title: qsTrc("engraving/spacertype", "Staff spacer down") },
        { spacer: EngraveModeModel.SpacerUp, title: qsTrc("engraving/spacertype", "Staff spacer up") },
        { spacer: EngraveModeModel.SpacerFixedDown, title: qsTrc("engraving/spacertype", "Staff spacer fixed down") },
        { code: "insert-vbox" },
        { code: "insert-hbox" },
        { code: "insert-textframe" },

        { section: qsTrc("global", "Reset") },
        { code: "reset" },
        { code: "reset-beammode" },
        { code: "reset-text-style-overrides" },
        { code: "reset-stretch" }
    ]

    NavigationPanel {
        id: navPanel
        name: "LayoutPane"
        section: root.navigationSection
        order: root.navigationOrderStart
        direction: NavigationPanel.Vertical
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("appshell", "Layout")
    }

    NavigationPanel {
        id: footerNavPanel
        name: "LayoutPaneFooter"
        section: root.navigationSection
        order: root.navigationOrderStart + 1
        direction: NavigationPanel.Horizontal
        enabled: root.enabled && root.visible
    }

    AtPaneHeader {
        id: header

        width: parent.width

        title: qsTrc("appshell", "Layout")
        leftPadding: 14
    }

    StyledFlickable {
        id: flickable

        anchors.top: header.bottom
        anchors.bottom: footer.top
        width: parent.width

        clip: true

        contentHeight: content.height + 12

        Column {
            id: content

            x: 10
            width: parent.width - 20

            // ----- the commands -----

            Repeater {
                model: root.lines

                delegate: Loader {
                    id: lineLoader

                    readonly property var line: modelData
                    readonly property int lineIndex: index

                    width: content.width

                    sourceComponent: Boolean(modelData.section) ? sectionComp : commandComp
                }
            }
        }
    }

    Component {
        id: sectionComp

        Item {
            height: 12 + title.implicitHeight + 4

            AtSectionTitle {
                id: title

                x: 8
                y: 12
                width: parent.width - 16

                text: line.section
            }
        }
    }

    Component {
        id: commandComp

        AtListRow {
            readonly property bool isSpacer: line.spacer !== undefined
            readonly property var item: isSpacer ? null : root.items[line.code]

            text: Boolean(line.title) ? line.title : (Boolean(item) ? AtStyle.plainTitle(item.title) : "")
            keyHint: Boolean(item) ? AtStyle.keyHint(item.shortcuts) : ""

            enabled: isSpacer ? engraveModel.hasSelection : (Boolean(item) && item.enabled)

            toolTipTitle: Boolean(item) && item.description !== text ? item.description : ""

            navigation.panel: navPanel
            navigation.name: isSpacer ? "Spacer" + line.spacer : line.code
            navigation.row: 10 + lineIndex

            onClicked: {
                if (isSpacer) {
                    engraveModel.addSpacer(line.spacer)
                } else {
                    actions.trigger(line.code)
                }
            }
        }
    }

    // ----- the style and the page -----

    Item {
        id: footer

        anchors.bottom: parent.bottom
        width: parent.width
        height: 51

        Rectangle {
            width: parent.width
            height: 1
            color: AtStyle.stroke
        }

        Row {
            x: 12
            y: 11
            width: parent.width - 24

            spacing: 8

            AtOutlineButton {
                width: (parent.width - parent.spacing) / 2

                readonly property var item: root.items["edit-style"]

                text: Boolean(item) ? AtStyle.plainTitle(item.title) : ""
                enabled: Boolean(item) && item.enabled

                navigation.panel: footerNavPanel
                navigation.name: "EditStyle"
                navigation.order: 1

                onClicked: {
                    actions.trigger("edit-style")
                }
            }

            AtOutlineButton {
                width: (parent.width - parent.spacing) / 2

                readonly property var item: root.items["page-settings"]

                text: Boolean(item) ? AtStyle.plainTitle(item.title) : ""
                enabled: Boolean(item) && item.enabled

                navigation.panel: footerNavPanel
                navigation.name: "PageSettings"
                navigation.order: 2

                onClicked: {
                    actions.trigger("page-settings")
                }
            }
        }
    }
}
