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
#include "previewpagesmodel.h"

#include <QLocale>
#include <QPageSize>

#include "engraving/dom/mscore.h"
#include "engraving/dom/page.h"

#include "translation.h"

using namespace mu::notation;
using namespace mu::engraving;

static QString millimeters(double valueMm, int decimals)
{
    //: A length in millimeters; %1 is the number
    return muse::qtrc("notation", "%1 mm").arg(QLocale().toString(valueMm, 'f', decimals));
}

static int pageSizeDecimals(double valueMm)
{
    //! NOTE The sizes are kept in inches, so a whole number of millimeters is not exactly whole
    static constexpr double TOLERANCE_MM = 0.15;
    return std::abs(valueMm - std::round(valueMm)) < TOLERANCE_MM ? 0 : 1;
}

static QString pageSizeTitle(double widthMm, double heightMm)
{
    //: The size of a page, width × height; %1 and %2 are numbers
    return muse::qtrc("notation", "%1 × %2 mm")
           .arg(QLocale().toString(widthMm, 'f', pageSizeDecimals(widthMm)),
                QLocale().toString(heightMm, 'f', pageSizeDecimals(heightMm)));
}

PreviewPagesModel::PreviewPagesModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void PreviewPagesModel::load()
{
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    });

    onCurrentNotationChanged();
}

void PreviewPagesModel::onCurrentNotationChanged()
{
    INotationPtr notation = globalContext()->currentNotation();
    if (notation) {
        //! NOTE The layout and the style (the size of the page) are parts of the notation
        notation->notationChanged().onNotify(this, [this]() {
            emit pagesChanged();
            updateCurrentPageIndex();
        });
    }

    emit pagesChanged();
    updateCurrentPageIndex();
}

QQuickItem* PreviewPagesModel::view() const
{
    return m_view;
}

void PreviewPagesModel::setView(QQuickItem* view)
{
    if (m_view == view) {
        return;
    }

    if (m_view) {
        disconnect(m_view, nullptr, this, nullptr);
    }

    m_view = view;

    if (m_view) {
        //! NOTE A signal of AbstractNotationPaintView
        connect(m_view, SIGNAL(viewportChanged()), this, SLOT(updateCurrentPageIndex()));

        //! NOTE The view reacts to its new size first (its connections were made before these)
        connect(m_view, &QQuickItem::widthChanged, this, &PreviewPagesModel::onViewSizeChanged);
        connect(m_view, &QQuickItem::heightChanged, this, &PreviewPagesModel::onViewSizeChanged);
    }

    m_lastViewSize = viewSize();

    emit viewChanged();
    updateCurrentPageIndex();
}

QSizeF PreviewPagesModel::viewSize() const
{
    return m_view ? QSizeF(m_view->width(), m_view->height()) : QSizeF();
}

bool PreviewPagesModel::isWholePageShown() const
{
    //! NOTE NotationPaintView::ZoomToWholePage
    static constexpr int ZOOM_TO_WHOLE_PAGE = 2;
    return m_view && m_view->property("zoomKind").toInt() == ZOOM_TO_WHOLE_PAGE;
}

void PreviewPagesModel::onViewSizeChanged()
{
    m_lastViewSize = viewSize();

    //! NOTE The view keeps its top left corner when its size changes;
    //! a page which is shown whole stays in the middle of the view
    if (isWholePageShown()) {
        showPage(m_currentPageIndex);
    } else {
        updateCurrentPageIndex();
    }
}

int PreviewPagesModel::pageCount() const
{
    INotationPtr notation = globalContext()->currentNotation();
    return notation ? static_cast<int>(notation->elements()->pages().size()) : 0;
}

int PreviewPagesModel::currentPageIndex() const
{
    return m_currentPageIndex;
}

qreal PreviewPagesModel::pageAspectRatio() const
{
    INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return 210.0 / 297.0;
    }

    const double width = notation->style()->styleValue(StyleId::pageWidth).toDouble();
    const double height = notation->style()->styleValue(StyleId::pageHeight).toDouble();

    return width > 0 && height > 0 ? width / height : 210.0 / 297.0;
}

QString PreviewPagesModel::paperTitle() const
{
    INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return QString();
    }

    //! NOTE The style has the size of the page in inches
    const double widthMm = notation->style()->styleValue(StyleId::pageWidth).toDouble() * INCH;
    const double heightMm = notation->style()->styleValue(StyleId::pageHeight).toDouble() * INCH;

    const QPageSize::PageSizeId id = QPageSize::id(QSizeF(widthMm, heightMm), QPageSize::Millimeter,
                                                   QPageSize::FuzzyOrientationMatch);
    if (id == QPageSize::Custom) {
        return pageSizeTitle(widthMm, heightMm);
    }

    //! NOTE A paper with a name has the size of that paper (in the orientation of the page)
    QSizeF paperSize = QPageSize::size(id, QPageSize::Millimeter);
    if ((widthMm > heightMm) != (paperSize.width() > paperSize.height())) {
        paperSize.transpose();
    }

    return QPageSize::name(id) + QStringLiteral(" · ") + pageSizeTitle(paperSize.width(), paperSize.height());
}

QString PreviewPagesModel::printableWidthTitle() const
{
    INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return QString();
    }

    const double widthMm = notation->style()->styleValue(StyleId::pagePrintableWidth).toDouble() * INCH;
    return millimeters(widthMm, pageSizeDecimals(widthMm));
}

QString PreviewPagesModel::staffSpaceTitle() const
{
    INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return QString();
    }

    //! NOTE The style has the staff space in the units of the layout
    const double spatiumMm = notation->style()->styleValue(StyleId::spatium).toDouble() / DPMM;
    return millimeters(spatiumMm, 3);
}

QRectF PreviewPagesModel::viewport() const
{
    return m_view ? m_view->property("viewport").toRectF() : QRectF();
}

void PreviewPagesModel::updateCurrentPageIndex()
{
    //! NOTE The size of the view is changing: the page is the one which was shown before (see onViewSizeChanged)
    if (viewSize() != m_lastViewSize && isWholePageShown()) {
        return;
    }

    int index = 0;

    INotationPtr notation = globalContext()->currentNotation();
    const QRectF viewport = this->viewport();

    if (notation && viewport.isValid()) {
        const QPointF center = viewport.center();
        const PageList& pages = notation->elements()->pages();

        //! NOTE The page in the middle of the view; between the pages, the nearest one
        double minDistance = -1.0;
        for (size_t i = 0; i < pages.size(); ++i) {
            const QRectF pageRect = pages.at(i)->canvasBoundingRect().toQRectF();
            if (pageRect.contains(center)) {
                index = static_cast<int>(i);
                break;
            }

            const double dx = std::max({ pageRect.left() - center.x(), 0.0, center.x() - pageRect.right() });
            const double dy = std::max({ pageRect.top() - center.y(), 0.0, center.y() - pageRect.bottom() });
            const double distance = dx * dx + dy * dy;

            if (minDistance < 0 || distance < minDistance) {
                minDistance = distance;
                index = static_cast<int>(i);
            }
        }
    }

    if (m_currentPageIndex != index) {
        m_currentPageIndex = index;
        emit currentPageIndexChanged();
    }
}

void PreviewPagesModel::showPage(int pageIndex)
{
    INotationPtr notation = globalContext()->currentNotation();
    const QRectF viewport = this->viewport();
    if (!notation || !m_view || !viewport.isValid()) {
        return;
    }

    const PageList& pages = notation->elements()->pages();
    if (pageIndex < 0 || pageIndex >= static_cast<int>(pages.size())) {
        return;
    }

    const QRectF pageRect = pages.at(pageIndex)->canvasBoundingRect().toQRectF();

    //! NOTE A page which fits in the view is put in its middle; otherwise the top of the page is shown
    const double dx = viewport.center().x() - pageRect.center().x();
    const double dy = pageRect.height() <= viewport.height() ? viewport.center().y() - pageRect.center().y()
                      : viewport.top() - pageRect.top();

    QMetaObject::invokeMethod(m_view, "moveCanvas", Q_ARG(qreal, dx), Q_ARG(qreal, dy));
}
