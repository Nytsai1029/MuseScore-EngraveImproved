/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "abstractstyledialogmodel.h"

namespace mu::notation {
class AccidentalsPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * bracketedAccidentalPadding READ bracketedAccidentalPadding CONSTANT)
    Q_PROPERTY(StyleItem * accidentalNoteDistance READ accidentalNoteDistance CONSTANT)
    Q_PROPERTY(StyleItem * accidentalDistance READ accidentalDistance CONSTANT)
    Q_PROPERTY(StyleItem * accidentalFlushToLedgerLine READ accidentalFlushToLedgerLine CONSTANT)

    Q_PROPERTY(StyleItem * keySigNaturals READ keySigNaturals CONSTANT)
    Q_PROPERTY(StyleItem * keySigSharpAccidentalDistance READ keySigSharpAccidentalDistance CONSTANT)
    Q_PROPERTY(StyleItem * keySigFlatAccidentalDistance READ keySigFlatAccidentalDistance CONSTANT)
    Q_PROPERTY(StyleItem * keySigNaturalDistance READ keySigNaturalDistance CONSTANT)

    Q_PROPERTY(StyleItem * accidFollowNoteOffset READ accidFollowNoteOffset CONSTANT)
    Q_PROPERTY(StyleItem * alignAccidentalOctavesAcrossSubChords READ alignAccidentalOctavesAcrossSubChords CONSTANT)
    Q_PROPERTY(StyleItem * keepAccidentalSecondsTogether READ keepAccidentalSecondsTogether CONSTANT)
    Q_PROPERTY(StyleItem * alignOffsetOctaveAccidentals READ alignOffsetOctaveAccidentals CONSTANT)

public:
    explicit AccidentalsPageModel(QObject* parent = nullptr);

    StyleItem* bracketedAccidentalPadding() const;
    StyleItem* accidentalNoteDistance() const;
    StyleItem* accidentalDistance() const;
    StyleItem* accidentalFlushToLedgerLine() const;

    StyleItem* keySigNaturals() const;
    StyleItem* keySigSharpAccidentalDistance() const;
    StyleItem* keySigFlatAccidentalDistance() const;
    StyleItem* keySigNaturalDistance() const;

    StyleItem* accidFollowNoteOffset() const;
    StyleItem* alignAccidentalOctavesAcrossSubChords() const;
    StyleItem* keepAccidentalSecondsTogether() const;
    StyleItem* alignOffsetOctaveAccidentals() const;
};
}
