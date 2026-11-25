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

//! NOTE: the column beside the options of a page: the picture of the music which the option being edited shows,
//! with what the option changes marked and a handle to drag its value, the situations which the picture can show,
//! and what the option is. The option is the one under the pointer or with the focus (see EoSession)
Item {
    id: root

    //! NOTE: the EoSession
    required property var session

    //! NOTE: the EngravingOptionsModel
    required property var optionsModel

    //! NOTE: the EngravingOptionsDialogState, which keeps whether the handles are shown
    property var dialogState: null

    property NavigationPanel navigationPanel: null

    readonly property var option: session.activeOption
    readonly property var section: session.activeSection
    readonly property var page: session.activePage

    readonly property var musicInfo: EoMusic.musicInfoOf(option, section, page)
    readonly property string music: musicInfo.music
    readonly property var musicStyle: EoMusic.musicStyleOf(option, section, page)
    readonly property var styleItem: Boolean(option) ? optionsModel.item(option.sid) : null
    readonly property string valueKind: Boolean(option) ? optionsModel.valueKind(option.sid) : ""

    //! NOTE: a switch, a distance, an offset or a number changes the picture in a way which can be seen
    readonly property bool isShownByEffect: ["spatium", "real", "int", "bool", "point"].indexOf(valueKind) >= 0

    //! NOTE: the zoom of the picture is the scale of the user times the fit of the picture to the paper
    //! (so that 100 % always fills the room inside it). Music is not made smaller than readable nor larger than
    //! sensible; of pages, one fits the width of the paper whatever its size (the others are scrolled to)
    property real userScale: 1.0
    readonly property real fit: {
        const innerWidth = card.width - 2 * card.padding
        const naturalWidth = musicInfo.showsPage ? card.naturalPageSize.width : card.naturalSize.width
        if (innerWidth <= 0.0 || naturalWidth <= 0.0) {
            return 1.0
        }
        if (musicInfo.showsPage) {
            return innerWidth / naturalWidth
        }
        return Math.max(0.8, Math.min(2.6, innerWidth / naturalWidth))
    }
    readonly property real zoom: userScale * fit

    readonly property int sideMargin: 20

    //! NOTE: the contexts of the section of the option, which are chosen here
    readonly property var contexts: {
        if (!Boolean(section)) {
            return []
        }

        return section.options.filter(function(candidate) {
            return candidate.kind === "context"
        })
    }

    //! NOTE: an option shown in other units than it is stored in (a percentage, quarter spaces)
    //! is dragged in the units of the style
    readonly property real unitsPerShownUnit: EoMusic.unitsPerShownUnit(option)

    onPageChanged: {
        userScale = 1.0
        compareButton.keyToggled = false
    }

    onOptionChanged: {
        compareButton.keyToggled = false
    }

    Rectangle {
        anchors.left: parent.left
        width: 1
        height: parent.height

        color: AtStyle.stroke
    }

    StyledFlickable {
        anchors.fill: parent
        anchors.leftMargin: 1

        contentWidth: width
        contentHeight: column.y + column.height + 24

        Column {
            id: column

            x: root.sideMargin
            y: 16
            width: parent.width - 2 * root.sideMargin

            spacing: 12

            // ----- the picture -----

            EoPreviewCard {
                id: card

                width: parent.width
                height: Math.max(190, implicitHeight)

                visible: Boolean(root.music)

                music: root.music
                systemWidth: root.musicInfo.systemWidth
                showsPage: root.musicInfo.showsPage
                overrides: root.musicStyle
                zoom: root.zoom

                activeSid: Boolean(root.option) ? root.option.sid : ""
                probeValue: EoMusic.probeValueOf(root.option, root.styleItem)
                //! NOTE: a point (an offset, of which an option may edit x and y or one axis) is washed, not dragged
                isDraggable: Boolean(root.option) && root.option.kind === "number" && !root.option.axis
                showsHandles: root.dialogState.showsHandles
                showingProbed: compareButton.held

                dragMinimum: Boolean(root.option) && root.option.min !== undefined
                             ? root.option.min * root.unitsPerShownUnit : -1e9
                dragMaximum: Boolean(root.option) && root.option.max !== undefined
                             ? root.option.max * root.unitsPerShownUnit : 1e9
                dragStep: Boolean(root.option) && root.option.step !== undefined
                          ? root.option.step * root.unitsPerShownUnit : 0.01

                onIsDraggingChanged: {
                    root.session.dragValue = isDragging ? dragValue : null
                    root.session.dragSid = isDragging ? root.option.sid : ""
                }

                onDragValueChanged: {
                    if (isDragging) {
                        root.session.dragValue = dragValue
                    }
                }

                onValueDropped: function(value) {
                    root.styleItem.value = value
                }
            }

            // ----- handles and zoom -----

            Item {
                width: parent.width
                height: 28

                visible: card.visible

                Row {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter

                    spacing: 8

                    AtSwitch {
                        anchors.verticalCenter: parent.verticalCenter

                        checked: root.dialogState.showsHandles

                        navigation.panel: root.navigationPanel
                        navigation.row: 900000
                        navigation.column: 1
                        navigation.name: "ShowHandles"
                        navigation.accessible.name: handlesLabel.text

                        onToggled: {
                            root.dialogState.showsHandles = !root.dialogState.showsHandles
                        }
                    }

                    StyledTextLabel {
                        id: handlesLabel

                        anchors.verticalCenter: parent.verticalCenter

                        text: qsTrc("notation/engravingoptions", "Show handles")
                        font: AtStyle.font(12)
                        color: AtStyle.text2
                    }
                }

                Row {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter

                    spacing: 2

                    AtToolButton {
                        width: 24
                        height: 24

                        shape: "minus"
                        enabled: root.userScale > 0.5

                        toolTipTitle: qsTrc("notation", "Zoom out")

                        navigation.panel: root.navigationPanel
                        navigation.row: 900000
                        navigation.column: 2
                        navigation.name: "PreviewZoomOut"

                        onClicked: {
                            root.userScale = Math.max(0.5, root.userScale - 0.25)
                        }
                    }

                    StyledTextLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 44

                        text: Math.round(root.userScale * 100) + " %"
                        font: AtStyle.font(11.5)
                        color: AtStyle.text2
                    }

                    AtToolButton {
                        width: 24
                        height: 24

                        shape: "plus"
                        enabled: root.userScale < 2.0

                        toolTipTitle: qsTrc("notation", "Zoom in")

                        navigation.panel: root.navigationPanel
                        navigation.row: 900000
                        navigation.column: 3
                        navigation.name: "PreviewZoomIn"

                        onClicked: {
                            root.userScale = Math.min(2.0, root.userScale + 0.25)
                        }
                    }
                }
            }

            // ----- before / after comparison -----

            AtOutlineButton {
                id: compareButton

                property bool keyToggled: false
                readonly property bool held: (mouseArea.pressed || keyToggled) && enabled

                height: 26
                visible: card.visible && card.hasEffect
                enabled: visible && !card.isDragging
                checked: held
                isClickOnKeyNavTriggered: false

                text: qsTrc("notation/engravingoptions", "Hold to preview change")
                textFont: AtStyle.font(12, Font.Medium)

                navigation.panel: root.navigationPanel
                navigation.row: 900050
                navigation.column: 1
                navigation.name: "PreviewCompare"

                navigation.onTriggered: {
                    keyToggled = !keyToggled
                }

                navigation.onActiveChanged: {
                    if (!navigation.active) {
                        keyToggled = false
                    }
                }

                onVisibleChanged: {
                    if (!visible) {
                        keyToggled = false
                    }
                }
            }

            // ----- the situations the picture can show -----

            Repeater {
                model: root.contexts

                delegate: Column {
                    width: column.width
                    spacing: 6

                    readonly property var contextChoices: modelData.choices
                    //! NOTE: a segmented control shows the situations when they fit; more than three
                    //! or ones which do not fit are a dropdown
                    readonly property int contextSegmentWidth: EoMusic.segmentWidthFor(contextChoices)
                    readonly property bool asDropdown: contextChoices.length > 3
                                                       || contextChoices.length * (contextSegmentWidth + 2) > width

                    StyledTextLabel {
                        width: parent.width
                        horizontalAlignment: Text.AlignLeft

                        text: modelData.title
                        font: AtStyle.font(12)
                        color: AtStyle.text2
                    }

                    AtSegmented {
                        visible: !parent.asDropdown

                        model: modelData.choices
                        currentValue: modelData.value
                        segmentWidth: parent.contextSegmentWidth

                        navigationPanel: root.navigationPanel
                        navigationOrderStart: 900100 + index * 10

                        onActivated: function(value) {
                            modelData.value = value
                        }
                    }

                    StyledDropdown {
                        visible: parent.asDropdown
                        width: Math.min(280, parent.width)

                        model: modelData.choices
                        textRole: "title"
                        valueRole: "value"

                        currentIndex: {
                            for (var i = 0; i < modelData.choices.length; ++i) {
                                if (modelData.choices[i].value === modelData.value) {
                                    return i
                                }
                            }
                            return -1
                        }

                        navigation.panel: root.navigationPanel
                        navigation.row: 900100 + index * 10
                        navigation.name: "Context_" + modelData.value

                        onActivated: function(index, value) {
                            modelData.value = value
                        }
                    }
                }
            }

            // ----- the option -----

            Column {
                width: parent.width
                spacing: 4

                visible: Boolean(root.option)

                StyledTextLabel {
                    width: parent.width
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.WordWrap

                    text: Boolean(root.option) ? EoTitles.plain(root.option.fullTitle) : ""
                    font: AtStyle.font(13, Font.Medium)
                }

                StyledTextLabel {
                    width: parent.width
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.WordWrap

                    visible: Boolean(root.option) && Boolean(root.option.hint)
                    text: Boolean(root.option) ? root.option.hint : ""
                    font: AtStyle.font(12)
                    color: AtStyle.text2
                }

                StyledTextLabel {
                    width: parent.width
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.WordWrap

                    visible: card.visible && root.isShownByEffect && !card.hasEffect
                    text: qsTrc("notation/engravingoptions", "This music does not show the option.")
                    font: AtStyle.font(12)
                    color: AtStyle.text3
                }
            }
        }
    }
}
