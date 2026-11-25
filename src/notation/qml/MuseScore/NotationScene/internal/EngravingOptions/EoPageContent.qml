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

import "EoFilter.js" as EoFilter

//! NOTE: the sections of a page one under the other, each with its options.
//! With a search text or with "changed only", the options which do not match are left out,
//! and so are the pictures, the paragraphs and the sections of which nothing is left
Column {
    id: root

    //! NOTE: an EoPage
    required property var page

    //! NOTE: the EngravingOptionsModel
    required property var optionsModel

    //! NOTE: the EoSession, which takes the option under the pointer or with the focus
    property var session: null

    property var searchWords: []
    property bool modifiedOnly: false

    property NavigationPanel navigationPanel: null

    //! NOTE: the rows of the navigation panel which the page has to itself begin here
    property int navigationRowStart: 10

    readonly property bool isFiltered: searchWords.length > 0 || modifiedOnly

    //! NOTE: where the section begins in the column
    function sectionY(sectionIndex) {
        var item = sectionsRepeater.itemAt(sectionIndex)
        return Boolean(item) ? item.y : 0
    }

    //! NOTE: the section which is at this height of the column (the last one which begins above it)
    function sectionIndexAt(y) {
        var result = 0

        for (var i = 0; i < sectionsRepeater.count; ++i) {
            var item = sectionsRepeater.itemAt(i)
            if (Boolean(item) && item.visible && item.y <= y) {
                result = i
            }
        }

        return result
    }

    spacing: 28

    Repeater {
        id: sectionsRepeater

        model: root.page.sections

        delegate: Column {
            id: sectionItem

            readonly property var section: modelData
            readonly property int sectionIndex: index
            readonly property int navigationRowStart: root.navigationRowStart + sectionIndex * 5000

            readonly property var sids: EoFilter.sectionSids(section)

            readonly property int modifiedCount: {
                root.optionsModel.revision
                return root.optionsModel.modifiedCount(sids)
            }

            readonly property bool isShown: {
                if (!section.visible) {
                    return false
                }

                if (!root.isFiltered) {
                    return true
                }

                root.optionsModel.revision
                return EoFilter.shownOptionCount(section, root.page, root.searchWords, root.modifiedOnly,
                                                 root.optionsModel) > 0
            }

            width: root.width
            visible: isShown
            spacing: 4

            // ----- the title of the section -----

            Item {
                width: parent.width
                height: Math.max(30, sectionTitles.implicitHeight + 6)

                visible: Boolean(sectionItem.section.title)

                Column {
                    id: sectionTitles

                    anchors.left: parent.left
                    anchors.right: sectionResetButton.left
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter

                    spacing: 2

                    StyledTextLabel {
                        width: parent.width
                        horizontalAlignment: Text.AlignLeft

                        text: sectionItem.section.title
                        font: AtStyle.font(15, Font.DemiBold)
                    }

                    StyledTextLabel {
                        width: parent.width
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap

                        visible: Boolean(sectionItem.section.hint)
                        text: sectionItem.section.hint
                        font: AtStyle.font(12)
                        color: AtStyle.text3
                    }
                }

                AtToolButton {
                    id: sectionResetButton

                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter

                    height: 24

                    visible: sectionItem.modifiedCount > 0

                    shape: "reset"
                    text: qsTrc("notation/engravingoptions", "Reset section")
                    textFont: AtStyle.font(12, Font.Medium)

                    //: %n is the number of options which differ from their defaults
                    toolTipTitle: qsTrc("notation/engravingoptions", "%n changed option(s)", "",
                                        sectionItem.modifiedCount)

                    navigation.panel: root.navigationPanel
                    navigation.row: sectionItem.navigationRowStart
                    navigation.column: 1
                    navigation.name: "ResetSection_" + root.page.code + "_" + sectionItem.sectionIndex

                    onClicked: {
                        root.optionsModel.resetValues(sectionItem.sids)
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 1

                visible: Boolean(sectionItem.section.title)
                color: AtStyle.stroke
            }

            Item {
                width: parent.width
                height: 4

                visible: Boolean(sectionItem.section.title)
            }

            // ----- the options of the section -----

            Repeater {
                model: sectionItem.section.options

                //! NOTE: an option which is not shown is not made
                delegate: Loader {
                    id: optionLoader

                    readonly property var option: modelData
                    readonly property int optionIndex: index

                    readonly property bool isShown: {
                        //! NOTE: a context is chosen beside the preview, it is not shown among the options
                        if (!option.visible || option.kind === "context") {
                            return false
                        }

                        if (!root.isFiltered) {
                            return true
                        }

                        root.optionsModel.revision
                        return EoFilter.isOptionShown(option, sectionItem.section, root.page, root.searchWords,
                                                      root.modifiedOnly, root.optionsModel)
                    }

                    width: sectionItem.width

                    active: isShown
                    visible: isShown

                    sourceComponent: optionLoader.option.kind === "grid" ? gridComp : optionComp

                    Component {
                        id: optionComp

                        EoOptionView {
                            option: optionLoader.option
                            section: sectionItem.section
                            page: root.page
                            session: root.session
                            optionsModel: root.optionsModel

                            navigationPanel: root.navigationPanel
                            navigationRow: sectionItem.navigationRowStart + optionLoader.optionIndex * 50 + 1
                        }
                    }

                    Component {
                        id: gridComp

                        EoGridView {
                            option: optionLoader.option
                            section: sectionItem.section
                            page: root.page
                            session: root.session
                            optionsModel: root.optionsModel

                            searchWords: root.searchWords
                            modifiedOnly: root.modifiedOnly

                            navigationPanel: root.navigationPanel
                            navigationRow: sectionItem.navigationRowStart + optionLoader.optionIndex * 50 + 1
                        }
                    }
                }
            }
        }
    }
}
