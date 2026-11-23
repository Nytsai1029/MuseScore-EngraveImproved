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
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "global/io/buffer.h"

#include "compat/engravingcompat.h"
#include "compat/mscxcompat.h"
#include "dom/chord.h"
#include "dom/excerpt.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/mscore.h"
#include "dom/note.h"
#include "dom/page.h"
#include "dom/part.h"
#include "dom/segment.h"
#include "dom/staff.h"
#include "dom/system.h"
#include "engravingproject.h"
#include "infrastructure/layoutfingerprint.h"
#include "infrastructure/mscreader.h"
#include "infrastructure/mscwriter.h"
#include "officialexport/officiallayoutbaker.h"
#include "rw/mscsaver.h"
#include "rw/write/writecontext.h"
#include "style/styledef.h"
#include "types/propertyvalue.h"

#include "utils/scorerw.h"

using namespace mu;
using namespace muse;
using namespace muse::io;
using namespace mu::engraving;

using Kind = LayoutDifference::Kind;
using Reason = LayoutFingerprintReport::Reason;

namespace {
const String CONSISTENCY_DIR(u"layoutconsistency_data/");
const String VTEST_DIR(u"../../../vtest/scores/");
const String SCORE_WITH_PARTS(u"parts_data/input-from-parts.mscz");
//! Three systems of one staff
const String SYSTEMS_SCORE = CONSISTENCY_DIR + u"articulations.mscx";
//! Two instruments of one staff each, a text above the first staff and a dynamic below the second (so that nothing
//! of one staff comes near the other), without the barline at the start of a system that
//! joins the staves: nothing in a measure stretches from one staff to the other
const String TWO_INSTRUMENTS_SCORE(u"layoutfingerprint_data/twoInstruments.mscx");

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
    const LayoutFingerprintReport& report() const { return project->masterScore()->layoutFingerprintReport(); }
};

//! Locked updates as the application holds the layout back until all is set up, and the compat steps of
//! NotationProject::doLoad before and after the first layout (its style migration dialog aside)
Project open(const String& path)
{
    Project result;
    result.project = EngravingProject::create(nullptr);
    result.ret = compat::loadMsczOrMscx(result.project, path, false);
    if (!result.ret) {
        result.project = nullptr;
        return result;
    }

    MasterScore* score = result.project->masterScore();
    score->lockUpdates(true);
    result.ret = result.project->setupMasterScore(true);
    if (!result.ret) {
        result.project = nullptr;
        score->lockUpdates(false);
        return result;
    }

    compat::EngravingCompat::doPreLayoutCompatIfNeeded(score);
    score->lockUpdates(false);
    score->setLayoutAll();
    score->update();
    compat::EngravingCompat::doPostLayoutCompatIfNeeded(score);
    return result;
}

bool save(MasterScore* score, const String& fileName, const write::WriteRange* range = nullptr)
{
    MscWriter::Params params;
    params.filePath = fileName;
    params.mode = MscIoMode::Zip;

    MscWriter writer(params);
    if (!writer.open()) {
        return false;
    }
    MscSaver saver(score->iocContext());
    const bool ok = saver.writeMscz(score, writer, false, range);
    writer.close();
    return ok && !writer.hasError();
}

//! What a saved file holds, to write it again with something changed
struct Container
{
    struct Part {
        String name;
        ByteArray style;
        ByteArray score;
    };

    ByteArray style;
    ByteArray score;
    ByteArray fingerprint;
    std::vector<Part> parts;
};

Container readContainer(const String& fileName)
{
    MscReader::Params params;
    params.filePath = fileName;
    params.mode = MscIoMode::Zip;
    MscReader reader(params);
    EXPECT_TRUE(reader.open());

    Container container;
    container.style = reader.readStyleFile();
    container.score = reader.readScoreFile();
    container.fingerprint = reader.readLayoutFingerprintFile();
    for (const String& name : reader.excerptFileNames()) {
        container.parts.push_back({ name, reader.readExcerptStyleFile(name), reader.readExcerptFile(name) });
    }
    return container;
}

String writeContainer(const Container& container, const String& fileName)
{
    MscWriter::Params params;
    params.filePath = fileName;
    params.mode = MscIoMode::Zip;

    MscWriter writer(params);
    EXPECT_TRUE(writer.open());
    writer.writeStyleFile(container.style);
    writer.writeScoreFile(container.score);
    for (const Container::Part& part : container.parts) {
        writer.addExcerptStyleFile(part.name, part.style);
        writer.addExcerptFile(part.name, part.score);
    }
    if (!container.fingerprint.empty()) {
        writer.writeLayoutFingerprintFile(container.fingerprint);
    }
    writer.close();
    EXPECT_FALSE(writer.hasError());
    return outputPath(fileName);
}

ByteArray toBytes(const std::string& text)
{
    return ByteArray(text.c_str(), text.size());
}

std::string toText(const ByteArray& data)
{
    return std::string(data.constChar(), data.size());
}

//! `<tag>value</tag>` with another value
bool setXmlValue(ByteArray& xml, const std::string& tag, const std::string& value)
{
    std::string text = toText(xml);
    const std::string opening = "<" + tag + ">";
    const size_t start = text.find(opening);
    const size_t end = text.find("</" + tag + ">");
    if (start == std::string::npos || end == std::string::npos) {
        return false;
    }
    text.replace(start + opening.size(), end - start - opening.size(), value);
    xml = toBytes(text);
    return true;
}

std::vector<const LayoutDifference*> differencesOf(const LayoutFingerprintReport& report, Kind kind)
{
    std::vector<const LayoutDifference*> result;
    for (const LayoutDifference& difference : report.differences) {
        if (difference.kind == kind) {
            result.push_back(&difference);
        }
    }
    return result;
}

std::string describe(const LayoutFingerprintReport& report)
{
    std::string text = std::string("comparable ") + (report.comparable ? "yes" : "no") + ", reason "
                       + LayoutFingerprintReport::reasonName(report.reason) + ", " + std::to_string(report.differences.size())
                       + " differences";
    size_t shown = 0;
    for (const LayoutDifference& d : report.differences) {
        if (++shown > 12) {
            text += "\n  …";
            break;
        }
        text += std::string("\n  ") + LayoutDifference::kindName(d.kind) + " part '" + d.part.toStdString() + "' page "
                + std::to_string(d.page) + " system " + std::to_string(d.system) + " measure " + std::to_string(d.measure)
                + " (number " + std::to_string(d.measureNumber) + ") staff " + std::to_string(d.staff) + ": saved "
                + std::to_string(d.saved) + ", now " + std::to_string(d.current);
    }
    return text;
}

Measure* measureAt(Score* score, int index)
{
    Measure* measure = score->firstMeasure();
    for (int i = 0; measure && i < index; ++i) {
        measure = measure->nextMeasure();
    }
    return measure;
}

Chord* firstChord(Measure* measure)
{
    for (Segment* segment = measure->first(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
        EngravingItem* item = segment->element(0);
        if (item && item->isChord()) {
            return toChord(item);
        }
    }
    return nullptr;
}

//! Saves the score, opens the file, and hands the opened score to `change`; what `change` leaves is saved
//! again, but with the layout fingerprint of the first file: a file whose layout is not the one its
//! fingerprint is of. Returns that file, opened.
Project openWithFingerprintOfBefore(const String& fixture, const String& name, const std::function<void(MasterScore*)>& change)
{
    Project original = open(dataPath(fixture));
    EXPECT_TRUE(original.score());
    if (!original.score()) {
        return Project();
    }
    EXPECT_TRUE(save(original.score(), name + u"-before.msdz"));

    Project changed = open(outputPath(name + u"-before.msdz"));
    EXPECT_TRUE(changed.score());
    if (!changed.score()) {
        return Project();
    }
    EXPECT_TRUE(changed.report().comparable);
    EXPECT_FALSE(changed.report().hasDifferences()) << describe(changed.report());
    change(changed.score());
    EXPECT_TRUE(save(changed.score(), name + u"-changed.msdz"));

    Container container = readContainer(name + u"-changed.msdz");
    container.fingerprint = readContainer(name + u"-before.msdz").fingerprint;
    EXPECT_FALSE(container.fingerprint.empty());
    return open(writeContainer(container, name + u"-stale.msdz"));
}

//! Where the measure with the given index (in the order of the fingerprint: all that the systems hold) is laid out
struct MeasurePlace {
    MeasureBase* measure = nullptr;
    int system = -1;
    int page = -1;
    //! Whether it is the first of its system
    bool firstInSystem = false;
};

MeasurePlace measurePlace(Score* score, int index)
{
    MeasurePlace place;
    int measureIndex = 0;
    int systemIndex = 0;
    for (const Page* page : score->pages()) {
        for (const System* system : page->systems()) {
            for (MeasureBase* measure : system->measures()) {
                if (measureIndex == index) {
                    place.measure = measure;
                    place.system = systemIndex;
                    place.page = static_cast<int>(score->pageIdx(page));
                    place.firstInSystem = measure == system->measures().front();
                    return place;
                }
                ++measureIndex;
            }
            ++systemIndex;
        }
    }
    return place;
}

int measureCountOf(Score* score)
{
    int count = 0;
    for (const Page* page : score->pages()) {
        for (const System* system : page->systems()) {
            count += static_cast<int>(system->measures().size());
        }
    }
    return count;
}

//! Saves the score of the fixture, as the first step of tests that then change the file
String saveFixture(const String& fixture, const String& name)
{
    Project original = open(dataPath(fixture));
    EXPECT_TRUE(original.score());
    if (original.score()) {
        EXPECT_TRUE(save(original.score(), name));
    }
    return outputPath(name);
}

void expectSameFingerprint(const LayoutFingerprint& a, const LayoutFingerprint& b)
{
    EXPECT_EQ(a.layoutMode, b.layoutMode);
    EXPECT_EQ(a.spatium, b.spatium);
    EXPECT_EQ(a.pageSystems, b.pageSystems);
    ASSERT_EQ(a.systems.size(), b.systems.size());
    for (size_t i = 0; i < a.systems.size(); ++i) {
        const LayoutFingerprint::SystemData& x = a.systems[i];
        const LayoutFingerprint::SystemData& y = b.systems[i];
        EXPECT_EQ(x.page, y.page) << "system " << i;
        EXPECT_EQ(x.firstMeasure, y.firstMeasure) << "system " << i;
        EXPECT_EQ(x.y, y.y) << "system " << i;
        EXPECT_EQ(x.height, y.height) << "system " << i;
        EXPECT_EQ(x.staffY, y.staffY) << "system " << i;
        EXPECT_EQ(x.content, y.content) << "system " << i;
        ASSERT_EQ(x.measures.size(), y.measures.size()) << "system " << i;
        for (size_t k = 0; k < x.measures.size(); ++k) {
            EXPECT_EQ(x.measures[k].x, y.measures[k].x) << "system " << i << " measure " << k;
            EXPECT_EQ(x.measures[k].width, y.measures[k].width) << "system " << i << " measure " << k;
            EXPECT_EQ(x.measures[k].content, y.measures[k].content) << "system " << i << " measure " << k;
        }
    }
}
}

class Engraving_LayoutFingerprintTests : public ::testing::Test
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

    //! Saves the fixture, opens the saved file, and expects the layout that was saved
    void expectSameLayoutAfterReopening(const String& fixturePath, const String& savedName)
    {
        const std::string name = fixturePath.toStdString();

        Project original = open(fixturePath);
        ASSERT_TRUE(original.score()) << name;
        ASSERT_TRUE(save(original.score(), savedName)) << name;
        ASSERT_FALSE(readContainer(savedName).fingerprint.empty()) << name;

        Project reopened = open(outputPath(savedName));
        ASSERT_TRUE(reopened.score()) << name;
        const LayoutFingerprintReport& report = reopened.report();
        EXPECT_TRUE(report.comparable) << name << ": " << describe(report);
        EXPECT_EQ(report.reason, Reason::None) << name;
        EXPECT_TRUE(report.sameProgram) << name;
        EXPECT_FALSE(report.hasDifferences()) << name << ": " << describe(report);
        ASSERT_FALSE(report.scores.empty()) << name;
        EXPECT_TRUE(report.scores.front().part.empty()) << name;
    }

private:
    bool m_useRead302 = false;
};

//---------------------------------------------------------
//   A file that is saved and opened again has the layout it was saved with
//---------------------------------------------------------

TEST_F(Engraving_LayoutFingerprintTests, reopenedFileHasTheSavedLayout)
{
    for (const char16_t* fixture : { u"articulations.mscx", u"crossStaffBeamTuplet.mscx", u"crossStaffBeamsInTwoMeasures.mscx",
                                     u"ledgerLines.mscx", u"slurs.mscx", u"twoStaves.mscx" }) {
        expectSameLayoutAfterReopening(dataPath(CONSISTENCY_DIR + fixture), u"layoutFingerprint-reopened.msdz");
    }
}

//! A file of an older format is laid out to its fixed point when it is opened: the layout the user first
//! sees is the one the file shows after a save and a reopen (EngravingCompat::doPostLayoutCompatIfNeeded)
TEST_F(Engraving_LayoutFingerprintTests, migratedFileIsLaidOutToItsFixedPoint)
{
    // A 4.2 file with user-modified cross-staff beams, which the compat step lays out once more
    Project original = open(dataPath(VTEST_DIR + u"cross-9.mscz"));
    ASSERT_TRUE(original.score());
    ASSERT_LT(original.score()->mscVersion(), 460);
    const LayoutFingerprint before = LayoutFingerprint::of(original.score());
    ASSERT_GT(before.systems.size(), 0u);

    ASSERT_TRUE(save(original.score(), u"layoutFingerprint-cross-9.msdz"));
    Project reopened = open(outputPath(u"layoutFingerprint-cross-9.msdz"));
    ASSERT_TRUE(reopened.score());
    expectSameFingerprint(before, LayoutFingerprint::of(reopened.score()));
}

//! An empty guitar-bend text is drawn nowhere, but its box must be the same box in every layout — the
//! fingerprint of a saved file must match the reopened layout
TEST_F(Engraving_LayoutFingerprintTests, emptyBendTextSurvivesSaveReload)
{
    expectSameLayoutAfterReopening(dataPath(VTEST_DIR + u"guitarBend-1.mscz"),
                                   u"layoutFingerprint-guitarBend-1.msdz");
}

//! The parts of an ornament (the accidentals it shows, the parentheses of its cue note) stand on its track, as
//! opened and after a reopen: the fingerprint, which tells the items apart by their track, finds no difference
TEST_F(Engraving_LayoutFingerprintTests, ornamentSurvivesSaveReload)
{
    expectSameLayoutAfterReopening(dataPath(CONSISTENCY_DIR + u"ornament-2.mscz"),
                                   u"layoutFingerprint-ornament-2.msdz");
}

//---------------------------------------------------------
//   The file
//---------------------------------------------------------

TEST_F(Engraving_LayoutFingerprintTests, fingerprintFileSurvivesJson)
{
    Project original = open(dataPath(SYSTEMS_SCORE));
    ASSERT_TRUE(original.score());

    LayoutFingerprintFile file;
    file.programVersion = u"4.6.5";
    file.programRevision = u"0123abc";
    file.scores.push_back({ String(), LayoutFingerprint::of(original.score()) });
    file.scores.push_back({ u"excerpt_1.mscx", LayoutFingerprint::of(original.score()) });

    const LayoutFingerprint& fingerprint = file.scores.front().fingerprint;
    ASSERT_GE(fingerprint.systems.size(), 3u);
    ASSERT_FALSE(fingerprint.pageSystems.empty());
    EXPECT_GT(fingerprint.spatium, 0);
    for (const LayoutFingerprint::SystemData& system : fingerprint.systems) {
        EXPECT_FALSE(system.staffY.empty());
        ASSERT_FALSE(system.measures.empty());
        EXPECT_NE(system.measures.front().content, LayoutContentHash());
    }

    LayoutFingerprintFile read;
    ASSERT_EQ(LayoutFingerprintFile::fromJson(file.toJson(), read), LayoutFingerprintFile::ReadResult::Ok);

    EXPECT_EQ(read.format, LayoutFingerprintFile::FORMAT);
    EXPECT_EQ(read.programVersion, file.programVersion);
    EXPECT_EQ(read.programRevision, file.programRevision);
    ASSERT_EQ(read.scores.size(), file.scores.size());
    for (size_t i = 0; i < file.scores.size(); ++i) {
        EXPECT_EQ(read.scores[i].part, file.scores[i].part);
        expectSameFingerprint(read.scores[i].fingerprint, file.scores[i].fingerprint);
    }
    EXPECT_TRUE(read.find(String()));
    EXPECT_TRUE(read.find(u"excerpt_1.mscx"));
    EXPECT_FALSE(read.find(u"excerpt_2.mscx"));
}

TEST_F(Engraving_LayoutFingerprintTests, unreadableFingerprintIsLeftAside)
{
    const String saved = saveFixture(SYSTEMS_SCORE, u"layoutFingerprint-unreadable-saved.msdz");
    const Container container = readContainer(saved);
    ASSERT_FALSE(container.fingerprint.empty());

    LayoutFingerprintFile valid;
    ASSERT_EQ(LayoutFingerprintFile::fromJson(container.fingerprint, valid), LayoutFingerprintFile::ReadResult::Ok);
    ASSERT_FALSE(valid.scores.empty());
    ASSERT_FALSE(valid.scores.front().fingerprint.systems.empty());

    LayoutFingerprintFile shortened = valid;
    shortened.scores.front().fingerprint.systems.pop_back();

    struct Case {
        const char* what;
        ByteArray fingerprint;
        Reason reason;
    };
    const Case cases[] = {
        { "not json", toBytes("not json"), Reason::Malformed },
        { "format 999", toBytes("{\"format\": 999, \"scores\": []}"), Reason::UnknownFormat },
        { "fewer systems than the pages hold", shortened.toJson(), Reason::Malformed },
    };

    for (const Case& c : cases) {
        Container changed = container;
        changed.fingerprint = c.fingerprint;
        Project reopened = open(writeContainer(changed, u"layoutFingerprint-unreadable.msdz"));
        ASSERT_TRUE(reopened.ret) << c.what;
        ASSERT_TRUE(reopened.score()) << c.what;
        EXPECT_FALSE(reopened.report().comparable) << c.what;
        EXPECT_EQ(reopened.report().reason, c.reason) << c.what << ": " << describe(reopened.report());
        EXPECT_FALSE(reopened.report().hasDifferences()) << c.what;
        ASSERT_FALSE(reopened.report().scores.empty()) << c.what;
        EXPECT_EQ(reopened.report().scores.front().reason, c.reason) << c.what;
    }
}

TEST_F(Engraving_LayoutFingerprintTests, fileWithoutFingerprintIsNotCompared)
{
    Container container = readContainer(saveFixture(SYSTEMS_SCORE, u"layoutFingerprint-without-saved.msdz"));
    ASSERT_FALSE(container.fingerprint.empty());
    container.fingerprint = ByteArray();

    Project reopened = open(writeContainer(container, u"layoutFingerprint-without.msdz"));
    ASSERT_TRUE(reopened.ret);
    ASSERT_TRUE(reopened.score());
    EXPECT_FALSE(reopened.report().comparable);
    EXPECT_EQ(reopened.report().reason, Reason::NoFingerprint);
    EXPECT_FALSE(reopened.report().hasDifferences());

    Project plain = open(dataPath(SYSTEMS_SCORE));
    ASSERT_TRUE(plain.score());
    EXPECT_FALSE(plain.report().comparable);
    EXPECT_EQ(plain.report().reason, Reason::NoFingerprint);
    EXPECT_FALSE(plain.report().hasDifferences());
}

//---------------------------------------------------------
//   What a difference says
//---------------------------------------------------------

TEST_F(Engraving_LayoutFingerprintTests, changedStyleIsReported)
{
    Project stale = openWithFingerprintOfBefore(SYSTEMS_SCORE, u"layoutFingerprint-style", [](MasterScore* score) {
        score->startCmd(TranslatableString::untranslatable("Widen measures test"));
        score->undoChangeStyleVal(Sid::minNoteDistance, Spatium(2.5));
        score->endCmd();
    });
    ASSERT_TRUE(stale.score());

    const LayoutFingerprintReport& report = stale.report();
    EXPECT_TRUE(report.comparable) << describe(report);
    EXPECT_TRUE(report.hasDifferences());
    EXPECT_TRUE(!differencesOf(report, Kind::MeasureWidth).empty() || !differencesOf(report, Kind::SystemBreak).empty())
        << describe(report);
    ASSERT_FALSE(report.differences.empty());
    EXPECT_EQ(report.differences.front().page, 0) << describe(report);
}

TEST_F(Engraving_LayoutFingerprintTests, movedItemIsReportedInItsMeasure)
{
    int changedIndex = -1;
    Project stale = openWithFingerprintOfBefore(SYSTEMS_SCORE, u"layoutFingerprint-moved", [&changedIndex](MasterScore* score) {
        // the measure with the index 2, or the first after it that has a chord
        for (int index = 2; index < measureCountOf(score); ++index) {
            MeasureBase* measureBase = measurePlace(score, index).measure;
            Chord* chord = measureBase && measureBase->isMeasure() ? firstChord(toMeasure(measureBase)) : nullptr;
            if (!chord) {
                continue;
            }
            changedIndex = index;
            score->startCmd(TranslatableString::untranslatable("Move chord test"));
            chord->undoChangeProperty(Pid::OFFSET, PointF(0.0, 2.0 * score->style().spatium()));
            score->endCmd();
            return;
        }
    });
    ASSERT_TRUE(stale.score());
    ASSERT_GE(changedIndex, 0);
    EXPECT_EQ(changedIndex, 2);

    const LayoutFingerprintReport& report = stale.report();
    EXPECT_TRUE(report.comparable) << describe(report);
    ASSERT_EQ(report.differences.size(), 1u) << describe(report);

    const LayoutDifference& difference = report.differences.front();
    const MeasurePlace place = measurePlace(stale.score(), changedIndex);
    ASSERT_TRUE(place.measure);
    EXPECT_EQ(difference.kind, Kind::MeasureContent) << describe(report);
    EXPECT_EQ(difference.measure, changedIndex);
    EXPECT_EQ(difference.measureNumber, toMeasure(place.measure)->no() + 1);
    EXPECT_EQ(difference.measureNumber, 3);
    EXPECT_EQ(difference.system, place.system);
    EXPECT_EQ(difference.page, place.page);
    EXPECT_TRUE(difference.part.empty());
}

TEST_F(Engraving_LayoutFingerprintTests, addedSystemBreakIsReported)
{
    int changedIndex = -1;
    Project stale = openWithFingerprintOfBefore(SYSTEMS_SCORE, u"layoutFingerprint-break", [&changedIndex](MasterScore* score) {
        // a measure that does not end its system, and is followed by one that is not the first of its system
        for (int index = 0; index + 1 < measureCountOf(score); ++index) {
            const MeasurePlace place = measurePlace(score, index);
            const MeasurePlace next = measurePlace(score, index + 1);
            if (!place.measure || !next.measure || next.firstInSystem || !place.measure->isMeasure()) {
                continue;
            }
            changedIndex = index;
            score->startCmd(TranslatableString::untranslatable("Add line break test"));
            place.measure->undoSetBreak(true, LayoutBreakType::LINE);
            score->endCmd();
            return;
        }
    });
    ASSERT_TRUE(stale.score());
    ASSERT_GE(changedIndex, 0);

    const LayoutFingerprintReport& report = stale.report();
    EXPECT_TRUE(report.comparable) << describe(report);
    bool found = false;
    for (const LayoutDifference* difference : differencesOf(report, Kind::SystemBreak)) {
        if (difference->measure == changedIndex + 1) {
            found = true;
            EXPECT_EQ(difference->saved, 0.0);
            EXPECT_EQ(difference->current, 1.0);
        }
    }
    EXPECT_TRUE(found) << "break after measure " << changedIndex << ": " << describe(report);
}

TEST_F(Engraving_LayoutFingerprintTests, staffDistanceChangeIsReported)
{
    Project stale = openWithFingerprintOfBefore(TWO_INSTRUMENTS_SCORE, u"layoutFingerprint-staffDistance", [](MasterScore* score) {
        ASSERT_GE(score->nstaves(), 2u);
        // With vertical spread, which is on by default, the least distance is what the style gives for it;
        // otherwise the staves of one instrument are the distance of the brace apart
        Sid distance = Sid::minStaffSpread;
        if (!score->style().styleB(Sid::enableVerticalSpread)) {
            distance = score->staff(0)->part() == score->staff(1)->part() ? Sid::akkoladeDistance : Sid::staffDistance;
        }
        score->startCmd(TranslatableString::untranslatable("Staff distance test"));
        score->undoChangeStyleVal(distance, Spatium(15.0));
        score->endCmd();
    });
    ASSERT_TRUE(stale.score());

    const LayoutFingerprintReport& report = stale.report();
    EXPECT_TRUE(report.comparable) << describe(report);
    bool secondStaffMoved = false;
    for (const LayoutDifference* difference : differencesOf(report, Kind::StaffPosition)) {
        secondStaffMoved |= difference->staff == 1 && difference->system == 0;
    }
    EXPECT_TRUE(secondStaffMoved) << describe(report);
    EXPECT_TRUE(differencesOf(report, Kind::MeasureContent).empty()) << describe(report);
}

//---------------------------------------------------------
//   What is not described
//---------------------------------------------------------

TEST_F(Engraving_LayoutFingerprintTests, selectionAndOfficialExportHaveNoFingerprint)
{
    Project project = open(dataPath(SYSTEMS_SCORE));
    ASSERT_TRUE(project.score());
    MasterScore* score = project.score();

    ASSERT_TRUE(save(score, u"layoutFingerprint-whole.msdz"));
    EXPECT_FALSE(readContainer(u"layoutFingerprint-whole.msdz").fingerprint.empty());

    write::WriteRange range;
    range.startMeasure = score->first();
    range.endMeasure = score->first()->next();
    range.startStaffIdx = 0;
    range.endStaffIdx = score->nstaves();
    ASSERT_TRUE(save(score, u"layoutFingerprint-range.msdz", &range));
    EXPECT_TRUE(readContainer(u"layoutFingerprint-range.msdz").fingerprint.empty());

    Project official = open(dataPath(SYSTEMS_SCORE));
    ASSERT_TRUE(official.score());
    OfficialLayoutBaker::bake(official.score());
    ASSERT_TRUE(official.score()->officialCompat());
    ASSERT_TRUE(save(official.score(), u"layoutFingerprint-official.mscz"));
    EXPECT_TRUE(readContainer(u"layoutFingerprint-official.mscz").fingerprint.empty());
}

//---------------------------------------------------------
//   When it is compared
//---------------------------------------------------------

TEST_F(Engraving_LayoutFingerprintTests, lockedUpdatesAreComparedAtTheFirstLayout)
{
    const String saved = saveFixture(SYSTEMS_SCORE, u"layoutFingerprint-locked.msdz");

    Project reopened = open(saved);
    ASSERT_TRUE(reopened.ret);
    ASSERT_TRUE(reopened.score());
    EXPECT_TRUE(reopened.report().comparable) << describe(reopened.report());
    EXPECT_EQ(reopened.report().reason, Reason::None);
    EXPECT_FALSE(reopened.report().hasDifferences()) << describe(reopened.report());
}

//! A part is laid out only while it is open (Score::update); the file says which parts were open
TEST_F(Engraving_LayoutFingerprintTests, partIsComparedWhenItIsLaidOut)
{
    const auto stateOf = [](const LayoutFingerprintReport& report, const String& partName) {
        for (const LayoutFingerprintReport::ScoreState& state : report.scores) {
            if (state.partName == partName) {
                return state;
            }
        }
        ADD_FAILURE() << "no state for the part " << partName.toStdString();
        return LayoutFingerprintReport::ScoreState();
    };

    // Saved with all parts laid out, and none of them open
    {
        Project original = open(dataPath(SCORE_WITH_PARTS));
        ASSERT_TRUE(original.score());
        const size_t partCount = original.score()->excerpts().size();
        ASSERT_GE(partCount, 2u);
        for (const Excerpt* excerpt : original.score()->excerpts()) {
            ASSERT_TRUE(excerpt->excerptScore()->isLayoutUpToDate()) << excerpt->name().toStdString();
            excerpt->excerptScore()->setIsOpen(false);
        }
        ASSERT_TRUE(save(original.score(), u"layoutFingerprint-parts.msdz"));

        LayoutFingerprintFile file;
        ASSERT_EQ(LayoutFingerprintFile::fromJson(readContainer(u"layoutFingerprint-parts.msdz").fingerprint, file),
                  LayoutFingerprintFile::ReadResult::Ok);
        EXPECT_EQ(file.scores.size(), partCount + 1);
        for (const Excerpt* excerpt : original.score()->excerpts()) {
            EXPECT_TRUE(file.find(excerpt->fileName())) << excerpt->name().toStdString();
        }

        Project reopened = open(outputPath(u"layoutFingerprint-parts.msdz"));
        ASSERT_TRUE(reopened.score());
        const LayoutFingerprintReport& report = reopened.report();
        EXPECT_TRUE(report.comparable) << describe(report);
        EXPECT_EQ(report.reason, Reason::None);
        EXPECT_FALSE(report.hasDifferences()) << describe(report);
        ASSERT_EQ(report.scores.size(), partCount + 1);

        // nothing is compared for a part until it is laid out, which is when it is opened
        for (const Excerpt* excerpt : reopened.score()->excerpts()) {
            EXPECT_FALSE(excerpt->excerptScore()->isLayoutUpToDate()) << excerpt->name().toStdString();
            const LayoutFingerprintReport::ScoreState state = stateOf(report, excerpt->name());
            EXPECT_EQ(state.reason, Reason::NotLaidOut) << excerpt->name().toStdString();
            EXPECT_FALSE(state.comparable) << excerpt->name().toStdString();
        }

        Excerpt* opened = reopened.score()->excerpts().front();
        Score* partScore = opened->excerptScore();
        partScore->setIsOpen(true);
        partScore->setLayoutAll();
        partScore->update();

        EXPECT_TRUE(partScore->isLayoutUpToDate());
        for (const Excerpt* excerpt : reopened.score()->excerpts()) {
            const LayoutFingerprintReport::ScoreState state = stateOf(report, excerpt->name());
            if (excerpt == opened) {
                EXPECT_EQ(state.reason, Reason::None) << describe(report);
                EXPECT_TRUE(state.comparable);
                EXPECT_EQ(state.differenceCount, 0u) << describe(report);
            } else {
                EXPECT_EQ(state.reason, Reason::NotLaidOut) << excerpt->name().toStdString();
            }
        }
        EXPECT_FALSE(report.hasDifferences()) << describe(report);
    }

    // Saved after an edit, with the first part open and so laid out again, and the others not: they are not described
    {
        Project original = open(dataPath(SCORE_WITH_PARTS));
        ASSERT_TRUE(original.score());
        MasterScore* master = original.score();
        ASSERT_GE(master->excerpts().size(), 2u);
        for (const Excerpt* excerpt : master->excerpts()) {
            excerpt->excerptScore()->setIsOpen(excerpt == master->excerpts().front());
        }
        EngravingItem* chordRest = nullptr;
        for (Measure* measure = master->firstMeasure(); measure && !chordRest; measure = measure->nextMeasure()) {
            for (Segment* segment = measure->first(SegmentType::ChordRest); segment && !chordRest;
                 segment = segment->next(SegmentType::ChordRest)) {
                for (track_idx_t track = 0; track < master->ntracks() && !chordRest; ++track) {
                    EngravingItem* item = segment->element(track);
                    chordRest = item && item->isChordRest() ? item : nullptr;
                }
            }
        }
        ASSERT_TRUE(chordRest);
        master->startCmd(TranslatableString::untranslatable("Part not laid out test"));
        chordRest->undoChangeProperty(Pid::OFFSET, PointF(0.0, master->style().spatium()));
        master->endCmd();

        Excerpt* laidOut = master->excerpts().front();
        ASSERT_TRUE(laidOut->excerptScore()->isLayoutUpToDate());
        for (const Excerpt* excerpt : master->excerpts()) {
            if (excerpt != laidOut) {
                ASSERT_FALSE(excerpt->excerptScore()->isLayoutUpToDate()) << excerpt->name().toStdString();
            }
        }
        ASSERT_TRUE(save(master, u"layoutFingerprint-parts-edited.msdz"));

        LayoutFingerprintFile file;
        ASSERT_EQ(LayoutFingerprintFile::fromJson(readContainer(u"layoutFingerprint-parts-edited.msdz").fingerprint, file),
                  LayoutFingerprintFile::ReadResult::Ok);
        EXPECT_EQ(file.scores.size(), 2u);
        EXPECT_TRUE(file.find(String()));
        EXPECT_TRUE(file.find(laidOut->fileName()));

        Project reopened = open(outputPath(u"layoutFingerprint-parts-edited.msdz"));
        ASSERT_TRUE(reopened.score());
        const LayoutFingerprintReport& report = reopened.report();
        EXPECT_TRUE(report.comparable) << describe(report);
        EXPECT_FALSE(report.hasDifferences()) << describe(report);
        for (const Excerpt* excerpt : reopened.score()->excerpts()) {
            const LayoutFingerprintReport::ScoreState state = stateOf(report, excerpt->name());
            if (excerpt->name() == laidOut->name()) {
                EXPECT_EQ(state.reason, Reason::None) << describe(report);
                EXPECT_TRUE(state.comparable);
            } else {
                EXPECT_EQ(state.reason, Reason::NoFingerprint) << excerpt->name().toStdString();
                EXPECT_FALSE(state.comparable);
            }
        }
    }
}

//! Every score of vtest: slow, and a few of them do not come back as they were saved, which is what the
//! fingerprint is there to tell. Run with --gtest_also_run_disabled_tests to see which.
TEST_F(Engraving_LayoutFingerprintTests, DISABLED_allVtestScores)
{
    std::vector<std::string> names;
    for (const auto& entry : std::filesystem::directory_iterator(dataPath(VTEST_DIR).toStdString())) {
        const std::string extension = entry.path().extension().string();
        if (extension == ".mscz" || extension == ".mscx") {
            names.push_back(entry.path().filename().string());
        }
    }
    std::sort(names.begin(), names.end());

    size_t withDifferences = 0;
    size_t notComparable = 0;
    for (const std::string& name : names) {
        Project original = open(dataPath(VTEST_DIR + String::fromStdString(name)));
        if (!original.score() || !save(original.score(), u"layoutFingerprint-vtest.msdz")) {
            std::cout << "[  SKIPPED ] " << name << std::endl;
            continue;
        }
        Project reopened = open(outputPath(u"layoutFingerprint-vtest.msdz"));
        if (!reopened.score() || !reopened.report().comparable) {
            ++notComparable;
            std::cout << "[ NO REPORT] " << name << std::endl;
        } else if (reopened.report().hasDifferences()) {
            ++withDifferences;
            std::cout << "[ DIFFERENT] " << name << ": " << describe(reopened.report()) << std::endl;
        }
    }
    std::cout << names.size() << " scores, " << withDifferences << " with differences, " << notComparable
              << " without a report" << std::endl;
    EXPECT_EQ(withDifferences, 0u);
    EXPECT_EQ(notComparable, 0u);
}
