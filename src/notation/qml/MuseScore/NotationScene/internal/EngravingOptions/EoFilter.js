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

//! NOTE: What the dialog asks of the descriptions of the pages (EoPage, EoSection, EoOption):
//! which style values an option stands for, and whether an option is found by a search

//! NOTE: the words of a search text, in lower case
function words(text) {
    return String(text).toLowerCase().split(/\s+/).filter(function(word) {
        return word.length > 0
    })
}

//! NOTE: the names of the style values of an option; none for a picture or a paragraph
function optionSids(option) {
    var result = []

    if (Boolean(option.sid)) {
        result.push(option.sid)
    }

    var extra = option.extraSids
    for (var i = 0; Boolean(extra) && i < extra.length; ++i) {
        result.push(extra[i])
    }

    return result
}

function sectionSids(section) {
    var result = []

    for (var i = 0; i < section.options.length; ++i) {
        result = result.concat(optionSids(section.options[i]))
    }

    return result
}

function pageSids(page) {
    var result = []

    for (var i = 0; i < page.sections.length; ++i) {
        result = result.concat(sectionSids(page.sections[i]))
    }

    return result
}

function joined(texts) {
    return texts.filter(function(text) {
        return Boolean(text)
    }).join(" ").toLowerCase()
}

//! NOTE: the text in which an option is searched: its own, the titles of its choices,
//! and the titles of its section and of its page, so that a search for the name of a page finds all of it
function searchedText(option, section, page) {
    var texts = [option.title, option.hint, option.sid, section.title, page.title]
    texts = texts.concat(option.keywords, page.keywords)

    var choices = option.choices
    for (var i = 0; Boolean(choices) && i < choices.length; ++i) {
        texts.push(choices[i].title)
    }

    return joined(texts)
}

function hasAllWords(text, searchWords) {
    for (var i = 0; i < searchWords.length; ++i) {
        if (text.indexOf(searchWords[i]) < 0) {
            return false
        }
    }

    return true
}

//! NOTE: whether an option with style values is shown: it is found by the search,
//! and, if only the changed values are wanted, one of its values is changed
function isOptionShown(option, section, page, searchWords, modifiedOnly, model) {
    if (!option.visible || !section.visible) {
        return false
    }

    var sids = optionSids(option)
    if (sids.length === 0) {
        return false
    }

    if (searchWords.length > 0 && !hasAllWords(searchedText(option, section, page), searchWords)) {
        return false
    }

    return !modifiedOnly || model.modifiedCount(sids) > 0
}

function shownOptionCount(section, page, searchWords, modifiedOnly, model) {
    var count = 0

    for (var i = 0; i < section.options.length; ++i) {
        if (isOptionShown(section.options[i], section, page, searchWords, modifiedOnly, model)) {
            ++count
        }
    }

    return count
}

function shownOptionCountOfPage(page, searchWords, modifiedOnly, model) {
    var count = 0

    for (var i = 0; i < page.sections.length; ++i) {
        count += shownOptionCount(page.sections[i], page, searchWords, modifiedOnly, model)
    }

    return count
}
