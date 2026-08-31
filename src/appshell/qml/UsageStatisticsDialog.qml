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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.AppShell 1.0

StyledDialogView {
    id: root

    title: qsTrc("appshell/statistics", "Usage statistics")

    contentHeight: 288
    contentWidth: 456

    UsageStatisticsModel {
        id: statisticsModel

        Component.onCompleted: {
            statisticsModel.load()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 24

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 32
            Layout.leftMargin: 40
            Layout.rightMargin: 40

            spacing: 12

            StyledTextLabel {
                Layout.fillWidth: true

                text: qsTrc("appshell/statistics", "Total app usage")
                font: ui.theme.bodyBoldFont
            }

            StyledTextLabel {
                Layout.fillWidth: true

                text: statisticsModel.totalUsageTime
            }

            SeparatorLine {
                Layout.fillWidth: true
                Layout.topMargin: 8
                Layout.bottomMargin: 8
            }

            StyledTextLabel {
                Layout.fillWidth: true

                text: qsTrc("appshell/statistics", "Current score usage")
                font: ui.theme.bodyBoldFont
            }

            StyledTextLabel {
                Layout.fillWidth: true
                visible: statisticsModel.hasCurrentScore

                text: statisticsModel.currentScoreName
                elide: Text.ElideMiddle
            }

            StyledTextLabel {
                Layout.fillWidth: true

                text: statisticsModel.hasCurrentScore
                      ? statisticsModel.currentScoreUsageTime
                      : qsTrc("appshell/statistics", "No score open")
            }
        }

        ButtonBox {
            Layout.fillWidth: true
            Layout.rightMargin: 16
            Layout.bottomMargin: 16

            buttons: [ ButtonBoxModel.Ok ]

            navigationPanel.section: root.navigationSection

            onStandardButtonClicked: function(buttonId) {
                if (buttonId === ButtonBoxModel.Ok) {
                    root.hide()
                }
            }
        }
    }
}
