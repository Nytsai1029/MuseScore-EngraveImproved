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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include "modularity/ioc.h"

#include "fontdesign/ifontdesignconfiguration.h"
#include "fontdesign/internal/fontdesignservice.h"
#include "fontdesign/internal/io/fontfacereader.h"
#include "fontdesign/internal/project/projectcommands.h"

using namespace mu::fontdesign;
using namespace muse;

namespace {
class ConfigurationStub : public IFontDesignConfiguration
{
public:
    io::path_t lastOpenedFontPath() const override { return m_last; }
    void setLastOpenedFontPath(const io::path_t& path) override { m_last = path; }

    io::paths_t recentFontPaths() const override { return m_recent; }
    void prependRecentFontPath(const io::path_t& path) override { m_recent.insert(m_recent.begin(), path); }
    void removeRecentFontPath(const io::path_t&) override {}

private:
    io::path_t m_last;
    io::paths_t m_recent;
};

QString fontsRoot()
{
#ifndef FONTDESIGN_FONTS_ROOT
#error FONTDESIGN_FONTS_ROOT is not defined
#endif
    return QString::fromUtf8(FONTDESIGN_FONTS_ROOT);
}

QByteArray readFile(const QString& path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

QStringList fileNames(const QTemporaryDir& dir)
{
    return QDir(dir.path()).entryList(QDir::Files | QDir::Hidden, QDir::Name);
}
}

//! 服务层的保存全流程（配对 → 备份 → 原子写盘）。界面确认在 scenario 层，这里不涉及。
//! 只覆盖「不改名」的保存：改名会把旧文件移入系统废纸篓，不适合在单测里做。
class FontDesign_ServiceTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_configuration = std::make_shared<ConfigurationStub>();
        modularity::globalIoc()->registerExport<IFontDesignConfiguration>("utests", m_configuration);
        ASSERT_TRUE(m_dir.isValid());
    }

    void TearDown() override
    {
        modularity::globalIoc()->unregister<IFontDesignConfiguration>("utests");
    }

    QString path(const QString& fileName) const { return m_dir.filePath(fileName); }

    std::shared_ptr<ConfigurationStub> m_configuration;
    QTemporaryDir m_dir;
};

//! 回归：在只有 Leland.otf / LelandText.otf / leland_metadata.json 的文件夹里打开 LelandText.otf 并保存，
//! 曾经会把 LelandText 的字形写进 Leland.otf，并把 LelandText.otf 与那份 JSON 移入废纸篓
TEST_F(FontDesign_ServiceTests, SavingCompanionFontLeavesSiblingFontUntouched)
{
    for (const char* name : { "Leland.otf", "LelandText.otf", "leland_metadata.json" }) {
        ASSERT_TRUE(QFile::copy(fontsRoot() + "/leland/" + name, path(name))) << name;
    }
    const QByteArray siblingFont = readFile(path("Leland.otf"));
    const QByteArray siblingMetadata = readFile(path("leland_metadata.json"));
    const QByteArray originalTextFont = readFile(path("LelandText.otf"));
    ASSERT_FALSE(siblingFont.isEmpty());
    ASSERT_FALSE(originalTextFont.isEmpty());

    FontDesignService service;
    ASSERT_TRUE(service.openProject(io::path_t(path("LelandText.otf"))));
    FontDesignProjectPtr project = service.currentProject();
    ASSERT_NE(project, nullptr);
    EXPECT_TRUE(project->metadataPath().empty()) << "paired with the sibling font's metadata";

    SaveTargets targets;
    ASSERT_TRUE(service.saveTargets(targets));
    EXPECT_TRUE(targets.foreignFiles.empty());

    // 任取一个有轮廓的普通字符做一次编辑
    char32_t code = 0;
    for (const auto& pair : project->glyphs()) {
        if (pair.first > 0x20 && !pair.second.outline.isEmpty()) {
            code = pair.first;
            break;
        }
    }
    ASSERT_NE(code, 0u);
    project->undoStack().push(std::make_unique<SetAdvanceCommand>(project.get(), code, 777.0));
    EXPECT_TRUE(project->isDirty());

    SaveReport report;
    Ret ret = service.saveProject(report);
    ASSERT_TRUE(ret) << ret.toString();
    EXPECT_FALSE(project->isDirty());

    // 同目录的另一个字体及其元数据原封不动
    EXPECT_EQ(readFile(path("Leland.otf")), siblingFont);
    EXPECT_EQ(readFile(path("leland_metadata.json")), siblingMetadata);

    // 外来字体首次被覆盖：原件留作 .bak
    ASSERT_EQ(report.backups.size(), 1u);
    EXPECT_EQ(readFile(path("LelandText.otf.bak")), originalTextFont);

    // 写出的是本模块重建的字体，且编辑已落盘
    FontFaceReader::FaceData saved;
    ASSERT_TRUE(FontFaceReader::read(io::path_t(path("LelandText.otf")), saved));
    EXPECT_EQ(saved.vendorId, std::string(FontFaceReader::VENDOR_ID));
    bool found = false;
    for (const FontFaceReader::FaceGlyph& glyph : saved.glyphs) {
        if (glyph.codepoint == code) {
            found = true;
            EXPECT_NEAR(glyph.advance, 777.0, 1.0);
        }
    }
    EXPECT_TRUE(found);

    // 没有多出别的文件（原子写盘的临时文件已清掉），也没有少文件
    EXPECT_EQ(fileNames(m_dir), QStringList({ "Leland.otf", "LelandText.json", "LelandText.otf", "LelandText.otf.bak",
                                              "leland_metadata.json" }));

    // 再保存：不再备份，最初的原件备份保持不变
    project->undoStack().push(std::make_unique<SetAdvanceCommand>(project.get(), code, 888.0));
    SaveReport second;
    ASSERT_TRUE(service.saveProject(second));
    EXPECT_TRUE(second.backups.empty());
    EXPECT_EQ(readFile(path("LelandText.otf.bak")), originalTextFont);
}

//! 本模块自己写出的字体（新建、或再次打开）不需要备份
TEST_F(FontDesign_ServiceTests, OwnFontsAreSavedWithoutBackup)
{
    NewFontParams params;
    params.fontName = "Fresh";
    params.folder = io::path_t(m_dir.path());

    FontDesignService service;
    ASSERT_TRUE(service.newProject(params));
    FontDesignProjectPtr project = service.currentProject();
    ASSERT_NE(project, nullptr);

    GlyphOutline outline;
    outline.contours().push_back(GlyphOutline::rectContour(RectF(0, 0, 300, 200)));
    project->undoStack().push(std::make_unique<ReplaceOutlineCommand>(project.get(), 0xE0A4, outline));

    SaveReport report;
    Ret ret = service.saveProject(report);
    ASSERT_TRUE(ret) << ret.toString();
    EXPECT_TRUE(report.backups.empty());
    EXPECT_EQ(fileNames(m_dir), QStringList({ "Fresh.json", "Fresh.otf" }));

    // 关掉再打开：元数据按文件名配对，保存仍不产生备份
    ASSERT_TRUE(service.openProject(io::path_t(path("Fresh.otf"))));
    project = service.currentProject();
    EXPECT_EQ(QFileInfo(project->metadataPath().toQString()).fileName(), QString("Fresh.json"));
    EXPECT_FALSE(project->backupBeforeOverwrite());

    project->undoStack().push(std::make_unique<SetAdvanceCommand>(project.get(), 0xE0A4, 500.0));
    SaveReport second;
    ASSERT_TRUE(service.saveProject(second));
    EXPECT_TRUE(second.backups.empty());
    EXPECT_EQ(fileNames(m_dir), QStringList({ "Fresh.json", "Fresh.otf" }));
}
