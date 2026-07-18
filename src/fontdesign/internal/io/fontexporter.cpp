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
#include "fontexporter.h"

#include <cmath>
#include <cstdio>

#include <QFile>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "cffwriter.h"
#include "sfntwriter.h"

#include "../project/fontdesignproject.h"

#include "log.h"

using namespace mu::fontdesign;
using namespace muse;

namespace {
std::string sanitizePsName(const std::string& name)
{
    std::string out;
    out.reserve(name.size());
    for (char c : name) {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-') {
            out.push_back(c);
        } else if (c == ' ') {
            out.push_back('-');
        }
    }
    if (out.empty()) {
        out = "Font";
    }
    return out;
}
}

Ret FontExporter::buildFontBytes(const FontDesignProject& project, std::vector<uint8_t>& out, Report* report)
{
    Report local;
    Report& rep = report ? *report : local;
    rep = Report();

    CffWriter::Input cffIn = CffWriter::fromProject(project);
    if (cffIn.glyphs.empty()) {
        return make_ret(Ret::Code::UnknownError, std::string("no glyphs to export"));
    }

    // 保证 glyph 0 = .notdef（空白新建字体可能一个字形都没有）
    if (cffIn.glyphs.empty() || cffIn.glyphs[0].name != ".notdef") {
        CffWriter::GlyphInput notdef;
        notdef.name = ".notdef";
        notdef.codepoint = 0;
        notdef.advance = cffIn.upem / 2;
        cffIn.glyphs.insert(cffIn.glyphs.begin(), std::move(notdef));
    }

    std::vector<uint8_t> cff;
    Ret cffRet = CffWriter::write(cffIn, cff);
    if (!cffRet) {
        return cffRet;
    }

    SfntWriter::Input sfntIn;
    sfntIn.metrics.upem = cffIn.upem;
    sfntIn.metrics.ascender = static_cast<int>(std::lround(project.ascender() != 0 ? project.ascender() : cffIn.upem * 0.8));
    sfntIn.metrics.descender = static_cast<int>(std::lround(project.descender() != 0 ? project.descender() : -cffIn.upem * 0.2));
    sfntIn.metrics.xMin = cffIn.fontBBoxXMin;
    sfntIn.metrics.yMin = cffIn.fontBBoxYMin;
    sfntIn.metrics.xMax = cffIn.fontBBoxXMax;
    sfntIn.metrics.yMax = cffIn.fontBBoxYMax;
    // 保留源字体的嵌入许可位（OFL/商业字体的授权信息）
    sfntIn.metrics.fsType = project.sourceFsType();

    const std::string family = cffIn.fontName.empty() ? "Untitled" : cffIn.fontName;
    sfntIn.names.family = family;
    sfntIn.names.subfamily = "Regular";
    sfntIn.names.fullName = family;
    sfntIn.names.postScriptName = sanitizePsName(family);
    char verBuf[64];
    std::snprintf(verBuf, sizeof(verBuf), "Version %.3f", cffIn.fontVersion);
    sfntIn.names.version = verBuf;
    // 保留源字体的版权/许可证/署名记录（OFL 要求派生字体保留）
    sfntIn.names.legalRecords = project.sourceLegalNameRecords();

    sfntIn.glyphMetrics.reserve(cffIn.glyphs.size());
    for (size_t i = 0; i < cffIn.glyphs.size(); ++i) {
        SfntWriter::GlyphMetrics gm;
        gm.advance = cffIn.glyphs[i].advance;
        if (!cffIn.glyphs[i].outline.isEmpty()) {
            const RectF bb = cffIn.glyphs[i].outline.boundingRect();
            gm.lsb = static_cast<int>(std::floor(bb.x()));
        }
        sfntIn.glyphMetrics.push_back(gm);

        // cmap: skip mapping .notdef at 0 unless it is a real character (usually not)
        if (cffIn.glyphs[i].codepoint != 0 || i == 0) {
            if (cffIn.glyphs[i].codepoint != 0) {
                sfntIn.cmap[cffIn.glyphs[i].codepoint] = static_cast<uint16_t>(i);
            }
        }
    }

    // Also map any project glyphs that share the ordered list
    for (size_t i = 0; i < cffIn.glyphs.size(); ++i) {
        if (cffIn.glyphs[i].codepoint != 0) {
            sfntIn.cmap[cffIn.glyphs[i].codepoint] = static_cast<uint16_t>(i);
        }
    }

    sfntIn.cffTable = std::move(cff);

    Ret sfntRet = SfntWriter::write(sfntIn, out);
    if (!sfntRet) {
        return sfntRet;
    }

    rep.numGlyphs = static_cast<int>(cffIn.glyphs.size());
    rep.cmapEntries = static_cast<int>(sfntIn.cmap.size());

    // FreeType 回读校验
    FT_Library library = nullptr;
    if (FT_Init_FreeType(&library) != 0) {
        rep.warnings.push_back("FreeType init failed; skipped validation");
        rep.ok = true;
        rep.message = "exported (validation skipped)";
        return make_ok();
    }

    FT_Face face = nullptr;
    FT_Error err = FT_New_Memory_Face(library, out.data(), static_cast<FT_Long>(out.size()), 0, &face);
    if (err != 0 || !face) {
        FT_Done_FreeType(library);
        rep.ok = false;
        rep.message = "FreeType cannot open exported font";
        return make_ret(Ret::Code::UnknownError, rep.message);
    }

    if (static_cast<int>(face->num_glyphs) != rep.numGlyphs) {
        char buf[128];
        std::snprintf(buf, sizeof(buf), "glyph count mismatch: exported %d, FT %ld",
                      rep.numGlyphs, static_cast<long>(face->num_glyphs));
        rep.warnings.push_back(buf);
    }

    if (face->units_per_EM != cffIn.upem) {
        char buf[128];
        std::snprintf(buf, sizeof(buf), "upem mismatch: %d vs FT %d", cffIn.upem, face->units_per_EM);
        rep.warnings.push_back(buf);
    }

    // 抽样校验若干有轮廓字形的 advance
    int checked = 0;
    for (size_t i = 0; i < cffIn.glyphs.size() && checked < 32; ++i) {
        if (cffIn.glyphs[i].outline.isEmpty() && cffIn.glyphs[i].codepoint == 0) {
            continue;
        }
        if (FT_Load_Glyph(face, static_cast<FT_UInt>(i), FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING) != 0) {
            continue;
        }
        const int ftAdv = static_cast<int>(face->glyph->metrics.horiAdvance);
        if (std::abs(ftAdv - cffIn.glyphs[i].advance) > 1) {
            char buf[160];
            std::snprintf(buf, sizeof(buf), "advance mismatch glyph %zu (%s): %d vs FT %d",
                          i, cffIn.glyphs[i].name.c_str(), cffIn.glyphs[i].advance, ftAdv);
            rep.warnings.push_back(buf);
        }
        ++checked;
    }

    // 抽样 cmap
    int cmapOk = 0;
    for (const auto& pair : sfntIn.cmap) {
        FT_UInt idx = FT_Get_Char_Index(face, pair.first);
        if (idx != pair.second) {
            char buf[128];
            std::snprintf(buf, sizeof(buf), "cmap U+%04X: expected glyph %u, FT %u",
                          static_cast<unsigned>(pair.first), pair.second, idx);
            rep.warnings.push_back(buf);
        } else {
            ++cmapOk;
        }
        if (cmapOk + static_cast<int>(rep.warnings.size()) > 64) {
            break;
        }
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);

    rep.ok = true;
    if (rep.warnings.empty()) {
        rep.message = "exported and validated";
    } else {
        rep.message = "exported with validation warnings";
    }

    for (const std::string& w : rep.warnings) {
        LOGW() << "FontExporter: " << w;
    }

    return make_ok();
}

Ret FontExporter::exportFont(const FontDesignProject& project, const io::path_t& path, Report* report)
{
    std::vector<uint8_t> bytes;
    Ret ret = buildFontBytes(project, bytes, report);
    if (!ret) {
        return ret;
    }

    QFile file(path.toQString());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return make_ret(Ret::Code::UnknownError, std::string("cannot write font: ") + path.toStdString());
    }

    if (file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<qint64>(bytes.size()))
        != static_cast<qint64>(bytes.size())) {
        return make_ret(Ret::Code::UnknownError, std::string("short write exporting font"));
    }

    file.close();
    LOGI() << "font exported: " << path << " (" << bytes.size() << " bytes)";
    return make_ok();
}
