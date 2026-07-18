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

#include "parenthesis.h"

#include "segment.h"
#include "types/typesconv.h"

using namespace mu::engraving;

Parenthesis::Parenthesis(EngravingItem* parent)
    : EngravingItem(ElementType::PARENTHESIS, parent, ElementFlag::MOVABLE)
{
}

Parenthesis::Parenthesis(const Parenthesis& p)
    : EngravingItem(p)
{
    _direction = p._direction;
    m_userStartYOffset = p.m_userStartYOffset;
    m_userEndYOffset = p.m_userEndYOffset;
}

PropertyValue Parenthesis::getProperty(Pid pid) const
{
    switch (pid) {
    case Pid::HORIZONTAL_DIRECTION:
        return direction();
    case Pid::PAREN_START_Y_OFFSET:
        return Spatium(m_userStartYOffset);
    case Pid::PAREN_END_Y_OFFSET:
        return Spatium(m_userEndYOffset);
    default:
        return EngravingItem::getProperty(pid);
    }
}

bool Parenthesis::setProperty(Pid pid, const PropertyValue& v)
{
    switch (pid) {
    case Pid::HORIZONTAL_DIRECTION:
        setDirection(v.value<DirectionH>());
        break;
    case Pid::PAREN_START_Y_OFFSET:
        m_userStartYOffset = v.value<Spatium>().val();
        break;
    case Pid::PAREN_END_Y_OFFSET:
        m_userEndYOffset = v.value<Spatium>().val();
        break;
    default:
        return EngravingItem::setProperty(pid, v);
    }

    triggerLayout();
    return true;
}

PropertyValue Parenthesis::propertyDefault(Pid pid) const
{
    switch (pid) {
    case Pid::HORIZONTAL_DIRECTION:
        return DirectionH::LEFT;
    case Pid::PAREN_START_Y_OFFSET:
    case Pid::PAREN_END_Y_OFFSET:
        return Spatium(0.0);
    default:
        return EngravingItem::propertyDefault(pid);
    }
}

//---------------------------------------------------------
//   edit support: the two grips move the top/bottom end of
//   the parenthesis; dragging the body moves the whole one
//---------------------------------------------------------

std::vector<PointF> Parenthesis::gripsPositions(const EditData&) const
{
    const double sp = spatium();
    const double height = ldata()->height + (m_userEndYOffset - m_userStartYOffset) * sp;
    const PointF top = pagePos();

    return { top, top + PointF(0.0, height) };
}

void Parenthesis::editDrag(EditData& ed)
{
    const double dySp = ed.delta.y() / spatium();
    if (ed.curGrip == Grip::START) {
        undoChangeProperty(Pid::PAREN_START_Y_OFFSET, Spatium(m_userStartYOffset + dySp));
    } else if (ed.curGrip == Grip::END) {
        undoChangeProperty(Pid::PAREN_END_Y_OFFSET, Spatium(m_userEndYOffset + dySp));
    }
    triggerLayout();
}

void Parenthesis::reset()
{
    undoResetProperty(Pid::PAREN_START_Y_OFFSET);
    undoResetProperty(Pid::PAREN_END_Y_OFFSET);
    EngravingItem::reset();
}

String Parenthesis::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), TConv::translatedUserName(direction()));
}

bool Parenthesis::followParentCurColor() const
{
    return m_followParentColor;
}

void Parenthesis::setFollowParentColor(bool val)
{
    m_followParentColor = val;
}

Color Parenthesis::curColor() const
{
    if (m_followParentColor) {
        return parentItem()->curColor();
    }

    return EngravingItem::curColor(getProperty(Pid::VISIBLE).toBool(),
                                   getProperty(Pid::COLOR).value<Color>());
}
