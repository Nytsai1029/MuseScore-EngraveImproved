/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
#include "upstreamformat.h"

#include <algorithm>
#include <iterator>

#include "style/style.h"

#include "officialexportreport.h"

using namespace mu::engraving;

static const std::string_view UPSTREAM_STYLE_KEYS[] = {
#include "upstreamstylenames.inc"
};

bool UpstreamFormat::hasStyleKey(std::string_view name)
{
    return std::binary_search(std::begin(UPSTREAM_STYLE_KEYS), std::end(UPSTREAM_STYLE_KEYS), name);
}

bool UpstreamFormat::hasStyle(Sid sid)
{
    if (sid == Sid::NOSTYLE || sid == Sid::STYLES) {
        return false;
    }
    return hasStyleKey(MStyle::valueName(sid));
}

bool UpstreamFormat::defaultDiffers(Sid sid)
{
    // Upstream masks barlines behind all text by default; the fork leaves that off and masks per text
    return sid == Sid::maskBarlinesForText;
}

muse::TranslatableString mu::engraving::droppedFeatureTitle(DroppedFeature feature)
{
    switch (feature) {
    case DroppedFeature::LetterSpacing:
        return muse::TranslatableString("engraving/officialexport", "Letter spacing of text");
    case DroppedFeature::LedgerLineLengths:
        return muse::TranslatableString("engraving/officialexport", "Adjusted and automatically shortened ledger lines");
    case DroppedFeature::HiddenLedgerLines:
        return muse::TranslatableString("engraving/officialexport", "Hidden ledger lines");
    case DroppedFeature::BrokenOttavaLines:
        return muse::TranslatableString("engraving/officialexport", "Broken ottava lines");
    case DroppedFeature::SlantedText:
        return muse::TranslatableString("engraving/officialexport", "Slanted text");
    case DroppedFeature::HairpinVerticalEnds:
        return muse::TranslatableString("engraving/officialexport", "Vertical ends of diagonal hairpins");
    case DroppedFeature::BracketSymbolSizes:
        return muse::TranslatableString("engraving/officialexport", "Sizes of hand and time signature brackets");
    case DroppedFeature::PerTextBarlineMask:
        return muse::TranslatableString("engraving/officialexport", "Barlines masked behind single text items");
    case DroppedFeature::KeySigAccidentalDistances:
        return muse::TranslatableString("engraving/officialexport",
                                        "Separate spacing of sharps and flats in key signatures");
    case DroppedFeature::TremoloStrokeOffsets:
        return muse::TranslatableString("engraving/officialexport", "Offsets of two-note tremolo strokes");
    case DroppedFeature::ParenthesisEndOffsets:
        return muse::TranslatableString("engraving/officialexport", "Adjusted ends of parentheses");
    case DroppedFeature::SeparateAccidentalParentheses:
        return muse::TranslatableString("engraving/officialexport", "Parentheses drawn apart from their accidentals");
    case DroppedFeature::BarlineConnectorEdits:
        return muse::TranslatableString("engraving/officialexport", "Edited barline segments between staves");
    case DroppedFeature::MusicSymbolTextCharacters:
        return muse::TranslatableString("engraving/officialexport", "Characters from the musical text font");
    case DroppedFeature::GraceNotesBeforeSystemBreak:
        return muse::TranslatableString("engraving/officialexport",
                                        "Grace notes shown at the end of the previous system");
    case DroppedFeature::SlurPiecesNotSplit:
        return muse::TranslatableString("engraving/officialexport",
                                        "Multi-curve slurs that could not be split into plain slurs");
    }
    return muse::TranslatableString();
}
