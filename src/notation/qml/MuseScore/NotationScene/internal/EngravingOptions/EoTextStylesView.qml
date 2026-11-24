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

//! NOTE: the view of the text styles page: the text styles on the left, the values of the chosen one on the right.
//! Each value is an option of the dialog like those of the other pages (its control, the button which resets it,
//! the music of the preview pane), made for the chosen style from its values
//! (EngravingOptionsModel::textStyleProperties) and the music the page gives the style (EoTextStyleMusic)
Item {
    id: root

    //! NOTE: set by the dialog: the TextStylesPage, the EngravingOptionsModel, the EoSession, and the code of the
    //! text style asked for from outside (an "Edit … text style" link, the inspector)
    property var page: null
    property var optionsModel: null
    property var session: null
    property NavigationPanel navigationPanel: null
    property string subPageCode: ""

    readonly property var styles: {
        //! NOTE: the names of the user styles are values of the style
        Boolean(root.optionsModel) && root.optionsModel.revision
        return Boolean(root.optionsModel) ? root.optionsModel.textStyles() : []
    }

    property int currentIndex: 0
    readonly property int styleLineHeight: 28
    readonly property var currentStyle: root.styles.length > root.currentIndex ? root.styles[root.currentIndex] : null

    //! NOTE: the options of the chosen style, made when it is chosen
    property var options: []

    readonly property var sids: {
        var result = []
        for (var i = 0; i < root.options.length; ++i) {
            result.push(root.options[i].sid)
        }
        return result
    }

    readonly property int modifiedCount: {
        Boolean(root.optionsModel) && root.optionsModel.revision
        return root.sids.length > 0 ? root.optionsModel.modifiedCount(root.sids) : 0
    }

    function indexOfCode(code) {
        for (var i = 0; i < root.styles.length; ++i) {
            if (root.styles[i].code === code) {
                return i
            }
        }
        return -1
    }

    function musicOf(code) {
        var musics = Boolean(root.page) && Boolean(root.page.musics) ? root.page.musics : []
        for (var i = 0; i < musics.length; ++i) {
            if (musics[i].styleCode === code) {
                return musics[i]
            }
        }
        return null
    }

    //! NOTE: the option of one value of a style: its kind, its title and its range, as the style dialog has them
    function optionProperties(property) {
        switch (property) {
        case "fontFace":
            return { component: fontComp, title: qsTrc("EditStyleBase", "Font:") }
        case "fontSize":
            return { component: numberComp, title: qsTrc("EditStyleBase", "Size:"), min: 1, max: 500, step: 0.5,
                     decimals: 1, unit: "pt" }
        case "lineSpacing":
            return { component: numberComp, title: qsTrc("EditStyleBase", "Line spacing:"), min: 0.1, max: 10,
                     step: 0.1, decimals: 2, unit: qsTrc("EditStyleBase", "li"),
                     hint: qsTrc("notation/engravingoptions",
                                 "The distance from one line of the text to the next, as a multiple of the usual " +
                                 "one.") }
        case "letterSpacing":
            return { component: numberComp, title: qsTrc("EditStyleBase", "Letter spacing:"), min: -95, max: 500,
                     step: 1, decimals: 0, unit: "%",
                     hint: qsTrc("notation/engravingoptions",
                                 "Space added between the letters, as a percentage of the usual one; 0 is the " +
                                 "font’s own spacing.") }
        case "sizeSpatiumDependent":
            return { component: toggleComp, title: qsTrc("EditStyleBase", "Follow staff size"),
                     hint: qsTrc("notation/engravingoptions",
                                 "The text grows and shrinks with the staff: on a small staff, or with another " +
                                 "staff space, it is smaller or larger.") }
        case "fontStyle":
            return { component: fontStyleComp, title: qsTrc("EditStyleBase", "Style:") }
        case "color":
            return { component: colorComp, title: qsTrc("EditStyleBase", "Color:") }
        case "textAlign":
            return { component: alignComp, title: qsTrc("EditStyleBase", "Align:"),
                     hint: qsTrc("notation/engravingoptions",
                                 "Which point of the text stands at its place: its left edge, centre or right " +
                                 "edge, and its top, middle, bottom or baseline.") }
        case "offset":
            return { component: offsetComp, title: qsTrc("EditStyleBase", "Offset:"),
                     hint: qsTrc("notation/engravingoptions",
                                 "Where the text stands from the place it belongs to: X to the right, Y down.") }
        case "frameType":
            return { component: choiceComp, title: qsTrc("EditStyleBase", "Frame:"), choices: [
                    { value: 0, title: qsTrc("notation/editstyle", "None", "no frame for text") },
                    { value: 1, title: qsTrc("notation/editstyle", "Rectangle") },
                    { value: 2, title: qsTrc("notation/editstyle", "Circle") }
                ] }
        case "framePadding":
            return { component: numberComp, title: qsTrc("EditStyleBase", "Padding:"), min: 0, max: 10, step: 0.05,
                     decimals: 2, unit: qsTrc("global", "sp"), framed: true }
        case "frameWidth":
            return { component: numberComp, title: qsTrc("EditStyleBase", "Thickness:"), min: 0, max: 5,
                     step: 0.01, decimals: 2, unit: qsTrc("global", "sp"), framed: true }
        case "frameRound":
            return { component: numberComp, title: qsTrc("EditStyleBase", "Corner radius:"), min: 0, max: 100,
                     step: 1, decimals: 0, unit: "", framed: true }
        case "frameBorderColor":
            return { component: colorComp, title: qsTrc("EditStyleBase", "Border:"), framed: true }
        case "frameFillColor":
            return { component: colorComp, title: qsTrc("EditStyleBase", "Highlight:"), framed: true }
        case "musicalSymbolsScale":
            return { component: numberComp, title: qsTrc("EditStyleBase", "Musical symbols scale:"), min: 0,
                     max: 1000, step: 5, decimals: 0, percent: true,
                     hint: qsTrc("notation/engravingoptions",
                                 "Size of the musical symbols in the text, as a percentage of their size in the " +
                                 "staff.") }
        case "position":
            return { component: choiceComp, title: qsTrc("EditStyleBase", "Position:"), choices: [
                    { value: 0, title: qsTrc("notation/engravingoptions", "Left") },
                    { value: 2, title: qsTrc("notation/engravingoptions", "Center") },
                    { value: 1, title: qsTrc("notation/engravingoptions", "Right") }
                ] }
        }
        return null
    }

    function makeOption(component, properties, music) {
        var initial = { sid: properties.sid, title: properties.title }
        for (var key of [ "hint", "min", "max", "step", "decimals", "unit", "percent", "choices" ]) {
            if (properties[key] !== undefined) {
                initial[key] = properties[key]
            }
        }
        if (Boolean(music)) {
            initial.music = music.music
            initial.musicStyle = music.musicStyle
            initial.systemWidth = music.systemWidth
        }
        return component.createObject(root, initial)
    }

    //! NOTE: the options of the chosen style: the name of a user style, its values in the order of the style,
    //! the tuplet's own switch; the values of the frame wait for a frame
    function makeOptions() {
        for (var old of root.options) {
            old.destroy()
        }

        var made = []
        var style = root.currentStyle
        if (!Boolean(style) || !Boolean(root.optionsModel)) {
            root.options = made
            return
        }

        var music = root.musicOf(style.code)

        if (Boolean(style.nameSid)) {
            made.push(root.makeOption(textComp, { sid: style.nameSid, title: qsTrc("EditStyleBase", "Name:") },
                                      music))
        }

        var frameSid = ""
        var values = root.optionsModel.textStyleProperties(style.type)
        for (var value of values) {
            var properties = root.optionProperties(value.property)
            if (!Boolean(properties)) {
                continue
            }
            properties.sid = value.sid
            var option = root.makeOption(properties.component, properties, music)
            if (value.property === "frameType") {
                frameSid = value.sid
            }
            if (properties.framed && Boolean(frameSid)) {
                const sid = frameSid
                option.enabled = Qt.binding(function() { return root.page.value(sid) !== 0 })
            }
            made.push(option)
        }

        if (style.code === "tuplet") {
            const title = qsTrc("EditStyleBase", "Use musical symbols font for tuplet numbers")
            made.push(root.makeOption(toggleComp, { sid: "tupletUseSymbols", title: title }, music))
        }

        root.options = made

        //! NOTE: the preview shows the chosen style at once, with its first value
        if (Boolean(root.session) && made.length > 0) {
            root.session.activate(made[0], null, root.page)
        }
    }

    onCurrentStyleChanged: {
        //! NOTE: the list of the styles is made again when a value changes; the options stay while the style does
        if (Boolean(root.currentStyle) && root.options.length > 0
            && root.options[0].styleCode === root.currentStyle.code) {
            return
        }
        root.makeOptions()
        for (var option of root.options) {
            option.styleCode = root.currentStyle ? root.currentStyle.code : ""
        }
    }

    //! NOTE: the chosen style is scrolled into the list (one asked for from outside may be far down)
    function showCurrentStyle() {
        var top = styleColumn.y + root.currentIndex * root.styleLineHeight
        if (top < styleList.contentY) {
            styleList.contentY = top
        } else if (top + root.styleLineHeight > styleList.contentY + styleList.height) {
            styleList.contentY = top + root.styleLineHeight - styleList.height
        }
    }

    onCurrentIndexChanged: {
        Qt.callLater(root.showCurrentStyle)
    }

    //! NOTE: the dialog gives the session after the view is made, with its first style
    onSessionChanged: {
        if (Boolean(root.session) && root.options.length > 0) {
            root.session.activate(root.options[0], null, root.page)
        }
    }

    onSubPageCodeChanged: {
        var index = root.indexOfCode(root.subPageCode)
        if (index >= 0) {
            root.currentIndex = index
        }
    }

    Component.onCompleted: {
        var index = root.indexOfCode(root.subPageCode)
        root.currentIndex = Math.max(0, index)
    }

    Component.onDestruction: {
        for (var option of root.options) {
            option.destroy()
        }
    }

    Component { id: textComp; EoText { property string styleCode: "" } }
    Component { id: fontComp; EoFont { property string styleCode: "" } }
    Component { id: numberComp; EoNumber { property string styleCode: "" } }
    Component { id: toggleComp; EoToggle { property string styleCode: "" } }
    Component { id: fontStyleComp; EoFontStyle { property string styleCode: "" } }
    Component { id: colorComp; EoColor { property string styleCode: "" } }
    Component { id: alignComp; EoAlign { property string styleCode: "" } }
    Component { id: offsetComp; EoOffset { property string styleCode: "" } }
    Component { id: choiceComp; EoChoice { property string styleCode: "" } }

    // ----- the text styles -----

    StyledFlickable {
        id: styleList

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 220

        contentWidth: width
        contentHeight: styleColumn.height + 24

        Column {
            id: styleColumn

            x: 12
            y: 12
            width: parent.width - 24

            Repeater {
                model: root.styles

                //! NOTE: a line as those of the pages in the sidebar
                delegate: FlatButton {
                    id: styleLine

                    readonly property bool isCurrent: index === root.currentIndex

                    width: styleColumn.width
                    height: root.styleLineHeight

                    transparent: true
                    isClickOnKeyNavTriggered: true

                    backgroundItem: Rectangle {
                        radius: 7
                        color: {
                            if (styleLine.isCurrent) {
                                return AtStyle.soft
                            }
                            if (styleLine.mouseArea.pressed) {
                                return AtStyle.pressed
                            }
                            return styleLine.mouseArea.containsMouse ? AtStyle.hover : "transparent"
                        }

                        NavigationFocusBorder {
                            navigationCtrl: styleLine.navigation
                        }
                    }

                    contentItem: Item {
                        implicitWidth: 0
                        implicitHeight: 0
                    }

                    StyledTextLabel {
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.right: parent.right
                        anchors.rightMargin: 8
                        anchors.verticalCenter: parent.verticalCenter

                        horizontalAlignment: Text.AlignLeft
                        text: modelData.name
                        font: AtStyle.font(13, styleLine.isCurrent ? Font.DemiBold : Font.Normal)
                        color: styleLine.isCurrent ? AtStyle.text : AtStyle.text2
                    }

                    navigation.panel: root.navigationPanel
                    navigation.row: index
                    navigation.column: 0
                    navigation.name: "TextStyle_" + modelData.code
                    navigation.accessible.name: modelData.name

                    onClicked: {
                        root.currentIndex = index
                    }
                }
            }
        }
    }

    Rectangle {
        id: divider

        anchors.left: styleList.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1

        color: AtStyle.stroke
    }

    // ----- the values of the chosen style -----

    StyledFlickable {
        anchors.left: divider.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        contentWidth: width
        contentHeight: valueColumn.y + valueColumn.height + 32

        Column {
            id: valueColumn

            x: 28
            y: 24
            width: Math.min(parent.width - 56, 720)

            spacing: 6

            Item {
                width: parent.width
                height: 30

                StyledTextLabel {
                    anchors.left: parent.left
                    anchors.right: resetStyleButton.left
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    horizontalAlignment: Text.AlignLeft

                    text: Boolean(root.currentStyle) ? root.currentStyle.name : ""
                    font: AtStyle.font(15, Font.DemiBold)
                }

                AtToolButton {
                    id: resetStyleButton

                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    height: 24

                    visible: root.modifiedCount > 0

                    shape: "reset"
                    text: qsTrc("notation/engravingoptions", "Reset text style")
                    textFont: AtStyle.font(12, Font.Medium)

                    navigation.panel: root.navigationPanel
                    navigation.row: 1000
                    navigation.column: 2
                    navigation.name: "ResetTextStyle"

                    onClicked: {
                        root.optionsModel.resetValues(root.sids)
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: AtStyle.stroke
            }

            Repeater {
                model: root.options

                //! NOTE: the view has required properties, so the model's are declared too
                delegate: EoOptionView {
                    required property var modelData
                    required property int index

                    width: valueColumn.width

                    option: modelData
                    page: root.page
                    session: root.session
                    optionsModel: root.optionsModel

                    navigationPanel: root.navigationPanel
                    navigationRow: 1001 + index * 20
                }
            }
        }
    }
}
