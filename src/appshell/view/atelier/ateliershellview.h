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

#include <QJsonObject>
#include <QQuickItem>
#include <QRect>

#include "async/asyncable.h"
#include "async/channel.h"
#include "modularity/ioc.h"

#include "dockwindow/idockwindow.h"
#include "dockwindow/idockwindowprovider.h"
#include "ui/inavigationcontroller.h"
#include "ui/iuiconfiguration.h"

namespace muse::ui {
class NavigationControl;
}

namespace mu::appshell {
//! NOTE The window content of the Atelier skin. Unlike the dock window of the other skins, it has fixed zones
//! (toolbar, tool strip, left pane, score, right pane, bottom drawer, status bar), which are laid out in QML.
//! This item keeps what the rest of the app expects from a dock window: the current page,
//! which panels are open on each page, and the geometry of the window
class AtelierShellView : public QQuickItem, public muse::dock::IDockWindow, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString currentPageUri READ currentPageUri NOTIFY currentPageUriChanged)
    Q_PROPERTY(QVariantMap currentPageParams READ currentPageParams NOTIFY currentPageParamsChanged)

    //! NOTE Changes whenever a panel is opened or closed; pass it to isPanelOpen() to make a binding follow the state
    Q_PROPERTY(int panelsRevision READ panelsRevision NOTIFY panelsChanged)

    muse::Inject<muse::ui::IUiConfiguration> uiConfiguration = { this };
    muse::Inject<muse::dock::IDockWindowProvider> dockWindowProvider = { this };
    muse::Inject<muse::ui::INavigationController> navigationController = { this };

public:
    explicit AtelierShellView(QQuickItem* parent = nullptr);
    ~AtelierShellView() override;

    QString currentPageUri() const;
    QVariantMap currentPageParams() const;
    int panelsRevision() const;

    Q_INVOKABLE void init();
    Q_INVOKABLE void loadPage(const QString& uri, const QVariantMap& params);

    Q_INVOKABLE void declarePanel(const QString& pageUri, const QString& name, bool openByDefault);
    Q_INVOKABLE bool isPanelOpen(const QString& pageUri, const QString& name, int revision = 0) const;
    Q_INVOKABLE void setPanelOpen(const QString& pageUri, const QString& name, bool open);
    Q_INVOKABLE void togglePanel(const QString& pageUri, const QString& name);

    //! NOTE The control which gets the keyboard navigation when no other one has it (the score)
    Q_INVOKABLE void setDefaultNavigationControl(muse::ui::NavigationControl* control);

    //! IDockWindow
    bool isDockOpen(const QString& dockName) const override;
    void setDockOpen(const QString& dockName, bool open) override;
    void toggleDock(const QString& dockName) override;

    muse::async::Channel<QStringList> docksOpenStatusChanged() const override;

    bool isDockFloating(const QString& dockName) const override;
    void toggleDockFloating(const QString& dockName) override;

    muse::dock::DockPageView* currentPage() const override;
    QQuickItem& asItem() const override;

    void restoreDefaultLayout() override;

signals:
    void pageLoaded();
    void currentPageUriChanged();
    void currentPageParamsChanged();
    void panelsChanged();

private:
    struct Panel {
        bool openByDefault = false;
        bool open = false;
    };

    static QString panelKey(const QString& pageUri, const QString& name);

    void readPanelStates();
    void writePanelStates();

    void restoreWindowGeometry();
    void saveWindowGeometry();
    void rememberNormalGeometry();

    void notifyAboutPanelsChanged(const QStringList& names);

    QString m_currentPageUri;
    QVariantMap m_currentPageParams;

    QMap<QString, Panel> m_panels;
    QJsonObject m_savedPanelStates;
    int m_panelsRevision = 0;

    muse::async::Channel<QStringList> m_docksOpenStatusChanged;

    QRect m_normalGeometry;
    bool m_hasGeometryBeenRestored = false;
    bool m_isFirstPageLoaded = false;
};
}
