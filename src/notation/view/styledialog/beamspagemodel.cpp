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
#include "beamspagemodel.h"

using namespace mu::notation;

BeamsPageModel::BeamsPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::useWideBeams,
    StyleId::beamWidth,
    StyleId::beamMinLen,
    StyleId::beamNoSlope,
    StyleId::frenchStyleBeams,
    StyleId::useDefaultBeamSlantRules,
    StyleId::beamCustomMaxSlantForTwoNotes,
    StyleId::beamCustomTwoNoteMaxSlantSecondInterval,
    StyleId::beamCustomTwoNoteMaxSlantThirdInterval,
    StyleId::beamCustomTwoNoteMaxSlantFourthInterval,
    StyleId::beamCustomTwoNoteMaxSlantFifthInterval,
    StyleId::beamCustomTwoNoteMaxSlantSixthInterval,
    StyleId::beamCustomTwoNoteMaxSlantSeventhInterval,
    StyleId::beamCustomTwoNoteMaxSlantOctaveInterval,
    StyleId::beamCustomTwoNoteMaxSlantNinthInterval,
    StyleId::beamCustomTwoNoteMaxSlantTenthInterval,
    StyleId::beamCustomTwoNoteMaxSlantGreaterThanTenthInterval,
    StyleId::beamCustomMaxSlantSecondInterval,
    StyleId::beamCustomMaxSlantThirdInterval,
    StyleId::beamCustomMaxSlantFourthInterval,
    StyleId::beamCustomMaxSlantFifthInterval,
    StyleId::beamCustomMaxSlantSixthInterval,
    StyleId::beamCustomMaxSlantSeventhInterval,
    StyleId::beamCustomMaxSlantOctave,
    StyleId::beamCustomMaxSlantNinthInterval,
    StyleId::beamCustomMaxSlantTenthInterval,
    StyleId::beamCustomMaxSlantGreaterThanTenthInterval
})
{
}

StyleItem* BeamsPageModel::useWideBeams() const
{
    return styleItem(StyleId::useWideBeams);
}

StyleItem* BeamsPageModel::beamWidth() const
{
    return styleItem(StyleId::beamWidth);
}

StyleItem* BeamsPageModel::beamMinLen() const
{
    return styleItem(StyleId::beamMinLen);
}

StyleItem* BeamsPageModel::beamNoSlope() const
{
    return styleItem(StyleId::beamNoSlope);
}

StyleItem* BeamsPageModel::frenchStyleBeams() const
{
    return styleItem(StyleId::frenchStyleBeams);
}

StyleItem* BeamsPageModel::useDefaultBeamSlantRules() const
{
    return styleItem(StyleId::useDefaultBeamSlantRules);
}

StyleItem* BeamsPageModel::beamCustomMaxSlantForTwoNotes() const
{
    return styleItem(StyleId::beamCustomMaxSlantForTwoNotes);
}

StyleItem* BeamsPageModel::beamCustomTwoNoteMaxSlantSecondInterval() const
{
    return styleItem(StyleId::beamCustomTwoNoteMaxSlantSecondInterval);
}

StyleItem* BeamsPageModel::beamCustomTwoNoteMaxSlantThirdInterval() const
{
    return styleItem(StyleId::beamCustomTwoNoteMaxSlantThirdInterval);
}

StyleItem* BeamsPageModel::beamCustomTwoNoteMaxSlantFourthInterval() const
{
    return styleItem(StyleId::beamCustomTwoNoteMaxSlantFourthInterval);
}

StyleItem* BeamsPageModel::beamCustomTwoNoteMaxSlantFifthInterval() const
{
    return styleItem(StyleId::beamCustomTwoNoteMaxSlantFifthInterval);
}

StyleItem* BeamsPageModel::beamCustomTwoNoteMaxSlantSixthInterval() const
{
    return styleItem(StyleId::beamCustomTwoNoteMaxSlantSixthInterval);
}

StyleItem* BeamsPageModel::beamCustomTwoNoteMaxSlantSeventhInterval() const
{
    return styleItem(StyleId::beamCustomTwoNoteMaxSlantSeventhInterval);
}

StyleItem* BeamsPageModel::beamCustomTwoNoteMaxSlantOctaveInterval() const
{
    return styleItem(StyleId::beamCustomTwoNoteMaxSlantOctaveInterval);
}

StyleItem* BeamsPageModel::beamCustomTwoNoteMaxSlantNinthInterval() const
{
    return styleItem(StyleId::beamCustomTwoNoteMaxSlantNinthInterval);
}

StyleItem* BeamsPageModel::beamCustomTwoNoteMaxSlantTenthInterval() const
{
    return styleItem(StyleId::beamCustomTwoNoteMaxSlantTenthInterval);
}

StyleItem* BeamsPageModel::beamCustomTwoNoteMaxSlantGreaterThanTenthInterval() const
{
    return styleItem(StyleId::beamCustomTwoNoteMaxSlantGreaterThanTenthInterval);
}

StyleItem* BeamsPageModel::beamCustomMaxSlantSecondInterval() const
{
    return styleItem(StyleId::beamCustomMaxSlantSecondInterval);
}

StyleItem* BeamsPageModel::beamCustomMaxSlantThirdInterval() const
{
    return styleItem(StyleId::beamCustomMaxSlantThirdInterval);
}

StyleItem* BeamsPageModel::beamCustomMaxSlantFourthInterval() const
{
    return styleItem(StyleId::beamCustomMaxSlantFourthInterval);
}

StyleItem* BeamsPageModel::beamCustomMaxSlantFifthInterval() const
{
    return styleItem(StyleId::beamCustomMaxSlantFifthInterval);
}

StyleItem* BeamsPageModel::beamCustomMaxSlantSixthInterval() const
{
    return styleItem(StyleId::beamCustomMaxSlantSixthInterval);
}

StyleItem* BeamsPageModel::beamCustomMaxSlantSeventhInterval() const
{
    return styleItem(StyleId::beamCustomMaxSlantSeventhInterval);
}

StyleItem* BeamsPageModel::beamCustomMaxSlantOctave() const
{
    return styleItem(StyleId::beamCustomMaxSlantOctave);
}

StyleItem* BeamsPageModel::beamCustomMaxSlantNinthInterval() const
{
    return styleItem(StyleId::beamCustomMaxSlantNinthInterval);
}

StyleItem* BeamsPageModel::beamCustomMaxSlantTenthInterval() const
{
    return styleItem(StyleId::beamCustomMaxSlantTenthInterval);
}

StyleItem* BeamsPageModel::beamCustomMaxSlantGreaterThanTenthInterval() const
{
    return styleItem(StyleId::beamCustomMaxSlantGreaterThanTenthInterval);
}
