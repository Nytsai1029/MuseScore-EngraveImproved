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
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

#include "compat/engravingcompat.h"
#include "compat/mscxcompat.h"
#include "dom/engravingitem.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/mscore.h"
#include "dom/page.h"
#include "dom/property.h"
#include "engravingproject.h"
#include "infrastructure/mscwriter.h"
#include "rw/mscsaver.h"
#include "style/style.h"
#include "types/propertyvalue.h"
#include "types/typesconv.h"

#include "utils/scorerw.h"

using namespace mu;
using namespace muse;
using namespace mu::engraving;

//---------------------------------------------------------
//   What is saved is what is opened: a score that is saved and opened again has to be laid out as it was.
//   Where it is not, the layout fingerprint tells which measure differs. This tells which items differ, and
//   what about them: it writes, for every score, a report of what the score as first opened (A) and the same
//   score saved and opened again (B) do not have in common, and of what a second layout changes in each.
//---------------------------------------------------------

namespace {
const String VTEST_DIR(u"../../../vtest/scores/");
//! Where a score is saved to be opened again: one file, written anew for every score
const String SAVED_FILE(u"reopenDiagnostics-saved.msdz");

//! The scores of vtest which are not laid out, once saved and opened again, as when they are first opened
const char* const KNOWN_SCORES
    = "beams-french-style.mscz,chord-space-1.mscx,cross-2.mscx,cross-5.mscz,cross-9.mscz,customCrossStaffBeams.mscz,"
      "gliss-1.mscx,gliss-3.mscz,grace-7.mscz,guitarBend-1.mscz,guitarBend-2.mscz,guitarBend-3.mscz,"
      "guitarBend-4.mscz,guitarBend-5.mscz,guitarBend-8.mscz,musejazz-8.mscx,note-line.mscz,ornament-1.mscz,"
      "ornament-2.mscz,ornament-3.mscz,ornament-4.mscz,ornament-6.mscz,tablature-1.mscx,tuplets-2.mscx";

//! What the lines of a report tell, at the head of each
const char* const LEGEND
    = "A2, B2: A and B after a second layout of all of the score\n"
      "item: type, staff, measure (the number as shown; -1: in none), tick, track, # (its place among the items of\n"
      "  which all these are the same, in the order of page, x, y)\n"
      "place: page (its index, from 0), pos (on the page), box (x, y, width, height from pos); in staff spaces\n"
      "[not drawn]: in both, the item is invisible or left out of the drawing\n"
      "[not counted]: in both, the page does not give the item when it is asked for what it shows, which is what\n"
      "  the layout fingerprint counts (so the items of a hidden staff, the number which a tuplet draws itself)\n"
      "under an item, what it tells differently: name() is asked of the item itself (counted(): see above), name\n"
      "  is a property, with [whether it follows the style]; an item also answers for the properties of the items\n"
      "  it is in\n";

//! In staff spaces: what is no further than this from where it was has not moved
constexpr double SAME_PLACE = 0.02;
//! What a length which is no number is taken as, so that it shows and the records can still be put in order
constexpr double NO_NUMBER = 1.0e9;

//---------------------------------------------------------
//   What the test is given
//---------------------------------------------------------

//! From the environment, so that another score is looked at without building again
std::string environmentValue(const char* name, const char* otherwise)
{
    const char* value = std::getenv(name);
    return value && *value ? value : otherwise;
}

std::vector<std::string> scoreNames()
{
    std::vector<std::string> names;
    std::istringstream list(environmentValue("EO_REOPEN_SCORES", KNOWN_SCORES));
    std::string name;
    while (std::getline(list, name, ',')) {
        name.erase(0, name.find_first_not_of(" \t"));
        name.erase(name.find_last_not_of(" \t") + 1);
        if (!name.empty()) {
            names.push_back(name);
        }
    }
    return names;
}

//---------------------------------------------------------
//   Opening and saving, as layoutfingerprint_tests.cpp does
//---------------------------------------------------------

String dataPath(const String& name)
{
    return ScoreRW::rootPath() + u"/" + name;
}

String outputPath(const String& fileName)
{
    return String::fromStdString(std::filesystem::absolute(fileName.toStdString()).string());
}

//! A score as the application has it: loaded and set up by its project
struct Project
{
    EngravingProjectPtr project;
    Ret ret = muse::make_ok();

    MasterScore* score() const { return project ? project->masterScore() : nullptr; }
};

Project open(const String& path)
{
    Project result;
    result.project = EngravingProject::create(nullptr);
    result.ret = compat::loadMsczOrMscx(result.project, path, false);
    if (!result.ret) {
        result.project = nullptr;
        return result;
    }

    //! As NotationProject::doLoad opens a score (its style migration dialog aside): setup with updates
    //! locked, the compat steps before and after the first layout
    MasterScore* masterScore = result.project->masterScore();
    masterScore->lockUpdates(true);
    result.ret = result.project->setupMasterScore(true);
    if (!result.ret) {
        result.project = nullptr;
        masterScore->lockUpdates(false);
        return result;
    }

    compat::EngravingCompat::doPreLayoutCompatIfNeeded(masterScore);

    masterScore->lockUpdates(false);
    masterScore->setLayoutAll();
    masterScore->update();

    compat::EngravingCompat::doPostLayoutCompatIfNeeded(masterScore);
    return result;
}

bool save(MasterScore* score, const String& fileName)
{
    MscWriter::Params params;
    params.filePath = fileName;
    params.mode = MscIoMode::Zip;

    MscWriter writer(params);
    if (!writer.open()) {
        return false;
    }
    MscSaver saver(score->iocContext());
    const bool ok = saver.writeMscz(score, writer, /*createThumbnail*/ false);
    writer.close();
    return ok && !writer.hasError();
}

//---------------------------------------------------------
//   Values as text
//---------------------------------------------------------

std::string toStd(const String& text)
{
    return text.toStdString();
}

std::string toStd(const AsciiStringView& text)
{
    return std::string(std::string_view(text));
}

//! A length of the layout, in staff spaces
std::string twoDecimals(double value)
{
    char text[64];
    // + 0.0: what rounds to nothing from below is not "-0.00"
    std::snprintf(text, sizeof(text), "%.2f", std::round(value * 100.0) / 100.0 + 0.0);
    return text;
}

//! A number of the style or of a property. The files hold six significant digits of a number: with ten of them,
//! a value which is not the one a file gives back shows, and what the arithmetic leaves in the last digits does not
std::string tenDigits(double value)
{
    char text[64];
    std::snprintf(text, sizeof(text), "%.10g", value + 0.0);
    return text;
}

std::string pairText(double first, double second)
{
    return "(" + tenDigits(first) + ", " + tenDigits(second) + ")";
}

//! Text which does not break the line of the report it is in
std::string oneLine(std::string text)
{
    for (size_t at = text.find('\n'); at != std::string::npos; at = text.find('\n', at)) {
        text.replace(at, 1, "\\n");
    }
    return text;
}

const char* yesNo(bool yes)
{
    return yes ? "yes" : "no";
}

std::string nameOrNumber(const std::string& name, int number)
{
    return name.empty() ? "#" + std::to_string(number) : name;
}

//! The name the files have for a value of an enumeration: TConv::toXml is what XmlWriter::tagProperty and
//! MStyle::save write it with. Its number where the engine has no name for the value.
template<typename T>
std::string writtenName(const PropertyValue& value)
{
    return nameOrNumber(toStd(TConv::toXml(value.value<T>())), value.toInt());
}

//! A value as the files have it, but for the digits of the numbers (see tenDigits). The engine has no function
//! which does this for every type (propertyToString knows six properties), so this goes by the type the value
//! holds, which is the one PropertyValue::value has to be asked for: asked for another, it fails an assert.
std::string valueText(const PropertyValue& value)
{
    switch (value.type()) {
    case P_TYPE::UNDEFINED:
        return "(none)";

    // Base
    case P_TYPE::BOOL:
        return value.toBool() ? "true" : "false";
    case P_TYPE::INT:
        return std::to_string(value.toInt());
    case P_TYPE::INT_VEC:
        return "[" + toStd(TConv::toXml(value.value<std::vector<int> >())) + "]";
    case P_TYPE::SIZE_T:
        return std::to_string(value.value<size_t>());
    case P_TYPE::REAL:
        return tenDigits(value.toReal());
    case P_TYPE::STRING:
        return "\"" + oneLine(toStd(value.value<String>())) + "\"";

    // Geometry
    case P_TYPE::POINT:
        return pairText(value.value<PointF>().x(), value.value<PointF>().y());
    case P_TYPE::SIZE:
        return pairText(value.value<SizeF>().width(), value.value<SizeF>().height());
    case P_TYPE::SCALE:
        return pairText(value.value<ScaleF>().width(), value.value<ScaleF>().height());
    case P_TYPE::PAIR_REAL:
        return pairText(value.value<PairF>().first, value.value<PairF>().second);
    case P_TYPE::SPATIUM:
        return tenDigits(value.value<Spatium>().val());
    case P_TYPE::MILLIMETRE:
        return tenDigits(value.value<Millimetre>().val());
    case P_TYPE::DRAW_PATH: {
        const PainterPath path = value.value<PainterPath>();
        std::string text = "path";
        for (size_t i = 0; i < path.elementCount(); ++i) {
            text += " " + pairText(path.elementAt(i).x, path.elementAt(i).y);
        }
        return text;
    }

    // What is more than one value
    case P_TYPE::COLOR: {
        const Color color = value.value<Color>();
        return "rgba(" + std::to_string(color.red()) + ", " + std::to_string(color.green()) + ", "
               + std::to_string(color.blue()) + ", " + std::to_string(color.alpha()) + ")";
    }
    case P_TYPE::ALIGN:
        return toStd(TConv::toXml(value.value<Align>()));
    case P_TYPE::ORNAMENT_INTERVAL:
        return toStd(TConv::toXml(value.value<OrnamentInterval>()));
    case P_TYPE::FRACTION:
        return toStd(value.value<Fraction>().toString());
    case P_TYPE::TEMPO:
        return tenDigits(value.value<BeatsPerSecond>().val);
    case P_TYPE::DURATION_TYPE_WITH_DOTS: {
        const DurationTypeWithDots duration = value.value<DurationTypeWithDots>();
        return nameOrNumber(toStd(TConv::toXml(duration.type)), static_cast<int>(duration.type))
               + ", dots " + std::to_string(duration.dots);
    }
    case P_TYPE::PITCH_VALUES: {
        std::string text = "points";
        for (const PitchValue& point : value.value<PitchValues>()) {
            text += " " + std::to_string(point.time) + ":" + std::to_string(point.pitch) + (point.vibrato ? "v" : "");
        }
        return text;
    }
    case P_TYPE::GROUPS: {
        std::string text = "groups";
        for (const GroupNode& node : value.value<GroupNodes>()) {
            text += " " + std::to_string(node.pos) + ":" + std::to_string(node.action);
        }
        return text;
    }

    // Enumerations the files have names for
    case P_TYPE::SYMID:                         return writtenName<SymId>(value);
    case P_TYPE::ORNAMENT_STYLE:                return writtenName<OrnamentStyle>(value);
    case P_TYPE::GLISS_STYLE:                   return writtenName<GlissandoStyle>(value);
    case P_TYPE::GLISS_TYPE:                    return writtenName<GlissandoType>(value);
    case P_TYPE::ALIGN_H:                       return writtenName<AlignH>(value);
    case P_TYPE::PLACEMENT_V:                   return writtenName<PlacementV>(value);
    case P_TYPE::PLACEMENT_H:                   return writtenName<PlacementH>(value);
    case P_TYPE::TEXT_PLACE:                    return writtenName<TextPlace>(value);
    case P_TYPE::DIRECTION_V:                   return writtenName<DirectionV>(value);
    case P_TYPE::DIRECTION_H:                   return writtenName<DirectionH>(value);
    case P_TYPE::ORIENTATION:                   return writtenName<Orientation>(value);
    case P_TYPE::BEAM_MODE:                     return writtenName<BeamMode>(value);
    case P_TYPE::ACCIDENTAL_ROLE:               return writtenName<AccidentalRole>(value);
    case P_TYPE::TIE_PLACEMENT:                 return writtenName<TiePlacement>(value);
    case P_TYPE::TIE_DOTS_PLACEMENT:            return writtenName<TieDotsPlacement>(value);
    case P_TYPE::TIMESIG_PLACEMENT:             return writtenName<TimeSigPlacement>(value);
    case P_TYPE::TIMESIG_STYLE:                 return writtenName<TimeSigStyle>(value);
    case P_TYPE::TIMESIG_MARGIN:                return writtenName<TimeSigVSMargin>(value);
    case P_TYPE::NOTE_SPELLING_TYPE:            return writtenName<NoteSpellingType>(value);
    case P_TYPE::CHORD_PRESET_TYPE:             return writtenName<ChordStylePreset>(value);
    case P_TYPE::PARENTHESES_MODE:              return writtenName<ParenthesesMode>(value);
    case P_TYPE::PLAY_COUNT_PRESET:             return writtenName<RepeatPlayCountPreset>(value);
    case P_TYPE::CHANGE_METHOD:                 return writtenName<ChangeMethod>(value);
    case P_TYPE::LAYOUTBREAK_TYPE:              return writtenName<LayoutBreakType>(value);
    case P_TYPE::VELO_TYPE:                     return writtenName<VeloType>(value);
    case P_TYPE::BARLINE_TYPE:                  return writtenName<BarLineType>(value);
    case P_TYPE::NOTEHEAD_TYPE:                 return writtenName<NoteHeadType>(value);
    case P_TYPE::NOTEHEAD_SCHEME:               return writtenName<NoteHeadScheme>(value);
    case P_TYPE::NOTEHEAD_GROUP:                return writtenName<NoteHeadGroup>(value);
    case P_TYPE::CLEF_TYPE:                     return writtenName<ClefType>(value);
    case P_TYPE::DYNAMIC_TYPE:                  return writtenName<DynamicType>(value);
    case P_TYPE::DYNAMIC_SPEED:                 return writtenName<DynamicSpeed>(value);
    case P_TYPE::LINE_TYPE:                     return writtenName<LineType>(value);
    case P_TYPE::HOOK_TYPE:                     return writtenName<HookType>(value);
    case P_TYPE::KEY_MODE:                      return writtenName<KeyMode>(value);
    case P_TYPE::TEXT_STYLE:                    return writtenName<TextStyleType>(value);
    case P_TYPE::PLAYTECH_TYPE:                 return writtenName<PlayingTechniqueType>(value);
    case P_TYPE::TEMPOCHANGE_TYPE:              return writtenName<GradualTempoChangeType>(value);
    case P_TYPE::LYRICS_DASH_SYSTEM_START_TYPE: return writtenName<LyricsDashSystemStart>(value);
    case P_TYPE::PARTIAL_SPANNER_DIRECTION:     return writtenName<PartialSpannerDirection>(value);
    case P_TYPE::MARKER_TYPE:                   return writtenName<MarkerType>(value);
    case P_TYPE::LH_TAPPING_SYMBOL:             return writtenName<LHTappingSymbol>(value);
    case P_TYPE::RH_TAPPING_SYMBOL:             return writtenName<RHTappingSymbol>(value);
    case P_TYPE::VOICE_ASSIGNMENT:              return writtenName<VoiceAssignment>(value);
    case P_TYPE::AUTO_ON_OFF:                   return writtenName<AutoOnOff>(value);
    case P_TYPE::AUTO_CUSTOM_HIDE:              return writtenName<AutoCustomHide>(value);
    case P_TYPE::MEASURE_NUMBER_PLACEMENT:      return writtenName<MeasureNumberPlacement>(value);

    // Those they have none for, and whatever type comes to be: never the end of the report
    default:
        break;
    }
    return "(type " + std::to_string(static_cast<int>(value.type())) + ")"
           + (value.isEnum() ? " #" + std::to_string(value.toInt()) : std::string());
}

//! The text of a property. Six properties are the number of an enumeration, which propertyToString writes by
//! its name; it is not asked about the properties of the types it has no text for, where it fails a check.
std::string propertyText(Pid pid, const PropertyValue& value)
{
    const P_TYPE type = propertyType(pid);
    const bool hasNoText = type == P_TYPE::DURATION_TYPE_WITH_DOTS || type == P_TYPE::TEMPO || type == P_TYPE::GROUPS;
    if (value.type() == P_TYPE::INT && !hasNoText) {
        const String written = propertyToString(pid, value, /*mscx*/ true);
        if (!written.isEmpty()) {
            return toStd(written);
        }
    }
    return valueText(value);
}

std::string_view flagName(PropertyFlags flags)
{
    switch (flags) {
    case PropertyFlags::STYLED:
        return "styled";
    case PropertyFlags::UNSTYLED:
        return "unstyled";
    case PropertyFlags::NOSTYLE:
        break;
    }
    return "no style";
}

//---------------------------------------------------------
//   A snapshot: the style and the items of a score, as they are when it is taken
//---------------------------------------------------------

//! Something an item tells: a property, or what the item itself is asked
struct Property
{
    int id = 0;             // of the property; below zero for what is not one
    std::string_view name;
    std::string value;
    std::string_view flag;  // whether the property follows the style; nothing for what is not a property

    std::string text() const { return flag.empty() ? value : value + " [" + std::string(flag) + "]"; }
};

//! One laid-out item
struct Record
{
    // What says which item of another snapshot it is, with its ordinal
    std::string_view type;
    int staff = -1;
    int measure = -1;       // the number as shown; -1: in no measure
    Fraction tick;
    int track = -1;
    int ordinal = 0;        // among the items of the same key, in the order of where they are

    // Where it is, in staff spaces
    int page = 0;
    PointF pos;             // on the page
    RectF box;              // from pos

    bool visible = true;
    bool skipDraw = false;
    //! Whether the layout fingerprint counts it: its page gives it when it is asked for what it shows. Not so the
    //! items of a hidden staff, nor the number of a tuplet, which the tuplet draws
    bool counted = true;
    std::vector<Property> properties;

    static Record of(const EngravingItem* item, int page, double spatium, bool counted);

    auto key() const { return std::tie(measure, tick, staff, track, type); }
    //! The layout fingerprint also counts the items which are not drawn; the page that is looked at does not have them
    bool isDrawn() const { return visible && !skipDraw; }

    std::string keyText() const
    {
        return std::string(type) + " staff " + std::to_string(staff) + " measure " + std::to_string(measure) + " tick "
               + toStd(tick.reduced().toString()) + " track " + std::to_string(track) + " #" + std::to_string(ordinal);
    }

    std::string placeText() const
    {
        return "page " + std::to_string(page) + " pos (" + twoDecimals(pos.x()) + ", " + twoDecimals(pos.y())
               + ") box (" + twoDecimals(box.x()) + ", " + twoDecimals(box.y()) + ", " + twoDecimals(box.width())
               + ", " + twoDecimals(box.height()) + ")";
    }
};

double staffSpaces(double length, double spatium)
{
    const double value = length / spatium;
    return std::isfinite(value) ? value : NO_NUMBER;
}

//! Whether the item can be asked for the property: a tremolo of one chord fails a check when asked for the style
//! of a tremolo of two
bool answersFor(const EngravingItem* item, Pid pid)
{
    return !(item->isType(ElementType::TREMOLO_SINGLECHORD) && pid == Pid::TREMOLO_STYLE);
}

Record Record::of(const EngravingItem* item, int page, double spatium, bool counted)
{
    const Measure* measure = item->findMeasure();
    const bool hasTrack = item->track() != muse::nidx;
    const PointF pos = item->pagePos();
    const RectF box = item->ldata()->bbox(LD_ACCESS::BAD);

    Record record;
    record.type = item->typeName();
    record.staff = hasTrack ? static_cast<int>(item->staffIdx()) : -1;
    record.measure = measure ? measure->no() + 1 : -1;
    record.tick = item->tick();
    record.track = hasTrack ? static_cast<int>(item->track()) : -1;
    record.page = page;
    record.pos = PointF(staffSpaces(pos.x(), spatium), staffSpaces(pos.y(), spatium));
    record.box = RectF(staffSpaces(box.x(), spatium), staffSpaces(box.y(), spatium),
                       staffSpaces(box.width(), spatium), staffSpaces(box.height(), spatium));
    record.visible = item->visible();
    record.skipDraw = item->ldata()->isSkipDraw();
    record.counted = counted;

    // What the item itself is asked, as the layout asks it: not always what the property of the same name tells
    // (the property of a segment of a line is that of its line, a chord is always visible)
    const PointF offset = item->offset();
    record.properties = {
        { -6, "counted()", yesNo(record.counted), {} },
        { -5, "visible()", yesNo(record.visible), {} },
        { -4, "isSkipDraw()", yesNo(record.skipDraw), {} },
        { -3, "generated()", yesNo(item->generated()), {} },
        { -2, "autoplace()", yesNo(item->autoplace()), {} },
        { -1, "offset()", "(" + twoDecimals(staffSpaces(offset.x(), spatium)) + ", "
          + twoDecimals(staffSpaces(offset.y(), spatium)) + ")", {} },
    };

    // An item answers with no value for a property it does not have, and with the value of the item it is in
    // for a property which only that one has
    for (int id = 0; id < static_cast<int>(Pid::END); ++id) {
        const Pid pid = static_cast<Pid>(id);
        const PropertyValue value = answersFor(item, pid) ? item->getProperty(pid) : PropertyValue();
        if (value.isValid()) {
            const std::string_view flag = flagName(item->propertyFlags(pid));
            record.properties.push_back({ id, propertyName(pid), propertyText(pid, value), flag });
        }
    }
    return record;
}

struct Snapshot
{
    int fileVersion = 0;                // of the file the score was read from
    size_t pageCount = 0;
    std::vector<std::string> style;     // the text of every style value, by Sid
    std::vector<Record> records;        // in the order of their keys, and of where they are for those of one key

    static Snapshot of(MasterScore* score);

    std::string describe() const
    {
        return "read from format " + std::to_string(fileVersion) + ", " + std::to_string(pageCount) + " pages, "
               + std::to_string(records.size()) + " items";
    }
};

using ItemSet = std::unordered_set<const EngravingItem*>;

struct Collector
{
    std::vector<Record>* records = nullptr;
    int page = 0;
    double spatium = 1.0;
    //! What the pages give when they are asked for what they show, which is what the layout fingerprint counts
    ItemSet shown;
};

void collectShown(void* data, EngravingItem* item)
{
    static_cast<ItemSet*>(data)->insert(item);
}

void collect(void* data, EngravingItem* item)
{
    Collector* collector = static_cast<Collector*>(data);

    switch (item->type()) {
    // What the layout fingerprint leaves out: there or not depending on what was edited, or on what is shown
    case ElementType::TIME_TICK_ANCHOR:
    case ElementType::SYSTEM_LOCK_INDICATOR:
    case ElementType::STAFF_VISIBILITY_INDICATOR:
    // What the items are on
    case ElementType::PAGE:
        return;
    default:
        break;
    }
    const bool counted = collector->shown.count(item) > 0;
    collector->records->push_back(Record::of(item, collector->page, collector->spatium, counted));
}

Snapshot Snapshot::of(MasterScore* score)
{
    Snapshot snapshot;
    snapshot.fileVersion = score->mscVersion();
    snapshot.pageCount = score->pages().size();

    for (int id = 0; id < static_cast<int>(Sid::STYLES); ++id) {
        snapshot.style.push_back(valueText(score->style().value(static_cast<Sid>(id))));
    }

    Collector collector { &snapshot.records, 0, score->style().spatium(), {} };
    for (Page* page : score->pages()) {
        page->scanElements(&collector.shown, collectShown, /*all*/ false);
        page->scanElements(&collector, collect, /*all*/ true);
        ++collector.page;
    }

    // stable: the items of one key which are in one place stay in the order the pages give them in
    std::stable_sort(snapshot.records.begin(), snapshot.records.end(), [](const Record& a, const Record& b) {
        if (a.key() != b.key()) {
            return a.key() < b.key();
        }
        return std::make_tuple(a.page, a.pos.x(), a.pos.y()) < std::make_tuple(b.page, b.pos.x(), b.pos.y());
    });
    for (size_t i = 1; i < snapshot.records.size(); ++i) {
        if (snapshot.records[i].key() == snapshot.records[i - 1].key()) {
            snapshot.records[i].ordinal = snapshot.records[i - 1].ordinal + 1;
        }
    }
    return snapshot;
}

//! The score laid out once more, all of it: what a layout which is at its fixed point leaves as it is
Snapshot afterAnotherLayout(MasterScore* score)
{
    score->setLayoutAll();
    score->update();
    return Snapshot::of(score);
}

//---------------------------------------------------------
//   Two snapshots
//---------------------------------------------------------

struct Pairing
{
    std::vector<std::pair<const Record*, const Record*> > pairs;
    std::vector<const Record*> onlyFirst;
    std::vector<const Record*> onlySecond;
};

//! The records of two snapshots which are of one item: those of the same key and of the same ordinal
Pairing pairUp(const Snapshot& first, const Snapshot& second)
{
    const auto rank = [](const Record& record) { return std::make_tuple(record.key(), record.ordinal); };

    Pairing pairing;
    auto a = first.records.begin();
    auto b = second.records.begin();
    while (a != first.records.end() || b != second.records.end()) {
        if (b == second.records.end() || (a != first.records.end() && rank(*a) < rank(*b))) {
            pairing.onlyFirst.push_back(&*a++);
        } else if (a == first.records.end() || rank(*b) < rank(*a)) {
            pairing.onlySecond.push_back(&*b++);
        } else {
            pairing.pairs.push_back({ &*a++, &*b++ });
        }
    }
    return pairing;
}

bool samePlace(const Record& a, const Record& b)
{
    const auto same = [](double x, double y) { return std::abs(x - y) <= SAME_PLACE; };
    return a.page == b.page && same(a.pos.x(), b.pos.x()) && same(a.pos.y(), b.pos.y())
           && same(a.box.x(), b.box.x()) && same(a.box.y(), b.box.y())
           && same(a.box.width(), b.box.width()) && same(a.box.height(), b.box.height());
}

//! One line for each property which the two do not tell the same of, or which only one of them has
std::vector<std::string> propertyDifferences(const Record& first, const Record& second)
{
    std::vector<std::string> lines;
    auto a = first.properties.begin();
    auto b = second.properties.begin();
    while (a != first.properties.end() || b != second.properties.end()) {
        const bool inFirst = a != first.properties.end() && (b == second.properties.end() || a->id <= b->id);
        const bool inSecond = b != second.properties.end() && (a == first.properties.end() || b->id <= a->id);
        const Property* p = inFirst ? &*a++ : nullptr;
        const Property* q = inSecond ? &*b++ : nullptr;
        if (p && q && p->value == q->value && p->flag == q->flag) {
            continue;
        }
        lines.push_back(std::string(p ? p->name : q->name) + ": " + (p ? p->text() : "(none)") + " -> "
                        + (q ? q->text() : "(none)"));
    }
    return lines;
}

//! One pair: where the item is in the two snapshots, and under it what else they do not tell the same of
void printPair(std::ostream& out, const Record& first, const Record& second,
               const std::vector<std::string>& differences)
{
    out << first.keyText() << ": " << first.placeText();
    if (second.placeText() != first.placeText()) {
        out << " -> " << second.placeText();
    }
    if (!first.isDrawn() && !second.isDrawn()) {
        out << " [not drawn]";
    }
    if (!first.counted && !second.counted) {
        out << " [not counted]";
    }
    out << '\n';
    for (const std::string& difference : differences) {
        out << "    " << difference << '\n';
    }
}

struct Findings
{
    size_t onlyFirst = 0;
    size_t onlySecond = 0;
    size_t moved = 0;
    size_t movedNotDrawn = 0;
    size_t movedNotCounted = 0;
    size_t otherProperties = 0;

    size_t ofLayout() const { return onlyFirst + onlySecond + moved; }
};

//! The section of the items of two snapshots. withProperties: also what the items tell differently, which is what
//! there is to look for between a score and the same score opened again, and not between two layouts of one score
Findings reportItems(std::ostream& out, const Snapshot& first, const std::string& firstName, const Snapshot& second,
                     const std::string& secondName, bool withProperties)
{
    const Pairing pairing = pairUp(first, second);
    Findings findings;
    findings.onlyFirst = pairing.onlyFirst.size();
    findings.onlySecond = pairing.onlySecond.size();

    out << "\n== " << (withProperties ? "ITEMS " : "PASSES ") << firstName << " vs " << secondName << " ==\n";

    const auto printUnpaired = [&out](const std::string& name, const std::vector<const Record*>& records) {
        out << "-- ONLY IN " << name << " --\n";
        for (const Record* record : records) {
            out << record->keyText() << ": " << record->placeText() << ", visible " << yesNo(record->visible)
                << ", skipDraw " << yesNo(record->skipDraw) << ", counted " << yesNo(record->counted) << '\n';
        }
    };
    printUnpaired(firstName, pairing.onlyFirst);
    printUnpaired(secondName, pairing.onlySecond);

    out << "-- MOVED --\n";
    std::ostringstream inPlace;     // those which did not move come after those which did
    for (const auto& [a, b] : pairing.pairs) {
        const bool moved = !samePlace(*a, *b);
        const std::vector<std::string> differences = withProperties ? propertyDifferences(*a, *b)
                                                     : std::vector<std::string>();
        if (moved) {
            ++findings.moved;
            findings.movedNotDrawn += !a->isDrawn() && !b->isDrawn() ? 1 : 0;
            findings.movedNotCounted += !a->counted && !b->counted ? 1 : 0;
            printPair(out, *a, *b, differences);
        } else if (!differences.empty()) {
            ++findings.otherProperties;
            printPair(inPlace, *a, *b, differences);
        }
    }
    if (withProperties) {
        out << "-- SAME PLACE, OTHER PROPERTIES --\n" << inPlace.str();
    }

    out << "counts: " << firstName << " " << first.records.size() << " items, " << secondName << " "
        << second.records.size() << " items, only in " << firstName << " " << findings.onlyFirst << ", only in "
        << secondName << " " << findings.onlySecond << ", moved " << findings.moved << " ("
        << findings.movedNotDrawn << " not drawn, " << findings.movedNotCounted << " not counted)";
    if (withProperties) {
        out << ", other properties " << findings.otherProperties;
    }
    out << '\n';
    return findings;
}

//! The section of the style values which are not the same; returns how many
size_t reportStyle(std::ostream& out, const Snapshot& first, const Snapshot& second)
{
    out << "\n== STYLE A vs B ==\n";
    size_t count = 0;
    for (size_t id = 0; id < first.style.size(); ++id) {
        if (first.style[id] != second.style[id]) {
            ++count;
            out << MStyle::valueName(static_cast<Sid>(id)) << ": " << first.style[id] << " -> " << second.style[id]
                << '\n';
        }
    }
    out << "counts: " << count << " of " << first.style.size() << " style values differ\n";
    return count;
}

//! Writes the report of one score; a failure if the score cannot be opened, saved, or opened again
void reportScore(const std::string& name, const std::filesystem::path& directory)
{
    Project original = open(dataPath(VTEST_DIR + String::fromStdString(name)));
    if (!original.score()) {
        ADD_FAILURE() << name << " could not be opened: " << original.ret.toString();
        return;
    }
    const Snapshot a = Snapshot::of(original.score());

    if (!save(original.score(), SAVED_FILE)) {
        ADD_FAILURE() << name << " could not be saved";
        return;
    }
    Project reopened = open(outputPath(SAVED_FILE));
    if (!reopened.score()) {
        ADD_FAILURE() << name << " could not be opened again once saved: " << reopened.ret.toString();
        return;
    }
    const Snapshot b = Snapshot::of(reopened.score());
    const Snapshot b2 = afterAnotherLayout(reopened.score());
    // Only now, so that what was saved is the score as it was opened
    const Snapshot a2 = afterAnotherLayout(original.score());

    const std::filesystem::path reportPath = directory / (name + ".txt");
    std::ofstream report(reportPath);
    report << "REOPEN DIAGNOSTICS " << name << '\n'
           << "A: the file as first opened: " << a.describe() << '\n'
           << "B: A saved and opened again: " << b.describe() << '\n'
           << LEGEND
           << "moved: on another page, or one of the numbers of its place differs by more than "
           << twoDecimals(SAME_PLACE) << '\n';

    const size_t styleCount = reportStyle(report, a, b);
    const Findings items = reportItems(report, a, "A", b, "B", /*withProperties*/ true);
    const Findings passesB = reportItems(report, b, "B", b2, "B2", /*withProperties*/ false);
    const Findings passesA = reportItems(report, a, "A", a2, "A2", /*withProperties*/ false);

    std::ostringstream summary;
    summary << "[ REOPEN ] " << name << ": style " << styleCount << ", onlyA " << items.onlyFirst << ", onlyB "
            << items.onlySecond << ", moved " << items.moved << " (" << items.movedNotDrawn << " not drawn), "
            << "otherProps " << items.otherProperties << ", passesB " << passesB.ofLayout() << ", passesA "
            << passesA.ofLayout();
    report << "\n== SUMMARY ==\n" << summary.str() << '\n';
    report.close();

    std::cout << summary.str() << std::endl;
    // a tool which tells nothing must not look as if it had found nothing
    EXPECT_FALSE(report.fail()) << "the report could not be written: " << reportPath.string();
}
}

class Engraving_ReopenDiagnosticsTests : public ::testing::Test
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

//! A tool, not a test of the scores: it writes a report for each of them, and fails only for a score it cannot
//! open, save and open again. Run it with --gtest_also_run_disabled_tests; EO_REOPEN_SCORES is the list of the
//! files of vtest/scores to look at, separated by commas (not set: those known to differ), EO_REOPEN_OUT the
//! directory of the reports (not set: reopen-reports in the working directory).
TEST_F(Engraving_ReopenDiagnosticsTests, DISABLED_reportScores)
{
    const std::filesystem::path directory = environmentValue("EO_REOPEN_OUT", "reopen-reports");
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    ASSERT_FALSE(error) << directory.string() << ": " << error.message();

    for (const std::string& name : scoreNames()) {
        reportScore(name, directory);
    }
}
