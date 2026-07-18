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
#include "fontfacereader.h"

#include <QString>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_IDS_H

#include "log.h"

using namespace mu::fontdesign;
using namespace muse;

namespace {
//! 需要从源字体保留的 name 记录（法律/署名类）
bool isLegalNameId(uint16_t nameId)
{
    switch (nameId) {
    case 0:   // copyright
    case 7:   // trademark
    case 8:   // manufacturer
    case 9:   // designer
    case 10:  // description
    case 11:  // vendor URL
    case 12:  // designer URL
    case 13:  // license description
    case 14:  // license URL
        return true;
    default:
        return false;
    }
}

std::string decodeSfntName(const FT_SfntName& sfnt)
{
    // platform 3 (Windows) 与 platform 0 (Unicode) 为 UTF-16BE；platform 1 (Mac Roman) 近似 Latin-1。
    // sfnt.string 是字体内的原始大端字节，须手动按大端解码（不能当作本机端序 char16_t）。
    if (sfnt.platform_id == TT_PLATFORM_MICROSOFT || sfnt.platform_id == TT_PLATFORM_APPLE_UNICODE) {
        QString s;
        s.reserve(static_cast<int>(sfnt.string_len / 2));
        for (FT_UInt i = 0; i + 1 < sfnt.string_len; i += 2) {
            const char16_t u = static_cast<char16_t>((sfnt.string[i] << 8) | sfnt.string[i + 1]);
            s.append(QChar(u));
        }
        return s.toStdString();
    }

    if (sfnt.platform_id == TT_PLATFORM_MACINTOSH) {
        return QString::fromLatin1(reinterpret_cast<const char*>(sfnt.string), static_cast<int>(sfnt.string_len)).toStdString();
    }

    return std::string();
}

//! 抽取需保留的 name 记录，优先 Windows 平台（Unicode）版本
void readLegalNameRecords(FT_Face face, std::map<uint16_t, std::string>& out)
{
    const FT_UInt count = FT_Get_Sfnt_Name_Count(face);
    for (FT_UInt i = 0; i < count; ++i) {
        FT_SfntName sfnt;
        if (FT_Get_Sfnt_Name(face, i, &sfnt) != 0) {
            continue;
        }
        if (!isLegalNameId(static_cast<uint16_t>(sfnt.name_id))) {
            continue;
        }

        std::string value = decodeSfntName(sfnt);
        if (value.empty()) {
            continue;
        }

        auto it = out.find(static_cast<uint16_t>(sfnt.name_id));
        // 优先 Windows 平台记录；否则保留首个非空
        if (it == out.end()) {
            out[static_cast<uint16_t>(sfnt.name_id)] = std::move(value);
        } else if (sfnt.platform_id == TT_PLATFORM_MICROSOFT) {
            it->second = std::move(value);
        }
    }
}
}

namespace {
struct DecomposeContext {
    GlyphOutline* outline = nullptr;
    PointF current;
};

inline PointF toPointF(const FT_Vector* v)
{
    return PointF(static_cast<double>(v->x), static_cast<double>(v->y));
}

int ftMoveTo(const FT_Vector* to, void* user)
{
    DecomposeContext* ctx = static_cast<DecomposeContext*>(user);
    GlyphOutline::Contour contour;
    contour.points.emplace_back(toPointF(to), GlyphOutline::PointType::OnCurve);
    ctx->outline->contours().push_back(std::move(contour));
    ctx->current = toPointF(to);
    return 0;
}

int ftLineTo(const FT_Vector* to, void* user)
{
    DecomposeContext* ctx = static_cast<DecomposeContext*>(user);
    ctx->outline->contours().back().points.emplace_back(toPointF(to), GlyphOutline::PointType::OnCurve);
    ctx->current = toPointF(to);
    return 0;
}

int ftConicTo(const FT_Vector* control, const FT_Vector* to, void* user)
{
    DecomposeContext* ctx = static_cast<DecomposeContext*>(user);

    // 二次贝塞尔无损升阶为三次
    PointF q0 = ctx->current;
    PointF qc = toPointF(control);
    PointF q2 = toPointF(to);
    PointF c1 = q0 + (qc - q0) * (2.0 / 3.0);
    PointF c2 = q2 + (qc - q2) * (2.0 / 3.0);

    std::vector<GlyphOutline::Point>& points = ctx->outline->contours().back().points;
    points.emplace_back(c1, GlyphOutline::PointType::Control);
    points.emplace_back(c2, GlyphOutline::PointType::Control);
    points.emplace_back(q2, GlyphOutline::PointType::OnCurve);

    ctx->current = q2;
    return 0;
}

int ftCubicTo(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user)
{
    DecomposeContext* ctx = static_cast<DecomposeContext*>(user);

    std::vector<GlyphOutline::Point>& points = ctx->outline->contours().back().points;
    points.emplace_back(toPointF(control1), GlyphOutline::PointType::Control);
    points.emplace_back(toPointF(control2), GlyphOutline::PointType::Control);
    points.emplace_back(toPointF(to), GlyphOutline::PointType::OnCurve);

    ctx->current = toPointF(to);
    return 0;
}

void removeClosingDuplicate(GlyphOutline::Contour& contour)
{
    // FT decompose 会以 line-to-start 显式闭合；我们的模型隐式闭合，去掉重复末点
    if (contour.points.size() > 2) {
        const GlyphOutline::Point& first = contour.points.front();
        const GlyphOutline::Point& last = contour.points.back();
        if (last.type == GlyphOutline::PointType::OnCurve && last.pos == first.pos) {
            contour.points.pop_back();
        }
    }
}
}

Ret FontFaceReader::read(const io::path_t& path, FaceData& out)
{
    FT_Library library = nullptr;
    if (FT_Init_FreeType(&library) != 0) {
        return make_ret(Ret::Code::InternalError, std::string("failed to init FreeType"));
    }

    std::string pathStr = path.toStdString();

    FT_Face face = nullptr;
    if (FT_New_Face(library, pathStr.c_str(), 0, &face) != 0) {
        FT_Done_FreeType(library);
        return make_ret(Ret::Code::UnknownError, std::string("failed to open font: ") + pathStr);
    }

    out.upem = face->units_per_EM > 0 ? face->units_per_EM : 1000.0;
    out.ascender = face->ascender;
    out.descender = face->descender;

    if (const TT_OS2* os2 = static_cast<const TT_OS2*>(FT_Get_Sfnt_Table(face, FT_SFNT_OS2))) {
        // version 0xFFFF 表示无有效 OS/2 表
        if (os2->version != 0xFFFF) {
            out.fsType = os2->fsType;
        }
    }

    readLegalNameRecords(face, out.legalNameRecords);

    FT_UInt glyphIndex = 0;
    FT_ULong charcode = FT_Get_First_Char(face, &glyphIndex);

    while (glyphIndex != 0) {
        if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP) == 0
            && face->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
            FaceGlyph glyph;
            glyph.codepoint = static_cast<char32_t>(charcode);
            glyph.advance = static_cast<double>(face->glyph->metrics.horiAdvance);

            DecomposeContext ctx;
            ctx.outline = &glyph.outline;

            FT_Outline_Funcs funcs;
            funcs.move_to = ftMoveTo;
            funcs.line_to = ftLineTo;
            funcs.conic_to = ftConicTo;
            funcs.cubic_to = ftCubicTo;
            funcs.shift = 0;
            funcs.delta = 0;

            if (FT_Outline_Decompose(&face->glyph->outline, &funcs, &ctx) == 0) {
                for (GlyphOutline::Contour& contour : glyph.outline.contours()) {
                    removeClosingDuplicate(contour);
                }

                out.glyphs.push_back(std::move(glyph));
            }
        }

        charcode = FT_Get_Next_Char(face, charcode, &glyphIndex);
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);

    if (out.glyphs.empty()) {
        return make_ret(Ret::Code::UnknownError, std::string("no outline glyphs found in font: ") + pathStr);
    }

    LOGI() << "loaded font: " << pathStr << ", upem: " << out.upem << ", glyphs: " << out.glyphs.size();

    return make_ok();
}
