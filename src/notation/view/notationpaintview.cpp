/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "notationpaintview.h"

#include <algorithm>

#include "engraving/dom/measure.h"
#include "engraving/dom/page.h"
#include "engraving/dom/score.h"
#include "engraving/dom/system.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::notation;

//! NOTE The distance from the point to the rectangle (0 if the rectangle has the point), squared
static double distanceSquared(const muse::RectF& rect, const muse::PointF& point)
{
    const double dx = std::max({ rect.left() - point.x(), 0.0, point.x() - rect.right() });
    const double dy = std::max({ rect.top() - point.y(), 0.0, point.y() - rect.bottom() });
    return dx * dx + dy * dy;
}

template<typename Item>
static const Item* nearestItem(const std::vector<Item*>& items, const muse::PointF& point)
{
    const Item* nearest = nullptr;
    double minDistance = 0.0;

    for (const Item* item : items) {
        const double distance = distanceSquared(item->canvasBoundingRect(), point);
        if (!nearest || distance < minDistance) {
            nearest = item;
            minDistance = distance;
        }
    }

    return nearest;
}

NotationPaintView::NotationPaintView(QQuickItem* parent)
    : AbstractNotationPaintView(parent)
{
}

bool NotationPaintView::syncViewState() const
{
    return m_syncViewState;
}

void NotationPaintView::setSyncViewState(bool sync)
{
    if (m_syncViewState == sync) {
        return;
    }

    if (!sync && m_viewStateMatrixConnectionActive && notation()) {
        notation()->viewState()->matrixChanged().resetOnReceive(this);
        m_viewStateMatrixConnectionActive = false;
    }

    m_syncViewState = sync;
    emit syncViewStateChanged();
}

bool NotationPaintView::startWithWholePage() const
{
    return m_startWithWholePage;
}

void NotationPaintView::setStartWithWholePage(bool wholePage)
{
    if (m_startWithWholePage == wholePage) {
        return;
    }

    m_startWithWholePage = wholePage;
    emit startWithWholePageChanged();
}

int NotationPaintView::zoomPercentage() const
{
    return notation() ? inputController()->currentZoomPercentage() : 100;
}

NotationPaintView::ZoomKind NotationPaintView::zoomKind() const
{
    switch (zoomType()) {
    case ZoomType::Percentage: return ZoomByPercentage;
    case ZoomType::PageWidth: return ZoomToPageWidth;
    case ZoomType::WholePage: return ZoomToWholePage;
    case ZoomType::TwoPages: return ZoomToTwoPages;
    }

    return ZoomByPercentage;
}

void NotationPaintView::zoomToPageWidth()
{
    if (notation()) {
        inputController()->zoomToPageWidth();
    }
}

void NotationPaintView::zoomToWholePage()
{
    if (notation()) {
        inputController()->zoomToWholePage();
    }
}

void NotationPaintView::zoomToTwoPages()
{
    if (notation()) {
        inputController()->zoomToTwoPages();
    }
}

void NotationPaintView::onLoadNotation(INotationPtr notation)
{
    m_isLocalMatrixInited = false;

    if (m_syncViewState) {
        m_isLocalMatrixUpdate = true;
        setMatrix(notation->viewState()->matrix());
        m_isLocalMatrixUpdate = false;

        notation->viewState()->matrixChanged().onReceive(this, [this](const Transform& matrix, NotationPaintView* sender) {
            if (sender != this) {
                m_isLocalMatrixUpdate = true;
                setMatrix(matrix);
                m_isLocalMatrixUpdate = false;
            }
        });
        m_viewStateMatrixConnectionActive = true;
    } else {
        m_isLocalMatrixUpdate = true;
        setMatrix(Transform());
        m_isLocalMatrixUpdate = false;
    }

    AbstractNotationPaintView::onLoadNotation(notation);
}

void NotationPaintView::onUnloadNotation(INotationPtr notation)
{
    AbstractNotationPaintView::onUnloadNotation(notation);

    if (m_viewStateMatrixConnectionActive) {
        notation->viewState()->matrixChanged().resetOnReceive(this);
        m_viewStateMatrixConnectionActive = false;
    }
}

void NotationPaintView::initZoomAndPosition()
{
    if (!notation()) {
        return;
    }

    if (m_syncViewState && AbstractNotationPaintView::isMatrixInited()) {
        return;
    }

    const bool wholePage = !m_syncViewState && m_startWithWholePage;
    inputController()->initZoom(wholePage ? std::optional<ZoomType>(ZoomType::WholePage) : std::nullopt);
    inputController()->initCanvasPos();

    if (wholePage) {
        showPageAt(notation()->viewState()->centerTick());
    }
}

void NotationPaintView::updateCenterTick()
{
    //! NOTE Only the view which edits the score, while the score is laid out for it
    //! (the preview lays the score out in pages)
    if (!m_syncViewState || publishMode() || !isVisible() || !notation()) {
        return;
    }

    if (notation()->painting()->viewMode() != notation()->viewState()->viewMode()) {
        return;
    }

    const int tick = findCenterTick();
    if (tick >= 0) {
        notation()->viewState()->setCenterTick(tick);
    }
}

int NotationPaintView::findCenterTick() const
{
    const muse::RectF viewport = this->viewport();
    if (!viewport.isValid()) {
        return -1;
    }

    const muse::PointF center = viewport.center();

    //! NOTE The page nearest to the middle of the view, its line nearest to the middle, and the measure of that line
    const engraving::Page* page = nearestItem(notation()->elements()->pages(), center);
    const engraving::System* system = page ? nearestItem(page->systems(), center) : nullptr;
    const engraving::MeasureBase* measure = system ? nearestItem(system->measures(), center) : nullptr;

    return measure ? measure->tick().ticks() : -1;
}

void NotationPaintView::showPageAt(int tick)
{
    if (tick < 0) {
        return;
    }

    const engraving::Score* score = notation()->elements()->msScore();
    const engraving::Measure* measure = score ? score->tick2measureMM(engraving::Fraction::fromTicks(tick)) : nullptr;
    const engraving::System* system = measure ? measure->system() : nullptr;
    const engraving::Page* page = system ? system->page() : nullptr;

    const muse::RectF viewport = this->viewport();
    if (!page || !viewport.isValid()) {
        return;
    }

    //! NOTE As the pages of the preview show a page (see PreviewPagesModel::showPage)
    const muse::RectF pageRect = page->canvasBoundingRect();
    const double dx = viewport.center().x() - pageRect.center().x();
    const double dy = pageRect.height() <= viewport.height() ? viewport.center().y() - pageRect.center().y()
                      : viewport.top() - pageRect.top();

    moveCanvas(dx, dy);
}

void NotationPaintView::onViewSizeChanged()
{
    AbstractNotationPaintView::onViewSizeChanged();

    updateCenterTick();
}

void NotationPaintView::onMatrixChanged(const Transform& oldMatrix, const Transform& newMatrix, bool overrideZoomType)
{
    AbstractNotationPaintView::onMatrixChanged(oldMatrix, newMatrix, overrideZoomType);

    //! NOTE The place where the score is edited, for the views which start there
    updateCenterTick();

    if (!m_syncViewState) {
        if (overrideZoomType && m_localZoomType != ZoomType::Percentage) {
            m_localZoomType = ZoomType::Percentage;
            emit zoomKindChanged();
        }
        return;
    }

    if (!m_isLocalMatrixUpdate && notation()) {
        notation()->viewState()->setMatrix(newMatrix, this);

        if (overrideZoomType) {
            notation()->viewState()->setZoomType(ZoomType::Percentage);
            emit zoomKindChanged();
        }
    }
}

bool NotationPaintView::isMatrixInited() const
{
    if (m_syncViewState) {
        return AbstractNotationPaintView::isMatrixInited();
    }

    return m_isLocalMatrixInited;
}

void NotationPaintView::setMatrixInited(bool inited)
{
    if (m_syncViewState) {
        AbstractNotationPaintView::setMatrixInited(inited);
        return;
    }

    m_isLocalMatrixInited = inited;
}

ZoomType NotationPaintView::zoomType() const
{
    if (m_syncViewState) {
        return AbstractNotationPaintView::zoomType();
    }

    return m_localZoomType;
}

void NotationPaintView::setZoomType(ZoomType type)
{
    if (m_syncViewState) {
        AbstractNotationPaintView::setZoomType(type);
        emit zoomKindChanged();
        return;
    }

    if (m_localZoomType != type) {
        m_localZoomType = type;
        emit zoomKindChanged();
    }
}
