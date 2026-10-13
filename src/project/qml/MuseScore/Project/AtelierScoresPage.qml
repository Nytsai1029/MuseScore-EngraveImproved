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
import Muse.GraphicalEffects 1.0
import Muse.Cloud 1.0
import MuseScore.Project 1.0
import MuseScore.Atelier 1.0

import "internal/ScoresPage"

//! NOTE: the Scores section of the home page in the Atelier skin: the score which was opened last
//! (it can be opened straight in a mode), and the recent scores below it as a list or a grid
FocusScope {
    id: root

    readonly property string inputUri: "musescore://notation"
    readonly property string engraveUri: "musescore://engrave"
    readonly property string previewUri: "musescore://publish"

    //! NOTE: the content has a largest width, and is in the middle of the page
    readonly property real contentWidth: Math.min(1040, root.width) - 72
    readonly property real contentX: (root.width - contentWidth) / 2

    readonly property string searchText: searchField.searchText.trim().toLowerCase()
    readonly property bool isSearching: Boolean(root.searchText)

    readonly property bool isRecentTab: scoresPageModel.tabIndex === 0
    readonly property bool isListView: scoresPageModel.viewType === ScoresPageModel.List

    //! NOTE: the recent scores, without the items which the other views of the model need (new score, no results)
    readonly property var scores: {
        var result = []

        for (var i = 0; i < scoreItems.count; ++i) {
            var item = scoreItems.objectAt(i)
            if (Boolean(item) && Boolean(item.score) && Boolean(item.score.path)) {
                result.push(item.score)
            }
        }

        return result
    }

    readonly property var lastScore: root.scores.length > 0 ? root.scores[0] : null

    readonly property var shownScores: {
        if (!root.isSearching) {
            return root.scores
        }

        return root.scores.filter(function(score) {
            return String(score.name).toLowerCase().indexOf(root.searchText) !== -1
        })
    }

    function openScore(score) {
        Qt.callLater(scoresPageModel.openScore, score.path, score.name)
    }

    //! NOTE: the owner of the page opens the score in the mode (this page is gone before the score is open)
    signal openScoreInPageRequested(string scorePath, string displayName, string pageUri)

    function openScoreInPage(score, pageUri) {
        root.openScoreInPageRequested(score.path, score.name, pageUri)
    }

    NavigationSection {
        id: navSec
        name: "RecentScores"
        enabled: root.enabled && root.visible
        order: 3

        onActiveChanged: {
            if (navSec.active) {
                root.forceActiveFocus()
            }
        }
    }

    ScoresPageModel {
        id: scoresPageModel
    }

    RecentScoresModel {
        id: recentScoresModel

        Component.onCompleted: {
            load()
        }
    }

    Instantiator {
        id: scoreItems

        model: recentScoresModel

        delegate: QtObject {
            readonly property var score: model.score
        }
    }

    Rectangle {
        anchors.fill: parent
        color: AtStyle.island
    }

    // ---------- the title, the search and the view ----------

    NavigationPanel {
        id: headerNavPanel
        name: "HomeScoresSearch"
        section: navSec
        order: 1
        direction: NavigationPanel.Horizontal
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("project", "Scores")
    }

    Item {
        id: titleRow

        x: root.contentX
        y: 26
        width: root.contentWidth
        height: 32

        StyledTextLabel {
            anchors.left: parent.left
            anchors.right: searchField.left
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter

            horizontalAlignment: Text.AlignLeft

            text: qsTrc("project", "Scores")
            font: AtStyle.font(26, Font.Bold)
        }

        SearchField {
            id: searchField

            anchors.right: viewToggle.left
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter

            width: Math.min(240, root.contentWidth - 200)
            height: 28

            hint: qsTrc("project", "Search recent scores")

            background.color: AtStyle.button
            background.border.color: "transparent"

            navigation.name: "Scores Search"
            navigation.panel: headerNavPanel
            navigation.order: 1
            accessible.name: qsTrc("project", "Search recent scores")
        }

        AtSegmented {
            id: viewToggle

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            segmentWidth: 30
            segmentHeight: 24
            segmentRadius: 6

            navigationPanel: headerNavPanel
            navigationOrderStart: 2

            currentValue: scoresPageModel.viewType

            model: [
                { title: qsTrc("project", "Grid view"), value: ScoresPageModel.Grid, icon: IconCode.GRID },
                { title: qsTrc("project", "List view"), value: ScoresPageModel.List, icon: IconCode.LIST }
            ]

            onActivated: function(value) {
                scoresPageModel.viewType = value
            }
        }
    }

    // ---------- the tabs ----------

    NavigationPanel {
        id: tabsNavPanel
        name: "HomeScoresTabs"
        section: navSec
        order: 2
        direction: NavigationPanel.Horizontal
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("project", "Scores tab bar")
    }

    Item {
        id: tabsRow

        x: root.contentX
        anchors.top: titleRow.bottom
        anchors.topMargin: 12
        width: root.contentWidth
        height: 36

        Row {
            height: parent.height
            spacing: 22

            Repeater {
                model: [
                    { title: qsTrc("project", "New & recent"), name: "NewAndRecent" },
                    { title: qsTrc("project", "My online scores"), name: "MyOnlineScores" }
                ]

                delegate: Item {
                    id: tab

                    readonly property bool isCurrent: index === scoresPageModel.tabIndex

                    width: tabLabel.implicitWidth
                    height: parent.height

                    NavigationControl {
                        id: tabNav

                        name: modelData.name
                        panel: tabsNavPanel
                        column: index + 1

                        accessible.role: MUAccessible.RadioButton
                        accessible.name: modelData.title
                        accessible.checked: tab.isCurrent

                        onTriggered: {
                            scoresPageModel.tabIndex = index
                        }
                    }

                    StyledTextLabel {
                        id: tabLabel

                        anchors.verticalCenter: parent.verticalCenter

                        text: modelData.title
                        font: AtStyle.font(13, tab.isCurrent ? Font.DemiBold : Font.Normal)
                        color: tab.isCurrent ? AtStyle.text : AtStyle.text2
                    }

                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 2

                        visible: tab.isCurrent
                        color: AtStyle.ink
                    }

                    NavigationFocusBorder {
                        navigationCtrl: tabNav
                    }

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            tabNav.requestActiveByInteraction()
                            scoresPageModel.tabIndex = index
                        }
                    }
                }
            }
        }

        AtToolButton {
            anchors.right: parent.right
            anchors.rightMargin: -8
            anchors.verticalCenter: parent.verticalCenter

            height: 24
            radius: 6

            text: qsTrc("project", "Score manager (online)")
            textFont: AtStyle.font(12)

            navigation.name: "ScoreManager"
            navigation.panel: tabsNavPanel
            navigation.column: 10

            onClicked: {
                scoresPageModel.openScoreManager()
            }
        }

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: AtStyle.stroke
        }
    }

    // ---------- the recent scores ----------

    NavigationPanel {
        id: heroNavPanel
        name: "LastOpenedScore"
        section: navSec
        order: 3
        direction: NavigationPanel.Horizontal
        enabled: root.enabled && root.visible && hero.visible
        accessible.name: qsTrc("project", "Last opened")
    }

    NavigationPanel {
        id: scoresNavPanel
        name: "RecentScoresList"
        section: navSec
        order: 4
        direction: NavigationPanel.Vertical
        enabled: root.enabled && root.visible && root.isRecentTab
        accessible.name: qsTrc("project", "Recent scores list")
    }

    StyledFlickable {
        id: recentFlickable

        anchors.top: tabsRow.bottom
        anchors.bottom: parent.bottom
        width: parent.width

        visible: root.isRecentTab
        clip: true

        contentHeight: recentColumn.y + recentColumn.height + 40

        Column {
            id: recentColumn

            x: root.contentX
            y: 20
            width: root.contentWidth

            // ----- the score which was opened last -----

            Item {
                id: hero

                readonly property var score: root.lastScore
                readonly property var info: Boolean(score) ? scoresPageModel.scoreInfo(score.path) : ({})

                width: parent.width
                height: visible ? 240 + 28 : 0

                visible: Boolean(root.lastScore) && !root.isSearching

                Rectangle {
                    id: card

                    width: parent.width
                    height: 240

                    radius: 12
                    color: AtStyle.raised
                    border.width: 1
                    border.color: AtStyle.stroke

                    layer.enabled: ui.isEffectsAllowed
                    layer.effect: RoundedCornersEffect {
                        radius: card.radius
                    }

                    Rectangle {
                        id: thumbnailArea

                        width: Math.min(380, Math.round(card.width * 0.4))
                        height: parent.height

                        color: AtStyle.canvas
                        clip: true

                        //! NOTE: the top of the first page
                        Loader {
                            x: Math.round((parent.width - width) / 2)
                            y: 24
                            width: Math.min(260, parent.width - 48)
                            height: Math.round(width * 297 / 210)

                            active: hero.visible

                            sourceComponent: ScoreThumbnail {
                                path: hero.score.path ?? ""
                                suffix: hero.score.suffix ?? ""
                                thumbnailUrl: hero.score.thumbnailUrl ?? ""
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor

                            onClicked: {
                                root.openScore(hero.score)
                            }
                        }
                    }

                    Column {
                        anchors.left: thumbnailArea.right
                        anchors.leftMargin: 28
                        anchors.right: parent.right
                        anchors.rightMargin: 28
                        anchors.verticalCenter: parent.verticalCenter

                        StyledTextLabel {
                            width: parent.width
                            horizontalAlignment: Text.AlignLeft

                            text: qsTrc("project", "Last opened")
                            font: AtStyle.font(11.5, Font.DemiBold)
                            color: AtStyle.text3
                        }

                        Item {
                            width: 1
                            height: 4
                        }

                        StyledTextLabel {
                            width: parent.width
                            horizontalAlignment: Text.AlignLeft

                            text: Boolean(hero.score) ? hero.score.name : ""
                            font: AtStyle.font(24, Font.Bold)
                        }

                        Item {
                            width: 1
                            height: 6
                        }

                        StyledTextLabel {
                            width: parent.width
                            horizontalAlignment: Text.AlignLeft

                            text: {
                                if (!Boolean(hero.score)) {
                                    return ""
                                }

                                var parts = []

                                if (Boolean(hero.info.composer)) {
                                    parts.push(hero.info.composer)
                                }

                                if (hero.info.partsCount > 0) {
                                    //: The number of instruments of a score
                                    parts.push(qsTrc("project", "%n instrument(s)", "", hero.info.partsCount))
                                }

                                if (Boolean(hero.score.timeSinceModified)) {
                                    //: %1 is when a score was modified, e.g. "yesterday" or a date
                                    parts.push(qsTrc("project", "Modified %1").arg(hero.score.timeSinceModified))
                                }

                                if (Boolean(hero.score.fileSize)) {
                                    parts.push(hero.score.fileSize)
                                }

                                return parts.join(" · ")
                            }

                            color: AtStyle.text2
                        }

                        Item {
                            width: 1
                            height: 18
                        }

                        Row {
                            spacing: 8

                            FlatButton {
                                height: 30
                                minWidth: 0
                                margins: 18

                                text: qsTrc("appshell", "Input")
                                accentButton: true

                                navigation.name: "OpenInInput"
                                navigation.panel: heroNavPanel
                                navigation.column: 1

                                onClicked: {
                                    root.openScoreInPage(hero.score, root.inputUri)
                                }
                            }

                            AtOutlineButton {
                                height: 30

                                text: qsTrc("appshell", "Engrave")

                                navigation.name: "OpenInEngrave"
                                navigation.panel: heroNavPanel
                                navigation.column: 2

                                onClicked: {
                                    root.openScoreInPage(hero.score, root.engraveUri)
                                }
                            }

                            AtOutlineButton {
                                height: 30

                                text: qsTrc("appshell", "Preview")

                                navigation.name: "OpenInPreview"
                                navigation.panel: heroNavPanel
                                navigation.column: 3

                                onClicked: {
                                    root.openScoreInPage(hero.score, root.previewUri)
                                }
                            }
                        }

                        Item {
                            width: 1
                            height: 10
                        }

                        StyledTextLabel {
                            width: parent.width
                            horizontalAlignment: Text.AlignLeft

                            text: qsTrc("project", "Opens the score straight in that mode.")
                            font: AtStyle.font(11.5)
                            color: AtStyle.text3
                        }
                    }
                }
            }

            // ----- the recent scores -----

            StyledTextLabel {
                width: parent.width
                height: implicitHeight + 10

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignTop

                visible: root.shownScores.length > 0

                text: root.isSearching ? qsTrc("project", "Search results") : qsTrc("project", "Recent")
                font: AtStyle.font(13, Font.DemiBold)
            }

            StyledTextLabel {
                width: parent.width
                height: 60

                visible: root.shownScores.length === 0

                text: root.isSearching ? qsTrc("global", "No results found")
                                       : qsTrc("project", "Scores which you open are listed here.")
                color: AtStyle.text2
            }

            // list

            Item {
                width: parent.width
                height: 25

                visible: root.isListView && root.shownScores.length > 0

                StyledTextLabel {
                    x: 10
                    width: parent.width - 20 - 150 - 80 - 24
                    horizontalAlignment: Text.AlignLeft

                    text: qsTrc("project", "Name")
                    font: AtStyle.font(11.5)
                    color: AtStyle.text3
                }

                StyledTextLabel {
                    x: parent.width - 10 - 80 - 12 - 150
                    width: 150
                    horizontalAlignment: Text.AlignLeft

                    //: Stands for "Last time that this score was modified".
                    //: Used as the header of this column in the scores list.
                    text: qsTrc("project", "Modified")
                    font: AtStyle.font(11.5)
                    color: AtStyle.text3
                }

                StyledTextLabel {
                    x: parent.width - 10 - 80
                    width: 80
                    horizontalAlignment: Text.AlignRight

                    text: qsTrc("global", "Size", "file size")
                    font: AtStyle.font(11.5)
                    color: AtStyle.text3
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: AtStyle.stroke
                }
            }

            Repeater {
                model: root.isListView ? root.shownScores : []

                delegate: Item {
                    id: row

                    readonly property var score: modelData

                    width: recentColumn.width
                    height: 40

                    Rectangle {
                        anchors.fill: parent

                        radius: 8
                        color: rowMouseArea.pressed ? AtStyle.pressed
                                                    : (rowMouseArea.containsMouse ? AtStyle.hover : "transparent")

                        NavigationFocusBorder {
                            navigationCtrl: rowNav
                        }
                    }

                    NavigationControl {
                        id: rowNav

                        name: "Score_" + index
                        panel: scoresNavPanel
                        row: index

                        accessible.role: MUAccessible.ListItem
                        accessible.name: row.score.name

                        onTriggered: {
                            root.openScore(row.score)
                        }

                        onActiveChanged: function(active) {
                            if (active) {
                                root.forceActiveFocus()
                            }
                        }
                    }

                    Rectangle {
                        id: rowThumbnail

                        x: 10
                        anchors.verticalCenter: parent.verticalCenter
                        width: 20
                        height: 26

                        color: "#FFFFFF"
                        border.width: 1
                        border.color: AtStyle.stroke2

                        ScoreThumbnail {
                            anchors.fill: parent
                            anchors.margins: 1

                            path: row.score.path ?? ""
                            suffix: row.score.suffix ?? ""
                            thumbnailUrl: row.score.thumbnailUrl ?? ""
                        }
                    }

                    StyledTextLabel {
                        anchors.left: rowThumbnail.right
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 40 - 10 - 150 - 80 - 24 - 12

                        horizontalAlignment: Text.AlignLeft

                        text: row.score.name
                        font: AtStyle.font(13, Font.Medium)
                    }

                    StyledTextLabel {
                        x: parent.width - 10 - 80 - 12 - 150
                        anchors.verticalCenter: parent.verticalCenter
                        width: 150

                        horizontalAlignment: Text.AlignLeft

                        text: row.score.timeSinceModified ?? ""
                        color: AtStyle.text2
                    }

                    StyledTextLabel {
                        x: parent.width - 10 - 80
                        anchors.verticalCenter: parent.verticalCenter
                        width: 80

                        horizontalAlignment: Text.AlignRight

                        text: Boolean(row.score.fileSize) ? row.score.fileSize : "–"
                        color: AtStyle.text2
                    }

                    MouseArea {
                        id: rowMouseArea

                        anchors.fill: parent
                        hoverEnabled: true

                        onClicked: {
                            rowNav.requestActiveByInteraction()
                            root.openScore(row.score)
                        }
                    }
                }
            }

            // grid

            Flow {
                width: parent.width
                spacing: 24

                visible: !root.isListView

                Repeater {
                    model: root.isListView ? [] : root.shownScores

                    delegate: ScoreGridItem {
                        readonly property var score: modelData

                        width: 172
                        height: 294

                        name: score.name
                        path: score.path ?? ""
                        suffix: score.suffix ?? ""
                        thumbnailUrl: score.thumbnailUrl ?? ""
                        timeSinceModified: score.timeSinceModified ?? ""
                        isCloud: score.isCloud ?? false
                        cloudScoreId: score.scoreId ?? 0

                        navigation.panel: scoresNavPanel
                        navigation.row: index

                        onClicked: {
                            root.openScore(score)
                        }
                    }
                }
            }
        }
    }

    // ---------- the scores of the account ----------

    Loader {
        anchors.top: tabsRow.bottom
        anchors.topMargin: 20
        anchors.bottom: parent.bottom
        width: parent.width

        active: !root.isRecentTab

        sourceComponent: CloudScoresView {
            viewType: scoresPageModel.viewType
            searchText: searchField.searchText

            backgroundColor: AtStyle.island
            sideMargin: root.contentX

            navigationSection: navSec
            navigationOrder: 4

            onCreateNewScoreRequested: {
                scoresPageModel.createNewScore()
            }

            onOpenScoreRequested: function(scorePath, displayName) {
                Qt.callLater(scoresPageModel.openScore, scorePath, displayName)
            }
        }
    }
}
