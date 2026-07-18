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
#include "fontdesignproject.h"

#include "io/fileinfo.h"

#include "../io/fontfacereader.h"
#include "../io/metadatareader.h"
#include "../smufldatabase.h"

#include "log.h"

using namespace mu::fontdesign;
using namespace muse;

static const char32_t DEFAULT_CURRENT_GLYPH = 0xE0A4; // noteheadBlack

Ret FontDesignProject::load(const io::path_t& fontPath, const io::path_t& metadataPath, const SmuflDatabase& db)
{
    FontFaceReader::FaceData faceData;
    Ret ret = FontFaceReader::read(fontPath, faceData);
    if (!ret) {
        return ret;
    }

    m_smuflDb = &db;
    m_fontPath = fontPath;
    m_upem = faceData.upem;
    m_ascender = faceData.ascender;
    m_descender = faceData.descender;
    m_sourceFsType = faceData.fsType;
    m_sourceLegalNameRecords = std::move(faceData.legalNameRecords);

    m_glyphs.clear();
    for (FontFaceReader::FaceGlyph& faceGlyph : faceData.glyphs) {
        GlyphItem item;
        item.codepoint = faceGlyph.codepoint;
        item.outline = std::move(faceGlyph.outline);
        item.advance = faceGlyph.advance;

        if (const SmuflDatabase::GlyphInfo* info = db.infoByCodepoint(faceGlyph.codepoint)) {
            item.smuflName = info->name;
        }

        m_glyphs.emplace(item.codepoint, std::move(item));
    }

    if (!metadataPath.empty()) {
        FontMetadata metadata;
        std::map<char32_t, std::map<AnchorId, PointF>> anchorsByCode;
        Ret metaRet = MetadataReader::read(metadataPath, db, metadata, anchorsByCode);
        if (metaRet) {
            m_metadataPath = metadataPath;
            m_metadata = std::move(metadata);

            // 可选字形（元数据声明的名字）也回填到字形项
            for (const auto& pair : m_metadata.optionalGlyphs) {
                auto it = m_glyphs.find(pair.second.codepoint);
                if (it != m_glyphs.end() && it->second.smuflName.empty()) {
                    it->second.smuflName = pair.first;
                }
            }

            for (const auto& pair : anchorsByCode) {
                auto it = m_glyphs.find(pair.first);
                if (it != m_glyphs.end()) {
                    it->second.anchors = pair.second;
                }
            }
        } else {
            LOGW() << "metadata not loaded: " << metaRet.text();
        }
    }

    if (m_metadata.fontName.empty()) {
        m_metadata.fontName = io::FileInfo(fontPath).baseName().toStdString();
    }

    m_currentGlyph = m_glyphs.count(DEFAULT_CURRENT_GLYPH) ? DEFAULT_CURRENT_GLYPH : m_glyphs.begin()->first;

    return make_ok();
}

Ret FontDesignProject::createNew(const NewFontParams& params, const SmuflDatabase& db)
{
    if (params.fontName.empty() || params.folder.empty()) {
        return make_ret(Ret::Code::UnknownError, std::string("font name and folder are required"));
    }
    if (params.upem < 16.0) {
        return make_ret(Ret::Code::UnknownError, std::string("invalid units per em"));
    }

    m_smuflDb = &db;
    m_fontPath = params.folder + "/" + params.fontName + ".otf";
    m_metadataPath = params.folder + "/" + params.fontName + ".json";

    m_upem = params.upem;
    //! 新字体的垂直度量默认值（文本布局用；对 SMuFL 谱面渲染无影响）
    m_ascender = params.upem;
    m_descender = -params.upem / 4.0;

    m_sourceFsType = 0;    // 可安装、无嵌入限制
    m_sourceLegalNameRecords.clear();
    if (!params.copyright.empty()) {
        m_sourceLegalNameRecords[0] = params.copyright;    // nameID 0 = copyright
    }

    m_glyphs.clear();
    m_metadata = FontMetadata();
    m_metadata.fontName = params.fontName;
    m_metadata.fontVersion = params.fontVersion;

    m_currentGlyph = 0;
    m_neverSaved = true;    // 文件尚未写盘：关闭前提示保存

    return make_ok();
}

std::string FontDesignProject::title() const
{
    return m_metadata.fontName;
}

const GlyphItem* FontDesignProject::glyph(char32_t code) const
{
    auto it = m_glyphs.find(code);
    return it != m_glyphs.end() ? &it->second : nullptr;
}

GlyphItem* FontDesignProject::glyphMut(char32_t code)
{
    auto it = m_glyphs.find(code);
    return it != m_glyphs.end() ? &it->second : nullptr;
}

GlyphItem& FontDesignProject::ensureGlyph(char32_t code)
{
    auto it = m_glyphs.find(code);
    if (it == m_glyphs.end()) {
        GlyphItem item;
        item.codepoint = code;
        if (m_smuflDb) {
            if (const SmuflDatabase::GlyphInfo* info = m_smuflDb->infoByCodepoint(code)) {
                item.smuflName = info->name;
            }
        }
        it = m_glyphs.emplace(code, std::move(item)).first;
    }
    return it->second;
}

bool FontDesignProject::removeGlyph(char32_t code)
{
    return m_glyphs.erase(code) > 0;
}

void FontDesignProject::setCurrentGlyph(char32_t code)
{
    if (m_currentGlyph == code) {
        return;
    }

    m_currentGlyph = code;
    m_currentGlyphChanged.notify();
}
