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
#include <QPointer>
#include <QQuickItem>

#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "context/iglobalcontext.h"

namespace mu::notation {
//! NOTE The pages of the current score for a preview: how many there are, which one a view shows,
//! and the facts of the page (paper, printable width, staff space)
class PreviewPagesModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    //! NOTE The paint view of the preview (a NotationPaintView)
    Q_PROPERTY(QQuickItem * view READ view WRITE setView NOTIFY viewChanged)

    Q_PROPERTY(int pageCount READ pageCount NOTIFY pagesChanged)
    Q_PROPERTY(int currentPageIndex READ currentPageIndex NOTIFY currentPageIndexChanged)

    //! NOTE The width of a page divided by its height
    Q_PROPERTY(qreal pageAspectRatio READ pageAspectRatio NOTIFY pagesChanged)

    //! NOTE E.g. "A4 · 210 × 297 mm", "182 mm", "1.499 mm"
    Q_PROPERTY(QString paperTitle READ paperTitle NOTIFY pagesChanged)
    Q_PROPERTY(QString printableWidthTitle READ printableWidthTitle NOTIFY pagesChanged)
    Q_PROPERTY(QString staffSpaceTitle READ staffSpaceTitle NOTIFY pagesChanged)

    muse::Inject<context::IGlobalContext> globalContext = { this };

public:
    explicit PreviewPagesModel(QObject* parent = nullptr);

    QQuickItem* view() const;
    void setView(QQuickItem* view);

    int pageCount() const;
    int currentPageIndex() const;
    qreal pageAspectRatio() const;

    QString paperTitle() const;
    QString printableWidthTitle() const;
    QString staffSpaceTitle() const;

    Q_INVOKABLE void load();

    //! NOTE Moves the view so that it shows the page
    Q_INVOKABLE void showPage(int pageIndex);

signals:
    void viewChanged();
    void pagesChanged();
    void currentPageIndexChanged();

private slots:
    void updateCurrentPageIndex();
    void onViewSizeChanged();

private:
    void onCurrentNotationChanged();

    QRectF viewport() const;
    QSizeF viewSize() const;
    bool isWholePageShown() const;

    QPointer<QQuickItem> m_view;
    QSizeF m_lastViewSize;
    int m_currentPageIndex = 0;
};
}
