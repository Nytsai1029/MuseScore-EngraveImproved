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
import MuseScore.Atelier 1.0

import ".."

EoPage {
    id: page

    code: "score"
    legacyCodes: ["score"]
    title: qsTrc("EditStyleBase", "Score")
    group: qsTrc("notation/engravingoptions", "Layout")

    EoSection {
        title: qsTrc("notation/engravingoptions", "General")

        EoChoice {
            sid: "musicalSymbolFont"
            title: qsTrc("EditStyleBase", "Musical symbols font:")
            presentation: "dropdown"
            choices: page.model.choices("musicFonts")
        }

        EoCustom {
            title: qsTrc("EditStyleBase", "Automatically load style settings based on font")
            hint: qsTrc("EditStyleBase", "MuseScore Studio will change the style to suit the font better")
            fullWidth: false

            component: AtSwitch {
                property var optionsModel: null

                checked: Boolean(optionsModel) && optionsModel.loadsFontDefaults

                onToggled: {
                    optionsModel.loadsFontDefaults = !optionsModel.loadsFontDefaults
                }
            }
        }

        EoChoice {
            sid: "musicalTextFont"
            title: qsTrc("EditStyleBase", "Musical text font:")
            presentation: "dropdown"
            choices: page.model.choices("musicTextFonts")
        }

        EoNumber {
            sid: "staffLineWidth"
            title: qsTrc("EditStyleBase", "Staff line thickness:")
            step: 0.1
        }

        EoToggle {
            sid: "concertPitch"
            title: qsTrc("EditStyleBase", "Display in concert pitch")
        }

        EoToggle {
            sid: "enableIndentationOnFirstSystem"
            title: qsTrc("EditStyleBase", "Enable indentation on first system")
        }

        EoNumber {
            sid: "firstSystemIndentationValue"
            title: qsTrc("EditStyleBase", "First system indentation:")
            max: 1000
            step: 0.5
            enabled: Boolean(page.value("enableIndentationOnFirstSystem"))
        }

        EoToggle {
            sid: "hideEmptyStaves"
            title: qsTrc("EditStyleBase", "Automatically hide all empty staves")
        }

        EoToggle {
            sid: "dontHideStavesInFirstSystem"
            title: qsTrc("EditStyleBase", "Don’t hide empty staves in first system")
            enabled: Boolean(page.value("hideEmptyStaves"))
        }

        EoToggle {
            sid: "alwaysShowBracketsWhenEmptyStavesAreHidden"
            title: qsTrc("EditStyleBase", "Show brackets when spanning a single staff")
            enabled: Boolean(page.value("hideEmptyStaves"))
        }

        EoToggle {
            sid: "alwaysShowSquareBracketsWhenEmptyStavesAreHidden"
            title: qsTrc("EditStyleBase", "Show square brackets when spanning a single staff")
            enabled: Boolean(page.value("hideEmptyStaves"))
        }

        EoToggle {
            sid: "crossMeasureValues"
            title: qsTrc("EditStyleBase",
                          "Display note values across measure boundaries (EXPERIMENTAL, early music only!)")
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Instrument names")

        EoToggle {
            sid: "hideInstrumentNameIfOneInstrument"
            title: qsTrc("EditStyleBase", "Hide if there is only 1 instrument")
        }

        EoNumber {
            sid: "instrumentNameOffset"
            title: qsTrc("EditStyleBase", "Distance from staff:")
            step: 0.1
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Swing settings")

        EoNumber {
            sid: "swingRatio"
            title: qsTrc("EditStyleBase", "Select swing ratio:")
            min: 25
            max: 99
            step: 1
            decimals: 0
        }
    }

    EoSection {
        title: qsTrc("EditStyleBase", "Autoplace")

        EoChoice {
            sid: "autoplaceVerticalAlignRange"
            title: qsTrc("EditStyleBase", "Vertical align range:")
            choices: [
                { value: 0, title: qsTrc("notation/editstyle", "Segment") },
                { value: 1, title: qsTrc("notation/editstyle", "Measure") },
                { value: 2, title: qsTrc("notation/editstyle", "System") }
            ]
        }

        EoNumber {
            sid: "minVerticalDistance"
            title: qsTrc("EditStyleBase", "Min. vertical distance:")
            min: -999.99
            step: 0.1
        }

        EoNumber {
            sid: "skylineMinHorizontalClearance"
            title: qsTrc("EditStyleBase", "Min. horizontal distance:")
        }
    }
}
