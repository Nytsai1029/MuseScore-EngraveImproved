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

#ifndef MU_ENGRAVING_LEDGERLINE_H
#define MU_ENGRAVING_LEDGERLINE_H

#include "engravingitem.h"

namespace mu::engraving {
class Chord;

//---------------------------------------------------------
//    @@ LedgerLine
///     Graphic representation of a ledger line.
//!
//!    parent:     Chord
//!    x-origin:   Chord
//!    y-origin:   SStaff
//---------------------------------------------------------

class LedgerLine final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, LedgerLine)
    DECLARE_CLASSOF(ElementType::LEDGER_LINE)

public:
    LedgerLine(EngravingItem*);
    ~LedgerLine();
    LedgerLine& operator=(const LedgerLine&) = delete;

    LedgerLine* clone() const override { return new LedgerLine(*this); }

    PointF pagePos() const override;        ///< position in page coordinates
    Chord* chord() const { return toChord(explicitParent()); }

    double len() const { return m_len; }
    void setLen(double v) { m_len = v; }

    Spatium ledgerLineLengthOffsetLeft() const { return m_lengthOffsetLeft; }
    void setLedgerLineLengthOffsetLeft(Spatium v) { m_lengthOffsetLeft = v; }
    Spatium ledgerLineLengthOffsetRight() const { return m_lengthOffsetRight; }
    void setLedgerLineLengthOffsetRight(Spatium v) { m_lengthOffsetRight = v; }

    void setVertical(bool v) { m_vertical = v; }
    bool vertical() const { return m_vertical; }

    double measureXPos() const;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& value) override;
    PropertyValue propertyDefault(Pid propertyId) const override;

    void startEdit(EditData& ed) override;
    void startEditDrag(EditData& ed) override;
    void editDrag(EditData& ed) override;
    void endEditDrag(EditData& ed) override;

    int gripsCount() const override { return 2; }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::END; }
    std::vector<PointF> gripsPositions(const EditData&) const override;

    void spatiumChanged(double /*oldValue*/, double /*newValue*/) override;

    struct LayoutData : public EngravingItem::LayoutData {
        double lineWidth = 0.0;
    };
    DECLARE_LAYOUTDATA_METHODS(LedgerLine);

private:

    double m_len = 0.0;
    Spatium m_lengthOffsetLeft = Spatium(0.0);
    Spatium m_lengthOffsetRight = Spatium(0.0);
    bool m_vertical = false;
};
} // namespace mu::engraving
#endif
