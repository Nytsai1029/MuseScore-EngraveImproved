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
#include "undostack.h"

using namespace mu::fontdesign;

void UndoStack::push(std::unique_ptr<UndoCommand> cmd)
{
    if (m_index < m_commands.size()) {
        m_commands.erase(m_commands.begin() + m_index, m_commands.end());
        if (m_cleanIndex > static_cast<int>(m_index)) {
            m_cleanIndex = -1;
        }
    }

    cmd->redo();
    m_commands.push_back(std::move(cmd));

    //! 命令是快照式的（元数据表命令每条存两份完整表）：不设上限的话长时间编辑会一直涨内存。
    //! 超限时丢最旧的；clean 位置随之前移，被丢掉则再也回不到 clean 状态
    if (m_commands.size() > MAX_COMMANDS) {
        const size_t excess = m_commands.size() - MAX_COMMANDS;
        m_commands.erase(m_commands.begin(), m_commands.begin() + excess);
        if (m_cleanIndex >= 0) {
            m_cleanIndex = m_cleanIndex >= static_cast<int>(excess) ? m_cleanIndex - static_cast<int>(excess) : -1;
        }
    }

    m_index = m_commands.size();

    m_stackChanged.notify();
}

void UndoStack::undo()
{
    if (!canUndo()) {
        return;
    }

    m_index -= 1;
    m_commands[m_index]->undo();

    m_stackChanged.notify();
}

void UndoStack::redo()
{
    if (!canRedo()) {
        return;
    }

    m_commands[m_index]->redo();
    m_index += 1;

    m_stackChanged.notify();
}

bool UndoStack::canUndo() const
{
    return m_index > 0;
}

bool UndoStack::canRedo() const
{
    return m_index < m_commands.size();
}

bool UndoStack::isClean() const
{
    return m_cleanIndex == static_cast<int>(m_index);
}

void UndoStack::markClean()
{
    m_cleanIndex = static_cast<int>(m_index);
    m_stackChanged.notify();
}

void UndoStack::clear()
{
    m_commands.clear();
    m_index = 0;
    m_cleanIndex = 0;
    m_stackChanged.notify();
}

muse::async::Notification UndoStack::stackChanged() const
{
    return m_stackChanged;
}
