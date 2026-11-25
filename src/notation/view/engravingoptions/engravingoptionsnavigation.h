/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include <QPointer>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "ui/inavigationcontroller.h"

class QQuickItem;

namespace mu::notation {
//! NOTE Follows MuseScore keyboard navigation inside the Engraving Options dialog: tells the dialog which
//! visual control became active in its navigation section (so the preview pane follows keyboard focus, not
//! only hover and Qt text focus) and scrolls enclosing Flickables to keep that control and its option row
//! in view
class EngravingOptionsNavigation : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QObject * section READ section WRITE setSection NOTIFY sectionChanged)
    Q_PROPERTY(QQuickItem * activeItem READ activeItem NOTIFY activeItemChanged)
    Q_PROPERTY(bool isHighlighted READ isHighlighted NOTIFY activeItemChanged)

    muse::Inject<muse::ui::INavigationController> navigationController = { this };

public:
    explicit EngravingOptionsNavigation(QObject* parent = nullptr);

    QObject* section() const;
    void setSection(QObject* section);

    QQuickItem* activeItem() const;
    bool isHighlighted() const;

    //! NOTE Scrolls every Flickable ancestor of controlItem so controlItem — and, when it fits, containerItem —
    //! is inside the viewport with breathing room. When revealContainerWhenVisible is false and controlItem is
    //! already fully inside the viewport, the scroll position is left alone (so mouse clicks do not jump)
    Q_INVOKABLE void ensureVisible(QQuickItem* controlItem, QQuickItem* containerItem = nullptr,
                                   bool revealContainerWhenVisible = false) const;

    //! NOTE Pure 1D ensure-visible scroll calculation, shared by X and Y axes and directly unit-tested
    static double scrollOffsetToReveal(double currentOffset, double minOffset, double maxOffset, double viewportSize,
                                       double controlStart, double controlEnd, double containerStart,
                                       double containerEnd, double margin, bool revealContainerWhenVisible);

signals:
    void sectionChanged();
    void activeItemChanged();
    void controlActivated(QQuickItem* item, bool highlighted);

private:
    void connectController();
    void scheduleSync();
    void syncActiveControl();

    QPointer<QObject> m_section;
    QPointer<QQuickItem> m_activeItem;
    const void* m_lastControlKey = nullptr;
    bool m_isHighlighted = false;
    bool m_controllerConnected = false;
    bool m_syncPending = false;
};
}
