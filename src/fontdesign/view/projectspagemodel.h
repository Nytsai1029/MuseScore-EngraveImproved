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
#include "iinteractive.h"

#include "../ifontdesignconfiguration.h"
#include "multiinstances/imultiinstancesprovider.h"

#include "../ifontdesignservice.h"

namespace mu::fontdesign {
class ProjectsPageModel : public QObject, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<IFontDesignService> fontDesignService = { this };
    muse::Inject<IFontDesignConfiguration> configuration = { this };
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<muse::mi::IMultiInstancesProvider> multiInstancesProvider = { this };

public:
    explicit ProjectsPageModel(QObject* parent = nullptr);

    Q_INVOKABLE void openFontDesignPage();
    Q_INVOKABLE void openFont();
    Q_INVOKABLE void openFontFile(const QString& path);
    Q_INVOKABLE void newFont();

    //! 最近打开的字体：[{name, path, exists}]，不存在的文件自动从列表清除
    Q_INVOKABLE QVariantList recentFonts() const;

private:
    //! 已有项目时询问 替换/新窗口/取消；新窗口 = 带 --fontdesign 参数的新应用实例
    void openFontPath(const muse::io::path_t& path);
    bool confirmDiscardOrSave();
};
}
