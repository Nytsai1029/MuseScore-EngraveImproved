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
#include "iinteractive.h"

#include "dockwindow/idockwindowprovider.h"
#include "multiinstances/imultiinstancesprovider.h"

#include "notation/iengravingfontsscanner.h"
#include "notation/inotationconfiguration.h"

#include "../ifontdesignconfiguration.h"
#include "../ifontdesignservice.h"

namespace mu::fontdesign {
class FontDesignPageModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool hasProject READ hasProject NOTIFY hasProjectChanged)
    Q_PROPERTY(QString projectTitle READ projectTitle NOTIFY hasProjectChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY stackStateChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY stackStateChanged)
    Q_PROPERTY(bool isDirty READ isDirty NOTIFY stackStateChanged)
    Q_PROPERTY(bool glyphsPanelOpen READ glyphsPanelOpen NOTIFY panelsOpenChanged)
    Q_PROPERTY(bool inspectorPanelOpen READ inspectorPanelOpen NOTIFY panelsOpenChanged)

    muse::Inject<IFontDesignService> fontDesignService = { this };
    muse::Inject<IFontDesignConfiguration> configuration = { this };
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<muse::dock::IDockWindowProvider> dockWindowProvider = { this };
    muse::Inject<muse::mi::IMultiInstancesProvider> multiInstancesProvider = { this };
    muse::Inject<mu::notation::INotationConfiguration> notationConfiguration = { this };
    muse::Inject<mu::notation::IEngravingFontsScanner> fontsScanner = { this };

public:
    explicit FontDesignPageModel(QObject* parent = nullptr);

    Q_INVOKABLE void init();
    Q_INVOKABLE void goToProjectsSection();
    Q_INVOKABLE void openFont();
    Q_INVOKABLE void newFont();
    Q_INVOKABLE void openAddGlyphDialog();
    Q_INVOKABLE void openLintDialog();

    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void save();
    Q_INVOKABLE void exportFontAs();
    Q_INVOKABLE void installToMuseScore();

    //! 面板被关闭（含浮动后关闭）后从工具栏重新打开
    Q_INVOKABLE void toggleGlyphsPanel();
    Q_INVOKABLE void toggleInspectorPanel();

    bool hasProject() const;
    QString projectTitle() const;
    bool canUndo() const;
    bool canRedo() const;
    bool isDirty() const;
    bool glyphsPanelOpen() const;
    bool inspectorPanelOpen() const;

signals:
    void hasProjectChanged();
    void stackStateChanged();
    void panelsOpenChanged();

private:
    void attachToProject();
    void listenDocksOpenStatus();
    bool isDockOpen(const QString& dockName) const;
    void toggleDock(const QString& dockName);
    //! 若当前项目脏：Save/DontSave/Cancel；返回 false 表示用户取消
    bool confirmDiscardOrSave();

    const FontDesignProject* m_attachedProject = nullptr;
};
}
