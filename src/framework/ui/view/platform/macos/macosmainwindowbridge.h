/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MUSE_UI_MACOS_MAINWINDOWBRIDGE_H
#define MUSE_UI_MACOS_MAINWINDOWBRIDGE_H

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"

#include "ui/view/mainwindowbridge.h"

namespace muse::ui {
class MacOSMainWindowBridge : public MainWindowBridge, public async::Asyncable
{
    Q_OBJECT

    INJECT(IUiConfiguration, uiConfiguration)

public:
    explicit MacOSMainWindowBridge(QObject* parent = nullptr);
    ~MacOSMainWindowBridge() override;

    bool fileModified() const override;

    void handleTitleBarPress() override;

public slots:
    void setFileModified(bool modified) override;
    void setTitleBarMerged(bool merged) override;
    void setTitleBarHeight(int height) override;

private:
    void init() override;

    bool canMergeTitleBar() const;
    void updateTitleBarMerged();
    void setTitleBarObserved(bool observed);
    void layoutWindowButtons();
    void updateTitleBarInsets();

    struct TitleBarObserver;
    TitleBarObserver* m_titleBarObserver = nullptr;

    bool m_titleBarMergeRequested = false;
    bool m_isLayingOutWindowButtons = false;
    int m_alphaBufferSizeBeforeMerge = -1;
};
}

#endif // MUSE_UI_MACOS_MAINWINDOWBRIDGE_H
