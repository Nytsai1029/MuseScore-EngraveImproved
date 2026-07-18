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
#include "sfntwriter.h"

#include <algorithm>
#include <cstring>

#include <QString>

using namespace mu::fontdesign;
using namespace muse;

namespace {
void appendU16(std::vector<uint8_t>& out, uint16_t v)
{
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(v & 0xFF));
}

void appendU32(std::vector<uint8_t>& out, uint32_t v)
{
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(v & 0xFF));
}

void appendI16(std::vector<uint8_t>& out, int16_t v)
{
    appendU16(out, static_cast<uint16_t>(v));
}

void pad4(std::vector<uint8_t>& out)
{
    while (out.size() % 4 != 0) {
        out.push_back(0);
    }
}

uint32_t tableChecksum(const uint8_t* data, size_t length)
{
    uint32_t sum = 0;
    size_t i = 0;
    for (; i + 3 < length; i += 4) {
        sum += (uint32_t(data[i]) << 24) | (uint32_t(data[i + 1]) << 16)
               | (uint32_t(data[i + 2]) << 8) | uint32_t(data[i + 3]);
    }
    if (i < length) {
        uint32_t last = 0;
        for (int s = 24; i < length; ++i, s -= 8) {
            last |= uint32_t(data[i]) << s;
        }
        sum += last;
    }
    return sum;
}

std::vector<uint8_t> encodeUtf16BE(const std::string& utf8)
{
    // UTF-8 → UTF-16BE。字体名通常为 ASCII，但保留的版权/许可证文本可能含非 ASCII 字符，
    // 因此走完整的 Unicode 转换（QString 内部即 UTF-16）而非逐字节扩展。
    std::vector<uint8_t> out;
    const QString s = QString::fromStdString(utf8);
    out.reserve(static_cast<size_t>(s.size()) * 2);
    for (const QChar ch : s) {
        appendU16(out, ch.unicode());
    }
    return out;
}

std::vector<uint8_t> buildNameTable(const SfntWriter::NameStrings& names)
{
    struct Rec {
        uint16_t nameId;
        std::vector<uint8_t> data;
    };

    std::vector<Rec> recs = {
        { 0, {} },  // copyright — 若源字体有则于下方填充
        { 1, encodeUtf16BE(names.family) },
        { 2, encodeUtf16BE(names.subfamily) },
        { 3, encodeUtf16BE(names.fullName + ":" + names.version) },
        { 4, encodeUtf16BE(names.fullName) },
        { 5, encodeUtf16BE(names.version) },
        { 6, encodeUtf16BE(names.postScriptName) },
    };

    // 移除 copyright 占位（若源字体未提供）
    if (names.legalRecords.find(0) == names.legalRecords.end()) {
        recs.erase(recs.begin());
    }

    // 保留源字体的法律/署名记录（版权、许可证等）；跳过 1-6（由上方重新生成）
    for (const auto& pair : names.legalRecords) {
        if (pair.first >= 1 && pair.first <= 6) {
            continue;
        }
        if (pair.first == 0) {
            recs.front().data = encodeUtf16BE(pair.second);
        } else {
            recs.push_back({ pair.first, encodeUtf16BE(pair.second) });
        }
    }

    // platform 3 (Windows), encoding 1 (Unicode BMP), language 0x0409
    const size_t count = recs.size();
    const uint16_t stringOffset = static_cast<uint16_t>(6 + count * 12);

    std::vector<uint8_t> out;
    appendU16(out, 0); // format
    appendU16(out, static_cast<uint16_t>(count));
    appendU16(out, stringOffset);

    std::vector<uint8_t> storage;
    for (const Rec& r : recs) {
        appendU16(out, 3);     // platform
        appendU16(out, 1);     // encoding
        appendU16(out, 0x0409);
        appendU16(out, r.nameId);
        appendU16(out, static_cast<uint16_t>(r.data.size()));
        appendU16(out, static_cast<uint16_t>(storage.size()));
        storage.insert(storage.end(), r.data.begin(), r.data.end());
    }
    out.insert(out.end(), storage.begin(), storage.end());
    return out;
}

std::vector<uint8_t> buildCmap(const std::map<char32_t, uint16_t>& cmap)
{
    // format 4 (BMP) + format 12 (full)
    std::vector<std::pair<char32_t, uint16_t> > entries(cmap.begin(), cmap.end());
    std::sort(entries.begin(), entries.end());

    // --- format 4: segment per contiguous range in BMP ---
    struct Seg {
        uint16_t start = 0;
        uint16_t end = 0;
        uint16_t startGlyph = 0;
    };
    std::vector<Seg> segs;
    for (const auto& e : entries) {
        if (e.first > 0xFFFF) {
            continue;
        }
        const uint16_t cp = static_cast<uint16_t>(e.first);
        if (!segs.empty() && segs.back().end + 1 == cp
            && segs.back().startGlyph + (segs.back().end - segs.back().start) + 1 == e.second) {
            segs.back().end = cp;
        } else {
            segs.push_back({ cp, cp, e.second });
        }
    }
    // trailing 0xFFFF required
    if (segs.empty() || segs.back().end != 0xFFFF) {
        segs.push_back({ 0xFFFF, 0xFFFF, 0 });
    }

    const uint16_t segCount = static_cast<uint16_t>(segs.size());
    const uint16_t segCountX2 = static_cast<uint16_t>(segCount * 2);
    uint16_t searchRange = 2;
    uint16_t entrySelector = 0;
    while (searchRange * 2 <= segCountX2) {
        searchRange = static_cast<uint16_t>(searchRange * 2);
        entrySelector++;
    }
    const uint16_t rangeShift = static_cast<uint16_t>(segCountX2 - searchRange);

    std::vector<uint8_t> fmt4;
    appendU16(fmt4, 4);
    appendU16(fmt4, 0); // length placeholder
    appendU16(fmt4, 0); // language
    appendU16(fmt4, segCountX2);
    appendU16(fmt4, searchRange);
    appendU16(fmt4, entrySelector);
    appendU16(fmt4, rangeShift);
    for (const Seg& s : segs) {
        appendU16(fmt4, s.end);
    }
    appendU16(fmt4, 0); // reservedPad
    for (const Seg& s : segs) {
        appendU16(fmt4, s.start);
    }
    for (const Seg& s : segs) {
        // idDelta = startGlyph - start
        const int16_t delta = static_cast<int16_t>(static_cast<int>(s.startGlyph) - static_cast<int>(s.start));
        appendI16(fmt4, delta);
    }
    for (size_t i = 0; i < segs.size(); ++i) {
        appendU16(fmt4, 0); // idRangeOffset = 0 → use delta
    }
    // patch length
    const uint16_t fmt4Len = static_cast<uint16_t>(fmt4.size());
    fmt4[2] = static_cast<uint8_t>((fmt4Len >> 8) & 0xFF);
    fmt4[3] = static_cast<uint8_t>(fmt4Len & 0xFF);

    // --- format 12 ---
    struct Group {
        uint32_t start = 0;
        uint32_t end = 0;
        uint32_t startGlyph = 0;
    };
    std::vector<Group> groups;
    for (const auto& e : entries) {
        if (!groups.empty() && groups.back().end + 1 == e.first
            && groups.back().startGlyph + (groups.back().end - groups.back().start) + 1 == e.second) {
            groups.back().end = e.first;
        } else {
            groups.push_back({ e.first, e.first, e.second });
        }
    }

    std::vector<uint8_t> fmt12;
    appendU16(fmt12, 12);
    appendU16(fmt12, 0); // reserved
    appendU32(fmt12, 0); // length placeholder
    appendU32(fmt12, 0); // language
    appendU32(fmt12, static_cast<uint32_t>(groups.size()));
    for (const Group& g : groups) {
        appendU32(fmt12, g.start);
        appendU32(fmt12, g.end);
        appendU32(fmt12, g.startGlyph);
    }
    const uint32_t fmt12Len = static_cast<uint32_t>(fmt12.size());
    fmt12[4] = static_cast<uint8_t>((fmt12Len >> 24) & 0xFF);
    fmt12[5] = static_cast<uint8_t>((fmt12Len >> 16) & 0xFF);
    fmt12[6] = static_cast<uint8_t>((fmt12Len >> 8) & 0xFF);
    fmt12[7] = static_cast<uint8_t>(fmt12Len & 0xFF);

    // cmap header: version, numTables=2
    // encoding records: platform 3 enc 1 → fmt4; platform 3 enc 10 → fmt12
    std::vector<uint8_t> out;
    appendU16(out, 0);
    appendU16(out, 2);
    const uint32_t recSize = 8;
    const uint32_t headerSize = 4 + 2 * recSize;
    const uint32_t fmt4Offset = headerSize;
    const uint32_t fmt12Offset = headerSize + static_cast<uint32_t>(fmt4.size());

    // record 0: Windows BMP
    appendU16(out, 3);
    appendU16(out, 1);
    appendU32(out, fmt4Offset);
    // record 1: Windows full
    appendU16(out, 3);
    appendU16(out, 10);
    appendU32(out, fmt12Offset);

    out.insert(out.end(), fmt4.begin(), fmt4.end());
    out.insert(out.end(), fmt12.begin(), fmt12.end());
    return out;
}

std::vector<uint8_t> buildHead(const SfntWriter::Metrics& m, uint32_t checkSumAdjustment)
{
    std::vector<uint8_t> out;
    appendU16(out, 1); // major
    appendU16(out, 0); // minor
    appendU32(out, 0x00010000); // fontRevision 1.0
    appendU32(out, checkSumAdjustment);
    appendU32(out, 0x5F0F3CF5); // magic
    appendU16(out, 0x000B); // flags
    appendU16(out, static_cast<uint16_t>(m.upem));
    // created / modified: zero
    for (int i = 0; i < 16; ++i) {
        out.push_back(0);
    }
    appendI16(out, static_cast<int16_t>(m.xMin));
    appendI16(out, static_cast<int16_t>(m.yMin));
    appendI16(out, static_cast<int16_t>(m.xMax));
    appendI16(out, static_cast<int16_t>(m.yMax));
    appendU16(out, 0); // macStyle
    appendU16(out, 8); // lowestRecPPEM
    appendI16(out, 2); // fontDirectionHint
    appendI16(out, 0); // indexToLocFormat
    appendI16(out, 0); // glyphDataFormat
    return out;
}

std::vector<uint8_t> buildHhea(const SfntWriter::Metrics& m, uint16_t numberOfHMetrics)
{
    std::vector<uint8_t> out;
    appendU16(out, 1);
    appendU16(out, 0);
    appendI16(out, static_cast<int16_t>(m.ascender));
    appendI16(out, static_cast<int16_t>(m.descender));
    appendI16(out, static_cast<int16_t>(m.lineGap));
    uint16_t maxAdv = 0;
    appendU16(out, 0); // advanceWidthMax placeholder — patched by caller if needed
    appendI16(out, 0); // minLeftSideBearing
    appendI16(out, 0); // minRightSideBearing
    appendI16(out, 0); // xMaxExtent
    appendI16(out, 1); // caretSlopeRise
    appendI16(out, 0);
    appendI16(out, 0);
    appendI16(out, 0);
    appendI16(out, 0);
    appendI16(out, 0);
    appendI16(out, 0);
    appendI16(out, 0); // metricDataFormat
    appendU16(out, numberOfHMetrics);
    (void)maxAdv;
    return out;
}

std::vector<uint8_t> buildHmtx(const std::vector<SfntWriter::GlyphMetrics>& glyphs)
{
    std::vector<uint8_t> out;
    out.reserve(glyphs.size() * 4);
    for (const auto& g : glyphs) {
        appendU16(out, static_cast<uint16_t>(std::max(0, g.advance)));
        appendI16(out, static_cast<int16_t>(g.lsb));
    }
    return out;
}

std::vector<uint8_t> buildMaxp(uint16_t numGlyphs)
{
    std::vector<uint8_t> out;
    appendU32(out, 0x00005000); // version 0.5
    appendU16(out, numGlyphs);
    return out;
}

std::vector<uint8_t> buildOS2(const SfntWriter::Metrics& m)
{
    std::vector<uint8_t> out(96, 0);
    // version 4
    out[0] = 0;
    out[1] = 4;
    // usWeightClass = 400
    out[4] = 0x01;
    out[5] = 0x90;
    // usWidthClass = 5
    out[6] = 0;
    out[7] = 5;
    // fsType
    out[8] = static_cast<uint8_t>((m.fsType >> 8) & 0xFF);
    out[9] = static_cast<uint8_t>(m.fsType & 0xFF);
    // ySubscriptXSize etc. leave 0
    // sFamilyClass 0
    // panose 10 bytes zero
    // ulUnicodeRange — set PUA bit (bit 57 → range 1 bit 25? )
    // Unicode range bit 57 is Private Use (plane 0): in ulUnicodeRange2 bit 25
    out[36] = 0;
    out[37] = 0;
    out[38] = 0x02; // bit 25 of range2? ranges are 4 uint32 LE in file as BE...
    // Actually OS/2 stores as big-endian uint32. Bit 57 overall → range index 57, which is in ulUnicodeRange1 (bits 32-63) bit 25.
    // ulUnicodeRange1 at offset 46 (after version through panose...)
    // Layout: version(2) avgCharWidth(2) weight(2) width(2) fsType(2) subscript(10) superscript(10) strikeout(4) familyClass(2) panose(10) = 46
    // ulUnicodeRange1 at 46, range2 at 50, range3 at 54, range4 at 58
    // Bit 57 → range1 bit (57-32)=25
    out[46 + 0] = 0;
    out[46 + 1] = 0;
    out[46 + 2] = 0x02; // bit 25 in high half? BE: byte0 is MSB bits 24-31
    out[46 + 3] = 0;
    // Actually bit 25 in a BE uint32 is in byte 0 (bits 24-31), value 0x02 is bit 25. Yes.

    // achVendID "MUE " 
    out[58] = 'M';
    out[59] = 'U';
    out[60] = 'E';
    out[61] = ' ';
    // fsSelection regular bit 6 = 0x40
    out[62] = 0;
    out[63] = 0x40;
    // usFirstCharIndex / usLastCharIndex
    out[64] = 0;
    out[65] = 0x20;
    out[66] = 0xFF;
    out[67] = 0xFF;
    // sTypoAscender/Descender/LineGap
    auto putI16 = [&](size_t at, int16_t v) {
        out[at] = static_cast<uint8_t>((v >> 8) & 0xFF);
        out[at + 1] = static_cast<uint8_t>(v & 0xFF);
    };
    putI16(68, static_cast<int16_t>(m.ascender));
    putI16(70, static_cast<int16_t>(m.descender));
    putI16(72, static_cast<int16_t>(m.lineGap));
    // usWinAscent / usWinDescent
    putI16(74, static_cast<int16_t>(std::max(0, m.ascender)));
    putI16(76, static_cast<int16_t>(std::max(0, -m.descender)));
    return out;
}

std::vector<uint8_t> buildPost()
{
    std::vector<uint8_t> out;
    appendU32(out, 0x00030000); // version 3
    appendU32(out, 0); // italicAngle
    appendI16(out, 0); // underlinePosition
    appendI16(out, 0); // underlineThickness
    appendU32(out, 1); // isFixedPitch
    appendU32(out, 0);
    appendU32(out, 0);
    appendU32(out, 0);
    appendU32(out, 0);
    return out;
}

struct Table {
    char tag[4];
    std::vector<uint8_t> data;
};

uint32_t tagValue(const char* t)
{
    return (uint32_t(uint8_t(t[0])) << 24) | (uint32_t(uint8_t(t[1])) << 16)
           | (uint32_t(uint8_t(t[2])) << 8) | uint32_t(uint8_t(t[3]));
}
}

Ret SfntWriter::write(const Input& input, std::vector<uint8_t>& out)
{
    out.clear();
    if (input.cffTable.empty()) {
        return make_ret(Ret::Code::UnknownError, std::string("sfnt: empty CFF table"));
    }
    if (input.glyphMetrics.empty()) {
        return make_ret(Ret::Code::UnknownError, std::string("sfnt: no glyph metrics"));
    }

    const uint16_t numGlyphs = static_cast<uint16_t>(input.glyphMetrics.size());

    // advanceWidthMax for hhea
    int maxAdv = 0;
    for (const auto& g : input.glyphMetrics) {
        maxAdv = std::max(maxAdv, g.advance);
    }

    std::vector<uint8_t> hhea = buildHhea(input.metrics, numGlyphs);
    // patch advanceWidthMax at offset 10
    hhea[10] = static_cast<uint8_t>((maxAdv >> 8) & 0xFF);
    hhea[11] = static_cast<uint8_t>(maxAdv & 0xFF);

    std::vector<Table> tables;
    tables.push_back({ { 'C', 'F', 'F', ' ' }, input.cffTable });
    tables.push_back({ { 'O', 'S', '/', '2' }, buildOS2(input.metrics) });
    tables.push_back({ { 'c', 'm', 'a', 'p' }, buildCmap(input.cmap) });
    // head with checkSumAdjustment=0 first
    tables.push_back({ { 'h', 'e', 'a', 'd' }, buildHead(input.metrics, 0) });
    tables.push_back({ { 'h', 'h', 'e', 'a' }, std::move(hhea) });
    tables.push_back({ { 'h', 'm', 't', 'x' }, buildHmtx(input.glyphMetrics) });
    tables.push_back({ { 'm', 'a', 'x', 'p' }, buildMaxp(numGlyphs) });
    tables.push_back({ { 'n', 'a', 'm', 'e' }, buildNameTable(input.names) });
    tables.push_back({ { 'p', 'o', 's', 't' }, buildPost() });

    std::sort(tables.begin(), tables.end(), [](const Table& a, const Table& b) {
        return tagValue(a.tag) < tagValue(b.tag);
    });

    const uint16_t numTables = static_cast<uint16_t>(tables.size());
    uint16_t searchRange = 16;
    uint16_t entrySelector = 0;
    while (searchRange * 2 <= numTables * 16) {
        searchRange = static_cast<uint16_t>(searchRange * 2);
        entrySelector++;
    }
    // searchRange is max power of 2 * 16
    searchRange = 16;
    entrySelector = 0;
    {
        uint16_t n = 1;
        while (n * 2 <= numTables) {
            n = static_cast<uint16_t>(n * 2);
            entrySelector++;
        }
        searchRange = static_cast<uint16_t>(n * 16);
    }
    const uint16_t rangeShift = static_cast<uint16_t>(numTables * 16 - searchRange);

    // Offset table
    std::vector<uint8_t> font;
    appendU32(font, 0x4F54544F); // 'OTTO'
    appendU16(font, numTables);
    appendU16(font, searchRange);
    appendU16(font, entrySelector);
    appendU16(font, rangeShift);

    const size_t tableRecordStart = font.size();
    // placeholder records
    for (uint16_t i = 0; i < numTables; ++i) {
        appendU32(font, 0);
        appendU32(font, 0);
        appendU32(font, 0);
        appendU32(font, 0);
    }

    size_t headTableOffset = 0;
    for (size_t i = 0; i < tables.size(); ++i) {
        pad4(font);
        const uint32_t offset = static_cast<uint32_t>(font.size());
        if (std::memcmp(tables[i].tag, "head", 4) == 0) {
            headTableOffset = offset;
        }
        font.insert(font.end(), tables[i].data.begin(), tables[i].data.end());
        const uint32_t length = static_cast<uint32_t>(tables[i].data.size());
        const uint32_t sum = tableChecksum(tables[i].data.data(), tables[i].data.size());

        const size_t rec = tableRecordStart + i * 16;
        font[rec + 0] = static_cast<uint8_t>(tables[i].tag[0]);
        font[rec + 1] = static_cast<uint8_t>(tables[i].tag[1]);
        font[rec + 2] = static_cast<uint8_t>(tables[i].tag[2]);
        font[rec + 3] = static_cast<uint8_t>(tables[i].tag[3]);
        font[rec + 4] = static_cast<uint8_t>((sum >> 24) & 0xFF);
        font[rec + 5] = static_cast<uint8_t>((sum >> 16) & 0xFF);
        font[rec + 6] = static_cast<uint8_t>((sum >> 8) & 0xFF);
        font[rec + 7] = static_cast<uint8_t>(sum & 0xFF);
        font[rec + 8] = static_cast<uint8_t>((offset >> 24) & 0xFF);
        font[rec + 9] = static_cast<uint8_t>((offset >> 16) & 0xFF);
        font[rec + 10] = static_cast<uint8_t>((offset >> 8) & 0xFF);
        font[rec + 11] = static_cast<uint8_t>(offset & 0xFF);
        font[rec + 12] = static_cast<uint8_t>((length >> 24) & 0xFF);
        font[rec + 13] = static_cast<uint8_t>((length >> 16) & 0xFF);
        font[rec + 14] = static_cast<uint8_t>((length >> 8) & 0xFF);
        font[rec + 15] = static_cast<uint8_t>(length & 0xFF);
    }

    pad4(font);

    // checkSumAdjustment
    const uint32_t entireSum = tableChecksum(font.data(), font.size());
    const uint32_t adjustment = 0xB1B0AFBAu - entireSum;
    if (headTableOffset + 12 <= font.size()) {
        font[headTableOffset + 8] = static_cast<uint8_t>((adjustment >> 24) & 0xFF);
        font[headTableOffset + 9] = static_cast<uint8_t>((adjustment >> 16) & 0xFF);
        font[headTableOffset + 10] = static_cast<uint8_t>((adjustment >> 8) & 0xFF);
        font[headTableOffset + 11] = static_cast<uint8_t>(adjustment & 0xFF);
    }

    out = std::move(font);
    return make_ok();
}
