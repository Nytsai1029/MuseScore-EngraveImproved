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
import QtQuick.Controls 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "."

Rectangle {
    id: root

    property alias model: sectionList.model
    property alias notationView: popupController.notationView

    property NavigationSection navigationSection: null
    property int navigationOrderStart: 1

    //! NOTE: when horizontal (the panel is docked above or below the score), the sections are placed side by side,
    //! and the content of a section continues in the next column when it is higher than the panel
    property int orientation: Qt.Vertical
    readonly property bool isHorizontal: orientation === Qt.Horizontal
    readonly property int sectionColumnWidth: 300

    //! NOTE: "input" or "engrave" (the modes of the Atelier skin), which show a part of the properties each;
    //! otherwise all properties are shown
    property string mode: ""

    //! NOTE: in the Atelier skin, the sections have small titles and thin lines between them, they do not collapse,
    //! and the section of what is selected comes before the General section
    readonly property bool atelierStyle: ui.theme.skin === "atelier"
    readonly property int atelierSidePadding: 16

    function isSectionShown(sectionType) {
        switch (root.mode) {
        case "input":
            return sectionType !== Inspector.SECTION_SCORE_APPEARANCE && sectionType !== Inspector.SECTION_EMPTY_STAVES
        case "engrave":
            return sectionType !== Inspector.SECTION_SCORE_DISPLAY
        }

        return true
    }

    //! NOTE: changes when the sections are added, removed or put in another order
    property int listRevision: 0

    Connections {
        target: inspectorListModel

        function onRowsInserted() { root.listRevision++ }
        function onRowsRemoved() { root.listRevision++ }
        function onDataChanged() { root.listRevision++ }
        function onModelReset() { root.listRevision++ }
    }

    //! NOTE: `revision` makes the bindings follow the changes of the list
    function hasShownSectionAfter(row, revision) {
        var count = inspectorListModel.rowCount()

        for (var i = row + 1; i < count; ++i) {
            if (root.isSectionShown(inspectorListModel.sectionTypeAt(i))) {
                return true
            }
        }

        return false
    }

    //! NOTE: in the Dorico skin, each section is a card with a title bar, on a darker background
    readonly property bool cardStyle: ui.theme.skin === "dorico"
    readonly property int cardSpacing: 3
    readonly property int cardTitlePadding: 6
    readonly property int cardTitleRowHeight: 20
    readonly property int cardBottomPadding: 10

    //! NOTE: the titles are centered in the title bars, which have the same height for all sections
    function cardTitleY(titleHeight) {
        return cardTitlePadding + Math.max((cardTitleRowHeight - titleHeight) / 2, 0)
    }

    function cardTitleBarHeight(titleHeight) {
        return 2 * cardTitlePadding + Math.max(cardTitleRowHeight, titleHeight)
    }

    color: root.cardStyle ? ui.theme.backgroundSecondaryColor : ui.theme.backgroundPrimaryColor

    onVisibleChanged: {
        inspectorListModel.setInspectorVisible(root.visible)
    }

    function focusFirstItem() {
        var item = sectionList.itemAtIndex(0)
        if (item) {
            item.navigation.requestActive()
        }
    }

    QtObject {
        id: prv

        function closePreviousOpenedPopup(newOpenedPopup, visualControl) {
            if (Boolean(popupController.popup) && popupController.popup !== newOpenedPopup) {
                popupController.popup.close()
            }

            popupController.visualControl = visualControl
            popupController.popup = newOpenedPopup

            popupController.popup.closed.connect(function() {
                if (sectionList.contentY + sectionList.height > sectionList.contentHeight) {
                    var invisibleContentHeight = sectionList.contentY + sectionList.height - sectionList.contentHeight
                    Qt.callLater(sectionList.ensureContentVisible, -invisibleContentHeight)
                }
            })
        }
    }

    InspectorPopupController {
        id: popupController
    }

    Rectangle {
        //! NOTE: a horizontal panel has a margin between its tabs and its content (see DockFrame.qml),
        //! which has the color of the panel itself
        readonly property int panelContentTopMargin: 12

        anchors.bottom: parent.top
        width: parent.width
        height: panelContentTopMargin

        color: root.color
        visible: root.cardStyle && root.isHorizontal
    }

    Component.onCompleted: {
        popupController.load()
    }

    StyledListView {
        id: sectionList
        anchors.fill: parent

        orientation: root.isHorizontal ? ListView.Horizontal : ListView.Vertical

        topMargin: root.isHorizontal || root.cardStyle || root.atelierStyle ? 0 : 12
        bottomMargin: root.isHorizontal || root.cardStyle ? 0 : 12

        spacing: root.cardStyle ? root.cardSpacing : (root.isHorizontal || root.atelierStyle ? 0 : 12)

        function ensureContentVisible(invisibleContentHeight) {
            if (root.isHorizontal) {
                return
            }

            if (sectionList.contentY + invisibleContentHeight > 0) {
                sectionList.contentY += invisibleContentHeight
            } else {
                sectionList.contentY = 0
            }
        }

        Behavior on contentY {
            NumberAnimation { duration: 250 }
        }

        model: InspectorListModel {
            id: inspectorListModel

            generalSectionLast: root.atelierStyle
        }

        onContentHeightChanged: {
            returnToBounds()

            if (contentHeight > cacheBuffer) {
                cacheBuffer = contentHeight
            }
        }

        onContentWidthChanged: {
            if (root.isHorizontal && contentWidth > cacheBuffer) {
                cacheBuffer = contentWidth
            }
        }

        delegate: root.isHorizontal ? sectionColumnComp
                                    : (root.cardStyle ? sectionCardRowComp
                                                      : (root.atelierStyle ? sectionAtelierRowComp : sectionRowComp))
    }

    Component {
        id: sectionAtelierRowComp

        Item {
            id: sectionRow

            readonly property var sectionModel: model.inspectorSectionModel
            readonly property bool isShown: Boolean(sectionModel) && root.isSectionShown(sectionModel.sectionType)

            //! NOTE: the General section has its own groups with their titles (see GeneralModeView)
            readonly property bool hasTitle: Boolean(sectionModel) && (sectionModel.sectionType !== Inspector.SECTION_GENERAL
                                                                       || root.mode === "")

            property var navigationPanel: _item.navigationPanel

            width: ListView.view.width
            height: isShown ? _item.y + _item.height + 10 : 0
            visible: isShown

            InspectorSectionDelegate {
                id: _item

                x: root.atelierSidePadding
                y: 12
                width: parent.width - 2 * x

                sectionModel: sectionRow.sectionModel
                mode: root.mode
                anchorItem: root

                flatHeader: true
                headerVisible: sectionRow.hasTitle
                titleFont.pixelSize: 11
                titleFont.weight: Font.DemiBold
                titleColor: ui.theme.skin === "atelier" && ui.theme.isDark ? "#BDBDC3" : "#55555A"
                contentSpacing: 8

                navigationPanel.section: root.navigationSection
                navigationPanel.order: root.navigationOrderStart + model.index

                onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                    sectionList.ensureContentVisible(invisibleContentHeight)
                }

                onPopupOpened: function(openedPopup, visualControl) {
                    prv.closePreviousOpenedPopup(openedPopup, visualControl)
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                x: root.atelierSidePadding
                width: parent.width - 2 * x
                height: 1

                color: ui.theme.strokeColor
                visible: root.hasShownSectionAfter(model.index, root.listRevision)
            }
        }
    }

    Component {
        id: sectionRowComp

        Column {
            width: ListView.view.width
            spacing: sectionList.spacing

            property var navigationPanel: _item.navigationPanel

            SeparatorLine {
                visible: model.index !== 0
            }

            InspectorSectionDelegate {
                id: _item

                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.right: parent.right
                anchors.rightMargin: 12

                sectionModel: model.inspectorSectionModel
                anchorItem: root
                navigationPanel.section: root.navigationSection
                navigationPanel.order: root.navigationOrderStart + model.index

                onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                    sectionList.ensureContentVisible(invisibleContentHeight)
                }

                onPopupOpened: function(openedPopup, visualControl) {
                    prv.closePreviousOpenedPopup(openedPopup, visualControl)
                }
            }
        }
    }

    Component {
        id: sectionCardRowComp

        Item {
            width: ListView.view.width
            height: _item.y + _item.height + root.cardBottomPadding

            property var navigationPanel: _item.navigationPanel

            Rectangle {
                anchors.fill: parent
                color: ui.theme.backgroundPrimaryColor
            }

            Rectangle {
                width: parent.width
                height: root.cardTitleBarHeight(_item.titleHeight)
                color: ui.theme.backgroundTertiaryColor
            }

            InspectorSectionDelegate {
                id: _item

                x: 12
                y: root.cardTitleY(titleHeight)
                width: parent.width - 2 * x

                sectionModel: model.inspectorSectionModel
                anchorItem: root
                navigationPanel.section: root.navigationSection
                navigationPanel.order: root.navigationOrderStart + model.index

                onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                    sectionList.ensureContentVisible(invisibleContentHeight)
                }

                onPopupOpened: function(openedPopup, visualControl) {
                    prv.closePreviousOpenedPopup(openedPopup, visualControl)
                }
            }
        }
    }

    Component {
        id: sectionColumnComp

        Item {
            id: sectionColumn

            width: 2 * _item.x + columnsFlow.contentWidth + (root.cardStyle ? 0 : separator.width)
            height: ListView.view.height

            property var navigationPanel: _item.navigationPanel

            readonly property real topPadding: root.cardStyle ? root.cardTitleY(_item.titleHeight) : 12
            readonly property real bottomPadding: root.cardStyle ? root.cardBottomPadding : 12

            //! NOTE: the height of the section without its content (the title)
            readonly property real headerHeight: _item.contentItem ? _item.height - _item.contentItem.height
                                                                   : _item.height

            //! NOTE: the content which is higher than the panel continues in the next columns
            ColumnsFlow {
                id: columnsFlow

                target: _item.contentItem

                columnWidth: _item.width
                columnHeight: sectionColumn.height - sectionColumn.headerHeight
                              - sectionColumn.topPadding - sectionColumn.bottomPadding
                columnSpacing: 2 * _item.x
            }

            Rectangle {
                anchors.fill: parent
                color: ui.theme.backgroundPrimaryColor
                visible: root.cardStyle
            }

            StyledFlickable {
                anchors.fill: parent
                anchors.rightMargin: root.cardStyle ? 0 : separator.width

                contentWidth: width
                contentHeight: sectionColumn.headerHeight + columnsFlow.contentHeight
                               + sectionColumn.topPadding + sectionColumn.bottomPadding

                Rectangle {
                    width: parent.width
                    height: root.cardTitleBarHeight(_item.titleHeight)
                    color: ui.theme.backgroundTertiaryColor
                    visible: root.cardStyle
                }

                InspectorSectionDelegate {
                    id: _item

                    x: 12
                    y: sectionColumn.topPadding
                    width: root.sectionColumnWidth - 2 * x

                    sectionModel: model.inspectorSectionModel
                    anchorItem: root
                    navigationPanel.section: root.navigationSection
                    navigationPanel.order: root.navigationOrderStart + model.index

                    onPopupOpened: function(openedPopup, visualControl) {
                        prv.closePreviousOpenedPopup(openedPopup, visualControl)
                    }
                }
            }

            SeparatorLine {
                id: separator

                anchors.right: parent.right
                orientation: Qt.Vertical

                visible: !root.cardStyle
            }
        }
    }
}
