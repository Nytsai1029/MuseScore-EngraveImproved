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
