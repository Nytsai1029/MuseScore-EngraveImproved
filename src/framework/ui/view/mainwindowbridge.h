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

#ifndef MUSE_DOCK_MAINWINDOWBRIDGE_H
#define MUSE_DOCK_MAINWINDOWBRIDGE_H

#include <QObject>
#include <QWindow>

#include "async/notification.h"

#include "modularity/ioc.h"
#include "ui/imainwindow.h"
#include "ui/iwindowscontroller.h"

namespace muse::ui {
class MainWindowBridge : public QObject, public Injectable
{
    Q_OBJECT

    Q_PROPERTY(QWindow * window READ qWindow WRITE setWindow NOTIFY windowChanged)
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(bool fileModified READ fileModified WRITE setFileModified NOTIFY fileModifiedChanged)

    //! NOTE The title bar is drawn over the top of the content, which leaves room for the buttons of the window.
    //! It is merged only where the platform can do it: read the value to know whether it is.
    //! titleBarHeight is the height of the bar of the content which takes the place of the title bar:
    //! the buttons of the window are put in the middle of it
    Q_PROPERTY(bool titleBarMerged READ titleBarMerged WRITE setTitleBarMerged NOTIFY titleBarMergedChanged)
    Q_PROPERTY(int titleBarHeight READ titleBarHeight WRITE setTitleBarHeight NOTIFY titleBarHeightChanged)
    Q_PROPERTY(int titleBarLeftInset READ titleBarLeftInset NOTIFY titleBarInsetsChanged)
    Q_PROPERTY(int titleBarRightInset READ titleBarRightInset NOTIFY titleBarInsetsChanged)

    Inject<IMainWindow> mainWindow = { this };
    Inject<IWindowsController> windowsController = { this };

public:
    explicit MainWindowBridge(QObject* parent = nullptr);
    ~MainWindowBridge();

    QWindow* qWindow() const;

    QString filePath() const;
    virtual bool fileModified() const;

    bool titleBarMerged() const;
    int titleBarHeight() const;
    int titleBarLeftInset() const;
    int titleBarRightInset() const;

    void showOnBack();
    void showOnFront();

    bool isFullScreen() const;
    async::Notification isFullScreenChanged() const;
    void toggleFullScreen();

    QScreen* screen() const;

    Q_INVOKABLE void showMinimizedWithSavePreviousState();

    //! NOTE For a press of the mouse on an empty place of the content which is under the merged title bar:
    //! does what the title bar does (moves the window, a double click zooms it)
    Q_INVOKABLE virtual void handleTitleBarPress();

signals:
    void windowChanged();
    void filePathChanged();
    void fileModifiedChanged();
    void titleBarMergedChanged();
    void titleBarHeightChanged();
    void titleBarInsetsChanged();

protected:
    virtual void init();

    void setIsTitleBarMerged(bool merged);
    void setTitleBarInsets(int leftInset, int rightInset);

    QWindow* m_window = nullptr;

    QWindow::Visibility m_windowVisibility = QWindow::AutomaticVisibility;

protected slots: // Should only be used from QML
    virtual void setTitleBarMerged(bool merged);
    virtual void setTitleBarHeight(int height);

private slots: // Should only be used from QML
    void setWindow(QWindow* window);
    void setFilePath(const QString& filePath);
    virtual void setFileModified(bool modified);

private:
    void updateFullScreen();

    bool m_isFullScreen = false;
    async::Notification m_isFullScreenChanged;

    bool m_isTitleBarMerged = false;
    int m_titleBarHeight = 0;
    int m_titleBarLeftInset = 0;
    int m_titleBarRightInset = 0;
};
}

#endif // MUSE_DOCK_MAINWINDOWBRIDGE_H
