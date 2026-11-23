/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "io/buffer.h"
#include "serialization/xmlstreamwriter.h"
#include "types/string.h"

using namespace muse;

class Serialization_XmlStreamWriterTests : public ::testing::Test
{
};

namespace {
//! What a writer writes for \p value, once as an element and once as an attribute
std::string written(bool exactDoubles, double value)
{
    io::Buffer buffer;
    buffer.open(io::IODevice::WriteOnly);
    {
        XmlStreamWriter xml(&buffer);
        xml.setExactDoubles(exactDoubles);
        xml.element("v", value);
        xml.element("p", { { "x", value } });
        xml.flush();
    }
    const ByteArray data = buffer.data();
    return std::string(data.constChar(), data.size());
}
}

//! The text of every double is read back, by the parser the readers use, as the same double
TEST_F(Serialization_XmlStreamWriterTests, exactDoubleIsReadBackAsTheSameDouble)
{
    for (double value : { 0.1, 1.0, 0.0, -0.0, 1.76389, 3.9370069444444447, 1e-7, 123456789.123, -2.5, 0.1 + 0.2,
                          -1.0 / 3.0, 1.0 / 25.4, std::numeric_limits<double>::max(), std::numeric_limits<double>::min(),
                          -std::numeric_limits<double>::max() }) {
        const std::string text = XmlStreamWriter::exactDoubleText(value);
        bool ok = false;
        const double read = String::fromStdString(text).toDouble(&ok);
        EXPECT_TRUE(ok) << text;
        EXPECT_EQ(read, value) << text;
        EXPECT_EQ(std::signbit(read), std::signbit(value)) << text;
    }
}

//! It is the shortest such text from 6 significant digits on: a value which 6 digits give is written as without the
//! option, more digits are added only where they are needed
TEST_F(Serialization_XmlStreamWriterTests, exactDoubleIsTheShortestTextFromSixDigits)
{
    const std::vector<std::pair<double, std::string> > expected {
        { 0.1, "0.1" },
        { 1.0, "1" },
        { 0.0, "0" },
        { -0.0, "-0" },
        { 1.76389, "1.76389" },
        { 3.9370069444444447, "3.9370069444444447" },
        { 1e-7, "1e-07" },
        { 123456789.123, "123456789.123" },
        { -2.5, "-2.5" },
        { 0.1 + 0.2, "0.30000000000000004" },
        { -1.0 / 3.0, "-0.3333333333333333" },
        { std::numeric_limits<double>::max(), "1.7976931348623157e+308" },
        { std::numeric_limits<double>::min(), "2.2250738585072014e-308" },
    };
    for (const auto& [value, text] : expected) {
        EXPECT_EQ(XmlStreamWriter::exactDoubleText(value), text);
    }
}

//! Elements and attributes are written exactly only when asked for: other files keep their 6 significant digits
TEST_F(Serialization_XmlStreamWriterTests, doublesAreWrittenExactlyOnlyWhenAskedFor)
{
    const double value = 3.9370069444444447;

    const std::string exact = written(true, value);
    EXPECT_NE(exact.find("<v>3.9370069444444447</v>"), std::string::npos) << exact;
    EXPECT_NE(exact.find("x=\"3.9370069444444447\""), std::string::npos) << exact;

    const std::string rounded = written(false, value);
    EXPECT_NE(rounded.find("<v>3.93701</v>"), std::string::npos) << rounded;
    EXPECT_NE(rounded.find("x=\"3.93701\""), std::string::npos) << rounded;
}
