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

#include <QObject>

#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "../../ifontdesignservice.h"
#include "../../internal/project/projectcommands.h"

namespace mu::fontdesign {
//! 检查器模型基类：统一处理 项目/当前字形/数据变更 的订阅与重载
class AbstractFontDesignModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

protected:
    muse::Inject<IFontDesignService> fontDesignService = { this };

public:
    explicit AbstractFontDesignModel(QObject* parent = nullptr);

    Q_INVOKABLE void init();

protected:
    FontDesignProjectPtr project() const;
    const GlyphItem* currentGlyphItem() const;

    void pushCommand(std::unique_ptr<UndoCommand> cmd);

    //! 数据 → 界面（项目切换/当前字形切换/任何数据变更后调用）
    virtual void reload() = 0;

private:
    void attachToProject();

    const FontDesignProject* m_attachedProject = nullptr;
};
}
