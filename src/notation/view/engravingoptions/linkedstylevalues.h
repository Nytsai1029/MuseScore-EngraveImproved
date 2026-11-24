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

#include <utility>
#include <vector>

#include "engraving/style/styledef.h"
#include "engraving/types/propertyvalue.h"
#include "engraving/types/types.h"

namespace mu::notation {
//! NOTE The values which a value takes with it when it is chosen, as the style dialog sets them
//! (ChordSymbolsPageModel::setChordStyle, EditStyle::styleValueChanged). The Engraving Options dialog sets them
//! with the value, and its preview shows the value with them, so that what is shown is what choosing it does:
//! - a preset of the chord symbols is its description file; the legacy one keeps the bass note on the line
//!   (a custom file is chosen by itself);
//! - an old-style multimeasure rest of a single measure can only be a whole rest.
inline std::vector<std::pair<engraving::Sid, engraving::PropertyValue> > linkedStyleValues(
    engraving::Sid id, const engraving::PropertyValue& value)
{
    using namespace mu::engraving;

    switch (id) {
    case Sid::chordStyle:
        switch (value.value<ChordStylePreset>()) {
        case ChordStylePreset::STANDARD:
            return { { Sid::chordDescriptionFile, String(u"chords_std.xml") } };
        case ChordStylePreset::JAZZ:
            return { { Sid::chordDescriptionFile, String(u"chords_jazz.xml") } };
        case ChordStylePreset::LEGACY:
            return { { Sid::chordDescriptionFile, String(u"chords_legacy.xml") }, { Sid::chordBassNoteStagger, false } };
        case ChordStylePreset::CUSTOM:
            break;
        }
        break;
    case Sid::oldStyleMultiMeasureRests:
        if (value.toBool()) {
            return { { Sid::singleMeasureMMRestUseNormalRest, true } };
        }
        break;
    default:
        break;
    }

    return {};
}
}
