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

#include "ledgerline.h"

#include "chord.h"
#include "measure.h"
#include "system.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {

//---------------------------------------------------------
//   LedgerLine
//---------------------------------------------------------

LedgerLine::LedgerLine(EngravingItem* s)
    : EngravingItem(ElementType::LEDGER_LINE, s)
{
    setSelectable(true);
    setGenerated(true);
    m_len = 0.;
}

LedgerLine::~LedgerLine()
{
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF LedgerLine::pagePos() const
{
    System* system = chord()->measure()->system();
    double yp = y() + system->staff(staffIdx())->y() + system->y();
    return PointF(pageX(), yp);
}

//---------------------------------------------------------
//   measureXPos
//---------------------------------------------------------

double LedgerLine::measureXPos() const
{
    double xp = x();                     // chord relative
    xp += chord()->x();                  // segment relative
    xp += chord()->segment()->x();       // measure relative
    return xp;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue LedgerLine::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LEDGER_LINE_LENGTH_OFFSET_LEFT:
        return ledgerLineLengthOffsetLeft();
    case Pid::LEDGER_LINE_LENGTH_OFFSET_RIGHT:
        return ledgerLineLengthOffsetRight();
    default:
        break;
    }

    return EngravingItem::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool LedgerLine::setProperty(Pid propertyId, const PropertyValue& value)
{
    switch (propertyId) {
    case Pid::LEDGER_LINE_LENGTH_OFFSET_LEFT:
        setLedgerLineLengthOffsetLeft(value.value<Spatium>());
        break;
    case Pid::LEDGER_LINE_LENGTH_OFFSET_RIGHT:
        setLedgerLineLengthOffsetRight(value.value<Spatium>());
        break;
    default:
        return EngravingItem::setProperty(propertyId, value);
    }

    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue LedgerLine::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LEDGER_LINE_LENGTH_OFFSET_LEFT:
    case Pid::LEDGER_LINE_LENGTH_OFFSET_RIGHT:
        return Spatium(0.0);
    default:
        break;
    }

    return EngravingItem::propertyDefault(propertyId);
}

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void LedgerLine::startEdit(EditData& ed)
{
    EngravingItem::startEdit(ed);
    ElementEditDataPtr eed = ed.getData(this);
    if (!eed) {
        return;
    }

    eed->pushProperty(Pid::LEDGER_LINE_LENGTH_OFFSET_LEFT);
    eed->pushProperty(Pid::LEDGER_LINE_LENGTH_OFFSET_RIGHT);
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void LedgerLine::startEditDrag(EditData& ed)
{
    EngravingItem::startEditDrag(ed);
    ElementEditDataPtr eed = ed.getData(this);
    if (!eed) {
        return;
    }

    eed->pushProperty(Pid::LEDGER_LINE_LENGTH_OFFSET_LEFT);
    eed->pushProperty(Pid::LEDGER_LINE_LENGTH_OFFSET_RIGHT);
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void LedgerLine::editDrag(EditData& ed)
{
    if (ed.curGrip != Grip::START && ed.curGrip != Grip::END) {
        return;
    }

    const Spatium deltaSp(ed.delta.x() / spatium());
    if (ed.curGrip == Grip::START) {
        setLedgerLineLengthOffsetLeft(ledgerLineLengthOffsetLeft() - deltaSp);
    } else {
        setLedgerLineLengthOffsetRight(ledgerLineLengthOffsetRight() + deltaSp);
    }

    triggerLayout();
}

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void LedgerLine::endEditDrag(EditData& ed)
{
    EngravingItem::endEditDrag(ed);
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> LedgerLine::gripsPositions(const EditData&) const
{
    if (!chord()) {
        return {};
    }

    const PointF startPos = pagePos();
    const PointF endPos = vertical() ? startPos + PointF(0.0, len()) : startPos + PointF(len(), 0.0);
    return { startPos, endPos };
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void LedgerLine::spatiumChanged(double oldValue, double newValue)
{
    m_len   = (m_len / oldValue) * newValue;
}
}
