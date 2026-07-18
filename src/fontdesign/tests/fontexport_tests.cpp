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

#include <QTemporaryDir>

#include "fontdesign/internal/io/fontexporter.h"
#include "fontdesign/internal/io/fontfacereader.h"
#include "fontdesign/internal/project/fontdesignproject.h"
#include "fontdesign/internal/project/projectcommands.h"
#include "fontdesign/internal/smufldatabase.h"

using namespace mu::fontdesign;
using namespace muse;

namespace {
io::path_t fontsRoot()
{
    return io::path_t(FONTDESIGN_FONTS_ROOT);
}

bool containsBytes(const std::vector<uint8_t>& haystack, const std::vector<uint8_t>& needle)
{
    return std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end()) != haystack.end();
}

//! 有向面积符号（shoelace，含控制点足以判方向）：+1 = 逆时针（y 向上）
double contourSignedArea(const GlyphOutline::Contour& c)
{
    double area = 0.0;
    const size_t n = c.points.size();
    for (size_t i = 0; i < n; ++i) {
        const muse::PointF& p = c.points[i].pos;
        const muse::PointF& q = c.points[(i + 1) % n].pos;
        area += p.x() * q.y() - q.x() * p.y();
    }
    return area / 2.0;
}

std::vector<int> contourDirectionSigns(const GlyphOutline& outline)
{
    std::vector<int> signs;
    for (const GlyphOutline::Contour& c : outline.contours()) {
        signs.push_back(contourSignedArea(c) > 0 ? 1 : -1);
    }
    return signs;
}
}

class FontDesign_FontExportTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_db.init((fontsRoot() + "/smufl").toStdString());
        ASSERT_TRUE(m_db.isInited());
    }

    SmuflDatabase m_db;
};

TEST_F(FontDesign_FontExportTests, LelandExportOpensInFreeType)
{
    FontDesignProject project;
    Ret ret = project.load(fontsRoot() + "/leland/Leland.otf",
                           fontsRoot() + "/leland/leland_metadata.json", m_db);
    ASSERT_TRUE(ret) << ret.toString();

    FontExporter::Report report;
    std::vector<uint8_t> bytes;
    Ret buildRet = FontExporter::buildFontBytes(project, bytes, &report);
    ASSERT_TRUE(buildRet) << buildRet.toString();
    ASSERT_FALSE(bytes.empty());
    EXPECT_TRUE(report.ok);
    EXPECT_GT(report.numGlyphs, 1);

    // 写盘后用 FontFaceReader 回读
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const io::path_t outPath = io::path_t(dir.path().toStdString()) + "/LelandExport.otf";
    Ret expRet = FontExporter::exportFont(project, outPath, &report);
    ASSERT_TRUE(expRet) << expRet.toString();

    FontFaceReader::FaceData face;
    Ret readRet = FontFaceReader::read(outPath, face);
    ASSERT_TRUE(readRet) << readRet.toString();
    EXPECT_EQ(static_cast<int>(face.upem), 1000);
    EXPECT_GE(static_cast<int>(face.glyphs.size()), 10);

    // 至少一个 SMuFL 符头码位存在
    bool hasNotehead = false;
    for (const auto& g : face.glyphs) {
        if (g.codepoint == 0xE0A4) {
            hasNotehead = true;
            EXPECT_FALSE(g.outline.isEmpty());
            EXPECT_GT(g.advance, 0);
            break;
        }
    }
    EXPECT_TRUE(hasNotehead);

    //! CFF Private DICT 操作符编码回归：defaultWidthX/nominalWidthX 是
    //! 单字节操作符 20/21（0x14/0x15），不是 12 x 转义（严格解析器会报错）
    const std::vector<uint8_t> GOOD_PRIVATE { 0x8b, 0x14, 0x8b, 0x15 };
    const std::vector<uint8_t> BAD_PRIVATE { 0x8b, 0x0c, 0x14, 0x8b, 0x0c, 0x15 };
    EXPECT_TRUE(containsBytes(bytes, GOOD_PRIVATE)) << "Private DICT missing single-byte width operators";
    EXPECT_FALSE(containsBytes(bytes, BAD_PRIVATE)) << "Private DICT uses invalid escaped width operators";

    //! 轮廓方向（镂空）在导出往返中保持：gClef 外轮廓与内孔方向符号序列一致
    auto findGlyph = [](const FontFaceReader::FaceData& data, char32_t code) -> const FontFaceReader::FaceGlyph* {
        for (const auto& g : data.glyphs) {
            if (g.codepoint == code) {
                return &g;
            }
        }
        return nullptr;
    };

    FontFaceReader::FaceData sourceFace;
    ASSERT_TRUE(FontFaceReader::read(fontsRoot() + "/leland/Leland.otf", sourceFace));

    const FontFaceReader::FaceGlyph* srcClef = findGlyph(sourceFace, 0xE050);
    const FontFaceReader::FaceGlyph* expClef = findGlyph(face, 0xE050);
    ASSERT_TRUE(srcClef != nullptr);
    ASSERT_TRUE(expClef != nullptr);
    EXPECT_GE(srcClef->outline.contours().size(), size_t(2)) << "gClef should have holes";
    EXPECT_EQ(contourDirectionSigns(srcClef->outline), contourDirectionSigns(expClef->outline))
        << "contour winding directions changed across export round-trip";
}

TEST_F(FontDesign_FontExportTests, PreservesSourceLicenseAndFsType)
{
    // 源字体的授权信息（版权/许可证名记录 + fsType 嵌入位）必须保留到导出字体，
    // 否则 OFL 派生字体会丢失必需的版权/许可声明。
    FontFaceReader::FaceData source;
    ASSERT_TRUE(FontFaceReader::read(fontsRoot() + "/leland/Leland.otf", source));
    // Leland 属 SIL OFL，含版权(0)与许可证(13)记录
    ASSERT_TRUE(source.legalNameRecords.count(0) > 0) << "source has no copyright record";

    FontDesignProject project;
    Ret ret = project.load(fontsRoot() + "/leland/Leland.otf",
                           fontsRoot() + "/leland/leland_metadata.json", m_db);
    ASSERT_TRUE(ret) << ret.toString();

    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const io::path_t outPath = io::path_t(dir.path().toStdString()) + "/LelandLicensed.otf";
    FontExporter::Report report;
    ASSERT_TRUE(FontExporter::exportFont(project, outPath, &report));

    FontFaceReader::FaceData exported;
    ASSERT_TRUE(FontFaceReader::read(outPath, exported));

    EXPECT_EQ(exported.fsType, source.fsType) << "fsType embedding bits not preserved";
    ASSERT_TRUE(exported.legalNameRecords.count(0) > 0) << "copyright record dropped on export";
    EXPECT_EQ(exported.legalNameRecords.at(0), source.legalNameRecords.at(0));
    if (source.legalNameRecords.count(13) > 0) {
        ASSERT_TRUE(exported.legalNameRecords.count(13) > 0) << "license record dropped on export";
        EXPECT_EQ(exported.legalNameRecords.at(13), source.legalNameRecords.at(13));
    }
}

TEST_F(FontDesign_FontExportTests, EditedAdvanceSurvivesExport)
{
    FontDesignProject project;
    Ret ret = project.load(fontsRoot() + "/leland/Leland.otf",
                           fontsRoot() + "/leland/leland_metadata.json", m_db);
    ASSERT_TRUE(ret) << ret.toString();

    const char32_t code = 0xE0A4;
    const GlyphItem* before = project.glyph(code);
    ASSERT_NE(before, nullptr);
    const double newAdv = project.spatium() * 2.5;
    project.undoStack().push(std::make_unique<SetAdvanceCommand>(&project, code, newAdv));

    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const io::path_t outPath = io::path_t(dir.path().toStdString()) + "/edited.otf";
    FontExporter::Report report;
    ASSERT_TRUE(FontExporter::exportFont(project, outPath, &report));

    FontFaceReader::FaceData face;
    ASSERT_TRUE(FontFaceReader::read(outPath, face));
    bool found = false;
    for (const auto& g : face.glyphs) {
        if (g.codepoint == code) {
            found = true;
            EXPECT_NEAR(g.advance, newAdv, 1.0);
            break;
        }
    }
    EXPECT_TRUE(found);
}
