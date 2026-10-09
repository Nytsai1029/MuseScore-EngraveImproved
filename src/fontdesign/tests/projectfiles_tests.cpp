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

#include "fontdesign/internal/io/projectfiles.h"
#include "fontdesign/internal/project/fontdesignproject.h"
#include "fontdesign/internal/smufldatabase.h"

using namespace mu::fontdesign;
using namespace muse;

namespace {
QString fontsRoot()
{
#ifndef FONTDESIGN_FONTS_ROOT
#error FONTDESIGN_FONTS_ROOT is not defined
#endif
    return QString::fromUtf8(FONTDESIGN_FONTS_ROOT);
}

void writeFile(const QString& path, const QByteArray& content)
{
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Truncate)) << path.toStdString();
    file.write(content);
}

QByteArray readFile(const QString& path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

//! 配对规则只看文件名与 JSON 的 fontName，字体文件内容无关紧要
QString pairedName(const QTemporaryDir& dir, const QString& fontFile)
{
    const io::path_t metadata = ProjectFiles::findMetadataFor(io::path_t(dir.filePath(fontFile)));
    return metadata.empty() ? QString() : QFileInfo(metadata.toQString()).fileName();
}

//! fonts/leland 的副本：Leland.otf + LelandText.otf + leland_metadata.json
void copyLelandFolder(const QTemporaryDir& dir)
{
    for (const char* name : { "Leland.otf", "LelandText.otf", "leland_metadata.json" }) {
        ASSERT_TRUE(QFile::copy(fontsRoot() + "/leland/" + name, dir.filePath(name))) << name;
    }
}
}

TEST(FontDesign_ProjectFilesTests, CompanionFontDoesNotTakeSiblingMetadata)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    writeFile(dir.filePath("A.otf"), "x");
    writeFile(dir.filePath("AText.otf"), "x");
    writeFile(dir.filePath("a_metadata.json"), R"({"fontName": "A"})");

    EXPECT_EQ(pairedName(dir, "A.otf"), QString("a_metadata.json"));
    // 目录里只有这一个 JSON，但它是 A 的：AText 不得认领
    EXPECT_EQ(pairedName(dir, "AText.otf"), QString());
}

TEST(FontDesign_ProjectFilesTests, ExactFileNameBeatsMetadataSuffix)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    writeFile(dir.filePath("A.otf"), "x");
    writeFile(dir.filePath("A.json"), R"({"fontName": "A"})");
    writeFile(dir.filePath("a_metadata.json"), R"({"fontName": "A"})");

    EXPECT_EQ(pairedName(dir, "A.otf"), QString("A.json"));
}

TEST(FontDesign_ProjectFilesTests, FileNameMatchIgnoresSpacesAndCase)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    writeFile(dir.filePath("FinaleMaestroText.otf"), "x");
    writeFile(dir.filePath("FinaleMaestro.json"), R"({"fontName": "Finale Maestro"})");
    writeFile(dir.filePath("Finale Maestro Text.json"), R"({"fontName": "Finale Maestro Text"})");

    EXPECT_EQ(pairedName(dir, "FinaleMaestroText.otf"), QString("Finale Maestro Text.json"));
}

TEST(FontDesign_ProjectFilesTests, UnnamedMetadataNeedsMatchingFontName)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    writeFile(dir.filePath("B.otf"), "x");
    writeFile(dir.filePath("BText.otf"), "x");
    writeFile(dir.filePath("metadata.json"), R"({"fontName": "B"})");

    EXPECT_EQ(pairedName(dir, "B.otf"), QString("metadata.json"));
    EXPECT_EQ(pairedName(dir, "BText.otf"), QString());
}

TEST(FontDesign_ProjectFilesTests, UnrelatedJsonIsNotMetadata)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    writeFile(dir.filePath("D.otf"), "x");
    writeFile(dir.filePath("package.json"), R"({"name": "something-else"})");
    writeFile(dir.filePath("broken.json"), "not json at all");

    EXPECT_EQ(pairedName(dir, "D.otf"), QString());
}

//! 回归：打开 LelandText.otf 再保存，曾经会认领 leland_metadata.json（fontName = "Leland"），
//! 于是把 LelandText 的字形写进 Leland.otf，并把 LelandText.otf 与那份 JSON 移入废纸篓
TEST(FontDesign_ProjectFilesTests, SavingCompanionFontTargetsItsOwnFile)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    copyLelandFolder(dir);

    SmuflDatabase db;
    db.init((fontsRoot() + "/smufl").toStdString());

    const io::path_t fontPath(dir.filePath("LelandText.otf"));
    FontDesignProject project;
    ASSERT_TRUE(project.load(fontPath, ProjectFiles::findMetadataFor(fontPath), db));
    EXPECT_TRUE(project.metadataPath().empty());

    SaveTargets targets;
    ASSERT_TRUE(ProjectFiles::saveTargets(project, targets));
    EXPECT_EQ(QFileInfo(targets.fontPath.toQString()).fileName(), QString("LelandText.otf"));
    EXPECT_EQ(QFileInfo(targets.metadataPath.toQString()).fileName(), QString("LelandText.json"));
    EXPECT_TRUE(targets.foreignFiles.empty());

    // 改名撞上同目录的另一个字体：必须作为「别人的文件」报出来，由界面确认
    project.metadata().fontName = "Leland";
    ASSERT_TRUE(ProjectFiles::saveTargets(project, targets));
    ASSERT_EQ(targets.foreignFiles.size(), 1u);
    EXPECT_EQ(QFileInfo(targets.foreignFiles.front().toQString()).fileName(), QString("Leland.otf"));
}

TEST(FontDesign_ProjectFilesTests, OwnFilesAreNotForeign)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    copyLelandFolder(dir);

    SmuflDatabase db;
    db.init((fontsRoot() + "/smufl").toStdString());

    const io::path_t fontPath(dir.filePath("Leland.otf"));
    FontDesignProject project;
    ASSERT_TRUE(project.load(fontPath, ProjectFiles::findMetadataFor(fontPath), db));
    EXPECT_EQ(QFileInfo(project.metadataPath().toQString()).fileName(), QString("leland_metadata.json"));

    SaveTargets targets;
    ASSERT_TRUE(ProjectFiles::saveTargets(project, targets));
    EXPECT_EQ(QFileInfo(targets.fontPath.toQString()).fileName(), QString("Leland.otf"));
    EXPECT_TRUE(targets.foreignFiles.empty());
}

TEST(FontDesign_ProjectFilesTests, NewFontOverExistingFileIsForeign)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    copyLelandFolder(dir);

    SmuflDatabase db;
    db.init((fontsRoot() + "/smufl").toStdString());

    NewFontParams params;
    params.fontName = "Leland";
    params.folder = io::path_t(dir.path());

    FontDesignProject project;
    ASSERT_TRUE(project.createNew(params, db));

    SaveTargets targets;
    ASSERT_TRUE(ProjectFiles::saveTargets(project, targets));
    ASSERT_EQ(targets.foreignFiles.size(), 1u);
    EXPECT_EQ(QFileInfo(targets.foreignFiles.front().toQString()).fileName(), QString("Leland.otf"));
}

TEST(FontDesign_ProjectFilesTests, InvalidFontNameIsRejected)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    SmuflDatabase db;
    db.init((fontsRoot() + "/smufl").toStdString());

    NewFontParams params;
    params.fontName = "sub/dir";
    params.folder = io::path_t(dir.path());

    FontDesignProject project;
    ASSERT_TRUE(project.createNew(params, db));

    SaveTargets targets;
    EXPECT_FALSE(ProjectFiles::saveTargets(project, targets));
}

TEST(FontDesign_ProjectFilesTests, BackupIsCreatedOnceAndNeverOverwritten)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString source = dir.filePath("Font.otf");
    writeFile(source, "original");

    io::path_t backup;
    ASSERT_TRUE(ProjectFiles::backupOriginal(io::path_t(source), backup));
    ASSERT_FALSE(backup.empty());
    EXPECT_EQ(QFileInfo(backup.toQString()).fileName(), QString("Font.otf.bak"));
    EXPECT_EQ(readFile(backup.toQString()), QByteArray("original"));

    // 文件已被重写过一次：再备份不得盖掉最初那份
    writeFile(source, "rebuilt");
    io::path_t second;
    ASSERT_TRUE(ProjectFiles::backupOriginal(io::path_t(source), second));
    EXPECT_TRUE(second.empty());
    EXPECT_EQ(readFile(dir.filePath("Font.otf.bak")), QByteArray("original"));

    // 不存在的文件（如没有配对元数据）不是错误
    io::path_t none;
    EXPECT_TRUE(ProjectFiles::backupOriginal(io::path_t(dir.filePath("missing.json")), none));
    EXPECT_TRUE(none.empty());
}
