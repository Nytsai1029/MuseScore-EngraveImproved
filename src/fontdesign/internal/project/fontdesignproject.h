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
#pragma once

#include <map>
#include <memory>

#include "async/notification.h"
#include "io/path.h"
#include "types/ret.h"

#include "fontmetadata.h"
#include "glyphitem.h"
#include "undostack.h"

namespace mu::fontdesign {
class SmuflDatabase;

//! 从零新建字体的基本信息（新建对话框收集）
struct NewFontParams {
    std::string fontName;
    double fontVersion = 1.0;
    double upem = 1000.0;
    std::string copyright;
    muse::io::path_t folder;    //! 保存位置：<folder>/<fontName>.otf + <fontName>_metadata.json
};

//! 一个打开的字体项目：字形表 + 元数据 + 当前编辑状态。
//! 本体 =（字体文件 + SMuFL 元数据 JSON）对。
class FontDesignProject
{
public:
    FontDesignProject() = default;

    muse::Ret load(const muse::io::path_t& fontPath, const muse::io::path_t& metadataPath, const SmuflDatabase& db);
    //! 空白项目：无字形，仅基本信息；保存路径由 params 决定，文件在首次保存时写出
    muse::Ret createNew(const NewFontParams& params, const SmuflDatabase& db);

    const muse::io::path_t& fontPath() const { return m_fontPath; }
    void setFontPath(const muse::io::path_t& path) { m_fontPath = path; }
    const muse::io::path_t& metadataPath() const { return m_metadataPath; }
    void setMetadataPath(const muse::io::path_t& path) { m_metadataPath = path; }

    std::string title() const;

    double upem() const { return m_upem; }
    //! 每 staff space 的字体单位数（1 sp = 0.25 em）
    double spatium() const { return m_upem / 4.0; }
    double ascender() const { return m_ascender; }
    double descender() const { return m_descender; }

    //! 源字体 OS/2 fsType 嵌入许可位（导出时保留）
    uint16_t sourceFsType() const { return m_sourceFsType; }
    //! 源字体保留的法律/署名 name 记录（nameID → UTF-8）
    const std::map<uint16_t, std::string>& sourceLegalNameRecords() const { return m_sourceLegalNameRecords; }

    const std::map<char32_t, GlyphItem>& glyphs() const { return m_glyphs; }
    const GlyphItem* glyph(char32_t code) const;
    //! 供命令/工具修改；改完须调用 notifyChanged()
    GlyphItem* glyphMut(char32_t code);
    //! 不存在则创建空字形（smuflName 由 SMuFL 数据库回填）——在空位上绘制时用
    GlyphItem& ensureGlyph(char32_t code);
    bool removeGlyph(char32_t code);

    FontMetadata& metadata() { return m_metadata; }
    const FontMetadata& metadata() const { return m_metadata; }

    char32_t currentGlyph() const { return m_currentGlyph; }
    void setCurrentGlyph(char32_t code);
    muse::async::Notification& currentGlyphChanged() { return m_currentGlyphChanged; }
    const muse::async::Notification& currentGlyphChanged() const { return m_currentGlyphChanged; }

    UndoStack& undoStack() { return m_undoStack; }
    const UndoStack& undoStack() const { return m_undoStack; }

    //! 新建项目在首次保存前始终视为脏（文件尚不存在于磁盘）
    bool isDirty() const { return m_neverSaved || !m_undoStack.isClean(); }
    void setNeverSaved(bool neverSaved) { m_neverSaved = neverSaved; }

    //! 任意数据修改后的粗粒度通知（advance/锚点/轮廓/元数据；检查器与画布刷新用）
    muse::async::Notification& changed() { return m_changed; }
    const muse::async::Notification& changed() const { return m_changed; }
    void notifyChanged() { m_changed.notify(); }

private:
    muse::io::path_t m_fontPath;
    muse::io::path_t m_metadataPath;

    double m_upem = 1000.0;
    double m_ascender = 0.0;
    double m_descender = 0.0;

    uint16_t m_sourceFsType = 0;
    std::map<uint16_t, std::string> m_sourceLegalNameRecords;

    const SmuflDatabase* m_smuflDb = nullptr;
    bool m_neverSaved = false;

    std::map<char32_t, GlyphItem> m_glyphs;
    FontMetadata m_metadata;

    char32_t m_currentGlyph = 0;
    muse::async::Notification m_currentGlyphChanged;

    UndoStack m_undoStack;
    muse::async::Notification m_changed;
};

using FontDesignProjectPtr = std::shared_ptr<FontDesignProject>;
}
