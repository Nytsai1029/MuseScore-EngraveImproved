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
#include "cffwriter.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>

#include "../project/fontdesignproject.h"

using namespace mu::fontdesign;
using namespace muse;

namespace {
void appendU8(std::vector<uint8_t>& out, uint8_t v)
{
    out.push_back(v);
}

void appendU16BE(std::vector<uint8_t>& out, uint16_t v)
{
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(v & 0xFF));
}

void appendU32BE(std::vector<uint8_t>& out, uint32_t v)
{
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(v & 0xFF));
}

void appendOffset(std::vector<uint8_t>& out, uint32_t value, int offSize)
{
    for (int i = offSize - 1; i >= 0; --i) {
        out.push_back(static_cast<uint8_t>((value >> (8 * i)) & 0xFF));
    }
}

int chooseOffSize(uint32_t maxOffset)
{
    if (maxOffset <= 0xFF) {
        return 1;
    }
    if (maxOffset <= 0xFFFF) {
        return 2;
    }
    if (maxOffset <= 0xFFFFFF) {
        return 3;
    }
    return 4;
}

//! CFF INDEX：count + offSize + offsets[count+1] + data
std::vector<uint8_t> buildIndex(const std::vector<std::vector<uint8_t> >& items)
{
    std::vector<uint8_t> out;
    const uint16_t count = static_cast<uint16_t>(items.size());
    appendU16BE(out, count);
    if (count == 0) {
        return out;
    }

    uint32_t dataSize = 0;
    for (const auto& item : items) {
        dataSize += static_cast<uint32_t>(item.size());
    }

    const int offSize = chooseOffSize(dataSize + 1);
    appendU8(out, static_cast<uint8_t>(offSize));

    uint32_t offset = 1;
    appendOffset(out, offset, offSize);
    for (const auto& item : items) {
        offset += static_cast<uint32_t>(item.size());
        appendOffset(out, offset, offSize);
    }

    for (const auto& item : items) {
        out.insert(out.end(), item.begin(), item.end());
    }

    return out;
}

void encodeInt(std::vector<uint8_t>& out, int v)
{
    if (v >= -107 && v <= 107) {
        appendU8(out, static_cast<uint8_t>(v + 139));
    } else if (v >= 108 && v <= 1131) {
        v -= 108;
        appendU8(out, static_cast<uint8_t>((v >> 8) + 247));
        appendU8(out, static_cast<uint8_t>(v & 0xFF));
    } else if (v >= -1131 && v <= -108) {
        v = -v - 108;
        appendU8(out, static_cast<uint8_t>((v >> 8) + 251));
        appendU8(out, static_cast<uint8_t>(v & 0xFF));
    } else if (v >= -32768 && v <= 32767) {
        appendU8(out, 28);
        appendU16BE(out, static_cast<uint16_t>(v));
    } else {
        appendU8(out, 29);
        appendU32BE(out, static_cast<uint32_t>(v));
    }
}

void encodeReal(std::vector<uint8_t>& out, double v)
{
    // 简化：整数则走 int；否则 16.16 fixed (op 255)
    const double rounded = std::round(v);
    if (std::abs(v - rounded) < 1e-6 && rounded >= -32768 && rounded <= 32767) {
        encodeInt(out, static_cast<int>(rounded));
        return;
    }

    const int32_t fixed = static_cast<int32_t>(std::llround(v * 65536.0));
    appendU8(out, 255);
    appendU32BE(out, static_cast<uint32_t>(fixed));
}

void encodeNumber(std::vector<uint8_t>& out, double v)
{
    encodeReal(out, v);
}

// Type2 operators
constexpr uint8_t OP_HSTEM = 1;
constexpr uint8_t OP_VMOVETO = 4;
constexpr uint8_t OP_RLINETO = 5;
constexpr uint8_t OP_RRCURVETO = 8;
constexpr uint8_t OP_ENDCHAR = 14;
constexpr uint8_t OP_RMOVETO = 21;
constexpr uint8_t OP_HMOVETO = 22;

std::vector<uint8_t> encodeCharstring(const GlyphOutline& outline, int advance, int defaultWidthX, int nominalWidthX)
{
    std::vector<uint8_t> cs;

    // 宽度前缀：若 advance ≠ defaultWidthX，写 (advance - nominalWidthX)
    if (advance != defaultWidthX) {
        encodeInt(cs, advance - nominalWidthX);
    }

    double curX = 0.0;
    double curY = 0.0;
    bool hasPoint = false;

    auto moveTo = [&](double x, double y) {
        const double dx = x - curX;
        const double dy = y - curY;
        if (std::abs(dx) < 1e-6 && std::abs(dy) >= 1e-6) {
            encodeNumber(cs, dy);
            appendU8(cs, OP_VMOVETO);
        } else if (std::abs(dy) < 1e-6 && std::abs(dx) >= 1e-6) {
            encodeNumber(cs, dx);
            appendU8(cs, OP_HMOVETO);
        } else {
            encodeNumber(cs, dx);
            encodeNumber(cs, dy);
            appendU8(cs, OP_RMOVETO);
        }
        curX = x;
        curY = y;
        hasPoint = true;
    };

    auto lineTo = [&](double x, double y) {
        encodeNumber(cs, x - curX);
        encodeNumber(cs, y - curY);
        appendU8(cs, OP_RLINETO);
        curX = x;
        curY = y;
    };

    auto curveTo = [&](double c1x, double c1y, double c2x, double c2y, double x, double y) {
        encodeNumber(cs, c1x - curX);
        encodeNumber(cs, c1y - curY);
        encodeNumber(cs, c2x - c1x);
        encodeNumber(cs, c2y - c1y);
        encodeNumber(cs, x - c2x);
        encodeNumber(cs, y - c2y);
        appendU8(cs, OP_RRCURVETO);
        curX = x;
        curY = y;
    };

    for (const GlyphOutline::Contour& contour : outline.contours()) {
        const auto& pts = contour.points;
        if (pts.size() < 2) {
            continue;
        }

        // 轮廓起点
        moveTo(pts[0].pos.x(), pts[0].pos.y());

        size_t i = 1;
        while (i < pts.size()) {
            if (pts[i].type == GlyphOutline::PointType::OnCurve) {
                lineTo(pts[i].pos.x(), pts[i].pos.y());
                i += 1;
            } else if (i + 1 < pts.size()) {
                const double c1x = pts[i].pos.x();
                const double c1y = pts[i].pos.y();
                const double c2x = pts[i + 1].pos.x();
                const double c2y = pts[i + 1].pos.y();
                const bool hasEnd = (i + 2 < pts.size());
                const double ex = hasEnd ? pts[i + 2].pos.x() : pts[0].pos.x();
                const double ey = hasEnd ? pts[i + 2].pos.y() : pts[0].pos.y();
                curveTo(c1x, c1y, c2x, c2y, ex, ey);
                i += hasEnd ? 3 : 2;
                if (!hasEnd) {
                    break;
                }
            } else {
                break;
            }
        }

        // 闭合回起点（若尚未在起点）
        if (hasPoint && (std::abs(curX - pts[0].pos.x()) > 1e-6 || std::abs(curY - pts[0].pos.y()) > 1e-6)) {
            lineTo(pts[0].pos.x(), pts[0].pos.y());
        }
    }

    appendU8(cs, OP_ENDCHAR);
    return cs;
}

// Top DICT / Private DICT operators (CFF)
void dictInt(std::vector<uint8_t>& out, int v)
{
    encodeInt(out, v);
}

void dictOp(std::vector<uint8_t>& out, uint8_t op)
{
    appendU8(out, op);
}

void dictOp2(std::vector<uint8_t>& out, uint8_t b0, uint8_t b1)
{
    appendU8(out, b0);
    appendU8(out, b1);
}

std::string makeVersionString(double version)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.3f", version);
    return buf;
}

std::string glyphNameOf(const CffWriter::GlyphInput& g)
{
    if (!g.name.empty()) {
        return g.name;
    }
    char buf[16];
    std::snprintf(buf, sizeof(buf), "uni%04X", static_cast<unsigned>(g.codepoint));
    return buf;
}
}

CffWriter::Input CffWriter::fromProject(const FontDesignProject& project)
{
    Input input;
    input.fontName = project.metadata().fontName.empty() ? "Untitled" : project.metadata().fontName;
    input.fontVersion = project.metadata().fontVersion;
    input.upem = static_cast<int>(std::lround(project.upem()));

    std::vector<std::pair<char32_t, const GlyphItem*> > ordered;
    for (const auto& pair : project.glyphs()) {
        ordered.emplace_back(pair.first, &pair.second);
    }
    std::sort(ordered.begin(), ordered.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });

    // glyph 0 = .notdef
    {
        GlyphInput notdef;
        notdef.name = ".notdef";
        notdef.codepoint = 0;
        notdef.advance = input.upem / 2;
        if (!ordered.empty() && ordered.front().first == 0) {
            notdef.outline = ordered.front().second->outline;
            notdef.advance = static_cast<int>(std::lround(ordered.front().second->advance));
            ordered.erase(ordered.begin());
        }
        input.glyphs.push_back(std::move(notdef));
    }

    int xMin = 0, yMin = 0, xMax = 0, yMax = 0;
    bool hasBBox = false;

    auto accumulateBBox = [&](const GlyphOutline& outline) {
        if (outline.isEmpty()) {
            return;
        }
        const RectF bb = outline.boundingRect();
        const int bx0 = static_cast<int>(std::floor(bb.x()));
        const int by0 = static_cast<int>(std::floor(bb.y()));
        const int bx1 = static_cast<int>(std::ceil(bb.x() + bb.width()));
        const int by1 = static_cast<int>(std::ceil(bb.y() + bb.height()));
        if (!hasBBox) {
            xMin = bx0;
            yMin = by0;
            xMax = bx1;
            yMax = by1;
            hasBBox = true;
        } else {
            xMin = std::min(xMin, bx0);
            yMin = std::min(yMin, by0);
            xMax = std::max(xMax, bx1);
            yMax = std::max(yMax, by1);
        }
    };

    accumulateBBox(input.glyphs.front().outline);

    for (const auto& pair : ordered) {
        if (pair.first == 0) {
            continue;
        }
        GlyphInput g;
        g.codepoint = pair.first;
        g.name = pair.second->smuflName;
        if (g.name.empty()) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "uni%04X", static_cast<unsigned>(pair.first));
            g.name = buf;
        }
        g.outline = pair.second->outline;
        g.advance = static_cast<int>(std::lround(pair.second->advance));
        accumulateBBox(g.outline);
        input.glyphs.push_back(std::move(g));
    }

    input.fontBBoxXMin = xMin;
    input.fontBBoxYMin = yMin;
    input.fontBBoxXMax = hasBBox ? xMax : input.upem;
    input.fontBBoxYMax = hasBBox ? yMax : input.upem;

    return input;
}

Ret CffWriter::write(const Input& input, std::vector<uint8_t>& out)
{
    out.clear();
    if (input.glyphs.empty()) {
        return make_ret(Ret::Code::UnknownError, std::string("CFF: no glyphs"));
    }

    const int defaultWidthX = 0;
    const int nominalWidthX = 0;

    // --- strings: standard SIDs 0..390 已内置；自定义从 391 起 ---
    std::vector<std::string> customStrings;
    auto sidOf = [&](const std::string& s) -> int {
        // 少数常用标准名直接用标准 SID（简化：全部进 custom）
        customStrings.push_back(s);
        return 390 + static_cast<int>(customStrings.size()); // SID = 391 + index
    };

    const int versionSid = sidOf(makeVersionString(input.fontVersion));
    const int fullNameSid = sidOf(input.fontName);
    const int familySid = sidOf(input.fontName);
    const int weightSid = sidOf("Regular");

    std::vector<int> charsetSids;
    charsetSids.reserve(input.glyphs.size());
    for (size_t i = 0; i < input.glyphs.size(); ++i) {
        if (i == 0) {
            charsetSids.push_back(0); // .notdef SID 0
            continue;
        }
        charsetSids.push_back(sidOf(glyphNameOf(input.glyphs[i])));
    }

    // CharStrings
    std::vector<std::vector<uint8_t> > charstrings;
    charstrings.reserve(input.glyphs.size());
    for (const GlyphInput& g : input.glyphs) {
        charstrings.push_back(encodeCharstring(g.outline, g.advance, defaultWidthX, nominalWidthX));
    }

    // Private DICT
    //! defaultWidthX / nominalWidthX 是单字节操作符 20/21（非 12 x 转义——
    //! 写错成转义符时 FreeType 宽容跳过，但 fontTools 等严格解析器直接报错）
    std::vector<uint8_t> privateDict;
    dictInt(privateDict, defaultWidthX);
    dictOp(privateDict, 20); // defaultWidthX
    dictInt(privateDict, nominalWidthX);
    dictOp(privateDict, 21); // nominalWidthX

    // 先构建可变偏移的组件：我们采用「单遍后填偏移」——先占位再回填。
    // 布局：
    // Header | Name INDEX | Top DICT INDEX | String INDEX | Global Subr INDEX |
    // Charsets | CharStrings INDEX | Private DICT

    // Name INDEX
    std::vector<uint8_t> nameBytes(input.fontName.begin(), input.fontName.end());
    // PostScript 名：仅允许有限字符
    for (uint8_t& c : nameBytes) {
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-')) {
            c = '-';
        }
    }
    if (nameBytes.empty()) {
        nameBytes = { 'F', 'o', 'n', 't' };
    }
    const std::vector<uint8_t> nameIndex = buildIndex({ nameBytes });

    // String INDEX
    std::vector<std::vector<uint8_t> > stringItems;
    for (const std::string& s : customStrings) {
        stringItems.emplace_back(s.begin(), s.end());
    }
    const std::vector<uint8_t> stringIndex = buildIndex(stringItems);

    // Global Subr INDEX 空
    const std::vector<uint8_t> globalSubrIndex = buildIndex({});

    // Charset format 0: format=0, SID[nGlyphs-1]
    std::vector<uint8_t> charset;
    appendU8(charset, 0);
    for (size_t i = 1; i < charsetSids.size(); ++i) {
        appendU16BE(charset, static_cast<uint16_t>(charsetSids[i]));
    }

    const std::vector<uint8_t> charStringsIndex = buildIndex(charstrings);

    // 计算各段绝对偏移（相对 CFF 起点）
    // Header = 4 bytes
    const size_t headerSize = 4;
    size_t cursor = headerSize;
    cursor += nameIndex.size();

    // Top DICT INDEX 大小未知（含偏移操作数）——先构造 Top DICT 内容时用 5 字节整数编码偏移
    auto encodeOffsetOperand = [](std::vector<uint8_t>& d, uint32_t off) {
        // 用 5-byte integer (29) 保证宽度固定，便于预留
        appendU8(d, 29);
        appendU32BE(d, off);
    };

    // 预计算 Top DICT 之后的偏移
    // topDictIndex = 2 + 1 + offSize*(count+1) + topDictData
    // 我们 count=1，先估算 topDict 数据长度

    // 先假设 topDict 数据长度，迭代一次即可（偏移用 5 字节定长）
    std::vector<uint8_t> topDictData;
    // version
    dictInt(topDictData, versionSid);
    dictOp(topDictData, 0);
    // FullName
    dictInt(topDictData, fullNameSid);
    dictOp(topDictData, 2);
    // FamilyName
    dictInt(topDictData, familySid);
    dictOp(topDictData, 3);
    // Weight
    dictInt(topDictData, weightSid);
    dictOp2(topDictData, 12, 1);
    // FontBBox
    dictInt(topDictData, input.fontBBoxXMin);
    dictInt(topDictData, input.fontBBoxYMin);
    dictInt(topDictData, input.fontBBoxXMax);
    dictInt(topDictData, input.fontBBoxYMax);
    dictOp(topDictData, 5);
    // FontMatrix if upem != 1000
    if (input.upem != 1000) {
        const double s = 1.0 / static_cast<double>(input.upem);
        // real encoding for matrix is painful; use 16.16 via encodeReal path in dict
        // CFF real format (30) — 使用简化：写整数近似不精确。改用标准 real nibble。
        auto appendReal = [](std::vector<uint8_t>& d, double v) {
            // CFF real: operator 30, nibbles
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%.8g", v);
            std::string s = buf;
            std::vector<uint8_t> nibbles;
            for (char c : s) {
                if (c >= '0' && c <= '9') {
                    nibbles.push_back(static_cast<uint8_t>(c - '0'));
                } else if (c == '.') {
                    nibbles.push_back(0xA);
                } else if (c == 'E' || c == 'e') {
                    nibbles.push_back(0xB);
                } else if (c == '-') {
                    nibbles.push_back(0xE);
                } else if (c == '+') {
                    // skip
                }
            }
            nibbles.push_back(0xF);
            appendU8(d, 30);
            for (size_t i = 0; i < nibbles.size(); i += 2) {
                uint8_t hi = nibbles[i];
                uint8_t lo = (i + 1 < nibbles.size()) ? nibbles[i + 1] : 0xF;
                appendU8(d, static_cast<uint8_t>((hi << 4) | lo));
            }
        };
        appendReal(topDictData, s);
        appendReal(topDictData, 0);
        appendReal(topDictData, 0);
        appendReal(topDictData, s);
        appendReal(topDictData, 0);
        appendReal(topDictData, 0);
        dictOp2(topDictData, 12, 7); // FontMatrix
    }

    // charset offset, CharStrings offset, Private size/offset — 先写占位再回填
    const size_t charsetOpPos = topDictData.size();
    encodeOffsetOperand(topDictData, 0);
    dictOp(topDictData, 15); // charset

    const size_t charStringsOpPos = topDictData.size();
    encodeOffsetOperand(topDictData, 0);
    dictOp(topDictData, 17); // CharStrings

    const size_t privateOpPos = topDictData.size();
    encodeOffsetOperand(topDictData, static_cast<uint32_t>(privateDict.size())); // size
    encodeOffsetOperand(topDictData, 0); // offset
    dictOp(topDictData, 18); // Private

    const std::vector<uint8_t> topDictIndex = buildIndex({ topDictData });

    // 绝对偏移
    size_t pos = headerSize;
    pos += nameIndex.size();
    pos += topDictIndex.size();
    pos += stringIndex.size();
    pos += globalSubrIndex.size();
    const uint32_t charsetOffset = static_cast<uint32_t>(pos);
    pos += charset.size();
    const uint32_t charStringsOffset = static_cast<uint32_t>(pos);
    pos += charStringsIndex.size();
    const uint32_t privateOffset = static_cast<uint32_t>(pos);

    // 回填 topDictData 中的偏移（在 INDEX 数据区内）
    // topDict 在 topDictIndex 内的数据起点：
    // 2 (count) + 1 (offSize) + offSize*2 (offsets for count=1) 
    auto patchU32 = [](std::vector<uint8_t>& buf, size_t at, uint32_t v) {
        // at 指向 op 29 之后的 4 字节
        buf[at + 0] = static_cast<uint8_t>((v >> 24) & 0xFF);
        buf[at + 1] = static_cast<uint8_t>((v >> 16) & 0xFF);
        buf[at + 2] = static_cast<uint8_t>((v >> 8) & 0xFF);
        buf[at + 3] = static_cast<uint8_t>(v & 0xFF);
    };

    // 重建 topDict 带正确偏移
    topDictData.clear();
    dictInt(topDictData, versionSid);
    dictOp(topDictData, 0);
    dictInt(topDictData, fullNameSid);
    dictOp(topDictData, 2);
    dictInt(topDictData, familySid);
    dictOp(topDictData, 3);
    dictInt(topDictData, weightSid);
    dictOp2(topDictData, 12, 1);
    dictInt(topDictData, input.fontBBoxXMin);
    dictInt(topDictData, input.fontBBoxYMin);
    dictInt(topDictData, input.fontBBoxXMax);
    dictInt(topDictData, input.fontBBoxYMax);
    dictOp(topDictData, 5);
    if (input.upem != 1000) {
        const double s = 1.0 / static_cast<double>(input.upem);
        auto appendReal = [](std::vector<uint8_t>& d, double v) {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%.8g", v);
            std::string str = buf;
            std::vector<uint8_t> nibbles;
            for (char c : str) {
                if (c >= '0' && c <= '9') {
                    nibbles.push_back(static_cast<uint8_t>(c - '0'));
                } else if (c == '.') {
                    nibbles.push_back(0xA);
                } else if (c == 'E' || c == 'e') {
                    nibbles.push_back(0xB);
                } else if (c == '-') {
                    nibbles.push_back(0xE);
                }
            }
            nibbles.push_back(0xF);
            appendU8(d, 30);
            for (size_t i = 0; i < nibbles.size(); i += 2) {
                uint8_t hi = nibbles[i];
                uint8_t lo = (i + 1 < nibbles.size()) ? nibbles[i + 1] : 0xF;
                appendU8(d, static_cast<uint8_t>((hi << 4) | lo));
            }
        };
        appendReal(topDictData, s);
        appendReal(topDictData, 0);
        appendReal(topDictData, 0);
        appendReal(topDictData, s);
        appendReal(topDictData, 0);
        appendReal(topDictData, 0);
        dictOp2(topDictData, 12, 7);
    }
    encodeOffsetOperand(topDictData, charsetOffset);
    dictOp(topDictData, 15);
    encodeOffsetOperand(topDictData, charStringsOffset);
    dictOp(topDictData, 17);
    encodeOffsetOperand(topDictData, static_cast<uint32_t>(privateDict.size()));
    encodeOffsetOperand(topDictData, privateOffset);
    dictOp(topDictData, 18);

    const std::vector<uint8_t> topDictIndexFinal = buildIndex({ topDictData });

    // 若 topDict 长度变化导致后续偏移变化，需重算——因我们用绝对偏移且 topDict 在 charset 之前，
    // topDict 变长会平移 charset 等。重新计算偏移。
    pos = headerSize + nameIndex.size() + topDictIndexFinal.size() + stringIndex.size() + globalSubrIndex.size();
    const uint32_t charsetOffset2 = static_cast<uint32_t>(pos);
    pos += charset.size();
    const uint32_t charStringsOffset2 = static_cast<uint32_t>(pos);
    pos += charStringsIndex.size();
    const uint32_t privateOffset2 = static_cast<uint32_t>(pos);

    // 第三次写 topDict（偏移已稳定：topDict 自身长度与 charsetOffset 无关的部分固定，
    // 但 encodeOffsetOperand 定长 5 字节，topDict 长度在 charset 偏移写入前后不变！）
    // 第一次估算与第二次 topDict 结构相同（仅偏移数值不同），长度相同 → 偏移稳定。
    topDictData.clear();
    dictInt(topDictData, versionSid);
    dictOp(topDictData, 0);
    dictInt(topDictData, fullNameSid);
    dictOp(topDictData, 2);
    dictInt(topDictData, familySid);
    dictOp(topDictData, 3);
    dictInt(topDictData, weightSid);
    dictOp2(topDictData, 12, 1);
    dictInt(topDictData, input.fontBBoxXMin);
    dictInt(topDictData, input.fontBBoxYMin);
    dictInt(topDictData, input.fontBBoxXMax);
    dictInt(topDictData, input.fontBBoxYMax);
    dictOp(topDictData, 5);
    if (input.upem != 1000) {
        const double s = 1.0 / static_cast<double>(input.upem);
        auto appendReal = [](std::vector<uint8_t>& d, double v) {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%.8g", v);
            std::string str = buf;
            std::vector<uint8_t> nibbles;
            for (char c : str) {
                if (c >= '0' && c <= '9') {
                    nibbles.push_back(static_cast<uint8_t>(c - '0'));
                } else if (c == '.') {
                    nibbles.push_back(0xA);
                } else if (c == 'E' || c == 'e') {
                    nibbles.push_back(0xB);
                } else if (c == '-') {
                    nibbles.push_back(0xE);
                }
            }
            nibbles.push_back(0xF);
            appendU8(d, 30);
            for (size_t i = 0; i < nibbles.size(); i += 2) {
                uint8_t hi = nibbles[i];
                uint8_t lo = (i + 1 < nibbles.size()) ? nibbles[i + 1] : 0xF;
                appendU8(d, static_cast<uint8_t>((hi << 4) | lo));
            }
        };
        appendReal(topDictData, s);
        appendReal(topDictData, 0);
        appendReal(topDictData, 0);
        appendReal(topDictData, s);
        appendReal(topDictData, 0);
        appendReal(topDictData, 0);
        dictOp2(topDictData, 12, 7);
    }
    encodeOffsetOperand(topDictData, charsetOffset2);
    dictOp(topDictData, 15);
    encodeOffsetOperand(topDictData, charStringsOffset2);
    dictOp(topDictData, 17);
    encodeOffsetOperand(topDictData, static_cast<uint32_t>(privateDict.size()));
    encodeOffsetOperand(topDictData, privateOffset2);
    dictOp(topDictData, 18);

    const std::vector<uint8_t> topDictIndexOk = buildIndex({ topDictData });

    // 组装
    out.reserve(headerSize + nameIndex.size() + topDictIndexOk.size() + stringIndex.size()
                + globalSubrIndex.size() + charset.size() + charStringsIndex.size() + privateDict.size());

    // Header
    appendU8(out, 1); // major
    appendU8(out, 0); // minor
    appendU8(out, 4); // hdrSize
    appendU8(out, 4); // offSize (hint for DICT)

    out.insert(out.end(), nameIndex.begin(), nameIndex.end());
    out.insert(out.end(), topDictIndexOk.begin(), topDictIndexOk.end());
    out.insert(out.end(), stringIndex.begin(), stringIndex.end());
    out.insert(out.end(), globalSubrIndex.begin(), globalSubrIndex.end());
    out.insert(out.end(), charset.begin(), charset.end());
    out.insert(out.end(), charStringsIndex.begin(), charStringsIndex.end());
    out.insert(out.end(), privateDict.begin(), privateDict.end());

    (void)charsetOpPos;
    (void)charStringsOpPos;
    (void)privateOpPos;
    (void)patchU32;
    (void)OP_HSTEM;

    return make_ok();
}
