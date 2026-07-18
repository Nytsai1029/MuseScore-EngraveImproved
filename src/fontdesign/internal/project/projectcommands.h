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

#include <optional>

#include "fontdesignproject.h"

namespace mu::fontdesign {
//! 快照式命令集：命令是数据的唯一修改入口（约定），
//! 修改后统一经 project->notifyChanged() 通知界面。

class SetAdvanceCommand : public UndoCommand
{
public:
    SetAdvanceCommand(FontDesignProject* project, char32_t code, double newValue);

    void undo() override;
    void redo() override;
    std::string name() const override { return "set advance"; }

private:
    FontDesignProject* m_project = nullptr;
    char32_t m_code = 0;
    double m_oldValue = 0.0;
    double m_newValue = 0.0;
};

//! 替换字形轮廓（粘贴/绘图/布尔运算等的统一入口，前后快照可撤销）。
//! 字形不存在时（空白字体/空码位）自动创建，撤销时随之移除。
class ReplaceOutlineCommand : public UndoCommand
{
public:
    ReplaceOutlineCommand(FontDesignProject* project, char32_t code, const GlyphOutline& newOutline);

    void undo() override;
    void redo() override;
    std::string name() const override { return "replace outline"; }

private:
    FontDesignProject* m_project = nullptr;
    char32_t m_code = 0;
    bool m_createdGlyph = false;
    GlyphOutline m_oldOutline;
    GlyphOutline m_newOutline;
};

//! 设置/新增锚点（oldValue 为空 = 原先不存在）
class SetAnchorCommand : public UndoCommand
{
public:
    SetAnchorCommand(FontDesignProject* project, char32_t code, AnchorId id, const muse::PointF& newValue);
    //! 拖拽结束时使用：显式给出拖拽前的旧值
    SetAnchorCommand(FontDesignProject* project, char32_t code, AnchorId id,
                     const std::optional<muse::PointF>& oldValue, const muse::PointF& newValue);

    void undo() override;
    void redo() override;
    std::string name() const override { return "set anchor"; }

private:
    FontDesignProject* m_project = nullptr;
    char32_t m_code = 0;
    AnchorId m_id = AnchorId::stemUpSE;
    std::optional<muse::PointF> m_oldValue;
    muse::PointF m_newValue;
};

class RemoveAnchorCommand : public UndoCommand
{
public:
    RemoveAnchorCommand(FontDesignProject* project, char32_t code, AnchorId id);

    void undo() override;
    void redo() override;
    std::string name() const override { return "remove anchor"; }

private:
    FontDesignProject* m_project = nullptr;
    char32_t m_code = 0;
    AnchorId m_id = AnchorId::stemUpSE;
    muse::PointF m_oldValue;
};

//! 设置/移除 engravingDefaults 数值键（newValue 为空 = 移除）
class SetEngravingDefaultCommand : public UndoCommand
{
public:
    SetEngravingDefaultCommand(FontDesignProject* project, const std::string& key, const std::optional<double>& newValue);

    void undo() override;
    void redo() override;
    std::string name() const override { return "set engraving default"; }

private:
    void apply(const std::optional<double>& value);

    FontDesignProject* m_project = nullptr;
    std::string m_key;
    std::optional<double> m_oldValue;
    std::optional<double> m_newValue;
};

//! 字体级标量信息（fontName/fontVersion/designSize/sizeRange/textFontFamily）快照
class SetFontInfoCommand : public UndoCommand
{
public:
    struct Info {
        std::string fontName;
        double fontVersion = 1.0;
        std::optional<int> designSize;
        std::optional<std::pair<int, int>> sizeRange;
        std::string textFontFamily;
    };

    static Info captureOf(const FontDesignProject& project);

    SetFontInfoCommand(FontDesignProject* project, const Info& newInfo);

    void undo() override;
    void redo() override;
    std::string name() const override { return "set font info"; }

private:
    void apply(const Info& info);

    FontDesignProject* m_project = nullptr;
    Info m_oldInfo;
    Info m_newInfo;
};

//! 四个表格段（alternates/ligatures/optionalGlyphs/sets）的快照命令
class SetMetadataTablesCommand : public UndoCommand
{
public:
    struct Tables {
        std::map<std::string, std::vector<AlternateInfo>> alternates;
        std::map<std::string, LigatureInfo> ligatures;
        std::map<std::string, OptionalGlyphInfo> optionalGlyphs;
        std::map<std::string, SetInfo> sets;
    };

    static Tables captureOf(const FontDesignProject& project);

    SetMetadataTablesCommand(FontDesignProject* project, const Tables& newTables, const std::string& name);

    void undo() override;
    void redo() override;
    std::string name() const override { return m_name; }

private:
    void apply(const Tables& tables);

    FontDesignProject* m_project = nullptr;
    Tables m_oldTables;
    Tables m_newTables;
    std::string m_name;
};
}
