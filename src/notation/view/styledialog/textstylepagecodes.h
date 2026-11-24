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
#pragma once

#include <QStringList>

namespace mu::notation {
//! NOTE The codes by which a text style is asked for from outside the style dialogs (the inspector, the staff type
//! dialog, the "Edit … text style" buttons of the pages), in the order of engraving::editableTextStyles()
inline const QStringList& textStyleSubPageCodes()
{
    static const QStringList CODES {
        "title", "subtitle", "composer", "poet", "translator", "frame", "instrument-name-part",
        "instrument-name-long", "instrument-name-short", "instrument-change", "header", "footer", "copyright",
        "page-number", "measure-number", "measure-number-alternate", "multimeasure-rest-range", "tempo",
        "tempo-change", "metronome", "repeat-play-count", "repeat-text-left", "repeat-text-right", "rehearsal-mark",
        "system", "staff", "expression", "hairpin", "lyrics-odd-lines", "lyrics-even-lines", "chord-symbols",
        "chord-symbols-alternate", "roman-numeral-analysis", "nashville-number", "tuplet", "articulation",
        "sticking", "fingering", "tab-fret-number", "lh-guitar-fingering", "rh-guitar-fingering",
        "hammer-ons-pull-offs-and-tapping", "string-number", "string-tunings", "fretboard-diagram-fingering",
        "fretboard-diagram-fret-number", "harp-pedal-diagram", "harp-pedal-text-diagram", "text-line", "note-line",
        "volta", "ottava", "glissando", "pedal", "bend", "let-ring", "palm-mute", "user1", "user2", "user3",
        "user4", "user5", "user6", "user7", "user8", "user9", "user10", "user11", "user12"
    };

    return CODES;
}
}
