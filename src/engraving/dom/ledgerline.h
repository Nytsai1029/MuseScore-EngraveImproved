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

#include <limits>

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

    // The staff line this ledger line sits on, set by layout. The length offsets are kept on the chord under this key.
    static constexpr int NO_LINE = std::numeric_limits<int>::min();
    int line() const { return m_line; }
    void setLine(int v) { m_line = v; }

    Spatium ledgerLineLengthOffsetLeft() const;
    void setLedgerLineLengthOffsetLeft(Spatium v);
    Spatium ledgerLineLengthOffsetRight() const;
    void setLedgerLineLengthOffsetRight(Spatium v);
    void moveLegacyOffsetsToChord();

    void setVertical(bool v) { m_vertical = v; }
    bool vertical() const { return m_vertical; }

    double measureXPos() const;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& value) override;
    PropertyValue propertyDefault(Pid propertyId) const override;

    void undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps) override;
    using EngravingItem::undoChangeProperty;

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

    bool storesOffsetsOnChord() const;

    double m_len = 0.0;
    int m_line = NO_LINE;
    // Offsets read from a score that still stored them per ledger line, held until layout knows the staff line
    Spatium m_legacyOffsetLeft = Spatium(0.0);
    Spatium m_legacyOffsetRight = Spatium(0.0);
    bool m_vertical = false;
};
} // namespace mu::engraving
#endif
