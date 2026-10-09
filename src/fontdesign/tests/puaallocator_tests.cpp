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

#include "fontdesign/internal/fontdesigntypes.h"
#include "fontdesign/internal/io/metadatareader.h"
#include "fontdesign/internal/puaallocator.h"
#include "fontdesign/internal/project/fontdesignproject.h"
#include "fontdesign/internal/smufldatabase.h"

using namespace mu::fontdesign;
using namespace muse;

TEST(FontDesign_PuaAllocatorTests, HexRoundTrip)
{
    EXPECT_EQ(PuaAllocator::toHex(0xF400), "U+F400");
    EXPECT_EQ(PuaAllocator::fromHex("U+F400"), 0xF400u);
    EXPECT_EQ(PuaAllocator::fromHex("u+e0a4"), 0xE0A4u);
    EXPECT_EQ(PuaAllocator::fromHex("not-a-code"), 0u);
}

TEST(FontDesign_PuaAllocatorTests, NextFreeSkipsUsed)
{
#ifndef FONTDESIGN_FONTS_ROOT
#error FONTDESIGN_FONTS_ROOT is not defined
#endif
    SmuflDatabase db;
    db.init(std::string(FONTDESIGN_FONTS_ROOT) + "/smufl");

    FontDesignProject project;
    Ret ret = project.load(io::path_t(std::string(FONTDESIGN_FONTS_ROOT) + "/leland/Leland.otf"),
                           io::path_t(std::string(FONTDESIGN_FONTS_ROOT) + "/leland/leland_metadata.json"),
                           db);
    ASSERT_TRUE(ret) << ret.toString();

    char32_t next = PuaAllocator::nextFreePua(project);
    ASSERT_GE(next, SMUFL_OPTIONAL_START);
    EXPECT_FALSE(PuaAllocator::isUsed(project, next));
}

//! 码位解析必须严格：宽松解析会把写坏的值读成另一个码位，保存时就指向了另一个字形
TEST(FontDesign_PuaAllocatorTests, CodepointParsingIsStrict)
{
    EXPECT_EQ(SmuflDatabase::codepointFromString(" U+E0A4 "), 0xE0A4u);
    EXPECT_EQ(SmuflDatabase::codepointFromString("U+1D100"), 0x1D100u);
    EXPECT_EQ(SmuflDatabase::codepointFromString("U+10FFFF"), 0x10FFFFu);

    EXPECT_EQ(SmuflDatabase::codepointFromString("U+E0A4xyz"), 0u);
    EXPECT_EQ(SmuflDatabase::codepointFromString("U+110000"), 0u);
    EXPECT_EQ(SmuflDatabase::codepointFromString("U+1234567"), 0u);
    EXPECT_EQ(SmuflDatabase::codepointFromString("U+"), 0u);
    EXPECT_EQ(SmuflDatabase::codepointFromString("E0A4"), 0u);
    EXPECT_EQ(SmuflDatabase::codepointFromString(""), 0u);
}

TEST(FontDesign_PuaAllocatorTests, UniGlyphNames)
{
    EXPECT_EQ(MetadataReader::codepointFromUniName("uniE0A4"), 0xE0A4u);
    EXPECT_EQ(MetadataReader::codepointFromUniName("uni1D15E"), 0x1D15Eu);
    EXPECT_EQ(MetadataReader::codepointFromUniName("u1D15E"), 0x1D15Eu);
    EXPECT_EQ(MetadataReader::codepointFromUniName("u0266D"), 0x266Du);

    EXPECT_EQ(MetadataReader::codepointFromUniName("uniform"), 0u);
    EXPECT_EQ(MetadataReader::codepointFromUniName("ufaced"), 0u);      // 小写不是 uniXXXX 命名
    EXPECT_EQ(MetadataReader::codepointFromUniName("uniE0"), 0u);
    EXPECT_EQ(MetadataReader::codepointFromUniName("noteheadBlack"), 0u);
}
