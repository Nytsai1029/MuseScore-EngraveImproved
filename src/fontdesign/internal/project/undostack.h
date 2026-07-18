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

#include <memory>
#include <string>
#include <vector>

#include "async/notification.h"

namespace mu::fontdesign {
class UndoCommand
{
public:
    virtual ~UndoCommand() = default;

    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual std::string name() const = 0;
};

//! 项目级撤销栈：快照式命令，clean 位置用于脏标记
class UndoStack
{
public:
    //! 立即执行（redo）并入栈；截断已撤销的分支
    void push(std::unique_ptr<UndoCommand> cmd);

    void undo();
    void redo();

    bool canUndo() const;
    bool canRedo() const;

    bool isClean() const;
    void markClean();

    void clear();

    muse::async::Notification stackChanged() const;

private:
    std::vector<std::unique_ptr<UndoCommand>> m_commands;
    size_t m_index = 0;      // 已应用的命令数
    int m_cleanIndex = 0;    // -1 = clean 状态已被截断，无法通过 undo/redo 回到
    muse::async::Notification m_stackChanged;
};
}
