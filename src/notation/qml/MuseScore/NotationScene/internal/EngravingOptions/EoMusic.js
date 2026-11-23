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
.pragma library

//! NOTE: Which music an option shows, and which option a page begins with

//! NOTE: the music of an option and the width its systems are laid out with: both of the object which
//! gives the music — the option itself, else its section, else its page
function musicInfoOf(option, section, page) {
    for (const object of [option, section, page]) {
        if (Boolean(object) && Boolean(object.music)) {
            return { music: object.music, systemWidth: object.systemWidth || 0 }
        }
    }

    return { music: "", systemWidth: 0 }
}



//! NOTE: the style values which the picture of an option sets: those of the page, then of the section,
//! then of the option
function musicStyleOf(option, section, page) {
    var result = {}

    var sources = [page, section, option]
    for (var i = 0; i < sources.length; ++i) {
        var style = Boolean(sources[i]) ? sources[i].musicStyle : null
        for (var name in style) {
            result[name] = style[name]
        }
    }

    return result
}

//! NOTE: the width of a segment of an AtSegmented so that the longest title fits:
//! between the width of a short word and of a phrase
function segmentWidthFor(choices) {
    var longest = 0
    for (var i = 0; i < choices.length; ++i) {
        longest = Math.max(longest, String(choices[i].title).length)
    }
    return Math.max(64, Math.min(168, 26 + longest * 11))
}

//! NOTE: the value an option of kind "choice" is shown with: the choice after the current one, wrapping
//! around; none when there is only one choice
function probeValueOf(option, styleItem) {
    if (!Boolean(option) || option.kind !== "choice" || !Boolean(option.choices) || option.choices.length < 2) {
        return undefined
    }

    var current = Boolean(styleItem) ? styleItem.value : undefined
    var index = 0
    for (var i = 0; i < option.choices.length; ++i) {
        if (option.choices[i].value === current) {
            index = i
            break
        }
    }

    return option.choices[(index + 1) % option.choices.length].value
}

//! NOTE: the first option of a page which is a style value: { option, section }, or null
function firstOption(page) {
    for (var i = 0; i < page.sections.length; ++i) {
        var section = page.sections[i]

        for (var k = 0; k < section.options.length; ++k) {
            var option = section.options[k]
            if (Boolean(option.sid) && option.visible && section.visible) {
                return { option: option, section: section }
            }
        }
    }

    return null
}
