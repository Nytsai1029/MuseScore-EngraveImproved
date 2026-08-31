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
#include "notespagemodel.h"

using namespace mu::notation;

NotesPageModel::NotesPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::useStraightNoteFlags,
    StyleId::useDefaultStemShorteningRules,
    StyleId::stemCustomLengthFirstLine,
    StyleId::stemCustomLengthFirstSpace,
    StyleId::stemCustomLengthSecondLine,
    StyleId::stemCustomLengthSecondSpace,
    StyleId::stemCustomLengthThirdLine,
    StyleId::stemCustomLengthThirdSpace,
    StyleId::stemCustomLengthFourthLine,
    StyleId::stemCustomLengthFourthSpace,
    StyleId::stemCustomLengthFifthLine
})
{
}

StyleItem* NotesPageModel::useStraightNoteFlags() const
{
    return styleItem(StyleId::useStraightNoteFlags);
}

StyleItem* NotesPageModel::useDefaultStemShorteningRules() const
{
    return styleItem(StyleId::useDefaultStemShorteningRules);
}

StyleItem* NotesPageModel::stemCustomLengthFirstLine() const
{
    return styleItem(StyleId::stemCustomLengthFirstLine);
}

StyleItem* NotesPageModel::stemCustomLengthFirstSpace() const
{
    return styleItem(StyleId::stemCustomLengthFirstSpace);
}

StyleItem* NotesPageModel::stemCustomLengthSecondLine() const
{
    return styleItem(StyleId::stemCustomLengthSecondLine);
}

StyleItem* NotesPageModel::stemCustomLengthSecondSpace() const
{
    return styleItem(StyleId::stemCustomLengthSecondSpace);
}

StyleItem* NotesPageModel::stemCustomLengthThirdLine() const
{
    return styleItem(StyleId::stemCustomLengthThirdLine);
}

StyleItem* NotesPageModel::stemCustomLengthThirdSpace() const
{
    return styleItem(StyleId::stemCustomLengthThirdSpace);
}

StyleItem* NotesPageModel::stemCustomLengthFourthLine() const
{
    return styleItem(StyleId::stemCustomLengthFourthLine);
}

StyleItem* NotesPageModel::stemCustomLengthFourthSpace() const
{
    return styleItem(StyleId::stemCustomLengthFourthSpace);
}

StyleItem* NotesPageModel::stemCustomLengthFifthLine() const
{
    return styleItem(StyleId::stemCustomLengthFifthLine);
}
