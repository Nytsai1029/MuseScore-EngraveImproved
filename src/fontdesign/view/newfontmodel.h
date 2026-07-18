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

#include "modularity/ioc.h"

#include "../ifontdesignconfiguration.h"
#include "../ifontdesignservice.h"

namespace mu::fontdesign {
//! 新建字体对话框的后端：校验基本信息表单并创建空白项目
class NewFontModel : public QObject, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<IFontDesignService> fontDesignService = { this };
    muse::Inject<IFontDesignConfiguration> configuration = { this };

public:
    explicit NewFontModel(QObject* parent = nullptr);

    Q_INVOKABLE QString defaultFolder() const;
    //! 返回空串 = 创建成功；否则为错误信息
    Q_INVOKABLE QString create(const QString& name, const QString& version, int upem,
                               const QString& copyright, const QString& folder);
};
}
