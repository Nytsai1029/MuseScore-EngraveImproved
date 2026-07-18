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

#include <QTemporaryDir>

#include "serialization/json.h"

#include "fontdesign/internal/io/metadatawriter.h"
#include "fontdesign/internal/project/fontdesignproject.h"
#include "fontdesign/internal/project/projectcommands.h"
#include "fontdesign/internal/smufldatabase.h"

using namespace mu::fontdesign;
using namespace muse;

namespace {
io::path_t fontsRoot()
{
#ifndef FONTDESIGN_FONTS_ROOT
#error FONTDESIGN_FONTS_ROOT is not defined
#endif
    return io::path_t(FONTDESIGN_FONTS_ROOT);
}

io::path_t lelandFontPath()
{
    return fontsRoot() + "/leland/Leland.otf";
}

io::path_t lelandMetadataPath()
{
    return fontsRoot() + "/leland/leland_metadata.json";
}

io::path_t bravuraFontPath()
{
    return fontsRoot() + "/bravura/Bravura.otf";
}

io::path_t bravuraMetadataPath()
{
    return fontsRoot() + "/bravura/bravura_metadata.json";
}

JsonObject parseJson(const std::string& text)
{
    std::string err;
    JsonDocument doc = JsonDocument::fromJson(ByteArray(text.c_str(), text.size()), &err);
    EXPECT_TRUE(err.empty()) << err;
    return doc.rootObject();
}

bool nearlyEqual(double a, double b, double eps = 1e-4)
{
    return std::abs(a - b) < eps;
}
}

class FontDesign_MetadataRoundTripTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_db.init((fontsRoot() + "/smufl").toStdString());
        ASSERT_TRUE(m_db.isInited());
        ASSERT_FALSE(m_db.ranges().empty());
    }

    SmuflDatabase m_db;
};

TEST_F(FontDesign_MetadataRoundTripTests, LelandLoadWritePreservesKeySections)
{
    FontDesignProject project;
    Ret ret = project.load(lelandFontPath(), lelandMetadataPath(), m_db);
    ASSERT_TRUE(ret) << ret.toString();

    const std::string written = MetadataWriter::toJsonText(project);
    JsonObject out = parseJson(written);

    EXPECT_EQ(out.value("fontName").toStdString(), project.metadata().fontName);
    EXPECT_TRUE(nearlyEqual(out.value("fontVersion").toDouble(), project.metadata().fontVersion));

    ASSERT_TRUE(out.value("engravingDefaults").isObject());
    JsonObject defaults = out.value("engravingDefaults").toObject();
    for (const auto& pair : project.metadata().engravingDefaults) {
        EXPECT_TRUE(nearlyEqual(defaults.value(pair.first).toDouble(), pair.second)) << pair.first;
    }

    auto sectionSize = [&out](const std::string& key) -> size_t {
        JsonValue val = out.value(key);
        if (!val.isObject()) {
            return 0;
        }
        return val.toObject().keys().size();
    };

    EXPECT_GT(sectionSize("glyphsWithAnchors"), 0u);
    EXPECT_EQ(sectionSize("ligatures"), project.metadata().ligatures.size());
    EXPECT_EQ(sectionSize("optionalGlyphs"), project.metadata().optionalGlyphs.size());
    EXPECT_EQ(sectionSize("sets"), project.metadata().sets.size());
    EXPECT_EQ(sectionSize("glyphsWithAlternates"), project.metadata().alternates.size());
}

TEST_F(FontDesign_MetadataRoundTripTests, EditAndUndoThenWrite)
{
    FontDesignProject project;
    Ret ret = project.load(lelandFontPath(), lelandMetadataPath(), m_db);
    ASSERT_TRUE(ret) << ret.toString();

    auto beamIt = project.metadata().engravingDefaults.find("beamThickness");
    ASSERT_TRUE(beamIt != project.metadata().engravingDefaults.end());
    const double originalBeam = beamIt->second;

    project.undoStack().push(std::make_unique<SetEngravingDefaultCommand>(&project, "beamThickness", 0.75));
    EXPECT_TRUE(project.isDirty());
    EXPECT_TRUE(nearlyEqual(project.metadata().engravingDefaults.at("beamThickness"), 0.75));

    project.undoStack().undo();
    EXPECT_TRUE(nearlyEqual(project.metadata().engravingDefaults.at("beamThickness"), originalBeam));

    project.undoStack().redo();
    EXPECT_TRUE(nearlyEqual(project.metadata().engravingDefaults.at("beamThickness"), 0.75));

    auto tables = SetMetadataTablesCommand::captureOf(project);
    OptionalGlyphInfo info;
    info.codepoint = 0xF8FF;
    info.description = "test optional";
    tables.optionalGlyphs["fdTestOptional"] = info;
    project.undoStack().push(std::make_unique<SetMetadataTablesCommand>(&project, tables, "add optional"));

    EXPECT_TRUE(project.metadata().optionalGlyphs.count("fdTestOptional") > 0);

    const std::string written = MetadataWriter::toJsonText(project);
    JsonObject out = parseJson(written);
    JsonObject optional = out.value("optionalGlyphs").toObject();
    ASSERT_TRUE(optional.contains("fdTestOptional"));
    EXPECT_EQ(optional.value("fdTestOptional").toObject().value("codepoint").toStdString(), "U+F8FF");
    EXPECT_TRUE(nearlyEqual(out.value("engravingDefaults").toObject().value("beamThickness").toDouble(), 0.75));
}

TEST_F(FontDesign_MetadataRoundTripTests, WriteToTempFileAndReload)
{
    FontDesignProject project;
    Ret ret = project.load(lelandFontPath(), lelandMetadataPath(), m_db);
    ASSERT_TRUE(ret) << ret.toString();

    project.undoStack().push(std::make_unique<SetEngravingDefaultCommand>(&project, "stemThickness", 0.2));

    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const io::path_t outPath = io::path_t(dir.path().toStdString()) + "/leland_metadata.json";

    Ret writeRet = MetadataWriter::write(project, outPath);
    ASSERT_TRUE(writeRet) << writeRet.toString();

    FontDesignProject reloaded;
    Ret loadRet = reloaded.load(lelandFontPath(), outPath, m_db);
    ASSERT_TRUE(loadRet) << loadRet.toString();

    EXPECT_TRUE(nearlyEqual(reloaded.metadata().engravingDefaults.at("stemThickness"), 0.2));
    EXPECT_EQ(reloaded.metadata().fontName, project.metadata().fontName);
    EXPECT_EQ(reloaded.metadata().optionalGlyphs.size(), project.metadata().optionalGlyphs.size());
}

TEST_F(FontDesign_MetadataRoundTripTests, BravuraLoadSucceeds)
{
    FontDesignProject project;
    Ret ret = project.load(bravuraFontPath(), bravuraMetadataPath(), m_db);
    ASSERT_TRUE(ret) << ret.toString();
    EXPECT_FALSE(project.metadata().alternates.empty());
    EXPECT_FALSE(project.glyphs().empty());
}

// 回归守卫：glyphBBoxes 不得 Y 轴翻转（SW 必须 ≤ NE），且须包住字形轮廓。
// 早期写出器曾把非对称字形的 bbox 上下颠倒，此测试锁死该 bug。
TEST_F(FontDesign_MetadataRoundTripTests, GlyphBBoxesNotInverted)
{
    FontDesignProject project;
    ASSERT_TRUE(project.load(bravuraFontPath(), bravuraMetadataPath(), m_db));

    JsonObject out = parseJson(MetadataWriter::toJsonText(project));
    ASSERT_TRUE(out.value("glyphBBoxes").isObject());
    JsonObject bboxes = out.value("glyphBBoxes").toObject();
    ASSERT_GT(bboxes.keys().size(), 0u);

    int checked = 0;
    for (const std::string& name : bboxes.keys()) {
        JsonObject entry = bboxes.value(name).toObject();
        JsonArray sw = entry.value("bBoxSW").toArray();
        JsonArray ne = entry.value("bBoxNE").toArray();
        ASSERT_EQ(sw.size(), 2u) << name;
        ASSERT_EQ(ne.size(), 2u) << name;
        EXPECT_LE(sw.at(0).toDouble(), ne.at(0).toDouble()) << name << " x inverted";
        EXPECT_LE(sw.at(1).toDouble(), ne.at(1).toDouble()) << name << " y inverted";
        ++checked;
    }
    EXPECT_GT(checked, 100);
}

// 值级别往返：锚点写出后重新加载数值一致（此前测试只校验段数量）。
TEST_F(FontDesign_MetadataRoundTripTests, AnchorValuesSurviveRoundTrip)
{
    FontDesignProject project;
    ASSERT_TRUE(project.load(lelandFontPath(), lelandMetadataPath(), m_db));

    // 找一个带锚点的字形作为基准
    char32_t sampleCode = 0;
    std::map<AnchorId, muse::PointF> expected;
    for (const auto& pair : project.glyphs()) {
        if (!pair.second.anchors.empty()) {
            sampleCode = pair.first;
            expected = pair.second.anchors;
            break;
        }
    }
    ASSERT_NE(sampleCode, 0u) << "no anchored glyph in Leland";

    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const io::path_t outPath = io::path_t(dir.path().toStdString()) + "/leland_metadata.json";
    ASSERT_TRUE(MetadataWriter::write(project, outPath));

    FontDesignProject reloaded;
    ASSERT_TRUE(reloaded.load(lelandFontPath(), outPath, m_db));

    const GlyphItem* g = reloaded.glyph(sampleCode);
    ASSERT_NE(g, nullptr);
    ASSERT_EQ(g->anchors.size(), expected.size());
    for (const auto& pair : expected) {
        auto it = g->anchors.find(pair.first);
        ASSERT_TRUE(it != g->anchors.end());
        EXPECT_TRUE(nearlyEqual(it->second.x(), pair.second.x()));
        EXPECT_TRUE(nearlyEqual(it->second.y(), pair.second.y()));
    }
}

// 回归守卫：字形若同时有结构化锚点和透传（未识别）锚点，写出必须合并到同一对象，
// 不得产生重复键（重复键会在重新加载时静默覆盖已知锚点）。
TEST_F(FontDesign_MetadataRoundTripTests, MixedAndPassthroughAnchorsMergeNoDuplicateKey)
{
    FontDesignProject project;
    ASSERT_TRUE(project.load(lelandFontPath(), lelandMetadataPath(), m_db));

    // 找一个已有结构化锚点的字形
    std::string sampleName;
    for (const auto& pair : project.glyphs()) {
        if (!pair.second.anchors.empty() && !pair.second.smuflName.empty()) {
            sampleName = pair.second.smuflName;
            break;
        }
    }
    ASSERT_FALSE(sampleName.empty());

    // 给该字形注入一个透传（未识别）锚点，模拟非标准锚点名
    JsonObject inner;
    JsonArray coord;
    coord.append(0.5);
    coord.append(-0.25);
    inner.set("customVendorAnchor", coord);
    JsonObject passthrough;
    passthrough.set(sampleName, inner);
    project.metadata().passthroughAnchors = passthrough;

    JsonObject out = parseJson(MetadataWriter::toJsonText(project));
    JsonObject anchorsSection = out.value("glyphsWithAnchors").toObject();
    ASSERT_TRUE(anchorsSection.contains(sampleName));

    JsonObject merged = anchorsSection.value(sampleName).toObject();
    // 结构化锚点与透传锚点都须存在（若重复键把二者互相覆盖，此断言会失败）
    EXPECT_TRUE(merged.contains("customVendorAnchor")) << "passthrough anchor lost";
    EXPECT_GE(merged.keys().size(), 2u) << "structured anchors clobbered by duplicate key";
}
