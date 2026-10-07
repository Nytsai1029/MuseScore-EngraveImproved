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
#include "score.h"
#include "system.h"
#include "undo.h"

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
//   length offsets
//---------------------------------------------------------

bool LedgerLine::storesOffsetsOnChord() const
{
    return m_line != NO_LINE && explicitParent() && explicitParent()->isChord();
}

Spatium LedgerLine::ledgerLineLengthOffsetLeft() const
{
    return storesOffsetsOnChord() ? chord()->ledgerLineOffsets(m_line).left : m_legacyOffsetLeft;
}

void LedgerLine::setLedgerLineLengthOffsetLeft(Spatium v)
{
    if (!storesOffsetsOnChord()) {
        m_legacyOffsetLeft = v;
        return;
    }

    LedgerLineOffsets offsets = chord()->ledgerLineOffsets(m_line);
    offsets.left = v;
    chord()->setLedgerLineOffsets(m_line, offsets);
}

Spatium LedgerLine::ledgerLineLengthOffsetRight() const
{
    return storesOffsetsOnChord() ? chord()->ledgerLineOffsets(m_line).right : m_legacyOffsetRight;
}

void LedgerLine::setLedgerLineLengthOffsetRight(Spatium v)
{
    if (!storesOffsetsOnChord()) {
        m_legacyOffsetRight = v;
        return;
    }

    LedgerLineOffsets offsets = chord()->ledgerLineOffsets(m_line);
    offsets.right = v;
    chord()->setLedgerLineOffsets(m_line, offsets);
}

//---------------------------------------------------------
//   moveLegacyOffsetsToChord
//    Scores saved before the offsets were kept on the chord stored them on the n-th ledger line.
//    Layout calls this once it has told the line which staff line it is on.
//---------------------------------------------------------

void LedgerLine::moveLegacyOffsetsToChord()
{
    if (!storesOffsetsOnChord() || (m_legacyOffsetLeft.isZero() && m_legacyOffsetRight.isZero())) {
        return;
    }

    chord()->setLedgerLineOffsets(m_line, { m_legacyOffsetLeft, m_legacyOffsetRight });
    m_legacyOffsetLeft = Spatium(0.0);
    m_legacyOffsetRight = Spatium(0.0);
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
//   undoChangeProperty
//    Layout creates and deletes ledger lines freely, so nothing on the undo stack may point at one.
//    The length offsets go through a command on the chord; layout sets everything else about a
//    ledger line anew each time, so other property changes are not recorded.
//---------------------------------------------------------

void LedgerLine::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags)
{
    if (id != Pid::LEDGER_LINE_LENGTH_OFFSET_LEFT && id != Pid::LEDGER_LINE_LENGTH_OFFSET_RIGHT) {
        return;
    }
    if (!storesOffsetsOnChord()) {
        return;
    }

    LedgerLineOffsets offsets = chord()->ledgerLineOffsets(m_line);
    if (id == Pid::LEDGER_LINE_LENGTH_OFFSET_LEFT) {
        offsets.left = v.value<Spatium>();
    } else {
        offsets.right = v.value<Spatium>();
    }
    chord()->undoChangeLedgerLineOffsets(m_line, offsets);
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void LedgerLine::startEditDrag(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    if (!eed) {
        eed = std::make_shared<ElementEditData>();
        eed->e = this;
        ed.addData(eed);
    }

    eed->propertyData.clear();
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

    // same scale as layout applies to the offsets, so that the handle follows the pointer on small chords too
    const Spatium deltaSp(ed.delta.x() / (spatium() * chord()->mag()));
    if (ed.curGrip == Grip::START) {
        setLedgerLineLengthOffsetLeft(ledgerLineLengthOffsetLeft() - deltaSp);
    } else {
        setLedgerLineLengthOffsetRight(ledgerLineLengthOffsetRight() + deltaSp);
    }

    triggerLayout();
}

//---------------------------------------------------------
//   endEditDrag
//    Not the base implementation: that one records the change against this ledger line (see undoChangeProperty)
//---------------------------------------------------------

void LedgerLine::endEditDrag(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    if (eed && storesOffsetsOnChord()) {
        const LedgerLineOffsets after = chord()->ledgerLineOffsets(m_line);
        LedgerLineOffsets before = after;
        for (const PropertyData& pd : eed->propertyData) {
            if (pd.id == Pid::LEDGER_LINE_LENGTH_OFFSET_LEFT) {
                before.left = pd.data.value<Spatium>();
            } else if (pd.id == Pid::LEDGER_LINE_LENGTH_OFFSET_RIGHT) {
                before.right = pd.data.value<Spatium>();
            }
        }
        eed->propertyData.clear();

        if (before != after) {
            score()->undoStack()->pushWithoutPerforming(new ChangeLedgerLineOffsets(chord(), m_line, before));
        }
    }

    score()->hideAnchors();
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
