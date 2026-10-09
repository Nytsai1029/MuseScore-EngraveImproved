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

#include <clocale>
#include <cmath>
#include <cstdio>
#include <set>

#include <QFile>
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

namespace {
JsonObject readJsonFile(const io::path_t& path)
{
    QFile file(path.toQString());
    EXPECT_TRUE(file.open(QIODevice::ReadOnly)) << path.toStdString();
    const QByteArray data = file.readAll();
    return parseJson(std::string(data.constData(), static_cast<size_t>(data.size())));
}

JsonObject sectionOf(const JsonObject& root, const std::string& key)
{
    JsonValue val = root.value(key);
    return val.isObject() ? val.toObject() : JsonObject();
}

//! 文本的 glyphsWithAnchors 段里 "<glyphKey>": { … } 这个对象中 anchorName 出现的次数。
//! 锚点对象里只有数组、没有嵌套对象，第一个 '}' 即对象结尾。
//! （解析后再数没用：重复键在解析时后者静默覆盖前者）
int anchorOccurrences(const std::string& text, const std::string& glyphKey, const std::string& anchorName)
{
    const size_t section = text.find("\"glyphsWithAnchors\": {");
    if (section == std::string::npos) {
        return -1;
    }
    const size_t start = text.find("\"" + glyphKey + "\": {", section + 1);
    if (start == std::string::npos) {
        return -1;
    }
    const size_t end = text.find('}', start);
    const std::string needle = "\"" + anchorName + "\"";

    int count = 0;
    size_t pos = text.find(needle, start);
    while (pos != std::string::npos && pos < end) {
        ++count;
        pos = text.find(needle, pos + 1);
    }
    return count;
}
}

//! 「读入→保存」不得丢条目。曾经只给 glyphnames / optionalGlyphs 里有名字的字形写 bbox：
//! Leland 丢 49 条、MuseJazz 15 条、Gootville 90 条（名字只在 ligatures / alternates
//! 里声明，或只是字体自带的字形名）。
TEST_F(FontDesign_MetadataRoundTripTests, LoadWriteKeepsEveryGlyphEntry)
{
    const std::vector<std::pair<io::path_t, io::path_t> > fonts = {
        { lelandFontPath(), lelandMetadataPath() },
        { bravuraFontPath(), bravuraMetadataPath() },
        { fontsRoot() + "/musejazz/MuseJazz.otf", fontsRoot() + "/musejazz/metadata.json" },
        { fontsRoot() + "/gootville/Gootville.otf", fontsRoot() + "/gootville/metadata.json" },
    };

    for (const auto& font : fonts) {
        SCOPED_TRACE(font.second.toStdString());

        FontDesignProject project;
        ASSERT_TRUE(project.load(font.first, font.second, m_db));

        const JsonObject source = readJsonFile(font.second);
        const JsonObject written = parseJson(MetadataWriter::toJsonText(project));

        for (const char* section : { "glyphBBoxes", "glyphAdvanceWidths", "glyphsWithAnchors" }) {
            const JsonObject before = sectionOf(source, section);
            const JsonObject after = sectionOf(written, section);
            for (const std::string& name : before.keys()) {
                EXPECT_TRUE(after.contains(name)) << section << " lost " << name;
            }
        }

        // 锚点条目内部的每个锚点名也都在
        const JsonObject anchorsBefore = sectionOf(source, "glyphsWithAnchors");
        const JsonObject anchorsAfter = sectionOf(written, "glyphsWithAnchors");
        for (const std::string& name : anchorsBefore.keys()) {
            if (!anchorsBefore.value(name).isObject() || !anchorsAfter.value(name).isObject()) {
                continue;
            }
            const JsonObject after = anchorsAfter.value(name).toObject();
            for (const std::string& anchor : anchorsBefore.value(name).toObject().keys()) {
                EXPECT_TRUE(after.contains(anchor)) << name << " lost anchor " << anchor;
            }
        }
    }
}

//! 对不上字形的 bbox 条目原样带回，数值不变
TEST_F(FontDesign_MetadataRoundTripTests, UnmatchedBBoxValuesAreUnchanged)
{
    FontDesignProject project;
    ASSERT_TRUE(project.load(lelandFontPath(), lelandMetadataPath(), m_db));

    std::set<std::string> glyphNames;
    for (const auto& pair : project.glyphs()) {
        glyphNames.insert(pair.second.smuflName);
    }

    const JsonObject before = sectionOf(readJsonFile(lelandMetadataPath()), "glyphBBoxes");
    const JsonObject after = sectionOf(parseJson(MetadataWriter::toJsonText(project)), "glyphBBoxes");

    int unmatched = 0;
    for (const std::string& name : before.keys()) {
        if (glyphNames.count(name) > 0) {
            continue;
        }
        ++unmatched;
        ASSERT_TRUE(after.value(name).isObject()) << name;
        for (const char* corner : { "bBoxNE", "bBoxSW" }) {
            JsonArray a = before.value(name).toObject().value(corner).toArray();
            JsonArray b = after.value(name).toObject().value(corner).toArray();
            ASSERT_EQ(a.size(), 2u);
            ASSERT_EQ(b.size(), 2u);
            EXPECT_TRUE(nearlyEqual(a.at(0).toDouble(), b.at(0).toDouble())) << name;
            EXPECT_TRUE(nearlyEqual(a.at(1).toDouble(), b.at(1).toDouble())) << name;
        }
    }
    EXPECT_GT(unmatched, 0) << "Leland is expected to have bbox entries no glyph name resolves to";
}

//! 无名字形的锚点以 uniXXXX 为键写出；读回时必须重新挂到字形上，
//! 再次编辑后同一对象里不得出现重复键
TEST_F(FontDesign_MetadataRoundTripTests, AnchorOnUnnamedGlyphReattachesAfterReload)
{
    FontDesignProject project;
    ASSERT_TRUE(project.load(lelandFontPath(), lelandMetadataPath(), m_db));

    char32_t code = 0;
    for (const auto& pair : project.glyphs()) {
        if (pair.second.smuflName.empty() && pair.second.anchors.empty() && !pair.second.outline.isEmpty()) {
            code = pair.first;
            break;
        }
    }
    ASSERT_NE(code, 0u) << "Leland is expected to contain an unnamed glyph";

    project.undoStack().push(std::make_unique<SetAnchorCommand>(&project, code, AnchorId::stemUpSE,
                                                                PointF(1.25, -0.5)));

    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const io::path_t tempPath = io::path_t(dir.path().toStdString()) + "/leland_roundtrip.json";
    ASSERT_TRUE(MetadataWriter::write(project, tempPath));

    FontDesignProject reloaded;
    ASSERT_TRUE(reloaded.load(lelandFontPath(), tempPath, m_db));
    const GlyphItem* glyph = reloaded.glyph(code);
    ASSERT_NE(glyph, nullptr);
    ASSERT_TRUE(glyph->anchors.count(AnchorId::stemUpSE)) << "anchor detached from its glyph after reload";
    EXPECT_TRUE(nearlyEqual(glyph->anchors.at(AnchorId::stemUpSE).x(), 1.25));
    EXPECT_TRUE(nearlyEqual(glyph->anchors.at(AnchorId::stemUpSE).y(), -0.5));

    reloaded.undoStack().push(std::make_unique<SetAnchorCommand>(&reloaded, code, AnchorId::stemUpSE,
                                                                 PointF(2.0, 0.0)));

    char key[16];
    std::snprintf(key, sizeof(key), "uni%04X", static_cast<unsigned>(code));
    EXPECT_EQ(anchorOccurrences(MetadataWriter::toJsonText(reloaded), key, "stemUpSE"), 1);
}

//! 整条透传的锚点对象（读入时字体里还没有这个字形）与结构化锚点同名时，只写结构化的那个
TEST_F(FontDesign_MetadataRoundTripTests, PassthroughAnchorNeverDuplicatesStructuredKey)
{
    FontDesignProject project;
    ASSERT_TRUE(project.load(lelandFontPath(), lelandMetadataPath(), m_db));

    std::string sampleName;
    std::string anchorName;
    for (const auto& pair : project.glyphs()) {
        if (!pair.second.anchors.empty() && !pair.second.smuflName.empty()) {
            sampleName = pair.second.smuflName;
            anchorName = anchorNameById(pair.second.anchors.begin()->first);
            break;
        }
    }
    ASSERT_FALSE(sampleName.empty());

    JsonArray coord;
    coord.append(9.0);
    coord.append(9.0);
    JsonObject inner;
    inner.set(anchorName, coord);
    inner.set("customVendorAnchor", coord);
    JsonObject passthrough;
    passthrough.set(sampleName, inner);
    project.metadata().passthroughAnchors = passthrough;

    const std::string text = MetadataWriter::toJsonText(project);
    EXPECT_EQ(anchorOccurrences(text, sampleName, anchorName), 1);
    EXPECT_EQ(anchorOccurrences(text, sampleName, "customVendorAnchor"), 1);
}

//! 小数点为逗号的 locale 下，snprintf("%f") 会写出 "0,25" —— 非法 JSON
TEST_F(FontDesign_MetadataRoundTripTests, JsonDoesNotDependOnNumericLocale)
{
    FontDesignProject project;
    ASSERT_TRUE(project.load(lelandFontPath(), lelandMetadataPath(), m_db));

    const std::string reference = MetadataWriter::toJsonText(project);

    const std::string previous = std::setlocale(LC_NUMERIC, nullptr);
    if (!std::setlocale(LC_NUMERIC, "de_DE.UTF-8")) {
        GTEST_SKIP() << "de_DE.UTF-8 locale is not available";
    }
    const std::string underCommaLocale = MetadataWriter::toJsonText(project);
    std::setlocale(LC_NUMERIC, previous.c_str());

    EXPECT_EQ(reference, underCommaLocale);
}

//! 类型不在预期内的值不解释、不丢弃
TEST_F(FontDesign_MetadataRoundTripTests, UnexpectedValueTypesPassThrough)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const io::path_t jsonPath = io::path_t(dir.path().toStdString()) + "/odd.json";
    {
        QFile file(jsonPath.toQString());
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        file.write(R"({
            "fontName": "Leland",
            "fontVersion": 1.5,
            "engravingDefaults": { "stemThickness": 0.12, "futureKey": "text value" },
            "glyphsWithAnchors": {
                "noteheadBlack": { "stemUpSE": [1.3, 0.16], "cutOutNE": [1.0] },
                "glyphTheFontDoesNotHave": { "stemUpSE": [1.0, 2.0] }
            }
        })");
    }

    FontDesignProject project;
    ASSERT_TRUE(project.load(lelandFontPath(), jsonPath, m_db));

    const JsonObject written = parseJson(MetadataWriter::toJsonText(project));

    const JsonObject defaults = sectionOf(written, "engravingDefaults");
    EXPECT_TRUE(nearlyEqual(defaults.value("stemThickness").toDouble(), 0.12));
    EXPECT_EQ(defaults.value("futureKey").toStdString(), std::string("text value"));

    const JsonObject anchors = sectionOf(written, "glyphsWithAnchors");
    ASSERT_TRUE(anchors.value("noteheadBlack").isObject());
    EXPECT_TRUE(anchors.value("noteheadBlack").toObject().contains("stemUpSE"));
    EXPECT_TRUE(anchors.value("noteheadBlack").toObject().contains("cutOutNE")) << "malformed anchor dropped";
    EXPECT_TRUE(anchors.contains("glyphTheFontDoesNotHave")) << "anchors of an absent glyph dropped";
}
