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
#include "stem.h"

#include <cmath>

#include "beam.h"
#include "chord.h"
#include "hook.h"
#include "note.h"

#include "tremolosinglechord.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

static const ElementStyle stemStyle {
    { Sid::stemWidth, Pid::LINE_WIDTH }
};

//---------------------------------------------------------
//   noteClusterSpreadSp
//    Vertical distance (sp) between a chord's top and bottom noteheads.
//    The stem attaches to the outermost note (farthest from its tip);
//    subtracting this spread reports the stem length from the notehead
//    nearest the tip instead. Zero for a single-note chord.
//---------------------------------------------------------

static double noteClusterSpreadSp(const Chord* chord)
{
    if (!chord) {
        return 0.0;
    }
    const Note* upNote = chord->upNote();
    const Note* downNote = chord->downNote();
    if (!upNote || !downNote || upNote == downNote || chord->spatium() <= 0.0) {
        return 0.0;
    }
    return std::abs(downNote->pos().y() - upNote->pos().y()) / chord->spatium();
}

Stem::Stem(Chord* parent)
    : EngravingItem(ElementType::STEM, parent)
{
    initElementStyle(&stemStyle);
    resetProperty(Pid::USER_LEN);
}

EngravingItem* Stem::elementBase() const
{
    return parentItem();
}

staff_idx_t Stem::vStaffIdx() const
{
    return staffIdx() + chord()->staffMove();
}

bool Stem::up() const
{
    return chord() ? chord()->up() : true;
}

void Stem::setBaseLength(Spatium baseLength)
{
    m_baseLength = Spatium(std::abs(baseLength.val()));
}

double Stem::lineWidthMag() const
{
    return absoluteFromSpatium(m_lineWidth) * chord()->intrinsicMag();
}

//! In chord coordinates
PointF Stem::flagPosition() const
{
    return pos() + PointF(ldata()->bbox().left(), up() ? -length() : length());
}

std::vector<PointF> Stem::gripsPositions(const EditData&) const
{
    return { pagePos() + ldata()->line.p2() };
}

void Stem::startEdit(EditData& ed)
{
    EngravingItem::startEdit(ed);
    ElementEditDataPtr eed = ed.getData(this);
    eed->pushProperty(Pid::USER_LEN);
}

void Stem::startEditDrag(EditData& ed)
{
    EngravingItem::startEditDrag(ed);
    ElementEditDataPtr eed = ed.getData(this);
    eed->pushProperty(Pid::USER_LEN);
}

void Stem::editDrag(EditData& ed)
{
    double yDelta = up() ? -ed.delta.y() : ed.delta.y();
    m_userLength += Spatium::fromMM(yDelta, spatium());
    Chord* c = chord();
    if (c->hook()) {
        c->hook()->move(PointF(0.0, ed.delta.y()));
    }
    triggerLayout();
}

void Stem::reset()
{
    undoChangeProperty(Pid::USER_LEN, Spatium(0.0));
    EngravingItem::reset();
}

bool Stem::acceptDrop(EditData& data) const
{
    const EngravingItem* e = data.dropElement;
    switch (e->type()) {
    case ElementType::TREMOLO_SINGLECHORD:
        return item_cast<const TremoloSingleChord*>(e)->tremoloType() <= TremoloType::R64;
    default:
        break;
    }

    return false;
}

EngravingItem* Stem::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    Chord* ch  = chord();

    switch (e->type()) {
    case ElementType::TREMOLO_SINGLECHORD:
        item_cast<TremoloSingleChord*>(e)->setParent(ch);
        undoAddElement(e);
        return e;
    default:
        delete e;
        break;
    }
    return 0;
}

PropertyValue Stem::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LINE_WIDTH:
        return lineWidth();
    case Pid::USER_LEN:
        return userLength();
    case Pid::STEM_LENGTH:
        // Actual drawn length: the layout-computed base (which already reaches the beam,
        // when there is one) plus the user adjustment. For a chord it is measured from the
        // notehead nearest the stem tip, so the note cluster's spread is removed.
        return Spatium((baseLength() + userLength()).val() - noteClusterSpreadSp(chord()));
    case Pid::STEM_DIRECTION:
        return PropertyValue::fromValue<DirectionV>(chord()->stemDirection());
    case Pid::APPEARANCE_LINKED_TO_MASTER:
        return chord() ? chord()->isPropertyLinkedToMaster(Pid::STEM_DIRECTION) : true;
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

bool Stem::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::LINE_WIDTH:
        setLineWidth(v.value<Spatium>());
        break;
    case Pid::USER_LEN:
        setUserLength(v.value<Spatium>());
        break;
    case Pid::STEM_DIRECTION:
        chord()->setStemDirection(v.value<DirectionV>());
        break;
    case Pid::APPEARANCE_LINKED_TO_MASTER:
        if (chord() && v.toBool() == true) {
            chord()->relinkPropertyToMaster(Pid::STEM_DIRECTION);
            break;
        }
    // fall through
    default:
        return EngravingItem::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

PropertyValue Stem::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::USER_LEN:
        return 0.0;
    case Pid::STEM_LENGTH: {
        // Automatic length is the base with the user adjustment removed (still measured from
        // the notehead nearest the tip). For a beamed stem whose beam was positioned by hand
        // the automatic length is not recoverable here, so report no value: that marks the
        // inspector field as modified, enabling its reset.
        const Beam* beam = chord() ? chord()->beam() : nullptr;
        if (beam && beam->userModified()) {
            return PropertyValue();
        }
        return Spatium(baseLength().val() - noteClusterSpreadSp(chord()));
    }
    case Pid::STEM_DIRECTION:
        return PropertyValue::fromValue<DirectionV>(DirectionV::AUTO);
    default:
        return EngravingItem::propertyDefault(id);
    }
}
