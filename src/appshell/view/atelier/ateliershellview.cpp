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
#include "ateliershellview.h"

#include <QGuiApplication>
#include <QJsonDocument>
#include <QQuickWindow>
#include <QScreen>
#include <QTimer>

#include "ui/view/navigationcontrol.h"

#include "log.h"

using namespace mu::appshell;
using namespace muse;
using namespace muse::dock;

static const QString PANELS_STATE_KEY("atelier/panels");
static const QString PANE_WIDTHS_KEY("atelier/paneWidths");
static const QString WINDOW_GEOMETRY_KEY("atelier/window");

static const QString MAXIMIZED_STATE("maximized");
static const QString FULL_SCREEN_STATE("fullscreen");
static const QString NORMAL_STATE("normal");

AtelierShellView::AtelierShellView(QQuickItem* parent)
    : QQuickItem(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

AtelierShellView::~AtelierShellView()
{
    if (dockWindowProvider()->window() == this) {
        dockWindowProvider()->deinit();
    }
}

QString AtelierShellView::currentPageUri() const
{
    return m_currentPageUri;
}

QVariantMap AtelierShellView::currentPageParams() const
{
    return m_currentPageParams;
}

int AtelierShellView::panelsRevision() const
{
    return m_panelsRevision;
}

int AtelierShellView::paneWidthsRevision() const
{
    return m_paneWidthsRevision;
}

void AtelierShellView::init()
{
    readPanelStates();
    readPaneWidths();
    restoreWindowGeometry();

    dockWindowProvider()->init(this);

    if (QQuickWindow* w = window()) {
        connect(w, &QWindow::xChanged, this, &AtelierShellView::rememberNormalGeometry);
        connect(w, &QWindow::yChanged, this, &AtelierShellView::rememberNormalGeometry);
        connect(w, &QWindow::widthChanged, this, &AtelierShellView::rememberNormalGeometry);
        connect(w, &QWindow::heightChanged, this, &AtelierShellView::rememberNormalGeometry);
    }

    connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
        saveWindowGeometry();
    });
}

void AtelierShellView::loadPage(const QString& uri, const QVariantMap& params)
{
    const bool uriChanged = m_currentPageUri != uri;

    m_currentPageUri = uri;
    m_currentPageParams = params;

    if (uriChanged) {
        emit currentPageUriChanged();
    }

    emit currentPageParamsChanged();

    if (!m_isFirstPageLoaded) {
        m_isFirstPageLoaded = true;

        //! NOTE: the window has been shown transparent by this moment (see GuiApp), the first page makes it visible
        QTimer::singleShot(0, this, [this]() {
            QQuickWindow* w = window();
            if (w && (!m_hasGeometryBeenRestored || (w->windowStates() & Qt::WindowFullScreen))) {
                w->showMaximized();
            }

            emit pageLoaded();
        });
    } else {
        emit pageLoaded();
    }

    //! NOTE: each page has its own panels, so the states of the toggle actions have to be re-read
    if (uriChanged) {
        QStringList names;
        for (auto it = m_panels.constBegin(); it != m_panels.constEnd(); ++it) {
            const QString name = it.key().section('|', 1);
            if (!names.contains(name)) {
                names << name;
            }
        }

        m_docksOpenStatusChanged.send(names);
    }
}

QString AtelierShellView::panelKey(const QString& pageUri, const QString& name)
{
    return pageUri + '|' + name;
}

void AtelierShellView::declarePanel(const QString& pageUri, const QString& name, bool openByDefault)
{
    const QString key = panelKey(pageUri, name);

    Panel panel;
    panel.openByDefault = openByDefault;
    panel.open = m_savedPanelStates.contains(key) ? m_savedPanelStates.value(key).toBool() : openByDefault;

    m_panels[key] = panel;

    notifyAboutPanelsChanged({ name });
}

bool AtelierShellView::isPanelOpen(const QString& pageUri, const QString& name, int) const
{
    auto it = m_panels.constFind(panelKey(pageUri, name));
    return it != m_panels.constEnd() && it->open;
}

void AtelierShellView::setPanelOpen(const QString& pageUri, const QString& name, bool open)
{
    auto it = m_panels.find(panelKey(pageUri, name));
    if (it == m_panels.end() || it->open == open) {
        return;
    }

    it->open = open;

    writePanelStates();
    notifyAboutPanelsChanged({ name });
}

void AtelierShellView::togglePanel(const QString& pageUri, const QString& name)
{
    setPanelOpen(pageUri, name, !isPanelOpen(pageUri, name));
}

int AtelierShellView::paneWidth(const QString& pageUri, const QString& name, int) const
{
    return m_paneWidths.value(panelKey(pageUri, name)).toInt();
}

void AtelierShellView::setPaneWidth(const QString& pageUri, const QString& name, int width)
{
    if (paneWidth(pageUri, name) == width) {
        return;
    }

    const QString key = panelKey(pageUri, name);
    if (width > 0) {
        m_paneWidths[key] = width;
    } else {
        m_paneWidths.remove(key);
    }

    writePaneWidths();

    ++m_paneWidthsRevision;
    emit paneWidthsChanged();
}

void AtelierShellView::setDefaultNavigationControl(muse::ui::NavigationControl* control)
{
    navigationController()->setDefaultNavigationControl(dynamic_cast<muse::ui::INavigationControl*>(control));
}

void AtelierShellView::notifyAboutPanelsChanged(const QStringList& names)
{
    ++m_panelsRevision;
    emit panelsChanged();

    m_docksOpenStatusChanged.send(names);
}

bool AtelierShellView::isDockOpen(const QString& dockName) const
{
    return isPanelOpen(m_currentPageUri, dockName);
}

void AtelierShellView::setDockOpen(const QString& dockName, bool open)
{
    setPanelOpen(m_currentPageUri, dockName, open);
}

void AtelierShellView::toggleDock(const QString& dockName)
{
    togglePanel(m_currentPageUri, dockName);
}

async::Channel<QStringList> AtelierShellView::docksOpenStatusChanged() const
{
    return m_docksOpenStatusChanged;
}

bool AtelierShellView::isDockFloating(const QString&) const
{
    //! NOTE: the panels of this skin stay in their zones
    return false;
}

void AtelierShellView::toggleDockFloating(const QString&)
{
}

DockPageView* AtelierShellView::currentPage() const
{
    return nullptr;
}

QQuickItem& AtelierShellView::asItem() const
{
    return const_cast<AtelierShellView&>(*this);
}

void AtelierShellView::restoreDefaultLayout()
{
    QStringList names;

    for (auto it = m_panels.begin(); it != m_panels.end(); ++it) {
        it->open = it->openByDefault;
        names << it.key().section('|', 1);
    }

    m_savedPanelStates = QJsonObject();
    uiConfiguration()->setUiItemState(PANELS_STATE_KEY, QString());

    names.removeDuplicates();
    notifyAboutPanelsChanged(names);

    m_paneWidths = QJsonObject();
    uiConfiguration()->setUiItemState(PANE_WIDTHS_KEY, QString());

    ++m_paneWidthsRevision;
    emit paneWidthsChanged();
}

void AtelierShellView::readPanelStates()
{
    const QByteArray json = uiConfiguration()->uiItemState(PANELS_STATE_KEY).toUtf8();
    m_savedPanelStates = QJsonDocument::fromJson(json).object();
}

void AtelierShellView::writePanelStates()
{
    for (auto it = m_panels.constBegin(); it != m_panels.constEnd(); ++it) {
        m_savedPanelStates[it.key()] = it->open;
    }

    const QByteArray json = QJsonDocument(m_savedPanelStates).toJson(QJsonDocument::Compact);
    uiConfiguration()->setUiItemState(PANELS_STATE_KEY, QString::fromUtf8(json));
}

void AtelierShellView::readPaneWidths()
{
    const QByteArray json = uiConfiguration()->uiItemState(PANE_WIDTHS_KEY).toUtf8();
    m_paneWidths = QJsonDocument::fromJson(json).object();
}

void AtelierShellView::writePaneWidths()
{
    const QByteArray json = QJsonDocument(m_paneWidths).toJson(QJsonDocument::Compact);
    uiConfiguration()->setUiItemState(PANE_WIDTHS_KEY, QString::fromUtf8(json));
}

void AtelierShellView::restoreWindowGeometry()
{
    QQuickWindow* w = window();
    if (!w) {
        return;
    }

    //! NOTE: "x,y,width,height,state"
    const QStringList parts = uiConfiguration()->uiItemState(WINDOW_GEOMETRY_KEY).split(',');
    if (parts.size() != 5) {
        return;
    }

    QRect geometry(parts.at(0).toInt(), parts.at(1).toInt(), parts.at(2).toInt(), parts.at(3).toInt());
    if (!geometry.isValid()) {
        return;
    }

    //! NOTE: the screen on which the window was closed may be gone, or it may have another size now
    const QScreen* screen = QGuiApplication::screenAt(geometry.center());
    if (!screen) {
        return;
    }

    const QRect available = screen->availableGeometry();
    geometry.setSize(geometry.size().boundedTo(available.size()));
    geometry.moveLeft(qBound(available.left(), geometry.left(), available.right() - geometry.width() + 1));
    geometry.moveTop(qBound(available.top(), geometry.top(), available.bottom() - geometry.height() + 1));

    m_normalGeometry = geometry;
    w->setGeometry(geometry);

    const QString state = parts.at(4);
    if (state == MAXIMIZED_STATE) {
        w->setWindowStates(Qt::WindowMaximized);
    } else if (state == FULL_SCREEN_STATE) {
        w->setWindowStates(Qt::WindowFullScreen);
    }

    m_hasGeometryBeenRestored = true;
}

void AtelierShellView::rememberNormalGeometry()
{
    const QQuickWindow* w = window();
    if (w && w->isVisible() && w->windowStates() == Qt::WindowNoState) {
        m_normalGeometry = w->geometry();
    }
}

void AtelierShellView::saveWindowGeometry()
{
    const QQuickWindow* w = window();
    if (!w) {
        return;
    }

    const QRect geometry = m_normalGeometry.isValid() ? m_normalGeometry : w->geometry();

    QString state = NORMAL_STATE;
    if (w->windowStates() & Qt::WindowFullScreen) {
        state = FULL_SCREEN_STATE;
    } else if (w->windowStates() & Qt::WindowMaximized) {
        state = MAXIMIZED_STATE;
    }

    uiConfiguration()->setUiItemState(WINDOW_GEOMETRY_KEY, QString("%1,%2,%3,%4,%5")
                                      .arg(geometry.x()).arg(geometry.y()).arg(geometry.width()).arg(geometry.height())
                                      .arg(state));
}
