/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#include "engravingitem.h"

namespace mu::engraving {
class Parenthesis : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Parenthesis)
    M_PROPERTY2(DirectionH, direction, setDirection, DirectionH::LEFT)

public:
    static constexpr double PARENTHESIS_END_WIDTH = 0.1;

    Parenthesis(EngravingItem* parent);
    Parenthesis(const Parenthesis& p);

    Parenthesis* clone() const override { return new Parenthesis(*this); }
    Segment* segment() const { return explicitParent() && explicitParent()->isSegment() ? toSegment(explicitParent()) : nullptr; }

    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    String accessibleInfo() const override;

    // user adjustments (in spatium units) to the top/bottom end of the parenthesis
    double userStartYOffset() const { return m_userStartYOffset; }
    void setUserStartYOffset(double v) { m_userStartYOffset = v; }
    double userEndYOffset() const { return m_userEndYOffset; }
    void setUserEndYOffset(double v) { m_userEndYOffset = v; }

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 2; }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<PointF> gripsPositions(const EditData&) const override;
    void editDrag(EditData&) override;
    void reset() override;

    bool followParentCurColor() const;
    void setFollowParentColor(bool val);

    Color curColor() const override;

    struct LayoutData : public EngravingItem::LayoutData
    {
        ld_field<muse::draw::PainterPath> path = "[Parenthesis] path";
        ld_field<double> startY = { "[Parenthesis] startY", 0.0 };
        ld_field<double> height = { "[Parenthesis] height", 0.0 };
        ld_field<double> midPointThickness = { "[Parenthesis] midPointThickness", 0.0 };
        ld_field<double> endPointThickness = { "[Parenthesis] endPointThickness", PARENTHESIS_END_WIDTH };
        ld_field<double> shoulderWidth = { "[Parenthesis] endPointThickness", 0.0 };
        ld_field<SymId> symId = { "[Parenthesis] symId", SymId::noSym };
    };
    DECLARE_LAYOUTDATA_METHODS(Parenthesis);

private:
    bool m_followParentColor = false;
    double m_userStartYOffset = 0.0;    // in spatium units
    double m_userEndYOffset = 0.0;      // in spatium units
};
} // namespace engraving
