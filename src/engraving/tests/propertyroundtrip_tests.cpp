/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include <gtest/gtest.h>

#include <algorithm>
#include <clocale>
#include <cmath>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "global/io/buffer.h"

#include "compat/mscxcompat.h"
#include "compat/scoreaccess.h"
#include "dom/barline.h"
#include "dom/chord.h"
#include "dom/engravingitem.h"
#include "dom/excerpt.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/mscore.h"
#include "dom/note.h"
#include "dom/ornament.h"
#include "dom/parenthesis.h"
#include "dom/part.h"
#include "dom/property.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/slur.h"
#include "dom/spanner.h"
#include "dom/staff.h"
#include "dom/stafftype.h"
#include "dom/symbol.h"
#include "dom/tie.h"
#include "infrastructure/mscwriter.h"
#include "rw/mscsaver.h"
#include "style/defaultstyle.h"
#include "style/style.h"
#include "types/propertyvalue.h"

#include "utils/scorerw.h"

using namespace mu;
using namespace mu::engraving;

namespace {
//---------------------------------------------------------
//   Values
//---------------------------------------------------------

bool isReal(P_TYPE type)
{
    return type == P_TYPE::REAL || type == P_TYPE::SPATIUM || type == P_TYPE::MILLIMETRE;
}

bool nearlyEqual(double a, double b)
{
    return std::abs(a - b) <= 1e-6 * std::max({ 1.0, std::abs(a), std::abs(b) });
}

//! Whether a value read back is the value that was set. Numbers are written with six significant digits
//! and lengths are stored in staff spaces, so numbers are compared with a relative tolerance of 1e-6.
//! Offsets are in the units of the score, whose staff space is itself written with six digits: \p bUnit
//! is the staff space of the score of \p b in staff spaces of the score of \p a.
bool sameValue(const PropertyValue& a, const PropertyValue& b, double bUnit = 1.0)
{
    if (!a.isValid() || !b.isValid()) {
        return a.isValid() == b.isValid();
    }
    if (isReal(a.type()) && isReal(b.type())) {
        const double unit = a.type() == P_TYPE::MILLIMETRE ? bUnit : 1.0;
        return nearlyEqual(a.toReal(), b.toReal() / unit);
    }
    if (a.type() != b.type()) {
        // an enum or a bool against an int
        const bool aIsInt = a.type() == P_TYPE::INT || a.type() == P_TYPE::BOOL || a.type() == P_TYPE::SIZE_T || a.isEnum();
        const bool bIsInt = b.type() == P_TYPE::INT || b.type() == P_TYPE::BOOL || b.type() == P_TYPE::SIZE_T || b.isEnum();
        return aIsInt && bIsInt && a.toInt() == b.toInt();
    }
    switch (a.type()) {
    case P_TYPE::POINT: {
        const PointF pa = a.value<PointF>();
        const PointF pb = b.value<PointF>() / bUnit;
        return nearlyEqual(pa.x(), pb.x()) && nearlyEqual(pa.y(), pb.y());
    }
    case P_TYPE::SIZE: {
        const SizeF sa = a.value<SizeF>();
        const SizeF sb = b.value<SizeF>();
        return nearlyEqual(sa.width(), sb.width()) && nearlyEqual(sa.height(), sb.height());
    }
    case P_TYPE::SCALE: {
        const ScaleF sa = a.value<ScaleF>();
        const ScaleF sb = b.value<ScaleF>();
        return nearlyEqual(sa.width(), sb.width()) && nearlyEqual(sa.height(), sb.height());
    }
    case P_TYPE::PAIR_REAL: {
        const PairF pa = a.value<PairF>();
        const PairF pb = b.value<PairF>();
        return nearlyEqual(pa.first, pb.first) && nearlyEqual(pa.second, pb.second);
    }
    default:
        return a == b;
    }
}

std::string valueText(const PropertyValue& v)
{
    std::ostringstream out;
    out.precision(9);
    switch (v.type()) {
    case P_TYPE::UNDEFINED:
        out << "(invalid)";
        break;
    case P_TYPE::BOOL:
        out << (v.toBool() ? "true" : "false");
        break;
    case P_TYPE::REAL:
    case P_TYPE::SPATIUM:
    case P_TYPE::MILLIMETRE:
        out << v.toReal();
        break;
    case P_TYPE::STRING:
        out << '"' << v.value<String>().toStdString() << '"';
        break;
    case P_TYPE::POINT:
        out << '(' << v.value<PointF>().x() << ", " << v.value<PointF>().y() << ')';
        break;
    case P_TYPE::SIZE:
        out << '(' << v.value<SizeF>().width() << " x " << v.value<SizeF>().height() << ')';
        break;
    case P_TYPE::SCALE:
        out << '(' << v.value<ScaleF>().width() << " x " << v.value<ScaleF>().height() << ')';
        break;
    case P_TYPE::PAIR_REAL:
        out << '(' << v.value<PairF>().first << ", " << v.value<PairF>().second << ')';
        break;
    case P_TYPE::COLOR:
        out << v.value<Color>().toString();
        break;
    case P_TYPE::ALIGN:
        out << "align(" << int(v.value<Align>().horizontal) << ", " << int(v.value<Align>().vertical) << ')';
        break;
    case P_TYPE::FRACTION:
        out << v.value<Fraction>().toString().toStdString();
        break;
    default:
        if (v.type() == P_TYPE::INT || v.type() == P_TYPE::SIZE_T || v.isEnum()) {
            out << v.toInt();
        } else {
            out << "(value of type " << int(v.type()) << ')';
        }
    }
    return out.str();
}

//! Another number that takes few digits: files hold six significant digits of a number
double otherNumber(double v)
{
    return std::round(v * 4.0) / 4.0 + 0.25;
}

//! Another of the first two values of an enumeration
int otherEnumerator(const PropertyValue& v)
{
    return v.toInt() == 0 ? 1 : 0;
}

Color otherColor(const Color& color)
{
    const Color other(0x20, 0x60, 0xA0, 0xC0);
    return color == other ? Color(0xA0, 0x60, 0x20, 0xFF) : other;
}

Align otherAlign(const Align& align)
{
    return Align(align.horizontal == AlignH::LEFT ? AlignH::HCENTER : AlignH::LEFT,
                 align.vertical == AlignV::TOP ? AlignV::BOTTOM : AlignV::TOP);
}

//---------------------------------------------------------
//   Style values
//---------------------------------------------------------

//! A value of the type of the style value that differs from \p v, or an invalid one for a type this test
//! does not know: every type handled in MStyle::readProperties and MStyle::save has to be here
PropertyValue otherStyleValue(const PropertyValue& v)
{
    switch (v.type()) {
    case P_TYPE::BOOL:
        return !v.toBool();
    case P_TYPE::INT:
        return v.toInt() + 1;
    case P_TYPE::REAL:
        return otherNumber(v.toReal());
    case P_TYPE::SPATIUM:
        return Spatium(otherNumber(v.value<Spatium>().val()));
    case P_TYPE::STRING:
        return v.value<String>() + u"x&";
    case P_TYPE::POINT:
        return PointF(otherNumber(v.value<PointF>().x()), otherNumber(v.value<PointF>().y()) + 0.25);
    case P_TYPE::SIZE:
        return SizeF(otherNumber(v.value<SizeF>().width()), otherNumber(v.value<SizeF>().height()) + 0.25);
    case P_TYPE::SCALE:
        return ScaleF(otherNumber(v.value<ScaleF>().width()), otherNumber(v.value<ScaleF>().height()) + 0.25);
    case P_TYPE::COLOR:
        return otherColor(v.value<Color>());
    case P_TYPE::ALIGN:
        return otherAlign(v.value<Align>());
    case P_TYPE::ALIGN_H:
        return AlignH(otherEnumerator(v));
    case P_TYPE::DIRECTION_V:
        return DirectionV(otherEnumerator(v));
    case P_TYPE::PLACEMENT_V:
        return PlacementV(otherEnumerator(v));
    case P_TYPE::PLACEMENT_H:
        return PlacementH(otherEnumerator(v));
    case P_TYPE::HOOK_TYPE:
        return HookType(otherEnumerator(v));
    case P_TYPE::LINE_TYPE:
        return LineType(otherEnumerator(v));
    case P_TYPE::CLEF_TO_BARLINE_POS:
        return ClefToBarlinePosition(otherEnumerator(v));
    case P_TYPE::TIE_PLACEMENT:
        return TiePlacement(otherEnumerator(v));
    case P_TYPE::TIE_DOTS_PLACEMENT:
        return TieDotsPlacement(otherEnumerator(v));
    case P_TYPE::GLISS_STYLE:
        return GlissandoStyle(otherEnumerator(v));
    case P_TYPE::GLISS_TYPE:
        return GlissandoType(otherEnumerator(v));
    case P_TYPE::TIMESIG_PLACEMENT:
        return TimeSigPlacement(otherEnumerator(v));
    case P_TYPE::TIMESIG_STYLE:
        return TimeSigStyle(otherEnumerator(v));
    case P_TYPE::TIMESIG_MARGIN:
        return TimeSigVSMargin(otherEnumerator(v));
    case P_TYPE::NOTE_SPELLING_TYPE:
        return NoteSpellingType(otherEnumerator(v));
    case P_TYPE::CHORD_PRESET_TYPE:
        return ChordStylePreset(otherEnumerator(v));
    case P_TYPE::PARENTHESES_MODE:
        return ParenthesesMode(otherEnumerator(v));
    case P_TYPE::LH_TAPPING_SYMBOL:
        return LHTappingSymbol(otherEnumerator(v));
    case P_TYPE::RH_TAPPING_SYMBOL:
        return RHTappingSymbol(otherEnumerator(v));
    case P_TYPE::TEXT_STYLE:
        return TextStyleType(otherEnumerator(v));
    case P_TYPE::PLAY_COUNT_PRESET:
        return RepeatPlayCountPreset(otherEnumerator(v));
    case P_TYPE::MEASURE_NUMBER_PLACEMENT:
        return MeasureNumberPlacement(otherEnumerator(v));
    default:
        return PropertyValue();
    }
}

//! Style values that are not written as they are set, each with the reason
const std::map<Sid, const char*>& styleValuesNotWrittenAsSet()
{
    static const std::map<Sid, const char*> SIDS {
        { Sid::defaultsVersion, "names the defaults the other values of a file are resolved against, and is not a setting" },
        { Sid::defaultFontSpatiumDependent, "obsolete: MStyle::read ignores it, as upstream does" },
    };
    return SIDS;
}

std::vector<Sid> allStyleIds()
{
    std::vector<Sid> sids;
    for (int i = 0; i < int(Sid::STYLES); ++i) {
        sids.push_back(Sid(i));
    }
    return sids;
}

//! Gives every style value another value than it has, and returns what was set
std::map<Sid, PropertyValue> changeEveryStyleValue(MStyle& style)
{
    std::map<Sid, PropertyValue> changed;
    for (Sid sid : allStyleIds()) {
        if (styleValuesNotWrittenAsSet().count(sid)) {
            continue;
        }
        // The staff space is written in millimetres
        const PropertyValue other = sid == Sid::spatium
                                    ? PropertyValue((nearlyEqual(style.spatium(), 1.5 * DPMM) ? 1.25 : 1.5) * DPMM)
                                    : otherStyleValue(style.value(sid));
        EXPECT_TRUE(other.isValid()) << "style value " << MStyle::valueName(sid) << " has a type this test does not know: "
                                     << int(MStyle::valueType(sid));
        if (!other.isValid()) {
            continue;
        }
        EXPECT_FALSE(sameValue(other, style.value(sid))) << MStyle::valueName(sid);
        style.set(sid, other);
        changed[sid] = other;
    }
    return changed;
}

void expectStyleValues(const MStyle& style, const std::map<Sid, PropertyValue>& expected)
{
    for (const auto& [sid, value] : expected) {
        EXPECT_TRUE(sameValue(style.value(sid), value))
            << "style value " << MStyle::valueName(sid) << ": set " << valueText(value) << ", read " << valueText(style.value(sid));
    }
}

//---------------------------------------------------------
//   Files
//---------------------------------------------------------

//! Saves the score with its parts and its styles, as the application does, and opens the file
MasterScore* saveAndReopen(MasterScore* score, const String& name)
{
    {
        MscWriter::Params params;
        params.filePath = name;
        params.mode = MscIoMode::Zip;

        MscWriter writer(params);
        EXPECT_TRUE(writer.open());
        MscSaver saver(score->iocContext());
        EXPECT_TRUE(saver.writeMscz(score, writer, false));
        writer.close();
        EXPECT_FALSE(writer.hasError());
    }

    const std::string path = std::filesystem::absolute(name.toStdString()).string();
    return ScoreRW::readScore(String::fromStdString(path), true);
}

//! Adds a part score that shows the given part of the score
Score* addPartScore(MasterScore* score, Part* part)
{
    Score* partScore = score->createScore();
    Excerpt* excerpt = new Excerpt(score);
    excerpt->setExcerptScore(partScore);
    partScore->setExcerpt(excerpt);
    score->excerpts().push_back(excerpt);
    excerpt->setName(part->partName());
    excerpt->setParts({ part });
    Excerpt::createExcerpt(excerpt);
    score->setExcerptsChanged(true);
    partScore->doLayout();
    return partScore;
}

//---------------------------------------------------------
//   Items
//---------------------------------------------------------

//! An item of a score and where it is in the score: the same item has the same key in another
//! instance of the score, such as the one read from the file this one was saved to
struct ScoreItem {
    std::string key;
    EngravingItem* item = nullptr;
};

//! The items of a score that belong to its measures, each after the item it belongs to: notes with
//! their chords, the segments of a line with the line. Multimeasure rests and the items of systems
//! and pages (brackets, instrument names, page numbers) are layout's and are not part of a file.
class ScoreItems
{
public:
    explicit ScoreItems(Score* score)
    {
        int index = 0;
        for (MeasureBase* measureBase = score->first(); measureBase; measureBase = measureBase->next(), ++index) {
            std::map<std::string, int> siblings;
            visit(measureBase, "m" + std::to_string(index), siblings, true);
        }
    }

    const std::vector<ScoreItem>& all() const { return m_items; }

    EngravingItem* find(const std::string& key) const
    {
        auto it = m_byKey.find(key);
        return it != m_byKey.end() ? it->second : nullptr;
    }

    //! The \p n th item of the type for which the predicate holds
    const ScoreItem* find(ElementType type, size_t n = 0,
                          const std::function<bool(const EngravingItem*)>& accepts = nullptr) const
    {
        for (const ScoreItem& scoreItem : m_items) {
            if (scoreItem.item->type() != type || (accepts && !accepts(scoreItem.item))) {
                continue;
            }
            if (n == 0) {
                return &scoreItem;
            }
            --n;
        }
        return nullptr;
    }

private:
    static std::string nameOf(const EngravingItem* item)
    {
        std::string name = item->typeName();
        if (item->isSegment()) {
            const Segment* segment = toSegment(item);
            name += std::string(":") + segment->subTypeName() + "@" + segment->rtick().toString().toStdString();
        } else if (item->track() != muse::nidx && !item->isMeasureBase()) {
            name += "@" + std::to_string(item->track());
        }
        return name;
    }

    static std::vector<EngravingItem*> childrenOf(EngravingItem* item)
    {
        std::vector<EngravingItem*> children;
        for (EngravingObject* child : item->scanChildren()) {
            if (child && child->isEngravingItem()) {
                children.push_back(toEngravingItem(child));
            }
        }
        // What the scan leaves out
        children.push_back(item->leftParen());
        children.push_back(item->rightParen());
        if (item->isNote()) {
            children.push_back(toNote(item)->tieFor());
        } else if (item->isBarLine()) {
            children.push_back(toBarLine(item)->spanConnector());
        } else if (item->isOrnament()) {
            children.push_back(toOrnament(item)->accidentalAbove());
            children.push_back(toOrnament(item)->accidentalBelow());
            children.push_back(toOrnament(item)->cueNoteChord());
        }
        return children;
    }

    void visit(EngravingItem* item, const std::string& parentKey, std::map<std::string, int>& siblings, bool isRoot = false)
    {
        if (!item || !m_seen.insert(item).second) {
            return;
        }
        std::string key = parentKey;
        if (!isRoot) {
            const std::string name = nameOf(item);
            const int ordinal = siblings[name]++;
            key += "/" + name + (ordinal > 0 ? "#" + std::to_string(ordinal) : "");
        }
        m_items.push_back({ key, item });
        m_byKey[key] = item;

        std::map<std::string, int> childSiblings;
        for (EngravingItem* child : childrenOf(item)) {
            visit(child, key, childSiblings);
        }
    }

    std::vector<ScoreItem> m_items;
    std::map<std::string, EngravingItem*> m_byKey;
    std::unordered_set<const EngravingItem*> m_seen;
};

//! Changes a property the way the properties panel does: a value that followed the style no longer does
void changeProperty(EngravingItem* item, Pid pid, const PropertyValue& value)
{
    PropertyFlags flags = item->propertyFlags(pid);
    if (flags == PropertyFlags::STYLED) {
        flags = PropertyFlags::UNSTYLED;
    }
    item->undoChangeProperty(pid, value, flags);
}

//---------------------------------------------------------
//   Properties of the fork
//---------------------------------------------------------

struct PropertyChange {
    PropertyChange(ElementType t, Pid p, const PropertyValue& v, const std::function<bool(const EngravingItem*)>& a = nullptr)
        : type(t), pid(p), value(v), accepts(a) {}

    ElementType type = ElementType::INVALID;
    Pid pid = Pid::END;
    PropertyValue value;
    std::function<bool(const EngravingItem*)> accepts = nullptr;

    std::string key;       // of the item the change is made on
    PropertyValue before;  // what the item had
    PropertyValue set;     // what the item has once the change is made
};

std::string describe(const PropertyChange& change)
{
    return std::string(propertyName(change.pid)) + " of " + change.key;
}

bool isSymbol(const EngravingItem* item, SymId sym)
{
    return item->isSymbol() && toSymbol(item)->sym() == sym;
}

//! Every property the fork added, on the items that have it, with a value it does not have by default.
//! A length is a number of the staff spaces of its staff; an offset is in the units of the score.
std::vector<PropertyChange> forkPropertyChanges(double sp)
{
    auto handBracket = [](const EngravingItem* item) { return isSymbol(item, SymId::keyboardPlayWithLH); };
    auto timeSigBracket = [](const EngravingItem* item) { return isSymbol(item, SymId::timeSigBracketLeft); };
    auto grace = [](const EngravingItem* item) { return toChord(item)->isGrace(); };

    std::vector<PropertyChange> changes {
        // ledger lines: of one line, and of all the lines of a note
        { ElementType::LEDGER_LINE, Pid::LEDGER_LINE_LENGTH_OFFSET_LEFT, Spatium(0.25) },
        { ElementType::LEDGER_LINE, Pid::LEDGER_LINE_LENGTH_OFFSET_RIGHT, Spatium(-0.125) },
        { ElementType::NOTE, Pid::LEDGER_LINE_LENGTH_OFFSET_LEFT, Spatium(0.375) },
        { ElementType::NOTE, Pid::LEDGER_LINE_LENGTH_OFFSET_RIGHT, Spatium(0.5) },

        // ottava with turning points
        { ElementType::OTTAVA, Pid::OTTAVA_ALLOW_BROKEN_LINE, true },
        { ElementType::OTTAVA, Pid::OTTAVA_BREAK_POINTS_COUNT, 4 },
        { ElementType::OTTAVA_SEGMENT, Pid::OTTAVA_BREAK_POINT_1_OFFSET, PointF(1.0 * sp, -0.5 * sp) },
        { ElementType::OTTAVA_SEGMENT, Pid::OTTAVA_BREAK_POINT_2_OFFSET, PointF(-0.25 * sp, 0.75 * sp) },
        { ElementType::OTTAVA_SEGMENT, Pid::OTTAVA_BREAK_POINT_3_OFFSET, PointF(0.5 * sp, 1.25 * sp) },
        { ElementType::OTTAVA_SEGMENT, Pid::OTTAVA_BREAK_POINT_4_OFFSET, PointF(-1.5 * sp, -0.75 * sp) },

        // expression before the dynamic
        { ElementType::EXPRESSION, Pid::SNAP_BEFORE_DYNAMICS, true },

        // slurs of several curves, and with a flat middle
        { ElementType::SLUR, Pid::SLUR_MULTI_BEZIER_ENABLED, false },
        { ElementType::SLUR, Pid::SLUR_MULTI_BEZIER_ENABLED, true },
        { ElementType::SLUR, Pid::SLUR_MULTI_BEZIER_KNOT_COUNT, 3 },
        { ElementType::SLUR_SEGMENT, Pid::SLUR_MULTI_BEZIER_DATA,
          String(u"0.5,-0.25,-0.75,0.125,0.75,-0.125;0,0.375,-0.5,0,0.5,0;-0.25,0,-0.625,0.25,0.625,-0.25") },
        { ElementType::SLUR_SEGMENT, Pid::SLUR_FROZEN_SHAPE, String(u"0.125,-0.25;1.5,-2.25;-1.5,-2.75;-0.125,0.375;0.0625") },
        { ElementType::HAMMER_ON_PULL_OFF, Pid::SLUR_CURVE_MODE, int(Slur::CurveMode::FlatMiddle) },
        { ElementType::HAMMER_ON_PULL_OFF_SEGMENT, Pid::SLUR_MULTI_BEZIER_DATA, String(u"0.25,-0.5,-0.5,0,0.5,0;-0.25,-0.5,-0.5,0,0.5,0") },

        // brackets of a keyboard hand and of a time signature
        { ElementType::SYMBOL, Pid::SYMBOL_SHORT_SIDE_LENGTH, Spatium(2.25), handBracket },
        { ElementType::SYMBOL, Pid::SYMBOL_LONG_SIDE_LENGTH, Spatium(5.5), handBracket },
        { ElementType::SYMBOL, Pid::LINE_WIDTH, Spatium(0.25), handBracket },
        { ElementType::SYMBOL, Pid::SYMBOL_LONG_SIDE_LENGTH, Spatium(6.25), timeSigBracket },
        { ElementType::SYMBOL, Pid::SYMBOL_TOP_HOOK_LENGTH, Spatium(0.75), timeSigBracket },
        { ElementType::SYMBOL, Pid::SYMBOL_BOTTOM_HOOK_LENGTH, Spatium(1.25), timeSigBracket },
        { ElementType::SYMBOL, Pid::LINE_WIDTH, Spatium(0.375), timeSigBracket },

        // tuplet bracket
        { ElementType::TUPLET, Pid::TUPLET_BRACKET_FIXED_SLANT, true },

        // strokes of a tremolo between two notes
        { ElementType::TREMOLO_TWOCHORD, Pid::TREMOLO_START_X_OFFSET, Spatium(0.25) },
        { ElementType::TREMOLO_TWOCHORD, Pid::TREMOLO_START_Y_OFFSET, Spatium(-0.5) },
        { ElementType::TREMOLO_TWOCHORD, Pid::TREMOLO_END_X_OFFSET, Spatium(-0.375) },
        { ElementType::TREMOLO_TWOCHORD, Pid::TREMOLO_END_Y_OFFSET, Spatium(0.75) },

        // ends of a parenthesis
        { ElementType::PARENTHESIS, Pid::PAREN_START_Y_OFFSET, Spatium(-0.5) },
        { ElementType::PARENTHESIS, Pid::PAREN_END_Y_OFFSET, Spatium(0.375) },

        // grace notes before the barline
        { ElementType::CHORD, Pid::GRACE_BEFORE_BARLINE, true, grace },

        // hairpin
        { ElementType::HAIRPIN, Pid::HAIRPIN_VERTICAL_ENDS, true },
        { ElementType::HAIRPIN, Pid::MASK_BARLINES, true },

    };

    // lines with text
    for (ElementType type : { ElementType::TEXTLINE, ElementType::OTTAVA, ElementType::PEDAL, ElementType::VOLTA,
                              ElementType::GRADUAL_TEMPO_CHANGE, ElementType::LET_RING, ElementType::PALM_MUTE }) {
        changes.emplace_back(type, Pid::MASK_BARLINES, true);
    }

    // text
    for (ElementType type : { ElementType::STAFF_TEXT, ElementType::SYSTEM_TEXT, ElementType::EXPRESSION, ElementType::DYNAMIC,
                              ElementType::TEMPO_TEXT, ElementType::REHEARSAL_MARK, ElementType::LYRICS, ElementType::FINGERING,
                              ElementType::HARMONY, ElementType::STICKING, ElementType::MARKER, ElementType::JUMP,
                              ElementType::TEXT }) {
        changes.emplace_back(type, Pid::TEXT_LETTER_SPACING, 0.25);
        changes.emplace_back(type, Pid::MASK_BARLINES, true);
        changes.emplace_back(type, Pid::DIAGONAL, true);
        changes.emplace_back(type, Pid::TEXT_SLANT_ANGLE, 12.5);
    }
    return changes;
}

//! Makes the changes one after the other in one command, and records what each item has then.
//! A property that the item does not take, or that a later change resets, shows as a failure.
void applyChanges(Score* score, std::vector<PropertyChange>& changes)
{
    {
        const ScoreItems items(score);
        score->startCmd(TranslatableString::untranslatable("Property round trip test"));
        for (PropertyChange& change : changes) {
            const ScoreItem* target = items.find(change.type, 0, change.accepts);
            ASSERT_TRUE(target) << "the score has no item for " << propertyName(change.pid) << " of type " << int(change.type);
            change.key = target->key;
            change.before = target->item->getProperty(change.pid);
            ASSERT_TRUE(change.before.isValid()) << describe(change);
            changeProperty(target->item, change.pid, change.value);
        }
        score->endCmd();
    }

    const ScoreItems items(score);
    for (PropertyChange& change : changes) {
        EngravingItem* item = items.find(change.key);
        ASSERT_TRUE(item) << describe(change);
        change.set = item->getProperty(change.pid);
    }
}

//! Each item has what was set on it; the last change to a property is the one that counts.
//! \p unit is the staff space of the score in staff spaces of the score the changes were made in.
void expectChanges(Score* score, const std::vector<PropertyChange>& changes, const char* when, double unit = 1.0)
{
    const ScoreItems items(score);
    std::map<std::pair<std::string, Pid>, const PropertyChange*> last;
    for (const PropertyChange& change : changes) {
        last[{ change.key, change.pid }] = &change;
    }
    for (const auto& [where, change] : last) {
        EngravingItem* item = items.find(change->key);
        EXPECT_TRUE(item) << when << ": no item for " << describe(*change);
        if (!item) {
            continue;
        }
        EXPECT_TRUE(sameValue(change->value, item->getProperty(change->pid), unit))
            << when << ": " << describe(*change) << " is " << valueText(item->getProperty(change->pid))
            << ", set " << valueText(change->value);
    }
}

//! A copy of each item has what the item has, and so has the item read from what copying the item
//! to the clipboard writes
void expectCopiesKeepChanges(Score* score, const std::vector<PropertyChange>& changes)
{
    const ScoreItems items(score);
    for (const PropertyChange& change : changes) {
        EngravingItem* item = items.find(change.key);
        ASSERT_TRUE(item) << describe(change);
        EngravingItem* copy = item->clone();
        EXPECT_TRUE(sameValue(copy->getProperty(change.pid), item->getProperty(change.pid)))
            << "copy: " << describe(change) << " is " << valueText(copy->getProperty(change.pid))
            << ", the item has " << valueText(item->getProperty(change.pid));
        delete copy;

        // Texts and symbols are copied on their own; other items with the range they are in
        if (!item->isTextBase() && !item->isSymbol()) {
            continue;
        }
        EngravingItem* pasted = ScoreRW::writeReadElement(item);
        ASSERT_TRUE(pasted) << describe(change);
        EXPECT_TRUE(sameValue(pasted->getProperty(change.pid), item->getProperty(change.pid)))
            << "clipboard: " << describe(change) << " is " << valueText(pasted->getProperty(change.pid))
            << ", the item has " << valueText(item->getProperty(change.pid));
        delete pasted;
    }
}

//! Undoes the changes: each item has again what it had, where the first change to a property is the one that counts
void expectUndoRestores(Score* score, const std::vector<PropertyChange>& changes)
{
    score->undoRedo(true, nullptr);
    const ScoreItems items(score);
    std::set<std::pair<std::string, Pid> > seen;
    for (const PropertyChange& change : changes) {
        if (!seen.insert({ change.key, change.pid }).second) {
            continue;
        }
        EngravingItem* item = items.find(change.key);
        EXPECT_TRUE(item) << "undo: no item for " << describe(change);
        if (!item) {
            continue;
        }
        EXPECT_TRUE(sameValue(item->getProperty(change.pid), change.before))
            << "undo: " << describe(change) << " is " << valueText(item->getProperty(change.pid))
            << ", was " << valueText(change.before);
    }
}

//! A number format with a decimal comma for as long as the object lives, if the system has one
class DecimalCommaLocale
{
public:
    DecimalCommaLocale()
    {
        m_previous = std::setlocale(LC_NUMERIC, nullptr);
        for (const char* name : { "de_DE.UTF-8", "fr_FR.UTF-8", "de_DE", "fr_FR", "it_IT.UTF-8", "nl_NL.UTF-8" }) {
            if (std::setlocale(LC_NUMERIC, name) && std::string(std::localeconv()->decimal_point) == ",") {
                m_set = true;
                return;
            }
        }
        std::setlocale(LC_NUMERIC, m_previous.c_str());
    }

    ~DecimalCommaLocale()
    {
        std::setlocale(LC_NUMERIC, m_previous.c_str());
    }

    bool isSet() const { return m_set; }

private:
    std::string m_previous;
    bool m_set = false;
};
}

class Engraving_PropertyRoundTripTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_useRead302 = MScore::useRead302InTestMode;
        MScore::useRead302InTestMode = false;
    }

    void TearDown() override
    {
        MScore::useRead302InTestMode = m_useRead302;
    }

private:
    bool m_useRead302 = false;
};

//---------------------------------------------------------
//   styleValuesSurviveWriteAndRead
//   A style file: every style value, set to a value it does not have by default, is read back as written.
//---------------------------------------------------------

TEST_F(Engraving_PropertyRoundTripTests, styleValuesSurviveWriteAndRead)
{
    MStyle style = DefaultStyle::baseStyle();
    const std::map<Sid, PropertyValue> changed = changeEveryStyleValue(style);
    EXPECT_GT(changed.size(), size_t(Sid::STYLES) - 20);

    muse::io::Buffer buffer;
    ASSERT_TRUE(buffer.open(muse::io::IODevice::WriteOnly));
    ASSERT_TRUE(style.write(&buffer));
    buffer.close();

    MStyle read = DefaultStyle::baseStyle();
    ASSERT_TRUE(buffer.open(muse::io::IODevice::ReadOnly));
    ASSERT_TRUE(read.read(&buffer));
    buffer.close();

    expectStyleValues(read, changed);
}

//---------------------------------------------------------
//   styleValuesSurviveSaveAndReopen
//   The same through a score file, in which the score and each of its parts have a style of their own.
//   The values are not ones a score can be laid out with, so the styles are changed after the last layout
//   and the file is opened with the layout of the default style.
//---------------------------------------------------------

TEST_F(Engraving_PropertyRoundTripTests, styleValuesSurviveSaveAndReopen)
{
    MasterScore* score = ScoreRW::readScore(u"layoutconsistency_data/twoStaves.mscx");
    ASSERT_TRUE(score);
    Score* partScore = addPartScore(score, score->parts().front());

    const std::map<Sid, PropertyValue> changed = changeEveryStyleValue(score->style());
    changeEveryStyleValue(partScore->style());
    const std::map<Sid, PropertyValue> changedInPart = changeEveryStyleValue(partScore->style());

    const String name = u"propertyRoundTrip-style.mscz";
    {
        MscWriter::Params params;
        params.filePath = name;
        params.mode = MscIoMode::Zip;
        MscWriter writer(params);
        ASSERT_TRUE(writer.open());
        MscSaver saver(score->iocContext());
        ASSERT_TRUE(saver.writeMscz(score, writer, false));
        writer.close();
        ASSERT_FALSE(writer.hasError());
    }

    MasterScore* reloaded = compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr);
    ASSERT_TRUE(compat::loadMsczOrMscx(reloaded, name, false));
    expectStyleValues(reloaded->style(), changed);
    ASSERT_EQ(reloaded->excerpts().size(), 1u);
    expectStyleValues(reloaded->excerpts().front()->excerptScore()->style(), changedInPart);

    delete reloaded;
    delete score;
}

//---------------------------------------------------------
//   doublesSurviveSaveAndReopenExactly
//   A number that needs more than 6 significant digits comes back as it was: a style value, stored as it is,
//   identically; an offset, stored in staff spaces, up to the rounding of the division by the staff space and
//   the multiplication by it (6 digits would differ in the 7th).
//---------------------------------------------------------

TEST_F(Engraving_PropertyRoundTripTests, doublesSurviveSaveAndReopenExactly)
{
    MasterScore* score = ScoreRW::readScore(u"layoutconsistency_data/twoStaves.mscx");
    ASSERT_TRUE(score);

    const double pageWidth = 210.0 / 25.4; // A4 in inches, as a conversion from millimetres gives it
    const double staffUpperBorder = 7.123456789012345;
    const PointF noteOffset(0.1234567890123456, -0.9876543210987654);
    score->style().set(Sid::pageWidth, pageWidth);
    score->style().set(Sid::staffUpperBorder, Spatium(staffUpperBorder));
    Note* note = score->firstMeasure()->findChord(Fraction(0, 1), 0)->upNote();
    note->setOffset(noteOffset * note->spatium());

    std::unique_ptr<MasterScore> reopened(saveAndReopen(score, u"propertyRoundTrip-exactDoubles.mscz"));
    ASSERT_TRUE(reopened);
    EXPECT_EQ(reopened->style().styleD(Sid::pageWidth), pageWidth);
    EXPECT_EQ(reopened->style().styleS(Sid::staffUpperBorder).val(), staffUpperBorder);
    const Note* reopenedNote = reopened->firstMeasure()->findChord(Fraction(0, 1), 0)->upNote();
    const PointF reopenedOffset = reopenedNote->offset() / reopenedNote->spatium();
    EXPECT_DOUBLE_EQ(reopenedOffset.x(), noteOffset.x());
    EXPECT_DOUBLE_EQ(reopenedOffset.y(), noteOffset.y());

    delete score;
}

//---------------------------------------------------------
//   forkPropertiesSurviveSaveAndReopen
//   Every property the fork added, on each kind of item that has it: the value set is the value a copy of
//   the item has, the value read from the saved file, and gone again when the change is undone. On staves
//   of the default size, on small staves and on scaled staves, whose staff space is not the score's.
//---------------------------------------------------------

namespace {
enum class StaffSize {
    Default, Small, Scaled
};

void expectForkPropertiesSurvive(StaffSize staffSize, const String& fileName)
{
    MasterScore* score = ScoreRW::readScore(u"propertyroundtrip_data/forkElements.mscx");
    ASSERT_TRUE(score);
    if (staffSize != StaffSize::Default) {
        for (Staff* staff : score->staves()) {
            StaffType* staffType = staff->staffType(Fraction(0, 1));
            if (staffSize == StaffSize::Small) {
                staffType->setSmall(true);
            } else {
                staffType->setUserMag(0.7);
            }
        }
        score->doLayout();
    }

    std::vector<PropertyChange> changes = forkPropertyChanges(score->style().spatium());
    applyChanges(score, changes);
    if (::testing::Test::HasFatalFailure()) {
        delete score;
        return;
    }
    expectChanges(score, changes, "set");
    expectCopiesKeepChanges(score, changes);

    MasterScore* reopened = saveAndReopen(score, fileName);
    ASSERT_TRUE(reopened);
    for (size_t i = 0; i < score->nstaves(); ++i) {
        EXPECT_NEAR(reopened->staff(i)->staffMag(Fraction(0, 1)), score->staff(i)->staffMag(Fraction(0, 1)), 1e-9);
    }
    const double unit = reopened->style().spatium() / score->style().spatium();
    expectChanges(reopened, changes, "reopened", unit);

    // The file of the reopened score is the file of the score
    MasterScore* reopenedAgain = saveAndReopen(reopened, fileName);
    ASSERT_TRUE(reopenedAgain);
    expectChanges(reopenedAgain, changes, "reopened twice", unit);

    expectUndoRestores(score, changes);

    delete reopenedAgain;
    delete reopened;
    delete score;
}
}

TEST_F(Engraving_PropertyRoundTripTests, forkPropertiesSurviveSaveAndReopen)
{
    expectForkPropertiesSurvive(StaffSize::Default, u"propertyRoundTrip-fork.mscz");
}

TEST_F(Engraving_PropertyRoundTripTests, forkPropertiesSurviveSaveAndReopenOnSmallStaves)
{
    expectForkPropertiesSurvive(StaffSize::Small, u"propertyRoundTrip-fork-small.mscz");
}

TEST_F(Engraving_PropertyRoundTripTests, forkPropertiesSurviveSaveAndReopenOnScaledStaves)
{
    expectForkPropertiesSurvive(StaffSize::Scaled, u"propertyRoundTrip-fork-scaled.mscz");
}

//---------------------------------------------------------
//   forkPropertiesSurviveSaveAndReopenWithDecimalComma
//   The numbers in a file are written with a decimal point whatever the number format of the system is.
//---------------------------------------------------------

TEST_F(Engraving_PropertyRoundTripTests, forkPropertiesSurviveSaveAndReopenWithDecimalComma)
{
    const DecimalCommaLocale locale;
    if (!locale.isSet()) {
        GTEST_SKIP() << "the system has no number format with a decimal comma";
    }
    expectForkPropertiesSurvive(StaffSize::Scaled, u"propertyRoundTrip-fork-comma.mscz");
}

//---------------------------------------------------------
//   forkPropertiesReachTheParts
//   What is set in the score is set on the linked item of a part too, as for any other property of
//   appearance, position or text while the part follows the score; score and part keep it in the file.
//   The length of a single ledger line is kept by the chord of the score it was set in.
//---------------------------------------------------------

namespace {
//! The changes that can be made in the part: those on items of its instrument
std::vector<PropertyChange> changesInPart(Score* partScore, const std::vector<PropertyChange>& changes)
{
    const ScoreItems partItems(partScore);
    std::vector<PropertyChange> result;
    for (const PropertyChange& change : changes) {
        if (partItems.find(change.type, 0, change.accepts)) {
            result.push_back(change);
        }
    }
    return result;
}

void setKeysIn(Score* score, std::vector<PropertyChange>& changes)
{
    const ScoreItems items(score);
    for (PropertyChange& change : changes) {
        const ScoreItem* target = items.find(change.type, 0, change.accepts);
        ASSERT_TRUE(target) << propertyName(change.pid);
        change.key = target->key;
    }
}

bool isOfSingleLedgerLine(const PropertyChange& change)
{
    return change.type == ElementType::LEDGER_LINE;
}
}

TEST_F(Engraving_PropertyRoundTripTests, forkPropertiesReachTheParts)
{
    MasterScore* score = ScoreRW::readScore(u"propertyroundtrip_data/forkElements.mscx");
    ASSERT_TRUE(score);
    Score* partScore = addPartScore(score, score->parts().front());

    std::vector<PropertyChange> changes = forkPropertyChanges(score->style().spatium());
    applyChanges(score, changes);
    ASSERT_FALSE(HasFatalFailure());
    expectChanges(score, changes, "score");

    std::vector<PropertyChange> inPart = changesInPart(partScore, changes);
    EXPECT_GT(inPart.size(), changes.size() / 2);
    inPart.erase(std::remove_if(inPart.begin(), inPart.end(), isOfSingleLedgerLine), inPart.end());
    setKeysIn(partScore, inPart);
    ASSERT_FALSE(HasFatalFailure());
    expectChanges(partScore, inPart, "part");

    MasterScore* reopened = saveAndReopen(score, u"propertyRoundTrip-parts.mscz");
    ASSERT_TRUE(reopened);
    ASSERT_EQ(reopened->excerpts().size(), 1u);
    const double unit = reopened->style().spatium() / score->style().spatium();
    expectChanges(reopened, changes, "reopened score", unit);
    expectChanges(reopened->excerpts().front()->excerptScore(), inPart, "reopened part", unit);

    delete reopened;
    delete score;
}

//---------------------------------------------------------
//   forkPropertiesOfAPartStayInThePart
//   What is set in a part is the part's alone, and is still the part's alone in the file. Except what score
//   and parts always share: whether a slur is drawn as several curves or with a flat middle, whether grace
//   notes are shown before the barline, and the properties of a text, which a part shares with the score
//   for as long as its text follows the score's.
//---------------------------------------------------------

namespace {
bool isSharedWithScore(const PropertyChange& change)
{
    return propertyLink(change.pid) || propertyGroup(change.pid) == PropertyGroup::TEXT;
}

void expectUnchanged(Score* score, const std::vector<PropertyChange>& changes, const char* when)
{
    const ScoreItems items(score);
    for (const PropertyChange& change : changes) {
        EngravingItem* item = items.find(change.key);
        ASSERT_TRUE(item) << when << ": " << describe(change);
        EXPECT_TRUE(sameValue(item->getProperty(change.pid), change.before))
            << when << ": " << describe(change) << " is " << valueText(item->getProperty(change.pid))
            << ", was " << valueText(change.before);
    }
}
}

TEST_F(Engraving_PropertyRoundTripTests, forkPropertiesOfAPartStayInThePart)
{
    MasterScore* score = ScoreRW::readScore(u"propertyroundtrip_data/forkElements.mscx");
    ASSERT_TRUE(score);
    Score* partScore = addPartScore(score, score->parts().front());

    std::vector<PropertyChange> inPart = changesInPart(partScore, forkPropertyChanges(score->style().spatium()));
    applyChanges(partScore, inPart);
    ASSERT_FALSE(HasFatalFailure());
    expectChanges(partScore, inPart, "part");

    std::vector<PropertyChange> shared;
    std::vector<PropertyChange> partOnly;
    for (const PropertyChange& change : inPart) {
        (isSharedWithScore(change) ? shared : partOnly).push_back(change);
    }
    EXPECT_GT(partOnly.size(), shared.size());
    setKeysIn(score, shared);
    setKeysIn(score, partOnly);
    ASSERT_FALSE(HasFatalFailure());
    expectChanges(score, shared, "score");
    expectUnchanged(score, partOnly, "score");

    MasterScore* reopened = saveAndReopen(score, u"propertyRoundTrip-partOnly.mscz");
    ASSERT_TRUE(reopened);
    ASSERT_EQ(reopened->excerpts().size(), 1u);
    const double unit = reopened->style().spatium() / score->style().spatium();
    expectChanges(reopened->excerpts().front()->excerptScore(), inPart, "reopened part", unit);
    expectChanges(reopened, shared, "reopened score", unit);
    expectUnchanged(reopened, partOnly, "reopened score");

    delete reopened;
    delete score;
}
