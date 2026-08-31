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
#ifndef MU_NOTATION_NOTESPAGEMODEL_H
#define MU_NOTATION_NOTESPAGEMODEL_H

#include "abstractstyledialogmodel.h"

namespace mu::notation {
class NotesPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * useStraightNoteFlags READ useStraightNoteFlags CONSTANT)

    Q_PROPERTY(StyleItem * useDefaultStemShorteningRules READ useDefaultStemShorteningRules CONSTANT)
    Q_PROPERTY(StyleItem * stemCustomLengthFirstLine READ stemCustomLengthFirstLine CONSTANT)
    Q_PROPERTY(StyleItem * stemCustomLengthFirstSpace READ stemCustomLengthFirstSpace CONSTANT)
    Q_PROPERTY(StyleItem * stemCustomLengthSecondLine READ stemCustomLengthSecondLine CONSTANT)
    Q_PROPERTY(StyleItem * stemCustomLengthSecondSpace READ stemCustomLengthSecondSpace CONSTANT)
    Q_PROPERTY(StyleItem * stemCustomLengthThirdLine READ stemCustomLengthThirdLine CONSTANT)
    Q_PROPERTY(StyleItem * stemCustomLengthThirdSpace READ stemCustomLengthThirdSpace CONSTANT)
    Q_PROPERTY(StyleItem * stemCustomLengthFourthLine READ stemCustomLengthFourthLine CONSTANT)
    Q_PROPERTY(StyleItem * stemCustomLengthFourthSpace READ stemCustomLengthFourthSpace CONSTANT)
    Q_PROPERTY(StyleItem * stemCustomLengthFifthLine READ stemCustomLengthFifthLine CONSTANT)

public:
    explicit NotesPageModel(QObject* parent = nullptr);

    StyleItem* useStraightNoteFlags() const;

    StyleItem* useDefaultStemShorteningRules() const;
    StyleItem* stemCustomLengthFirstLine() const;
    StyleItem* stemCustomLengthFirstSpace() const;
    StyleItem* stemCustomLengthSecondLine() const;
    StyleItem* stemCustomLengthSecondSpace() const;
    StyleItem* stemCustomLengthThirdLine() const;
    StyleItem* stemCustomLengthThirdSpace() const;
    StyleItem* stemCustomLengthFourthLine() const;
    StyleItem* stemCustomLengthFourthSpace() const;
    StyleItem* stemCustomLengthFifthLine() const;
};
}

#endif // MU_NOTATION_NOTESPAGEMODEL_H
