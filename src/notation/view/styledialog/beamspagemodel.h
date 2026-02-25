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
#ifndef MU_NOTATION_BEAMSPAGEMODEL_H
#define MU_NOTATION_BEAMSPAGEMODEL_H

#include "abstractstyledialogmodel.h"

namespace mu::notation {
class BeamsPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * useWideBeams READ useWideBeams CONSTANT)
    Q_PROPERTY(StyleItem * beamWidth READ beamWidth CONSTANT)
    Q_PROPERTY(StyleItem * beamMinLen READ beamMinLen CONSTANT)
    Q_PROPERTY(StyleItem * beamNoSlope READ beamNoSlope CONSTANT)
    Q_PROPERTY(StyleItem * frenchStyleBeams READ frenchStyleBeams CONSTANT)
    Q_PROPERTY(StyleItem * useDefaultBeamSlantRules READ useDefaultBeamSlantRules CONSTANT)
    Q_PROPERTY(StyleItem * beamCustomMaxSlantForTwoNotes READ beamCustomMaxSlantForTwoNotes CONSTANT)
    Q_PROPERTY(StyleItem * beamCustomTwoNoteMaxSlantSecondInterval READ beamCustomTwoNoteMaxSlantSecondInterval CONSTANT)
    Q_PROPERTY(StyleItem * beamCustomTwoNoteMaxSlantThirdInterval READ beamCustomTwoNoteMaxSlantThirdInterval CONSTANT)
    Q_PROPERTY(StyleItem * beamCustomTwoNoteMaxSlantFourthToNinthInterval READ beamCustomTwoNoteMaxSlantFourthToNinthInterval CONSTANT)
    Q_PROPERTY(StyleItem * beamCustomTwoNoteMaxSlantTenthInterval READ beamCustomTwoNoteMaxSlantTenthInterval CONSTANT)
    Q_PROPERTY(StyleItem * beamCustomTwoNoteMaxSlantGreaterThanTenthInterval READ beamCustomTwoNoteMaxSlantGreaterThanTenthInterval CONSTANT)
    Q_PROPERTY(StyleItem * beamCustomMaxSlantSecondInterval READ beamCustomMaxSlantSecondInterval CONSTANT)
    Q_PROPERTY(StyleItem * beamCustomMaxSlantThirdInterval READ beamCustomMaxSlantThirdInterval CONSTANT)
    Q_PROPERTY(StyleItem * beamCustomMaxSlantFourthInterval READ beamCustomMaxSlantFourthInterval CONSTANT)
    Q_PROPERTY(StyleItem * beamCustomMaxSlantFifthInterval READ beamCustomMaxSlantFifthInterval CONSTANT)
    Q_PROPERTY(StyleItem * beamCustomMaxSlantSixthInterval READ beamCustomMaxSlantSixthInterval CONSTANT)
    Q_PROPERTY(StyleItem * beamCustomMaxSlantSeventhInterval READ beamCustomMaxSlantSeventhInterval CONSTANT)
    Q_PROPERTY(StyleItem * beamCustomMaxSlantOctave READ beamCustomMaxSlantOctave CONSTANT)
    Q_PROPERTY(StyleItem * beamCustomMaxSlantGreaterThanOctave READ beamCustomMaxSlantGreaterThanOctave CONSTANT)

public:
    explicit BeamsPageModel(QObject* parent = nullptr);

    StyleItem* useWideBeams() const;
    StyleItem* beamWidth() const;
    StyleItem* beamMinLen() const;
    StyleItem* beamNoSlope() const;
    StyleItem* frenchStyleBeams() const;
    StyleItem* useDefaultBeamSlantRules() const;
    StyleItem* beamCustomMaxSlantForTwoNotes() const;
    StyleItem* beamCustomTwoNoteMaxSlantSecondInterval() const;
    StyleItem* beamCustomTwoNoteMaxSlantThirdInterval() const;
    StyleItem* beamCustomTwoNoteMaxSlantFourthToNinthInterval() const;
    StyleItem* beamCustomTwoNoteMaxSlantTenthInterval() const;
    StyleItem* beamCustomTwoNoteMaxSlantGreaterThanTenthInterval() const;
    StyleItem* beamCustomMaxSlantSecondInterval() const;
    StyleItem* beamCustomMaxSlantThirdInterval() const;
    StyleItem* beamCustomMaxSlantFourthInterval() const;
    StyleItem* beamCustomMaxSlantFifthInterval() const;
    StyleItem* beamCustomMaxSlantSixthInterval() const;
    StyleItem* beamCustomMaxSlantSeventhInterval() const;
    StyleItem* beamCustomMaxSlantOctave() const;
    StyleItem* beamCustomMaxSlantGreaterThanOctave() const;
};
}

#endif // MU_NOTATION_BEAMSPAGEMODEL_H
