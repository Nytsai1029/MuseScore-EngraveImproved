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

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "global/io/buffer.h"
#include "global/io/file.h"
#include "global/serialization/xmlstreamreader.h"

#include "compat/mscxcompat.h"
#include "compat/scoreaccess.h"
#include "dom/masterscore.h"
#include "dom/mscore.h"
#include "engravingerrors.h"
#include "infrastructure/mscreader.h"
#include "infrastructure/mscwriter.h"
#include "officialexport/officiallayoutbaker.h"
#include "rw/mscsaver.h"
#include "rw/rwregister.h"
#include "types/constants.h"

#include "utils/scorerw.h"

using namespace mu;
using namespace muse;
using namespace muse::io;
using namespace mu::engraving;

static const String SIMPLE_SCORE(u"layoutconsistency_data/twoStaves.mscx");
static const String SCORE_WITH_PARTS(u"parts_data/input-from-parts.mscz");

namespace {
//! The root elements of the files inside a saved container
struct SavedRoots {
    std::string score;
    std::string style;
    std::vector<std::string> excerpts;
    std::vector<std::string> excerptStyles;
};

//! `<museScore …>` as it stands in the file
std::string rootElement(const ByteArray& xml)
{
    const std::string text(xml.constChar(), xml.size());
    const size_t start = text.find("<museScore");
    if (start == std::string::npos) {
        return std::string();
    }
    return text.substr(start, text.find('>', start) - start + 1);
}

std::string markedRoot(int forkFormatVersion)
{
    return "<museScore version=\"4.60\" engraveImprovedVersion=\"" + std::to_string(forkFormatVersion) + "\">";
}

const std::string UNMARKED_ROOT("<museScore version=\"4.60\">");

bool saveContainer(MasterScore* score, const String& fileName)
{
    MscWriter::Params params;
    params.filePath = fileName;
    params.mode = MscIoMode::Zip;

    MscWriter writer(params);
    if (!writer.open()) {
        return false;
    }
    MscSaver saver(score->iocContext());
    const bool ok = saver.writeMscz(score, writer, false);
    writer.close();
    return ok && !writer.hasError();
}

SavedRoots savedRoots(const String& fileName)
{
    MscReader::Params params;
    params.filePath = fileName;
    params.mode = MscIoMode::Zip;
    MscReader reader(params);
    EXPECT_TRUE(reader.open());

    SavedRoots roots;
    roots.score = rootElement(reader.readScoreFile());
    roots.style = rootElement(reader.readStyleFile());
    for (const String& excerpt : reader.excerptFileNames()) {
        roots.excerpts.push_back(rootElement(reader.readExcerptFile(excerpt)));
        roots.excerptStyles.push_back(rootElement(reader.readExcerptStyleFile(excerpt)));
    }
    return roots;
}

ByteArray scoreXml(Score* score)
{
    ByteArray data;
    Buffer buffer(&data);
    buffer.open(IODevice::WriteOnly);
    EXPECT_TRUE(rw::RWRegister::writer(score->iocContext())->writeScore(score, &buffer));
    buffer.close();
    return data;
}

//! The file with another root element, written next to the test binary
String writeWithRoot(const ByteArray& xml, const std::string& root, const String& fileName)
{
    std::string text(xml.constChar(), xml.size());
    const std::string oldRoot = rootElement(xml);
    EXPECT_FALSE(oldRoot.empty());
    text.replace(text.find(oldRoot), oldRoot.size(), root);

    const std::string path = std::filesystem::absolute(fileName.toStdString()).string();
    EXPECT_TRUE(File::writeFile(path, ByteArray(text.c_str(), text.size())));
    return String::fromStdString(path);
}

Ret load(std::unique_ptr<MasterScore>& score, const String& path, bool ignoreVersionError)
{
    score.reset(compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr));
    return compat::loadMsczOrMscx(score.get(), path, ignoreVersionError);
}
}

class Engraving_ForkFormatTests : public ::testing::Test
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
        MScore::testMode = true;
    }

    //! As the application runs: the tests otherwise write what the reference files hold
    void leaveTestMode() { MScore::testMode = false; }
    void enterTestMode() { MScore::testMode = true; }

private:
    bool m_useRead302 = false;
};

//---------------------------------------------------------
//   Every score and style file the fork writes says which format of the fork it is in
//---------------------------------------------------------

TEST_F(Engraving_ForkFormatTests, savedFilesCarryForkVersion)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(SCORE_WITH_PARTS));
    ASSERT_TRUE(score);
    ASSERT_FALSE(score->excerpts().empty());
    EXPECT_EQ(score->forkFormatVersion(), 0);

    leaveTestMode();
    ASSERT_TRUE(saveContainer(score.get(), u"forkFormat-parts.mscz"));
    enterTestMode();

    const std::string wanted = markedRoot(Constants::FORK_FORMAT_VERSION);
    const SavedRoots roots = savedRoots(u"forkFormat-parts.mscz");
    EXPECT_EQ(roots.score, wanted);
    EXPECT_EQ(roots.style, wanted);
    ASSERT_EQ(roots.excerpts.size(), score->excerpts().size());
    for (size_t i = 0; i < roots.excerpts.size(); ++i) {
        EXPECT_EQ(roots.excerpts[i], wanted) << "part " << i;
        EXPECT_EQ(roots.excerptStyles[i], wanted) << "style of part " << i;
    }

    // What was written is what the score now is
    EXPECT_EQ(score->forkFormatVersion(), Constants::FORK_FORMAT_VERSION);
}

//! Hundreds of reference files are compared byte for byte with what test mode writes
TEST_F(Engraving_ForkFormatTests, testModeWritesNoForkVersion)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(SCORE_WITH_PARTS));
    ASSERT_TRUE(score);

    ASSERT_TRUE(saveContainer(score.get(), u"forkFormat-testMode.mscz"));

    const SavedRoots roots = savedRoots(u"forkFormat-testMode.mscz");
    EXPECT_EQ(roots.score, UNMARKED_ROOT);
    EXPECT_EQ(roots.style, UNMARKED_ROOT);
    for (size_t i = 0; i < roots.excerpts.size(); ++i) {
        EXPECT_EQ(roots.excerpts[i], UNMARKED_ROOT) << "part " << i;
        EXPECT_EQ(roots.excerptStyles[i], UNMARKED_ROOT) << "style of part " << i;
    }
    EXPECT_EQ(score->forkFormatVersion(), 0);
}

//---------------------------------------------------------
//   The export for official MuseScore Studio has nothing of the fork in its root elements
//---------------------------------------------------------

TEST_F(Engraving_ForkFormatTests, officialExportHasNoForkVersion)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(SCORE_WITH_PARTS));
    ASSERT_TRUE(score);
    OfficialLayoutBaker::bake(score.get());
    ASSERT_TRUE(score->officialCompat());

    leaveTestMode();
    ASSERT_TRUE(saveContainer(score.get(), u"forkFormat-official.mscz"));
    enterTestMode();

    const SavedRoots roots = savedRoots(u"forkFormat-official.mscz");
    EXPECT_EQ(roots.score, UNMARKED_ROOT);
    EXPECT_EQ(roots.style, UNMARKED_ROOT);
    ASSERT_FALSE(roots.excerpts.empty());
    for (size_t i = 0; i < roots.excerpts.size(); ++i) {
        EXPECT_EQ(roots.excerpts[i], UNMARKED_ROOT) << "part " << i;
        EXPECT_EQ(roots.excerptStyles[i], UNMARKED_ROOT) << "style of part " << i;
    }
    EXPECT_EQ(score->forkFormatVersion(), 0);
}

//---------------------------------------------------------
//   Reading
//---------------------------------------------------------

TEST_F(Engraving_ForkFormatTests, forkVersionIsReadBack)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(SIMPLE_SCORE));
    ASSERT_TRUE(score);

    leaveTestMode();
    ASSERT_TRUE(saveContainer(score.get(), u"forkFormat-readBack.mscz"));
    enterTestMode();

    std::unique_ptr<MasterScore> reopened;
    const String path = String::fromStdString(std::filesystem::absolute("forkFormat-readBack.mscz").string());
    ASSERT_TRUE(load(reopened, path, false));
    EXPECT_EQ(reopened->forkFormatVersion(), Constants::FORK_FORMAT_VERSION);
    EXPECT_EQ(reopened->mscVersion(), Constants::MSC_VERSION);
}

//! An official file, or one the fork wrote before it had the marker
TEST_F(Engraving_ForkFormatTests, fileWithoutMarkerReadsAsZero)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(SIMPLE_SCORE));
    ASSERT_TRUE(score);
    EXPECT_EQ(score->forkFormatVersion(), 0);

    std::unique_ptr<MasterScore> container(ScoreRW::readScore(SCORE_WITH_PARTS));
    ASSERT_TRUE(container);
    EXPECT_EQ(container->forkFormatVersion(), 0);

    // A value that is no version is no marker either
    const ByteArray xml = scoreXml(score.get());
    for (const char* value : { "", "x", "-3" }) {
        const std::string root = std::string("<museScore version=\"4.60\" engraveImprovedVersion=\"") + value + "\">";
        std::unique_ptr<MasterScore> reopened;
        ASSERT_TRUE(load(reopened, writeWithRoot(xml, root, u"forkFormat-noVersion.mscx"), false)) << value;
        EXPECT_EQ(reopened->forkFormatVersion(), 0) << value;
    }
}

//! A later format of the fork is refused like a later MuseScore Studio, and opened when the user insists
TEST_F(Engraving_ForkFormatTests, tooNewForkVersionIsRejectedUnlessForced)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(SIMPLE_SCORE));
    ASSERT_TRUE(score);
    const ByteArray xml = scoreXml(score.get());

    const int tooNew = Constants::FORK_FORMAT_VERSION + 1;
    const String path = writeWithRoot(xml, markedRoot(tooNew), u"forkFormat-tooNew.mscx");

    std::unique_ptr<MasterScore> reopened;
    const Ret refused = load(reopened, path, false);
    EXPECT_FALSE(refused);
    EXPECT_EQ(refused.code(), static_cast<int>(Err::FileTooNew));

    const Ret forced = load(reopened, path, true);
    EXPECT_TRUE(forced);
    EXPECT_EQ(reopened->forkFormatVersion(), tooNew);
    EXPECT_NE(reopened->firstMeasure(), nullptr);

    // The version this build writes is not too new
    const String current = writeWithRoot(xml, markedRoot(Constants::FORK_FORMAT_VERSION), u"forkFormat-current.mscx");
    EXPECT_TRUE(load(reopened, current, false));
    EXPECT_EQ(reopened->forkFormatVersion(), Constants::FORK_FORMAT_VERSION);
}

//! Nothing else hangs on the marker: without it the same score is read
TEST_F(Engraving_ForkFormatTests, fileReadsTheSameWithoutMarker)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(SIMPLE_SCORE));
    ASSERT_TRUE(score);

    leaveTestMode();
    const ByteArray xml = scoreXml(score.get());
    enterTestMode();
    ASSERT_EQ(rootElement(xml), markedRoot(Constants::FORK_FORMAT_VERSION));

    std::unique_ptr<MasterScore> marked;
    ASSERT_TRUE(load(marked, writeWithRoot(xml, rootElement(xml), u"forkFormat-marked.mscx"), false));
    std::unique_ptr<MasterScore> unmarked;
    ASSERT_TRUE(load(unmarked, writeWithRoot(xml, UNMARKED_ROOT, u"forkFormat-unmarked.mscx"), false));

    EXPECT_EQ(marked->forkFormatVersion(), Constants::FORK_FORMAT_VERSION);
    EXPECT_EQ(unmarked->forkFormatVersion(), 0);

    const ByteArray fromMarked = scoreXml(marked.get());
    const ByteArray fromUnmarked = scoreXml(unmarked.get());
    EXPECT_FALSE(fromMarked.empty());
    EXPECT_TRUE(fromMarked == fromUnmarked);
}
