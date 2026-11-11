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

#include <cmath>
#include <memory>
#include <vector>

#include "global/io/buffer.h"

#include "dom/chord.h"
#include "dom/masterscore.h"
#include "dom/note.h"
#include "dom/segment.h"
#include "dom/slur.h"
#include "dom/stem.h"
#include "infrastructure/mscreader.h"
#include "infrastructure/mscwriter.h"
#include "officialexport/officiallayoutbaker.h"
#include "rw/mscsaver.h"
#include "style/style.h"

#include "utils/scorerw.h"

using namespace mu;
using namespace mu::engraving;

//! Many notes on ledger lines, which the fork leaves out of note spacing and upstream does not
static const String NOTES_SCORE(u"layoutconsistency_data/ledgerLines.mscx");
static const String SLUR_SCORE(u"join_data/join02.mscx");

class Engraving_OfficialExportTests : public ::testing::Test
{
};

namespace {
struct NotePlace {
    PointF note;       // on its page, in spatium
    double stem = 0.0; // the length of its chord's stem, in spatium
};
}

static std::vector<NotePlace> notePlaces(Score* score)
{
    std::vector<NotePlace> places;
    const double spatium = score->style().spatium();
    for (Segment* segment = score->firstSegment(SegmentType::ChordRest); segment; segment = segment->next1(SegmentType::ChordRest)) {
        for (EngravingItem* item : segment->elist()) {
            if (!item || !item->isChord()) {
                continue;
            }
            const Chord* chord = toChord(item);
            for (const Note* note : chord->notes()) {
                places.push_back({ note->pagePos() / spatium, chord->stem() ? chord->stem()->length() / spatium : 0.0 });
            }
        }
    }
    return places;
}

//! Settings of the fork that official MuseScore Studio does not have, and that change where things stand
static void useForkLayout(MasterScore* score)
{
    score->style().set(Sid::keepEqualDurationSpacing, true);
    score->style().set(Sid::useDefaultBeamSlantRules, false);
    score->style().set(Sid::useDefaultStemShorteningRules, false);
    score->style().set(Sid::stemCustomLengthFirstLine, 3.0);
    score->style().set(Sid::stemCustomLengthFirstSpace, 3.0);
    score->style().set(Sid::stemCustomLengthSecondLine, 3.25);
    score->setLayoutAll();
    score->doLayout();
}

//! Saves the score as official MuseScore Studio would get it and opens the file again, laid out by
//! upstream's rules as that application lays it out
static MasterScore* saveAndReopen(MasterScore* score, const String& fileName, muse::ByteArray* scoreXml = nullptr)
{
    {
        MscWriter::Params params;
        params.filePath = fileName;
        params.mode = MscIoMode::Zip;

        MscWriter writer(params);
        EXPECT_TRUE(writer.open());
        MscSaver saver(score->iocContext());
        EXPECT_TRUE(saver.writeMscz(score, writer, false));
        writer.close();
        EXPECT_FALSE(writer.hasError());
    }

    if (scoreXml) {
        MscReader::Params params;
        params.filePath = fileName;
        params.mode = MscIoMode::Zip;
        MscReader reader(params);
        EXPECT_TRUE(reader.open());
        *scoreXml = reader.readScoreFile();
        scoreXml->push_back(reader.readStyleFile());
    }

    MasterScore* reopened = ScoreRW::readScore(fileName, true);
    if (reopened) {
        reopened->setOfficialCompat(true);
        for (Score* s : reopened->scoreList()) {
            s->setLayoutAll();
            s->doLayout();
        }
    }
    return reopened;
}

//---------------------------------------------------------
//   What the fork laid out comes back when the exported file is laid out by upstream's rules
//---------------------------------------------------------

TEST_F(Engraving_OfficialExportTests, reopenedFileKeepsNotesAndStemsInPlace)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(NOTES_SCORE));
    ASSERT_TRUE(score);
    useForkLayout(score.get());
    const std::vector<NotePlace> wanted = notePlaces(score.get());
    ASSERT_FALSE(wanted.empty());

    const OfficialExportReport report = OfficialLayoutBaker::bake(score.get());
    EXPECT_GT(report.pinnedItems, 0);
    EXPECT_EQ(report.unpinnedItems, 0);

    std::unique_ptr<MasterScore> reopened(saveAndReopen(score.get(), u"officialExport-notes.mscz"));
    ASSERT_TRUE(reopened);
    const std::vector<NotePlace> found = notePlaces(reopened.get());
    ASSERT_EQ(found.size(), wanted.size());

    for (size_t i = 0; i < wanted.size(); ++i) {
        EXPECT_NEAR(found[i].note.x(), wanted[i].note.x(), 0.03) << "note " << i;
        EXPECT_NEAR(found[i].note.y(), wanted[i].note.y(), 0.03) << "note " << i;
        EXPECT_NEAR(found[i].stem, wanted[i].stem, 0.03) << "stem of note " << i;
    }
}

//! Without the explicit values the same file comes out elsewhere: the test above would pass for nothing
TEST_F(Engraving_OfficialExportTests, forkLayoutDiffersFromUpstream)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(NOTES_SCORE));
    ASSERT_TRUE(score);
    useForkLayout(score.get());
    const std::vector<NotePlace> wanted = notePlaces(score.get());

    std::unique_ptr<MasterScore> reopened(saveAndReopen(score.get(), u"officialExport-unpinned.mscz"));
    ASSERT_TRUE(reopened);
    const std::vector<NotePlace> found = notePlaces(reopened.get());
    ASSERT_EQ(found.size(), wanted.size());

    double farthest = 0.0;
    for (size_t i = 0; i < wanted.size(); ++i) {
        farthest = std::max(farthest, std::abs(found[i].note.x() - wanted[i].note.x()));
    }
    EXPECT_GT(farthest, 0.2);
}

//---------------------------------------------------------
//   The file holds nothing official MuseScore Studio does not know
//---------------------------------------------------------

TEST_F(Engraving_OfficialExportTests, savedFileHasNoForkSettings)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(NOTES_SCORE));
    ASSERT_TRUE(score);
    useForkLayout(score.get());
    OfficialLayoutBaker::bake(score.get());

    muse::ByteArray xml;
    std::unique_ptr<MasterScore> reopened(saveAndReopen(score.get(), u"officialExport-settings.mscz", &xml));
    ASSERT_TRUE(reopened);

    const std::string text(xml.constChar(), xml.size());
    for (const char* name : { "keepEqualDurationSpacing", "useDefaultBeamSlantRules", "useDefaultStemShorteningRules",
                              "stemCustomLengthFirstLine", "showAlignmentGuides", "LedgerLineLength" }) {
        EXPECT_EQ(text.find(name), std::string::npos) << name;
    }
    // What stands in their place
    EXPECT_NE(text.find("<leadingSpace>"), std::string::npos);
    EXPECT_NE(text.find("<systemLock>"), std::string::npos);
}

//---------------------------------------------------------
//   A slur of several curves becomes one plain slur for each curve
//---------------------------------------------------------

TEST_F(Engraving_OfficialExportTests, multiCurveSlurBecomesPlainSlurs)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(SLUR_SCORE));
    ASSERT_TRUE(score);

    Slur* slur = nullptr;
    size_t slurCount = 0;
    for (const auto& pair : score->spanner()) {
        if (pair.second->isSlur()) {
            slur = slur ? slur : toSlur(pair.second);
            ++slurCount;
        }
    }
    ASSERT_TRUE(slur);
    ASSERT_EQ(slur->spannerSegments().size(), size_t(1));

    slur->setMultiBezierEnabled(true); // with two knots: three curves
    score->setLayoutAll();
    score->doLayout();
    const SlurSegment* segment = toSlurSegment(slur->frontSegment());
    ASSERT_TRUE(segment->useMultiBezier());
    const PointF start = segment->pagePos() + segment->ups(Grip::START).pos();
    const PointF end = segment->pagePos() + segment->ups(Grip::END).pos();

    const OfficialExportReport report = OfficialLayoutBaker::bake(score.get());
    EXPECT_EQ(report.dropped.count(DroppedFeature::SlurPiecesNotSplit), size_t(0));

    // Between the same two notes, one after the other from where the slur started to where it ended
    std::vector<const Slur*> pieces;
    for (const auto& pair : score->spanner()) {
        if (!pair.second->isSlur()) {
            continue;
        }
        const Slur* found = toSlur(pair.second);
        EXPECT_FALSE(found->multiBezierEnabled());
        if (found->startElement() == slur->startElement() && found->endElement() == slur->endElement()) {
            pieces.push_back(found);
        }
    }
    EXPECT_EQ(pieces.size(), size_t(3));
    size_t slursNow = 0;
    for (const auto& pair : score->spanner()) {
        slursNow += pair.second->isSlur() ? 1 : 0;
    }
    EXPECT_EQ(slursNow, slurCount + 2);

    const double tolerance = 0.03 * score->style().spatium();
    double leftmost = 1e9;
    double rightmost = -1e9;
    PointF first;
    PointF last;
    for (const Slur* piece : pieces) {
        ASSERT_EQ(piece->spannerSegments().size(), size_t(1));
        EXPECT_NE(piece->slurDirection(), DirectionV::AUTO);
        const SlurTieSegment* pieceSegment = toSlurTieSegment(piece->frontSegment());
        const PointF from = pieceSegment->pagePos() + pieceSegment->ups(Grip::START).pos();
        const PointF to = pieceSegment->pagePos() + pieceSegment->ups(Grip::END).pos();
        if (from.x() < leftmost) {
            leftmost = from.x();
            first = from;
        }
        if (to.x() > rightmost) {
            rightmost = to.x();
            last = to;
        }
    }
    EXPECT_NEAR(first.x(), start.x(), tolerance);
    EXPECT_NEAR(first.y(), start.y(), tolerance);
    EXPECT_NEAR(last.x(), end.x(), tolerance);
    EXPECT_NEAR(last.y(), end.y(), tolerance);
}
