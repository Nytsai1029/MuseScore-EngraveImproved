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
#include <gtest/gtest.h>

#include "fontdesign/internal/project/undostack.h"

using namespace mu::fontdesign;

namespace {
class CountCommand : public UndoCommand
{
public:
    explicit CountCommand(int& counter)
        : m_counter(counter) {}

    void undo() override { m_counter -= 1; }
    void redo() override { m_counter += 1; }
    std::string name() const override { return "count"; }

private:
    int& m_counter;
};

int undoAll(UndoStack& stack)
{
    int undone = 0;
    while (stack.canUndo()) {
        stack.undo();
        ++undone;
    }
    return undone;
}
}

TEST(FontDesign_UndoStackTests, CapDropsOldestCommands)
{
    UndoStack stack;
    int counter = 0;
    const int extra = 10;
    for (size_t i = 0; i < UndoStack::MAX_COMMANDS + extra; ++i) {
        stack.push(std::make_unique<CountCommand>(counter));
    }

    EXPECT_EQ(undoAll(stack), static_cast<int>(UndoStack::MAX_COMMANDS));
    // 被丢掉的最旧命令不再可撤销，它们的效果保留
    EXPECT_EQ(counter, extra);
}

TEST(FontDesign_UndoStackTests, CleanStateIsReachableWithinCap)
{
    UndoStack stack;
    int counter = 0;
    for (int i = 0; i < 3; ++i) {
        stack.push(std::make_unique<CountCommand>(counter));
    }
    stack.markClean();
    EXPECT_TRUE(stack.isClean());

    stack.push(std::make_unique<CountCommand>(counter));
    EXPECT_FALSE(stack.isClean());

    stack.undo();
    EXPECT_TRUE(stack.isClean());
}

TEST(FontDesign_UndoStackTests, CleanIndexFollowsDroppedCommands)
{
    UndoStack stack;
    int counter = 0;
    for (int i = 0; i < 5; ++i) {
        stack.push(std::make_unique<CountCommand>(counter));
    }
    stack.markClean();

    for (size_t i = 0; i < UndoStack::MAX_COMMANDS; ++i) {
        stack.push(std::make_unique<CountCommand>(counter));
    }
    EXPECT_FALSE(stack.isClean());

    // 撤销到底正好回到保存时的状态
    undoAll(stack);
    EXPECT_EQ(counter, 5);
    EXPECT_TRUE(stack.isClean());
}

TEST(FontDesign_UndoStackTests, CleanStateIsLostOnceDropped)
{
    UndoStack stack;
    int counter = 0;
    EXPECT_TRUE(stack.isClean());

    for (size_t i = 0; i < UndoStack::MAX_COMMANDS + 1; ++i) {
        stack.push(std::make_unique<CountCommand>(counter));
    }

    // 初始（已保存）状态所在的位置已被丢弃：撤销到底也不算 clean
    undoAll(stack);
    EXPECT_FALSE(stack.isClean());
}
