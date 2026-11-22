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
#include "layoutfingerprint.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>

#include "global/serialization/json.h"

#include "dom/engravingitem.h"
#include "dom/excerpt.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/measurebase.h"
#include "dom/page.h"
#include "dom/score.h"
#include "dom/slurtie.h"
#include "dom/spanner.h"
#include "dom/stem.h"
#include "dom/system.h"
#include "dom/tuplet.h"
#include "dom/undo.h"

#include "log.h"

using namespace muse;
using namespace mu::engraving;

using Reason = LayoutFingerprintReport::Reason;

//---------------------------------------------------------
//   Hashing
//---------------------------------------------------------

namespace {
uint64_t mix(uint64_t h)
{
    h ^= h >> 30;
    h *= 0xbf58476d1ce4e5b9ULL;
    h ^= h >> 27;
    h *= 0x94d049bb133111ebULL;
    h ^= h >> 31;
    return h;
}

uint64_t combine(uint64_t h, int64_t value)
{
    return mix(h ^ mix(static_cast<uint64_t>(value) + 0x9e3779b97f4a7c15ULL));
}

//! By name: the numbers of the types are not the same in every version of the program
uint64_t typeSeed(const EngravingItem* item)
{
    uint64_t h = 0xcbf29ce484222325ULL;
    for (const char* c = item->typeName(); c && *c; ++c) {
        h ^= static_cast<unsigned char>(*c);
        h *= 0x100000001b3ULL;
    }
    return h;
}

//! Of the geometry of one item, on the two grids of LayoutContentHash. Their edges are away from the values
//! that come up all the time, whole and half steps and simple fractions of a staff space.
class ItemHash
{
public:
    ItemHash(uint64_t seed, double spatium)
        : m_a(seed), m_b(seed), m_spatium(spatium) {}

    void add(int64_t value)
    {
        m_a = combine(m_a, value);
        m_b = combine(m_b, value);
    }

    void add(double length)
    {
        const double steps = length / m_spatium / LayoutFingerprint::CONTENT_STEP;
        if (!std::isfinite(steps) || std::abs(steps) > 1e15) {
            add(std::numeric_limits<int64_t>::min());
            return;
        }
        m_a = combine(m_a, static_cast<int64_t>(std::floor(steps + EDGE_A)));
        m_b = combine(m_b, static_cast<int64_t>(std::floor(steps + EDGE_B)));
    }

    void add(const PointF& p)
    {
        add(p.x());
        add(p.y());
    }

    uint64_t a() const { return m_a; }
    uint64_t b() const { return m_b; }

private:
    static constexpr double EDGE_A = 0.7113;
    static constexpr double EDGE_B = 0.2113;

    uint64_t m_a = 0;
    uint64_t m_b = 0;
    double m_spatium = 1.0;
};

//! The staff an item stands on, as EngravingItem::pagePos takes it: its own, or that of the item it hangs on.
//! Staff lines, barlines and instrument names are not on a staff in that sense, but are put at the height of the
//! staff they belong to.
staff_idx_t staffOf(const EngravingItem* item)
{
    for (const EngravingItem* e = item; e; e = e->parentItem()) {
        if (e->flag(ElementFlag::ON_STAFF)) {
            const staff_idx_t idx = e->effectiveStaffIdx();
            return idx != muse::nidx ? idx : e->vStaffIdx();
        }
    }
    if (item->isStaffLines() || item->isBarLine() || item->isInstrumentName()) {
        return item->staffIdx();
    }
    return muse::nidx;
}

//! What the items of a measure, or those of a system itself, add up to
struct ContentCollector
{
    const System* system = nullptr;
    PointF origin;      // of the measure or the system, on the page
    double spatium = 1.0;
    LayoutContentHash hash;
};

void collectItem(void* data, EngravingItem* item)
{
    ContentCollector* collector = static_cast<ContentCollector*>(data);

    switch (item->type()) {
    // Not part of the layout: there or not depending on what was edited, or on what is shown
    case ElementType::TIME_TICK_ANCHOR:
    case ElementType::SYSTEM_LOCK_INDICATOR:
    case ElementType::STAFF_VISIBILITY_INDICATOR:
        return;
    default:
        break;
    }

    // Relative to the staff, so that a staff that moved leaves the items on it where they were
    double staffY = 0.0;
    const staff_idx_t staffIdx = staffOf(item);
    if (staffIdx != muse::nidx && staffIdx < collector->system->staves().size()) {
        staffY = collector->system->staff(staffIdx)->y();
    }

    const PointF pagePos = item->pagePos();
    const RectF bbox = item->ldata()->bbox(LD_ACCESS::BAD);

    ItemHash hash(typeSeed(item), collector->spatium);
    hash.add(item->track() == muse::nidx ? int64_t(-1) : static_cast<int64_t>(item->track()));
    hash.add(pagePos.x() - collector->origin.x());
    hash.add(pagePos.y() - collector->origin.y() - staffY);
    hash.add(bbox.x());
    hash.add(bbox.y());
    hash.add(bbox.width());
    hash.add(bbox.height());

    // What the rectangle does not tell
    if (item->isSpannerSegment()) {
        hash.add(toSpannerSegment(item)->pos2());
    }
    if (item->isSlurTieSegment()) {
        const SlurTieSegment* segment = toSlurTieSegment(item);
        for (Grip grip : { Grip::START, Grip::BEZIER1, Grip::BEZIER2, Grip::END }) {
            hash.add(segment->ups(grip).pos());
        }
    } else if (item->isStem()) {
        hash.add(toStem(item)->length());
    } else if (item->isTuplet()) {
        hash.add(toTuplet(item)->p1());
        hash.add(toTuplet(item)->p2());
    }

    // Added up, so that the order the items come in does not matter
    collector->hash.a += hash.a();
    collector->hash.b += hash.b();
}

int toSteps(double length, double spatium)
{
    const double steps = length / spatium / LayoutFingerprint::POSITION_STEP;
    if (!std::isfinite(steps)) {
        return 0;
    }
    return static_cast<int>(std::clamp(std::round(steps), -2.0e9, 2.0e9));
}

double toStaffSpaces(int steps)
{
    return steps * LayoutFingerprint::POSITION_STEP;
}
}

//---------------------------------------------------------
//   LayoutFingerprint
//---------------------------------------------------------

int LayoutFingerprint::measureCount() const
{
    return systems.empty() ? 0 : systems.back().firstMeasure + static_cast<int>(systems.back().measures.size());
}

bool LayoutFingerprint::isAvailableFor(const Score* score)
{
    return score && score->isLayoutMode(LayoutMode::PAGE) && score->isLayoutUpToDate();
}

LayoutFingerprint LayoutFingerprint::of(Score* score)
{
    TRACEFUNC;

    LayoutFingerprint fingerprint;
    fingerprint.layoutMode = score->layoutMode();

    const double spatium = score->style().spatium();
    fingerprint.spatium = static_cast<int>(std::clamp(std::round(spatium * 1000.0), 0.0, 2.0e9));
    if (!(spatium > 0.0)) {
        return fingerprint;
    }

    int pageIndex = 0;
    int measureIndex = 0;
    for (const Page* page : score->pages()) {
        fingerprint.pageSystems.push_back(static_cast<int>(page->systems().size()));

        for (System* system : page->systems()) {
            SystemData systemData;
            systemData.page = pageIndex;
            systemData.firstMeasure = measureIndex;
            systemData.y = toSteps(system->y(), spatium);
            systemData.height = toSteps(system->height(), spatium);
            for (const SysStaff* staff : system->staves()) {
                systemData.staffY.push_back(staff->show() ? toSteps(staff->y(), spatium) : HIDDEN_STAFF);
            }

            const PointF systemPos = system->pagePos();

            // As Page::scanElements goes through what is on a page
            for (MeasureBase* measure : system->measures()) {
                MeasureData measureData;
                measureData.x = toSteps(measure->x(), spatium);
                measureData.width = toSteps(measure->width(), spatium);

                ContentCollector collector { system, PointF(measure->pagePos().x(), systemPos.y()), spatium, {} };
                measure->scanElements(&collector, collectItem, false);
                measureData.content = collector.hash;

                systemData.measures.push_back(measureData);
            }

            ContentCollector collector { system, systemPos, spatium, {} };
            system->scanElements(&collector, collectItem, false);
            systemData.content = collector.hash;

            measureIndex += static_cast<int>(systemData.measures.size());
            fingerprint.systems.push_back(std::move(systemData));
        }
        ++pageIndex;
    }

    return fingerprint;
}

//---------------------------------------------------------
//   LayoutFingerprintFile
//
//   {
//     "format": 1,
//     "programVersion": "4.6.5", "programRevision": "abc1234",
//     "scores": [ {
//       "part": "",                       file name of the part's excerpt; "" for the master score
//       "layoutMode": "page",
//       "spatium": 24800,                 see LayoutFingerprint::spatium
//       "pages": [5, 6],                  systems on each page
//       "systems": [ {
//         "p": 0,                         index of the page
//         "m": 0,                         index of the first measure
//         "y": 10500, "h": 14000,         on the page; in thousandths of a staff space, like all that follows
//         "s": [0, 9500, null],           y of each staff in the system; null: not shown
//         "c": "0123…",                   hash of the system's own items: 32 hexadecimal digits, a then b
//         "mm": [[0, 21034, "0123…"], …]  each measure: x in the system, width, hash of what is in it
//       }, … ]
//     }, … ]
//   }
//---------------------------------------------------------

namespace {
struct ModeName {
    LayoutMode mode;
    const char* name;
};

const ModeName MODE_NAMES[] = {
    { LayoutMode::PAGE, "page" },
    { LayoutMode::FLOAT, "float" },
    { LayoutMode::LINE, "line" },
    { LayoutMode::SYSTEM, "system" },
    { LayoutMode::HORIZONTAL_FIXED, "horizontalFixed" },
};

std::string hashToString(const LayoutContentHash& hash)
{
    static const char DIGITS[] = "0123456789abcdef";
    std::string text(32, '0');
    for (int i = 0; i < 16; ++i) {
        text[15 - i] = DIGITS[(hash.a >> (4 * i)) & 0xf];
        text[31 - i] = DIGITS[(hash.b >> (4 * i)) & 0xf];
    }
    return text;
}

bool readHash(const JsonValue& value, LayoutContentHash& hash)
{
    if (!value.isString()) {
        return false;
    }
    const std::string& text = value.toStdString();
    if (text.size() != 32) {
        return false;
    }
    uint64_t parts[2] = { 0, 0 };
    for (size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        uint64_t digit = 0;
        if (c >= '0' && c <= '9') {
            digit = static_cast<uint64_t>(c - '0');
        } else if (c >= 'a' && c <= 'f') {
            digit = static_cast<uint64_t>(c - 'a' + 10);
        } else {
            return false;
        }
        parts[i / 16] = (parts[i / 16] << 4) | digit;
    }
    hash.a = parts[0];
    hash.b = parts[1];
    return true;
}

bool readInt(const JsonValue& value, int& result)
{
    if (!value.isNumber()) {
        return false;
    }
    const double number = value.toDouble();
    if (!std::isfinite(number) || std::abs(number) > 2.0e9 || number != std::floor(number)) {
        return false;
    }
    result = static_cast<int>(number);
    return true;
}

bool readSystem(const JsonValue& value, int measureIndex, LayoutFingerprint::SystemData& system)
{
    if (!value.isObject()) {
        return false;
    }
    const JsonObject object = value.toObject();

    if (!readInt(object.value("p"), system.page) || !readInt(object.value("m"), system.firstMeasure)
        || !readInt(object.value("y"), system.y) || !readInt(object.value("h"), system.height)
        || !readHash(object.value("c"), system.content)) {
        return false;
    }
    if (system.page < 0 || system.firstMeasure != measureIndex) {
        return false;
    }

    const JsonValue staves = object.value("s");
    const JsonValue measures = object.value("mm");
    if (!staves.isArray() || !measures.isArray()) {
        return false;
    }

    const JsonArray staffArray = staves.toArray();
    for (size_t i = 0; i < staffArray.size(); ++i) {
        const JsonValue staff = staffArray.at(i);
        int y = LayoutFingerprint::HIDDEN_STAFF;
        if (!staff.isNull() && !readInt(staff, y)) {
            return false;
        }
        system.staffY.push_back(y);
    }

    const JsonArray measureArray = measures.toArray();
    for (size_t i = 0; i < measureArray.size(); ++i) {
        const JsonValue measure = measureArray.at(i);
        if (!measure.isArray()) {
            return false;
        }
        const JsonArray fields = measure.toArray();
        LayoutFingerprint::MeasureData measureData;
        if (fields.size() != 3 || !readInt(fields.at(0), measureData.x) || !readInt(fields.at(1), measureData.width)
            || !readHash(fields.at(2), measureData.content)) {
            return false;
        }
        system.measures.push_back(measureData);
    }

    return true;
}

bool readScore(const JsonValue& value, LayoutFingerprintFile::Entry& entry)
{
    if (!value.isObject()) {
        return false;
    }
    const JsonObject object = value.toObject();

    const JsonValue part = object.value("part");
    const JsonValue mode = object.value("layoutMode");
    const JsonValue pages = object.value("pages");
    const JsonValue systems = object.value("systems");
    LayoutFingerprint& fingerprint = entry.fingerprint;
    if (!part.isString() || !mode.isString() || !pages.isArray() || !systems.isArray()
        || !readInt(object.value("spatium"), fingerprint.spatium)) {
        return false;
    }
    entry.part = part.toString();

    const ModeName* modeName = std::find_if(std::begin(MODE_NAMES), std::end(MODE_NAMES), [&mode](const ModeName& m) {
        return mode.toStdString() == m.name;
    });
    if (modeName == std::end(MODE_NAMES)) {
        return false;
    }
    fingerprint.layoutMode = modeName->mode;

    int systemCount = 0;
    const JsonArray pageArray = pages.toArray();
    for (size_t i = 0; i < pageArray.size(); ++i) {
        int count = 0;
        if (!readInt(pageArray.at(i), count) || count < 0) {
            return false;
        }
        fingerprint.pageSystems.push_back(count);
        systemCount += count;
    }

    const JsonArray systemArray = systems.toArray();
    if (systemArray.size() != static_cast<size_t>(systemCount)) {
        return false;
    }
    int measureIndex = 0;
    for (size_t i = 0; i < systemArray.size(); ++i) {
        LayoutFingerprint::SystemData system;
        if (!readSystem(systemArray.at(i), measureIndex, system)
            || static_cast<size_t>(system.page) >= fingerprint.pageSystems.size()) {
            return false;
        }
        measureIndex += static_cast<int>(system.measures.size());
        fingerprint.systems.push_back(std::move(system));
    }

    return true;
}
}

const LayoutFingerprint* LayoutFingerprintFile::find(const String& part) const
{
    for (const Entry& entry : scores) {
        if (entry.part == part) {
            return &entry.fingerprint;
        }
    }
    return nullptr;
}

ByteArray LayoutFingerprintFile::toJson() const
{
    TRACEFUNC;

    JsonArray scoreArray;
    for (const Entry& entry : scores) {
        const LayoutFingerprint& fingerprint = entry.fingerprint;

        JsonArray pageArray;
        for (int count : fingerprint.pageSystems) {
            pageArray.append(count);
        }

        JsonArray systemArray;
        for (const LayoutFingerprint::SystemData& system : fingerprint.systems) {
            JsonArray staffArray;
            for (int y : system.staffY) {
                if (y == LayoutFingerprint::HIDDEN_STAFF) {
                    JsonValue hidden;
                    hidden.setNull();
                    staffArray.append(hidden);
                } else {
                    staffArray.append(y);
                }
            }

            JsonArray measureArray;
            for (const LayoutFingerprint::MeasureData& measure : system.measures) {
                JsonArray fields;
                fields.append(measure.x);
                fields.append(measure.width);
                fields.append(hashToString(measure.content));
                measureArray.append(fields);
            }

            JsonObject systemObject;
            systemObject.set("p", system.page);
            systemObject.set("m", system.firstMeasure);
            systemObject.set("y", system.y);
            systemObject.set("h", system.height);
            systemObject.set("s", staffArray);
            systemObject.set("c", hashToString(system.content));
            systemObject.set("mm", measureArray);
            systemArray.append(systemObject);
        }

        const ModeName* modeName = std::find_if(std::begin(MODE_NAMES), std::end(MODE_NAMES), [&fingerprint](const ModeName& m) {
            return fingerprint.layoutMode == m.mode;
        });

        JsonObject scoreObject;
        scoreObject.set("part", entry.part);
        scoreObject.set("layoutMode", modeName != std::end(MODE_NAMES) ? modeName->name : "");
        scoreObject.set("spatium", fingerprint.spatium);
        scoreObject.set("pages", pageArray);
        scoreObject.set("systems", systemArray);
        scoreArray.append(scoreObject);
    }

    JsonObject root;
    root.set("format", format);
    root.set("programVersion", programVersion);
    root.set("programRevision", programRevision);
    root.set("scores", scoreArray);

    return JsonDocument(root).toJson(JsonDocument::Format::Compact);
}

LayoutFingerprintFile::ReadResult LayoutFingerprintFile::fromJson(const ByteArray& json, LayoutFingerprintFile& file)
{
    TRACEFUNC;

    file = LayoutFingerprintFile();

    std::string error;
    const JsonDocument document = JsonDocument::fromJson(json, &error);
    if (!error.empty() || !document.isObject()) {
        return ReadResult::Malformed;
    }
    const JsonObject root = document.rootObject();

    if (!readInt(root.value("format"), file.format)) {
        return ReadResult::Malformed;
    }
    if (file.format != FORMAT) {
        return ReadResult::UnknownFormat;
    }

    const JsonValue version = root.value("programVersion");
    const JsonValue revision = root.value("programRevision");
    const JsonValue scoreValues = root.value("scores");
    if (!version.isString() || !revision.isString() || !scoreValues.isArray()) {
        return ReadResult::Malformed;
    }
    file.programVersion = version.toString();
    file.programRevision = revision.toString();

    const JsonArray scoreArray = scoreValues.toArray();
    for (size_t i = 0; i < scoreArray.size(); ++i) {
        Entry entry;
        if (!readScore(scoreArray.at(i), entry) || file.find(entry.part)) {
            file = LayoutFingerprintFile();
            return ReadResult::Malformed;
        }
        file.scores.push_back(std::move(entry));
    }

    return ReadResult::Ok;
}

//---------------------------------------------------------
//   LayoutDifference, LayoutFingerprintReport
//---------------------------------------------------------

const char* LayoutDifference::kindName(Kind kind)
{
    switch (kind) {
    case Kind::Spatium: return "spatium";
    case Kind::PageCount: return "pageCount";
    case Kind::MeasureCount: return "measureCount";
    case Kind::SystemBreak: return "systemBreak";
    case Kind::PageBreak: return "pageBreak";
    case Kind::SystemPosition: return "systemPosition";
    case Kind::SystemHeight: return "systemHeight";
    case Kind::StaffPosition: return "staffPosition";
    case Kind::MeasurePosition: return "measurePosition";
    case Kind::MeasureWidth: return "measureWidth";
    case Kind::MeasureContent: return "measureContent";
    case Kind::SystemContent: return "systemContent";
    }
    return "";
}

const char* LayoutFingerprintReport::reasonName(Reason reason)
{
    switch (reason) {
    case Reason::None: return "none";
    case Reason::NotChecked: return "notChecked";
    case Reason::NoFingerprint: return "noFingerprint";
    case Reason::Malformed: return "malformed";
    case Reason::UnknownFormat: return "unknownFormat";
    case Reason::LayoutModeDiffers: return "layoutModeDiffers";
    case Reason::NotLaidOut: return "notLaidOut";
    case Reason::ChangedBeforeLayout: return "changedBeforeLayout";
    }
    return "";
}

//---------------------------------------------------------
//   compare
//---------------------------------------------------------

namespace {
constexpr int TOLERANCE_STEPS = static_cast<int>(LayoutFingerprintReport::TOLERANCE / LayoutFingerprint::POSITION_STEP + 0.5);

bool differs(int saved, int current)
{
    return std::abs(static_cast<int64_t>(saved) - static_cast<int64_t>(current)) > TOLERANCE_STEPS;
}

//! Whether something stands elsewhere, apart from what the shift of the thing before it explains
bool movedOnItsOwn(int saved, int current, int64_t shiftBefore)
{
    return std::abs(static_cast<int64_t>(current) - static_cast<int64_t>(saved) - shiftBefore) > TOLERANCE_STEPS;
}

//! Where the measures and systems of the layout of now are, for telling where a difference is
class Places
{
public:
    Places(const Score* score, const LayoutFingerprint& current, const String& part)
        : m_current(current), m_part(part)
    {
        for (const Page* page : score->pages()) {
            for (const System* system : page->systems()) {
                for (const MeasureBase* measure : system->measures()) {
                    m_measureNumbers.push_back(measure->isMeasure() ? measure->no() + 1 : 0);
                }
            }
        }
        for (size_t i = 0; i < current.systems.size(); ++i) {
            for (size_t k = 0; k < current.systems[i].measures.size(); ++k) {
                m_systemOfMeasure.push_back(static_cast<int>(i));
            }
        }
    }

    LayoutDifference atScore(LayoutDifference::Kind kind, double saved, double current) const
    {
        LayoutDifference difference;
        difference.kind = kind;
        difference.part = m_part;
        difference.saved = saved;
        difference.current = current;
        return difference;
    }

    LayoutDifference atSystem(LayoutDifference::Kind kind, int system, double saved, double current) const
    {
        LayoutDifference difference = atScore(kind, saved, current);
        if (system >= 0 && static_cast<size_t>(system) < m_current.systems.size()) {
            difference.system = system;
            difference.page = m_current.systems[system].page;
        }
        return difference;
    }

    LayoutDifference atMeasure(LayoutDifference::Kind kind, int measure, double saved, double current) const
    {
        const bool known = measure >= 0 && static_cast<size_t>(measure) < m_systemOfMeasure.size();
        LayoutDifference difference = atSystem(kind, known ? m_systemOfMeasure[measure] : -1, saved, current);
        difference.measure = measure;
        if (known && static_cast<size_t>(measure) < m_measureNumbers.size()) {
            difference.measureNumber = m_measureNumbers[measure];
        }
        return difference;
    }

private:
    const LayoutFingerprint& m_current;
    String m_part;
    std::vector<int> m_measureNumbers;
    std::vector<int> m_systemOfMeasure;
};
}

std::vector<LayoutDifference> LayoutFingerprintCheck::compare(const LayoutFingerprint& saved, const LayoutFingerprint& current,
                                                              const Score* score, const String& part)
{
    TRACEFUNC;

    using Kind = LayoutDifference::Kind;

    std::vector<LayoutDifference> differences;
    const Places places(score, current, part);

    if (saved.spatium != current.spatium) {
        differences.push_back(places.atScore(Kind::Spatium, saved.spatium / 1000.0, current.spatium / 1000.0));
    }
    if (saved.pageSystems.size() != current.pageSystems.size()) {
        differences.push_back(places.atScore(Kind::PageCount, static_cast<double>(saved.pageSystems.size()),
                                             static_cast<double>(current.pageSystems.size())));
    }
    if (saved.measureCount() != current.measureCount()) {
        differences.push_back(places.atScore(Kind::MeasureCount, saved.measureCount(), current.measureCount()));
    }

    // Systems are known by the measure they begin with
    std::map<int, const LayoutFingerprint::SystemData*> savedSystems;
    std::set<int> starts;
    for (const LayoutFingerprint::SystemData& system : saved.systems) {
        savedSystems[system.firstMeasure] = &system;
        starts.insert(system.firstMeasure);
    }
    std::set<int> currentStarts;
    for (const LayoutFingerprint::SystemData& system : current.systems) {
        currentStarts.insert(system.firstMeasure);
        starts.insert(system.firstMeasure);
    }
    for (int measure : starts) {
        const bool wasStart = savedSystems.count(measure) > 0;
        const bool isStart = currentStarts.count(measure) > 0;
        if (wasStart != isStart) {
            differences.push_back(places.atMeasure(Kind::SystemBreak, measure, wasStart ? 1.0 : 0.0, isStart ? 1.0 : 0.0));
        }
    }

    // What the system above, on the same page of both layouts, moved and grew by
    int pageAbove = -1;
    int64_t shiftAbove = 0;

    for (size_t i = 0; i < current.systems.size(); ++i) {
        const LayoutFingerprint::SystemData& now = current.systems[i];
        const int systemIndex = static_cast<int>(i);

        auto found = savedSystems.find(now.firstMeasure);
        if (found == savedSystems.end() || found->second->measures.size() != now.measures.size()) {
            // Other measures than it had: its measures cannot stand as they did
            pageAbove = -1;
            continue;
        }
        const LayoutFingerprint::SystemData& before = *found->second;

        if (before.page != now.page) {
            differences.push_back(places.atSystem(Kind::PageBreak, systemIndex, before.page, now.page));
            pageAbove = -1;
        } else {
            if (pageAbove != now.page) {
                shiftAbove = 0;
            }
            if (movedOnItsOwn(before.y, now.y, shiftAbove)) {
                differences.push_back(places.atSystem(Kind::SystemPosition, systemIndex, toStaffSpaces(before.y),
                                                      toStaffSpaces(now.y)));
            }
            pageAbove = now.page;
            shiftAbove = (static_cast<int64_t>(now.y) - before.y) + (static_cast<int64_t>(now.height) - before.height);
        }

        if (differs(before.height, now.height)) {
            differences.push_back(places.atSystem(Kind::SystemHeight, systemIndex, toStaffSpaces(before.height),
                                                  toStaffSpaces(now.height)));
        }

        // Staves: each against what the staff above it moved by
        int64_t staffShift = 0;
        const size_t staffCount = std::max(before.staffY.size(), now.staffY.size());
        for (size_t staff = 0; staff < staffCount; ++staff) {
            const int wasY = staff < before.staffY.size() ? before.staffY[staff] : LayoutFingerprint::HIDDEN_STAFF;
            const int isY = staff < now.staffY.size() ? now.staffY[staff] : LayoutFingerprint::HIDDEN_STAFF;
            const bool wasShown = wasY != LayoutFingerprint::HIDDEN_STAFF;
            const bool isShown = isY != LayoutFingerprint::HIDDEN_STAFF;
            if (!wasShown && !isShown) {
                continue;
            }
            if (wasShown != isShown || movedOnItsOwn(wasY, isY, staffShift)) {
                const double nan = std::numeric_limits<double>::quiet_NaN();
                LayoutDifference difference = places.atSystem(Kind::StaffPosition, systemIndex,
                                                              wasShown ? toStaffSpaces(wasY) : nan,
                                                              isShown ? toStaffSpaces(isY) : nan);
                difference.staff = static_cast<int>(staff);
                differences.push_back(difference);
            }
            if (wasShown && isShown) {
                staffShift = static_cast<int64_t>(isY) - wasY;
            }
        }

        if (!before.content.matches(now.content)) {
            differences.push_back(places.atSystem(Kind::SystemContent, systemIndex, 0.0, 0.0));
        }

        // Measures: each against what the measure before it moved and grew by
        int64_t shiftBefore = 0;
        for (size_t k = 0; k < now.measures.size(); ++k) {
            const LayoutFingerprint::MeasureData& was = before.measures[k];
            const LayoutFingerprint::MeasureData& is = now.measures[k];
            const int measureIndex = now.firstMeasure + static_cast<int>(k);

            if (movedOnItsOwn(was.x, is.x, shiftBefore)) {
                differences.push_back(places.atMeasure(Kind::MeasurePosition, measureIndex, toStaffSpaces(was.x),
                                                       toStaffSpaces(is.x)));
            }
            if (differs(was.width, is.width)) {
                differences.push_back(places.atMeasure(Kind::MeasureWidth, measureIndex, toStaffSpaces(was.width),
                                                       toStaffSpaces(is.width)));
            }
            if (!was.content.matches(is.content)) {
                differences.push_back(places.atMeasure(Kind::MeasureContent, measureIndex, 0.0, 0.0));
            }
            shiftBefore = (static_cast<int64_t>(is.x) - was.x) + (static_cast<int64_t>(is.width) - was.width);
        }
    }

    return differences;
}

//---------------------------------------------------------
//   LayoutFingerprintCheck
//---------------------------------------------------------

LayoutFingerprintCheck::LayoutFingerprintCheck(const muse::modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx)
{
}

void LayoutFingerprintCheck::setSaved(const ByteArray& json)
{
    m_saved.reset();
    m_noSavedReason = Reason::NoFingerprint;
    if (json.empty()) {
        return;
    }

    LayoutFingerprintFile file;
    switch (LayoutFingerprintFile::fromJson(json, file)) {
    case LayoutFingerprintFile::ReadResult::Ok:
        m_saved = std::move(file);
        break;
    case LayoutFingerprintFile::ReadResult::UnknownFormat:
        m_noSavedReason = Reason::UnknownFormat;
        LOGI() << "The layout fingerprint of the file is of format " << file.format << ", which this build does not know";
        break;
    case LayoutFingerprintFile::ReadResult::Malformed:
        m_noSavedReason = Reason::Malformed;
        LOGW() << "The layout fingerprint of the file cannot be read and is left aside";
        break;
    }
}

void LayoutFingerprintCheck::start(MasterScore* masterScore)
{
    TRACEFUNC;

    for (const auto& pending : m_pending) {
        // Of an earlier start; scores that are no more are not touched
        for (Score* score : masterScore->scoreList()) {
            if (score == pending.first) {
                score->m_layoutFingerprintCheckPending = false;
            }
        }
    }
    m_pending.clear();
    m_report = LayoutFingerprintReport();

    if (m_saved) {
        m_report.savedProgramVersion = m_saved->programVersion;
        m_report.savedProgramRevision = m_saved->programRevision;
        m_report.sameProgram = application()
                               && m_saved->programVersion == application()->version().toString()
                               && m_saved->programRevision == application()->revision();
    }

    for (Score* score : masterScore->scoreList()) {
        LayoutFingerprintReport::ScoreState state;
        Excerpt* excerpt = score->isMaster() ? nullptr : score->excerpt();
        if (excerpt) {
            state.part = excerpt->fileName();
            state.partName = excerpt->name();
        }
        const size_t index = m_report.scores.size();
        m_report.scores.push_back(state);

        if (!m_saved) {
            m_report.scores[index].reason = m_noSavedReason;
        } else if ((!score->isMaster() && state.part.empty()) || !m_saved->find(state.part)) {
            m_report.scores[index].reason = Reason::NoFingerprint;
        } else if (score->isLayoutUpToDate()) {
            compareScore(score, index);
        } else {
            m_report.scores[index].reason = Reason::NotLaidOut;
            m_pending.emplace_back(score, index);
            score->m_layoutFingerprintCheckPending = true;
        }
    }

    if (!m_report.scores.empty()) {
        m_report.comparable = m_report.scores.front().comparable;
        m_report.reason = m_report.scores.front().reason;
    }
    if (m_pending.empty()) {
        m_saved.reset();
    }

    m_reportChanged.notify();
}

void LayoutFingerprintCheck::scoreLaidOut(Score* score)
{
    auto pending = std::find_if(m_pending.begin(), m_pending.end(), [score](const std::pair<const Score*, size_t>& p) {
        return p.first == score;
    });
    if (pending == m_pending.end() || !m_saved) {
        score->m_layoutFingerprintCheckPending = false;
        return;
    }

    const MasterScore* masterScore = score->masterScore();
    const UndoStack* undoStack = masterScore ? masterScore->undoStack() : nullptr;
    if (undoStack && undoStack->hasActiveCommand()) {
        // A layout in the middle of a command is of a score that is being changed: wait for the next
        return;
    }

    const size_t index = pending->second;
    m_pending.erase(pending);
    score->m_layoutFingerprintCheckPending = false;

    if (undoStack && (undoStack->canUndo() || undoStack->canRedo())) {
        // Laid out for the first time only now, from a score that is no longer the one that was saved
        m_report.scores[index].comparable = false;
        m_report.scores[index].reason = Reason::ChangedBeforeLayout;
    } else {
        compareScore(score, index);
    }

    if (index == 0) {
        m_report.comparable = m_report.scores.front().comparable;
        m_report.reason = m_report.scores.front().reason;
    }
    if (m_pending.empty()) {
        m_saved.reset();
    }

    m_reportChanged.notify();
}

void LayoutFingerprintCheck::compareScore(Score* score, size_t stateIndex)
{
    TRACEFUNC;

    LayoutFingerprintReport::ScoreState& state = m_report.scores[stateIndex];
    const LayoutFingerprint* saved = m_saved ? m_saved->find(state.part) : nullptr;
    IF_ASSERT_FAILED(saved) {
        state.reason = Reason::NoFingerprint;
        return;
    }

    if (saved->layoutMode != score->layoutMode()) {
        state.comparable = false;
        state.reason = Reason::LayoutModeDiffers;
        return;
    }

    const std::vector<LayoutDifference> differences = compare(*saved, LayoutFingerprint::of(score), score, state.part);

    state.comparable = true;
    state.reason = Reason::None;
    state.differenceCount = differences.size();
    if (differences.empty()) {
        return;
    }

    const LayoutDifference& first = differences.front();
    LOGW() << "The layout of " << (state.part.empty() ? String(u"the score") : u"the part " + state.partName)
           << " is not the layout the file was saved with: " << differences.size() << " differences, the first "
           << LayoutDifference::kindName(first.kind)
           << " on page " << first.page + 1 << ", system " << first.system + 1 << ", measure " << first.measureNumber
           << " (saved " << first.saved << ", now " << first.current << "); saved by "
           << m_report.savedProgramVersion << " (" << m_report.savedProgramRevision << "), "
           << (m_report.sameProgram ? "which is this program" : "which is not this program");

    m_report.differences.insert(m_report.differences.end(), differences.begin(), differences.end());
}
