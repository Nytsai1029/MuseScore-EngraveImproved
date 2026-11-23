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
import MuseScore.Atelier 1.0

//! NOTE: the footer of the Engraving Options dialog: reset all, apply to all parts, Cancel and OK
Rectangle {
    id: root

    required property var optionsModel
    required property int modifiedCount
    required property var navigationPanel

    signal acceptClicked()
    signal cancelClicked()

    height: 56
    color: AtStyle.island

        Rectangle {
            width: parent.width
            height: 1

            color: AtStyle.stroke
        }

        Row {
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter

            spacing: 8

            //! NOTE: all the options are reset with the second click, so that it is not done by mistake
            AtOutlineButton {
                id: resetAllButton

                property bool isArmed: false

                enabled: root.modifiedCount > 0 || isArmed

                text: isArmed ? qsTrc("notation/engravingoptions", "Click again to reset all options")
                              : qsTrc("notation/engravingoptions", "Reset all to defaults")
                toolTipTitle: qsTrc("notation/engravingoptions",
                                    "Sets all the options to their defaults, except the page size and margins and the staff size")

                navigation.panel: root.navigationPanel
                navigation.column: 1
                navigation.name: "ResetAll"

                onClicked: {
                    if (isArmed) {
                        isArmed = false
                        root.optionsModel.resetAllValues()
                        return
                    }

                    isArmed = true
                    disarmTimer.restart()
                }

                Timer {
                    id: disarmTimer

                    interval: 4000

                    onTriggered: {
                        resetAllButton.isArmed = false
                    }
                }
            }

            AtOutlineButton {
                visible: root.optionsModel.canApplyToAllParts

                text: qsTrc("notation/editstyle", "Apply to all parts")

                navigation.panel: root.navigationPanel
                navigation.column: 2
                navigation.name: "ApplyToAllParts"

                onClicked: {
                    root.optionsModel.applyToAllParts()
                }
            }
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter

            spacing: 8

            AtOutlineButton {
                width: Math.max(96, implicitWidth)

                text: qsTrc("global", "Cancel")

                navigation.panel: root.navigationPanel
                navigation.column: 3
                navigation.name: "Cancel"

                onClicked: {
                    root.cancelClicked()
                }
            }

            FlatButton {
                height: 28
                minWidth: 96
                margins: 16

                text: qsTrc("global", "OK")
                accentButton: true
                backgroundRadius: 7
                textFont: AtStyle.font(13, Font.Medium)

                navigation.panel: root.navigationPanel
                navigation.column: 4
                navigation.name: "OK"

                onClicked: {
                    root.acceptClicked()
                }
            }
        }
}
