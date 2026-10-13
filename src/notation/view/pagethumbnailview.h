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

#include <QImage>
#include <QQuickPaintedItem>
#include <QTimer>

#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "context/iglobalcontext.h"

namespace mu::notation {
//! NOTE A small picture of one page of the current score, as it will be printed
class PageThumbnailView : public QQuickPaintedItem, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(int pageIndex READ pageIndex WRITE setPageIndex NOTIFY pageIndexChanged)

    muse::Inject<context::IGlobalContext> globalContext = { this };

public:
    explicit PageThumbnailView(QQuickItem* parent = nullptr);

    int pageIndex() const;
    void setPageIndex(int index);

    void paint(QPainter* painter) override;

signals:
    void pageIndexChanged();

protected:
    void componentComplete() override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

private:
    void onCurrentNotationChanged();
    void invalidate();
    void render();

    int m_pageIndex = 0;

    QImage m_image;
    bool m_isDirty = true;

    //! NOTE The score may change many times in a row; the picture is made again when the changes stop
    QTimer m_invalidateTimer;
};
}
