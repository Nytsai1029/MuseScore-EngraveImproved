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

#ifndef MU_ENGRAVING_SYMBOL_H
#define MU_ENGRAVING_SYMBOL_H

#include <array>
#include <memory>
#include <vector>

#include "draw/types/font.h"
#include "draw/types/painterpath.h"

#include "modularity/ioc.h"
#include "../iengravingfontsprovider.h"
#include "../types/dimension.h"

#include "bsymbol.h"

namespace mu::engraving {
class Segment;
class IEngravingFont;

//---------------------------------------------------------
//   @@ Symbol
///    Symbol constructed from builtin symbol.
//
//   @P symbol       string       the SMuFL name of the symbol
//---------------------------------------------------------

class Symbol : public BSymbol
{
    OBJECT_ALLOCATOR(engraving, Symbol)
    DECLARE_CLASSOF(ElementType::SYMBOL)

public:
    Symbol(const ElementType& type, EngravingItem* parent, ElementFlags f = ElementFlag::MOVABLE);
    Symbol(EngravingItem* parent, ElementFlags f = ElementFlag::MOVABLE);
    Symbol(const Symbol&);

    Symbol& operator=(const Symbol&) = delete;

    Symbol* clone() const override { return new Symbol(*this); }

    void setSym(SymId s, const std::shared_ptr<IEngravingFont>& sf = nullptr) { m_sym  = s; m_scoreFont = sf; }
    SymId sym() const { return m_sym; }
    const std::shared_ptr<IEngravingFont>& scoreFont() const { return m_scoreFont; }
    double symbolsSize() const { return m_symbolsSize; }
    double symAngle() const { return m_symAngle; }
    AsciiStringView symName() const;
    static bool isKeyboardHandBracketSymbol(SymId sym);
    bool isKeyboardHandBracketSymbol() const { return isKeyboardHandBracketSymbol(m_sym); }
    static bool isTimeSigBracketSymbol(SymId sym);
    bool isTimeSigBracketSymbol() const { return isTimeSigBracketSymbol(m_sym); }
    bool isEditableBracketSymbol() const { return isKeyboardHandBracketSymbol() || isTimeSigBracketSymbol(); }
    Spatium keyboardHandBracketShortSide() const { return m_keyboardHandBracketShortSide; }
    Spatium keyboardHandBracketLongSide() const { return m_keyboardHandBracketLongSide; }
    Spatium keyboardHandBracketLineWidthSpatium() const { return m_keyboardHandBracketLineWidth; }
    double keyboardHandBracketLineWidth() const;
    RectF keyboardHandBracketBBox() const;
    std::array<LineF, 2> keyboardHandBracketLines() const;
    PainterPath keyboardHandBracketPath() const;
    Spatium timeSigBracketHeight() const { return m_timeSigBracketHeight; }
    Spatium timeSigBracketTopHook() const { return m_timeSigBracketTopHook; }
    Spatium timeSigBracketBottomHook() const { return m_timeSigBracketBottomHook; }
    Spatium timeSigBracketLineWidthSpatium() const { return m_timeSigBracketLineWidth; }
    double timeSigBracketLineWidth() const;
    RectF timeSigBracketBBox() const;
    std::array<LineF, 3> timeSigBracketLines() const;
    PainterPath timeSigBracketPath() const;

    String accessibleInfo() const override;

    int subtype() const override { return int(m_sym); }
    muse::TranslatableString subtypeUserName() const override;

    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    int gripsCount() const override { return isKeyboardHandBracketSymbol() ? 2 : isTimeSigBracketSymbol() ? 3 : 0; }
    Grip initialEditModeGrip() const override { return isEditableBracketSymbol() ? Grip::END : Grip::NO_GRIP; }
    Grip defaultGrip() const override { return initialEditModeGrip(); }
    std::vector<PointF> gripsPositions(const EditData& = EditData()) const override;
    std::vector<LineF> gripAnchorLines(Grip) const override;
    void startEditDrag(EditData&) override;
    void editDrag(EditData&) override;

    double baseLine() const override { return 0.0; }
    virtual Segment* segment() const { return (Segment*)explicitParent(); }

protected:
    SymId m_sym = SymId::noSym;
    std::shared_ptr<IEngravingFont> m_scoreFont = nullptr;
    double m_symbolsSize =  1.0;
    double m_symAngle = 0.0;
    Spatium m_keyboardHandBracketShortSide;
    Spatium m_keyboardHandBracketLongSide;
    Spatium m_keyboardHandBracketLineWidth;
    Spatium m_timeSigBracketHeight;
    Spatium m_timeSigBracketTopHook;
    Spatium m_timeSigBracketBottomHook;
    Spatium m_timeSigBracketLineWidth;
};

//---------------------------------------------------------
//   @@ FSymbol
///    Symbol constructed from a font glyph (i.e. a text character or emoji).
//---------------------------------------------------------

class FSymbol final : public BSymbol
{
    OBJECT_ALLOCATOR(engraving, FSymbol)
    DECLARE_CLASSOF(ElementType::FSYMBOL)

public:
    FSymbol(EngravingItem* parent);
    FSymbol(const FSymbol&);

    FSymbol* clone() const override { return new FSymbol(*this); }

    String toString() const;
    String accessibleInfo() const override;

    double baseLine() const override { return 0.0; }
    Segment* segment() const { return (Segment*)explicitParent(); }
    const muse::draw::Font& font() const { return m_font; }
    char32_t code() const { return m_code; }
    void setFont(const muse::draw::Font& f);
    void setCode(char32_t val) { m_code = val; }

private:
    muse::draw::Font m_font;
    char32_t m_code = 0; // character code point (Unicode)
};
} // namespace mu::engraving
#endif
