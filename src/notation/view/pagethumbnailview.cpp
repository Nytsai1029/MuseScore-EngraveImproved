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
#include "pagethumbnailview.h"

#include <QPainter>
#include <QQuickWindow>

#include "draw/painter.h"

#include "engraving/dom/mscore.h"

using namespace mu::notation;

//! NOTE The picture is not made smaller than this, so it stays sharp when the item is small
static constexpr int MIN_RENDER_DPI = 24;
static constexpr int INVALIDATE_INTERVAL_MS = 400;

PageThumbnailView::PageThumbnailView(QQuickItem* parent)
    : QQuickPaintedItem(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
    m_invalidateTimer.setSingleShot(true);
    m_invalidateTimer.setInterval(INVALIDATE_INTERVAL_MS);
    connect(&m_invalidateTimer, &QTimer::timeout, this, &PageThumbnailView::invalidate);
}

void PageThumbnailView::componentComplete()
{
    QQuickPaintedItem::componentComplete();

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    });

    onCurrentNotationChanged();
}

void PageThumbnailView::onCurrentNotationChanged()
{
    INotationPtr notation = globalContext()->currentNotation();
    if (notation) {
        notation->notationChanged().onNotify(this, [this]() {
            m_invalidateTimer.start();
        });
    }

    invalidate();
}

int PageThumbnailView::pageIndex() const
{
    return m_pageIndex;
}

void PageThumbnailView::setPageIndex(int index)
{
    if (m_pageIndex == index) {
        return;
    }

    m_pageIndex = index;
    emit pageIndexChanged();

    invalidate();
}

void PageThumbnailView::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);

    if (newGeometry.size() != oldGeometry.size()) {
        invalidate();
    }
}

void PageThumbnailView::invalidate()
{
    m_isDirty = true;
    update();
}

void PageThumbnailView::render()
{
    m_image = QImage();
    m_isDirty = false;

    INotationPtr notation = globalContext()->currentNotation();
    if (!notation || width() <= 0 || height() <= 0) {
        return;
    }

    INotationPaintingPtr painting = notation->painting();
    if (m_pageIndex < 0 || m_pageIndex >= painting->pageCount()) {
        return;
    }

    INotationPainting::Options options;
    options.fromPage = m_pageIndex;
    options.toPage = m_pageIndex;
    options.printPageBackground = false; // the picture is filled below

    const muse::SizeF pageSizeInch = painting->pageSizeInch(options);
    if (pageSizeInch.width() <= 0 || pageSizeInch.height() <= 0) {
        return;
    }

    const qreal pixelRatio = window() ? window()->effectiveDevicePixelRatio() : 1.0;
    const int dpi = std::max(MIN_RENDER_DPI, static_cast<int>(std::ceil(width() * pixelRatio / pageSizeInch.width())));
    options.deviceDpi = dpi;

    QImage image(std::lrint(pageSizeInch.width() * dpi), std::lrint(pageSizeInch.height() * dpi),
                 QImage::Format_ARGB32_Premultiplied);

    //! NOTE: the painting is scaled by the resolution of the picture
    image.setDotsPerMeterX(std::lrint((dpi * 1000) / mu::engraving::INCH));
    image.setDotsPerMeterY(std::lrint((dpi * 1000) / mu::engraving::INCH));
    image.fill(Qt::white);

    {
        muse::draw::Painter painter(&image, "pagethumbnail");
        painting->paintPng(&painter, options);
    }

    m_image = image;
}

void PageThumbnailView::paint(QPainter* painter)
{
    if (m_isDirty) {
        render();
    }

    if (m_image.isNull()) {
        return;
    }

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->drawImage(QRectF(0, 0, width(), height()), m_image);
}
