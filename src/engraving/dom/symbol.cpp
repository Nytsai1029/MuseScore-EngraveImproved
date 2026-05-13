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

#include "symbol.h"

#include <algorithm>
#include <cmath>

#include "score.h"

#include "draw/fontmetrics.h"
#include "iengravingfont.h"

#include "mscore.h"
#include "types/symnames.h"

#include "image.h"
#include "page.h"
#include "staff.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

namespace mu::engraving {
static constexpr double KEYBOARD_HAND_BRACKET_DEFAULT_SHORT_SIDE = 1.564;
static constexpr double KEYBOARD_HAND_BRACKET_DEFAULT_LONG_SIDE = 4.064;
static constexpr double KEYBOARD_HAND_BRACKET_DEFAULT_LINE_WIDTH = 0.13;
static constexpr double KEYBOARD_HAND_BRACKET_MIN_SIDE = 0.1;
static constexpr double DEGREES_TO_RADIANS = 3.14159265358979323846 / 180.0;

static const ElementStyle symbolStyle {
    { Sid::keyboardHandBracketShortSide, Pid::SYMBOL_SHORT_SIDE_LENGTH },
    { Sid::keyboardHandBracketLongSide,  Pid::SYMBOL_LONG_SIDE_LENGTH },
    { Sid::keyboardHandBracketLineWidth, Pid::LINE_WIDTH },
};

static PointF rotateKeyboardHandBracketPoint(const PointF& point, double angle)
{
    if (angle == 0.0) {
        return point;
    }

    const double radians = angle * DEGREES_TO_RADIANS;
    const double sinAngle = std::sin(radians);
    const double cosAngle = std::cos(radians);
    return PointF(point.x() * cosAngle - point.y() * sinAngle, point.x() * sinAngle + point.y() * cosAngle);
}

static PointF unrotateKeyboardHandBracketDelta(const PointF& delta, double angle)
{
    return rotateKeyboardHandBracketPoint(delta, -angle);
}

//---------------------------------------------------------
//   Symbol
//---------------------------------------------------------

Symbol::Symbol(const ElementType& type, EngravingItem* parent, ElementFlags f)
    : BSymbol(type, parent, f)
{
    m_sym = SymId::accidentalSharp;          // arbitrary valid default
    m_keyboardHandBracketShortSide = Spatium(KEYBOARD_HAND_BRACKET_DEFAULT_SHORT_SIDE);
    m_keyboardHandBracketLongSide = Spatium(KEYBOARD_HAND_BRACKET_DEFAULT_LONG_SIDE);
    m_keyboardHandBracketLineWidth = Spatium(KEYBOARD_HAND_BRACKET_DEFAULT_LINE_WIDTH);
    initElementStyle(&symbolStyle);
}

Symbol::Symbol(EngravingItem* parent, ElementFlags f)
    : Symbol(ElementType::SYMBOL, parent, f)
{
}

Symbol::Symbol(const Symbol& s)
    : BSymbol(s)
{
    m_sym         = s.m_sym;
    m_scoreFont   = s.m_scoreFont;
    m_symbolsSize = s.m_symbolsSize;
    m_symAngle    = s.m_symAngle;
    m_keyboardHandBracketShortSide = s.m_keyboardHandBracketShortSide;
    m_keyboardHandBracketLongSide = s.m_keyboardHandBracketLongSide;
    m_keyboardHandBracketLineWidth = s.m_keyboardHandBracketLineWidth;
}

//---------------------------------------------------------
//   symName
//---------------------------------------------------------

AsciiStringView Symbol::symName() const
{
    return SymNames::nameForSymId(m_sym);
}

//---------------------------------------------------------
//   isKeyboardHandBracketSymbol
//---------------------------------------------------------

bool Symbol::isKeyboardHandBracketSymbol(SymId sym)
{
    return sym == SymId::keyboardPlayWithLH
           || sym == SymId::keyboardPlayWithLHEnd
           || sym == SymId::keyboardPlayWithRH
           || sym == SymId::keyboardPlayWithRHEnd;
}

//---------------------------------------------------------
//   keyboardHandBracketLineWidth
//---------------------------------------------------------

double Symbol::keyboardHandBracketLineWidth() const
{
    return std::max(0.0, m_keyboardHandBracketLineWidth.val() * SPATIUM20 * magS() * m_symbolsSize);
}

//---------------------------------------------------------
//   keyboardHandBracketBBox
//---------------------------------------------------------

RectF Symbol::keyboardHandBracketBBox() const
{
    const double scale = SPATIUM20 * magS() * m_symbolsSize;
    const double width = std::max(0.0, m_keyboardHandBracketShortSide.val() * scale);
    const double height = std::max(0.0, m_keyboardHandBracketLongSide.val() * scale);
    return RectF(0.0, -height, width, height);
}

//---------------------------------------------------------
//   keyboardHandBracketLines
//---------------------------------------------------------

std::array<LineF, 2> Symbol::keyboardHandBracketLines() const
{
    const RectF box = keyboardHandBracketBBox();
    const double lw = keyboardHandBracketLineWidth();

    const double left = std::min(lw * 0.5, box.width() * 0.5);
    const double right = std::max(left, box.width() - lw * 0.5);
    const double top = std::min(-lw * 0.5, box.top() + lw * 0.5);
    const double bottom = std::max(top, -lw * 0.5);

    const bool rightSide = m_sym == SymId::keyboardPlayWithLHEnd || m_sym == SymId::keyboardPlayWithRHEnd;
    const bool topSide = m_sym == SymId::keyboardPlayWithRH || m_sym == SymId::keyboardPlayWithRHEnd;

    const double x = rightSide ? right : left;
    const double y = topSide ? top : bottom;

    return {
        LineF(x, top, x, bottom),
        LineF(left, y, right, y)
    };
}

//---------------------------------------------------------
//   keyboardHandBracketPath
//---------------------------------------------------------

PainterPath Symbol::keyboardHandBracketPath() const
{
    const std::array<LineF, 2> lines = keyboardHandBracketLines();
    const LineF& vertical = lines[0];
    const LineF& horizontal = lines[1];

    const bool rightSide = m_sym == SymId::keyboardPlayWithLHEnd || m_sym == SymId::keyboardPlayWithRHEnd;
    const bool topSide = m_sym == SymId::keyboardPlayWithRH || m_sym == SymId::keyboardPlayWithRHEnd;

    const PointF verticalStart(vertical.x1(), topSide ? vertical.y2() : vertical.y1());
    const PointF corner(vertical.x1(), horizontal.y1());
    const PointF horizontalEnd = rightSide ? horizontal.p1() : horizontal.p2();

    PainterPath path;
    path.moveTo(verticalStart);
    path.lineTo(corner);
    path.lineTo(horizontalEnd);
    return path;
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> Symbol::gripsPositions(const EditData&) const
{
    if (!isKeyboardHandBracketSymbol()) {
        return {};
    }

    const std::array<LineF, 2> lines = keyboardHandBracketLines();
    const LineF& vertical = lines[0];
    const LineF& horizontal = lines[1];

    const bool rightSide = m_sym == SymId::keyboardPlayWithLHEnd || m_sym == SymId::keyboardPlayWithRHEnd;
    const bool topSide = m_sym == SymId::keyboardPlayWithRH || m_sym == SymId::keyboardPlayWithRHEnd;

    const PointF verticalEnd(vertical.x1(), topSide ? vertical.y2() : vertical.y1());
    const PointF horizontalEnd = rightSide ? horizontal.p1() : horizontal.p2();
    const PointF origin = pagePos();

    return {
        origin + rotateKeyboardHandBracketPoint(verticalEnd, m_symAngle),
        origin + rotateKeyboardHandBracketPoint(horizontalEnd, m_symAngle)
    };
}

//---------------------------------------------------------
//   gripAnchorLines
//---------------------------------------------------------

std::vector<LineF> Symbol::gripAnchorLines(Grip grip) const
{
    if (!isKeyboardHandBracketSymbol() || (grip != Grip::START && grip != Grip::END)) {
        return {};
    }

    const std::array<LineF, 2> lines = keyboardHandBracketLines();
    const LineF& vertical = lines[0];
    const LineF& horizontal = lines[1];
    const PointF localCorner(vertical.x1(), horizontal.y1());

    const Page* page = toPage(findAncestor(ElementType::PAGE));
    const PointF pageOffset = page ? page->pos() : PointF();
    const PointF corner = pagePos() + rotateKeyboardHandBracketPoint(localCorner, m_symAngle) + pageOffset;
    const std::vector<PointF> grips = gripsPositions();
    const int gripIndex = static_cast<int>(grip);
    if (gripIndex < 0 || gripIndex >= static_cast<int>(grips.size())) {
        return {};
    }

    return { LineF(corner, grips[gripIndex] + pageOffset) };
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void Symbol::startEditDrag(EditData& ed)
{
    if (!isKeyboardHandBracketSymbol()) {
        EngravingItem::startEditDrag(ed);
        return;
    }

    ElementEditDataPtr eed = ed.getData(this);
    if (!eed) {
        eed = std::make_shared<ElementEditData>();
        eed->e = this;
        ed.addData(eed);
    }

    eed->pushProperty(Pid::SYMBOL_SHORT_SIDE_LENGTH);
    eed->pushProperty(Pid::SYMBOL_LONG_SIDE_LENGTH);
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Symbol::editDrag(EditData& ed)
{
    if (!isKeyboardHandBracketSymbol()) {
        EngravingItem::editDrag(ed);
        return;
    }

    if (ed.curGrip != Grip::START && ed.curGrip != Grip::END) {
        return;
    }

    const PointF localDelta = unrotateKeyboardHandBracketDelta(ed.delta, m_symAngle);
    const bool rightSide = m_sym == SymId::keyboardPlayWithLHEnd || m_sym == SymId::keyboardPlayWithRHEnd;
    const bool topSide = m_sym == SymId::keyboardPlayWithRH || m_sym == SymId::keyboardPlayWithRHEnd;
    const double scale = std::max(0.001, SPATIUM20 * magS() * m_symbolsSize);

    if (ed.curGrip == Grip::START) {
        const double delta = topSide ? localDelta.y() : -localDelta.y();
        m_keyboardHandBracketLongSide = Spatium(std::max(KEYBOARD_HAND_BRACKET_MIN_SIDE, m_keyboardHandBracketLongSide.val() + delta / scale));
    } else {
        const double delta = rightSide ? -localDelta.x() : localDelta.x();
        m_keyboardHandBracketShortSide = Spatium(std::max(KEYBOARD_HAND_BRACKET_MIN_SIDE,
                                                          m_keyboardHandBracketShortSide.val() + delta / scale));
    }

    triggerLayout();
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Symbol::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), SymNames::userNameForSymId(m_sym).translated());
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString Symbol::subtypeUserName() const
{
    return SymNames::userNameForSymId(m_sym);
}

//---------------------------------------------------------
//   Symbol::getProperty
//---------------------------------------------------------
PropertyValue Symbol::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SYMBOL:
        return PropertyValue::fromValue(m_sym);
    case Pid::SCORE_FONT:
        if (m_scoreFont) {
            return PropertyValue::fromValue(String::fromStdString(m_scoreFont->name()));
        } else {
            return PropertyValue::fromValue(String());
        }
    case Pid::SYMBOLS_SIZE:
        return PropertyValue::fromValue(m_symbolsSize);
    case Pid::SYMBOL_ANGLE:
        return PropertyValue::fromValue(m_symAngle);
    case Pid::SYMBOL_SHORT_SIDE_LENGTH:
        return isKeyboardHandBracketSymbol() ? PropertyValue::fromValue(m_keyboardHandBracketShortSide) : PropertyValue();
    case Pid::SYMBOL_LONG_SIDE_LENGTH:
        return isKeyboardHandBracketSymbol() ? PropertyValue::fromValue(m_keyboardHandBracketLongSide) : PropertyValue();
    case Pid::LINE_WIDTH:
        return isKeyboardHandBracketSymbol() ? PropertyValue::fromValue(m_keyboardHandBracketLineWidth) : PropertyValue();
    default:
        break;
    }
    return BSymbol::getProperty(propertyId);
}

//---------------------------------------------------------
//   Symbol::setProperty
//---------------------------------------------------------

bool Symbol::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SYMBOL:
        m_sym = v.value<SymId>();
        break;
    case Pid::SCORE_FONT:
        m_scoreFont = score()->engravingFonts()->fontByName(v.value<String>().toStdString());
        break;
    case Pid::SYMBOLS_SIZE:
        m_symbolsSize = v.toDouble();
        break;
    case Pid::SYMBOL_ANGLE:
        m_symAngle = v.toDouble();
        break;
    case Pid::SYMBOL_SHORT_SIDE_LENGTH:
        m_keyboardHandBracketShortSide = Spatium(std::max(0.0, v.value<Spatium>().val()));
        break;
    case Pid::SYMBOL_LONG_SIDE_LENGTH:
        m_keyboardHandBracketLongSide = Spatium(std::max(0.0, v.value<Spatium>().val()));
        break;
    case Pid::LINE_WIDTH:
        m_keyboardHandBracketLineWidth = Spatium(std::max(0.0, v.value<Spatium>().val()));
        break;
    default:
        break;
    }
    triggerLayout();
    return BSymbol::setProperty(propertyId, v);
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Symbol::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SYMBOL:
        return SymId::accidentalSharp;
    case Pid::SYMBOL_ANGLE:
        return 0.0;
    case Pid::SYMBOLS_SIZE:
        return 1.0;
    case Pid::SYMBOL_SHORT_SIDE_LENGTH: {
        if (!isKeyboardHandBracketSymbol()) {
            return PropertyValue();
        }
        PropertyValue v = EngravingObject::propertyDefault(propertyId);
        return v.isValid() ? v : PropertyValue::fromValue(Spatium(KEYBOARD_HAND_BRACKET_DEFAULT_SHORT_SIDE));
    }
    case Pid::SYMBOL_LONG_SIDE_LENGTH: {
        if (!isKeyboardHandBracketSymbol()) {
            return PropertyValue();
        }
        PropertyValue v = EngravingObject::propertyDefault(propertyId);
        return v.isValid() ? v : PropertyValue::fromValue(Spatium(KEYBOARD_HAND_BRACKET_DEFAULT_LONG_SIDE));
    }
    case Pid::LINE_WIDTH: {
        if (!isKeyboardHandBracketSymbol()) {
            return PropertyValue();
        }
        PropertyValue v = EngravingObject::propertyDefault(propertyId);
        return v.isValid() ? v : PropertyValue::fromValue(Spatium(KEYBOARD_HAND_BRACKET_DEFAULT_LINE_WIDTH));
    }
    case Pid::SCORE_FONT:
        if (m_scoreFont) {
            return style().styleSt(Sid::musicalSymbolFont);
        } else {
            return String();
        }
    default:
        break;
    }
    return EngravingItem::propertyDefault(propertyId);
}

//---------------------------------------------------------
//   FSymbol
//---------------------------------------------------------

FSymbol::FSymbol(EngravingItem* parent)
    : BSymbol(ElementType::FSYMBOL, parent)
{
    m_code = 0;
    m_font.setNoFontMerging(true);
}

FSymbol::FSymbol(const FSymbol& s)
    : BSymbol(s)
{
    m_font = s.m_font;
    m_code = s.m_code;
}

//---------------------------------------------------------
//   toString
// FSymbol is a single code point but code points above 2^16 cannot be
// represented by a single Char, hence we return a String instead. Char
// and String use the UTF-16 encoding internally (like QChar, QString).
//---------------------------------------------------------

String FSymbol::toString() const
{
    return String::fromUcs4(m_code);
}

//---------------------------------------------------------
//   accessibleInfo
// Screen readers know how to pronounce the common font symbols so we can
// return just the character itself. Similarly, common characters should
// be rendered correctly by Braille terminals.
//---------------------------------------------------------

String FSymbol::accessibleInfo() const
{
    return toString();
}

//---------------------------------------------------------
//   setFont
//---------------------------------------------------------

void FSymbol::setFont(const Font& f)
{
    m_font = f;
    m_font.setNoFontMerging(true);
}
}
