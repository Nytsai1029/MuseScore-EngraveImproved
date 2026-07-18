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
#include "projectcommands.h"

using namespace mu::fontdesign;
using namespace muse;

// SetAdvanceCommand

SetAdvanceCommand::SetAdvanceCommand(FontDesignProject* project, char32_t code, double newValue)
    : m_project(project), m_code(code), m_newValue(newValue)
{
    if (const GlyphItem* glyph = project->glyph(code)) {
        m_oldValue = glyph->advance;
    }
}

void SetAdvanceCommand::undo()
{
    if (GlyphItem* glyph = m_project->glyphMut(m_code)) {
        glyph->advance = m_oldValue;
        m_project->notifyChanged();
    }
}

void SetAdvanceCommand::redo()
{
    if (GlyphItem* glyph = m_project->glyphMut(m_code)) {
        glyph->advance = m_newValue;
        m_project->notifyChanged();
    }
}

// ReplaceOutlineCommand

ReplaceOutlineCommand::ReplaceOutlineCommand(FontDesignProject* project, char32_t code, const GlyphOutline& newOutline)
    : m_project(project), m_code(code), m_newOutline(newOutline)
{
    if (const GlyphItem* glyph = project->glyph(code)) {
        m_oldOutline = glyph->outline;
    }
}

void ReplaceOutlineCommand::undo()
{
    if (m_createdGlyph) {
        // 本命令创建的字形：撤销即整体移除
        m_project->removeGlyph(m_code);
        m_project->notifyChanged();
        return;
    }

    if (GlyphItem* glyph = m_project->glyphMut(m_code)) {
        glyph->outline = m_oldOutline;
        m_project->notifyChanged();
    }
}

void ReplaceOutlineCommand::redo()
{
    GlyphItem* glyph = m_project->glyphMut(m_code);
    if (!glyph) {
        m_createdGlyph = true;
        glyph = &m_project->ensureGlyph(m_code);
    }
    glyph->outline = m_newOutline;
    m_project->notifyChanged();
}

// SetAnchorCommand

SetAnchorCommand::SetAnchorCommand(FontDesignProject* project, char32_t code, AnchorId id, const PointF& newValue)
    : m_project(project), m_code(code), m_id(id), m_newValue(newValue)
{
    if (const GlyphItem* glyph = project->glyph(code)) {
        auto it = glyph->anchors.find(id);
        if (it != glyph->anchors.end()) {
            m_oldValue = it->second;
        }
    }
}

SetAnchorCommand::SetAnchorCommand(FontDesignProject* project, char32_t code, AnchorId id,
                                   const std::optional<PointF>& oldValue, const PointF& newValue)
    : m_project(project), m_code(code), m_id(id), m_oldValue(oldValue), m_newValue(newValue)
{
}

void SetAnchorCommand::undo()
{
    if (GlyphItem* glyph = m_project->glyphMut(m_code)) {
        if (m_oldValue.has_value()) {
            glyph->anchors[m_id] = m_oldValue.value();
        } else {
            glyph->anchors.erase(m_id);
        }
        m_project->notifyChanged();
    }
}

void SetAnchorCommand::redo()
{
    if (GlyphItem* glyph = m_project->glyphMut(m_code)) {
        glyph->anchors[m_id] = m_newValue;
        m_project->notifyChanged();
    }
}

// RemoveAnchorCommand

RemoveAnchorCommand::RemoveAnchorCommand(FontDesignProject* project, char32_t code, AnchorId id)
    : m_project(project), m_code(code), m_id(id)
{
    if (const GlyphItem* glyph = project->glyph(code)) {
        auto it = glyph->anchors.find(id);
        if (it != glyph->anchors.end()) {
            m_oldValue = it->second;
        }
    }
}

void RemoveAnchorCommand::undo()
{
    if (GlyphItem* glyph = m_project->glyphMut(m_code)) {
        glyph->anchors[m_id] = m_oldValue;
        m_project->notifyChanged();
    }
}

void RemoveAnchorCommand::redo()
{
    if (GlyphItem* glyph = m_project->glyphMut(m_code)) {
        glyph->anchors.erase(m_id);
        m_project->notifyChanged();
    }
}

// SetEngravingDefaultCommand

SetEngravingDefaultCommand::SetEngravingDefaultCommand(FontDesignProject* project, const std::string& key,
                                                       const std::optional<double>& newValue)
    : m_project(project), m_key(key), m_newValue(newValue)
{
    const auto& defaults = project->metadata().engravingDefaults;
    auto it = defaults.find(key);
    if (it != defaults.end()) {
        m_oldValue = it->second;
    }
}

void SetEngravingDefaultCommand::apply(const std::optional<double>& value)
{
    if (value.has_value()) {
        m_project->metadata().engravingDefaults[m_key] = value.value();
    } else {
        m_project->metadata().engravingDefaults.erase(m_key);
    }
    m_project->notifyChanged();
}

void SetEngravingDefaultCommand::undo()
{
    apply(m_oldValue);
}

void SetEngravingDefaultCommand::redo()
{
    apply(m_newValue);
}

// SetFontInfoCommand

SetFontInfoCommand::Info SetFontInfoCommand::captureOf(const FontDesignProject& project)
{
    const FontMetadata& metadata = project.metadata();

    Info info;
    info.fontName = metadata.fontName;
    info.fontVersion = metadata.fontVersion;
    info.designSize = metadata.designSize;
    info.sizeRange = metadata.sizeRange;
    info.textFontFamily = metadata.textFontFamily;

    return info;
}

SetFontInfoCommand::SetFontInfoCommand(FontDesignProject* project, const Info& newInfo)
    : m_project(project), m_oldInfo(captureOf(*project)), m_newInfo(newInfo)
{
}

void SetFontInfoCommand::apply(const Info& info)
{
    FontMetadata& metadata = m_project->metadata();
    metadata.fontName = info.fontName;
    metadata.fontVersion = info.fontVersion;
    metadata.designSize = info.designSize;
    metadata.sizeRange = info.sizeRange;
    metadata.textFontFamily = info.textFontFamily;

    m_project->notifyChanged();
}

void SetFontInfoCommand::undo()
{
    apply(m_oldInfo);
}

void SetFontInfoCommand::redo()
{
    apply(m_newInfo);
}

// SetMetadataTablesCommand

SetMetadataTablesCommand::Tables SetMetadataTablesCommand::captureOf(const FontDesignProject& project)
{
    const FontMetadata& metadata = project.metadata();

    Tables tables;
    tables.alternates = metadata.alternates;
    tables.ligatures = metadata.ligatures;
    tables.optionalGlyphs = metadata.optionalGlyphs;
    tables.sets = metadata.sets;

    return tables;
}

SetMetadataTablesCommand::SetMetadataTablesCommand(FontDesignProject* project, const Tables& newTables, const std::string& name)
    : m_project(project), m_oldTables(captureOf(*project)), m_newTables(newTables), m_name(name)
{
}

void SetMetadataTablesCommand::apply(const Tables& tables)
{
    FontMetadata& metadata = m_project->metadata();
    metadata.alternates = tables.alternates;
    metadata.ligatures = tables.ligatures;
    metadata.optionalGlyphs = tables.optionalGlyphs;
    metadata.sets = tables.sets;

    m_project->notifyChanged();
}

void SetMetadataTablesCommand::undo()
{
    apply(m_oldTables);
}

void SetMetadataTablesCommand::redo()
{
    apply(m_newTables);
}
