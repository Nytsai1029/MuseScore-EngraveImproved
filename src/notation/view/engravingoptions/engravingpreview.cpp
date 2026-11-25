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
#include "engravingpreview.h"

#include <algorithm>
#include <cmath>

#include <QHoverEvent>
#include <QKeyEvent>
#include <QList>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QQuickWindow>
#include <QTimer>

#include "draw/painter.h"
#include "engraving/dom/score.h"
#include "engraving/style/style.h"

#include "notation/inotation.h"
#include "notation/inotationelements.h"
#include "notation/inotationstyle.h"

#include "../styledialog/styleuivalue.h"
#include "previewoverlay.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;

//! NOTE The staff space of a picture at the zoom 1.0, in pixels: a staff is as tall as two lines of text
static constexpr double BASE_STAFF_SPACE = 7.0;

//! NOTE The hairline around each page of a picture of the pages
static const QColor PAGE_EDGE_COLOR(0, 0, 0, 56);

//! NOTE How far the washes reach beyond the boxes of the items they are behind, in staff spaces
static constexpr double WASH_GROWTH = 0.15;

namespace mu::notation {
//! NOTE A dialog page has many pictures, and a change of the style makes all of them out of date at once.
//! They are made again one by one, each in a turn of the event loop of its own, so that the interface
//! stays alive while they are
class EngravingPreviewQueue : public QObject
{
public:
    static EngravingPreviewQueue* instance()
    {
        //! NOTE A child of the application: its timer is gone when the application is, not after it
        static EngravingPreviewQueue* queue = new EngravingPreviewQueue(qApp);
        return queue;
    }

    void add(EngravingPreview* preview)
    {
        if (!m_previews.contains(preview)) {
            m_previews.append(preview);
        }

        if (!m_timer.isActive()) {
            m_timer.start();
        }
    }

private:
    explicit EngravingPreviewQueue(QObject* parent)
        : QObject(parent)
    {
        m_timer.setInterval(0);
        m_timer.setSingleShot(true);

        connect(&m_timer, &QTimer::timeout, this, [this]() {
            while (!m_previews.isEmpty()) {
                const QPointer<EngravingPreview> preview = m_previews.takeFirst();
                if (preview) {
                    preview->render();
                    break;
                }
            }

            if (!m_previews.isEmpty()) {
                m_timer.start();
            }
        });
    }

    QList<QPointer<EngravingPreview> > m_previews;
    QTimer m_timer;
};
}

EngravingPreview::EngravingPreview(QQuickItem* parent)
    : QQuickPaintedItem(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
}

EngravingPreview::~EngravingPreview() = default;

QString EngravingPreview::music() const
{
    return m_music;
}

void EngravingPreview::setMusic(const QString& music)
{
    if (m_music == music) {
        return;
    }

    m_music = music;
    m_isMusicLoaded = false;
    emit musicChanged();

    invalidateLayout();
}

QVariantMap EngravingPreview::overrides() const
{
    return m_overrides;
}

void EngravingPreview::setOverrides(const QVariantMap& overrides)
{
    if (m_overrides == overrides) {
        return;
    }

    m_overrides = overrides;
    emit overridesChanged();

    invalidateLayout();
}

double EngravingPreview::systemWidth() const
{
    return m_systemWidth;
}

void EngravingPreview::setSystemWidth(double widthSp)
{
    if (qFuzzyCompare(m_systemWidth, widthSp)) {
        return;
    }

    m_systemWidth = widthSp;
    emit systemWidthChanged();

    invalidateLayout();
}

bool EngravingPreview::showsPage() const
{
    return m_showsPage;
}

void EngravingPreview::setShowsPage(bool showsPage)
{
    if (m_showsPage == showsPage) {
        return;
    }

    m_showsPage = showsPage;
    emit showsPageChanged();

    invalidateLayout();
}

double EngravingPreview::zoom() const
{
    return m_zoom;
}

void EngravingPreview::setZoom(double zoom)
{
    if (zoom <= 0.0 || qFuzzyCompare(m_zoom, zoom)) {
        return;
    }

    m_zoom = zoom;
    emit zoomChanged();

    invalidatePaint();
}

QSizeF EngravingPreview::naturalSize() const
{
    return m_naturalSize;
}

QSizeF EngravingPreview::naturalPageSize() const
{
    return m_naturalPageSize;
}

QSizeF EngravingPreview::contentSize() const
{
    return m_contentSize;
}

bool EngravingPreview::isValid() const
{
    return !m_image.isNull();
}

QString EngravingPreview::error() const
{
    return m_error;
}

QString EngravingPreview::activeSid() const
{
    return m_activeSid;
}

void EngravingPreview::setActiveSid(const QString& sid)
{
    if (m_activeSid == sid) {
        return;
    }

    m_activeSid = sid;
    m_activeStyleId = MStyle::styleIdx(String::fromQString(sid));
    emit activeSidChanged();

    invalidateLayout();
}

QVariant EngravingPreview::probeValue() const
{
    return m_probeValue;
}

void EngravingPreview::setProbeValue(const QVariant& value)
{
    if (m_probeValue == value) {
        return;
    }

    m_probeValue = value;
    emit probeValueChanged();

    invalidateLayout();
}

bool EngravingPreview::isDraggable() const
{
    return m_isDraggable;
}

void EngravingPreview::setIsDraggable(bool draggable)
{
    if (m_isDraggable == draggable) {
        return;
    }

    m_isDraggable = draggable;
    emit isDraggableChanged();

    update();
}

bool EngravingPreview::showsHandles() const
{
    return m_showsHandles;
}

void EngravingPreview::setShowsHandles(bool shows)
{
    if (m_showsHandles == shows) {
        return;
    }

    m_showsHandles = shows;
    emit showsHandlesChanged();

    update();
}

bool EngravingPreview::showsControlPoints() const
{
    return m_showsControlPoints;
}

void EngravingPreview::setShowsControlPoints(bool shows)
{
    if (m_showsControlPoints == shows) {
        return;
    }

    m_showsControlPoints = shows;
    emit showsControlPointsChanged();

    invalidatePaint();
}

QColor EngravingPreview::accentColor() const
{
    return m_accentColor;
}

void EngravingPreview::setAccentColor(const QColor& color)
{
    if (m_accentColor == color) {
        return;
    }

    m_accentColor = color;
    emit accentColorChanged();

    update();
}

bool EngravingPreview::hasEffect() const
{
    return !m_effect.isEmpty();
}

bool EngravingPreview::hasHandle() const
{
    return m_effect.hasHandle;
}

QRectF EngravingPreview::changeBox() const
{
    const double pixels = pixelsPerStaffSpace();
    return QRectF(toItem(m_changeBox.topLeft()), QSizeF(m_changeBox.width() * pixels, m_changeBox.height() * pixels));
}

bool EngravingPreview::isDragging() const
{
    return m_handle.isDragging();
}

QVariant EngravingPreview::dragValue() const
{
    return m_dragValue;
}

double EngravingPreview::dragMinimum() const
{
    return m_dragMinimum;
}

void EngravingPreview::setDragMinimum(double minimum)
{
    if (qFuzzyCompare(m_dragMinimum, minimum)) {
        return;
    }

    m_dragMinimum = minimum;
    emit dragRangeChanged();

    invalidateLayout();
}

double EngravingPreview::dragMaximum() const
{
    return m_dragMaximum;
}

void EngravingPreview::setDragMaximum(double maximum)
{
    if (qFuzzyCompare(m_dragMaximum, maximum)) {
        return;
    }

    m_dragMaximum = maximum;
    emit dragRangeChanged();

    invalidateLayout();
}

double EngravingPreview::dragStep() const
{
    return m_dragStep;
}

void EngravingPreview::setDragStep(double step)
{
    if (qFuzzyCompare(m_dragStep, step)) {
        return;
    }

    m_dragStep = step;
    emit dragRangeChanged();
}

void EngravingPreview::componentComplete()
{
    QQuickPaintedItem::componentComplete();

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        listenStyle();
        invalidateLayout();
    });

    listenStyle();
    invalidateLayout();
}

void EngravingPreview::listenStyle()
{
    const INotationPtr notation = globalContext()->currentNotation();
    const INotationStylePtr style = notation ? notation->style() : nullptr;
    if (style == m_style) {
        return;
    }

    //! NOTE The style of a score which is no longer the current one is not followed any more
    if (m_style) {
        m_style->styleChanged().resetOnNotify(this);
    }

    m_style = style;

    if (m_style) {
        m_style->styleChanged().onNotify(this, [this]() {
            invalidateLayout();
        });
    }
}

void EngravingPreview::itemChange(ItemChange change, const ItemChangeData& value)
{
    QQuickPaintedItem::itemChange(change, value);

    //! NOTE On another screen the pixels may be twice as many: a new picture of the same layout
    if (change == ItemDevicePixelRatioHasChanged || (change == ItemSceneChange && value.window)) {
        invalidatePaint();
    }

    //! NOTE A picture which is not shown is not made; it is made when it is shown, if it is out of date
    const bool isShown = (change == ItemVisibleHasChanged && value.boolValue)
                         || (change == ItemSceneChange && value.window);
    if (isShown && (m_needsLayout || m_needsPaint)) {
        enqueue();
    }
}

void EngravingPreview::invalidateLayout()
{
    m_needsLayout = true;
    m_needsPaint = true;

    enqueue();
}

void EngravingPreview::invalidatePaint()
{
    m_needsPaint = true;

    enqueue();
}

void EngravingPreview::enqueue()
{
    if (isComponentComplete() && isVisible() && window()) {
        EngravingPreviewQueue::instance()->add(this);
    }
}

//---------------------------------------------------------
//   Making the picture
//---------------------------------------------------------

//! NOTE The values the picture sets: those of the page, and the one which is dragged
std::vector<EngravingPreviewRenderer::Override> EngravingPreview::currentOverrides() const
{
    QVariantMap values = m_overrides;
    if (isDragging()) {
        values.insert(m_activeSid, m_dragValue);
    }

    std::vector<EngravingPreviewRenderer::Override> overrides;
    for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
        const StyleId id = MStyle::styleIdx(String::fromQString(it.key()));
        if (id == StyleId::NOSTYLE) {
            LOGE() << "no style value with the name: " << it.key() << ", music: " << m_music;
            continue;
        }

        overrides.push_back({ id, styleValueFromUi(id, it.value()) });
    }

    return overrides;
}

void EngravingPreview::layoutMusic(const MStyle& style)
{
    m_renderer->layout(style, currentOverrides(), m_systemWidth, m_activeStyleId);
}

//! NOTE Lays the music out, and finds what `activeSid` does to it. The layout is left as the real values have it
void EngravingPreview::updateEffect(const MStyle& style)
{
    const StyleId id = m_activeStyleId;
    if (id == StyleId::NOSTYLE) {
        LOGE() << "no style value with the name: " << m_activeSid;
    }

    //! NOTE While the handle is dragged the picture is laid out once, with the value under the pointer; the
    //! effect found when it was pressed stands (the handle is drawn where the value takes it). The full
    //! effect is found again when the drag ends
    if (isDragging()) {
        layoutMusic(style);
        return;
    }

    std::optional<PropertyValue> probe;
    if (id != StyleId::NOSTYLE && m_probeValue.isValid()) {
        //! NOTE The option is a choice: what it is shown with is the next choice, not an automatic step
        probe = styleValueFromUi(id, m_probeValue);
    } else if (id != StyleId::NOSTYLE) {
        probe = EngravingPreviewRenderer::probeOf(style, currentOverrides(), id, m_dragMaximum);
    }

    if (!probe) {
        layoutMusic(style);
        return;
    }

    m_effect = m_renderer->effectOf(style, currentOverrides(), id, *probe, m_systemWidth);

    m_changeBox = muse::RectF();
    for (const muse::RectF& box : m_effect.changed) {
        m_changeBox.unite(box);
    }
}

void EngravingPreview::render()
{
    if (!m_needsLayout && !m_needsPaint) {
        return;
    }

    const bool wasValid = isValid();
    const bool hadEffect = hasEffect();
    const bool hadHandle = hasHandle();
    const QSizeF oldContentSize = m_contentSize;
    const QSizeF oldNaturalSize = m_naturalSize;
    const QSizeF oldNaturalPageSize = m_naturalPageSize;
    const QString oldError = m_error;

    if (m_needsLayout) {
        m_needsLayout = false;

        m_image = QImage();
        m_contentSize = QSizeF();
        m_naturalSize = QSizeF();
        m_naturalPageSize = QSizeF();
        m_pages.clear();
        //! NOTE A drag goes on with the effect found when the handle was pressed (see updateEffect)
        if (!isDragging()) {
            m_effect = {};
            m_changeBox = muse::RectF();
            m_curves.clear();
        }
        m_error.clear();

        const INotationPtr notation = globalContext()->currentNotation();
        if (notation && !m_music.isEmpty()) {
            if (!m_renderer) {
                m_renderer = std::make_unique<EngravingPreviewRenderer>(iocContext());
            }

            if (!m_isMusicLoaded) {
                m_isMusicLoaded = bool(m_renderer->setMusic(m_music));
            }

            if (!m_isMusicLoaded) {
                m_error = m_renderer->error();
            } else {
                m_renderer->setShowsPage(m_showsPage);
                const MStyle& style = notation->elements()->msScore()->style();
                if (m_activeSid.isEmpty()) {
                    layoutMusic(style);
                } else {
                    updateEffect(style);
                }

                const muse::SizeF natural = m_renderer->sizeInStaffSpaces();
                m_naturalSize = QSizeF(natural.width() * BASE_STAFF_SPACE, natural.height() * BASE_STAFF_SPACE);

                m_pages = m_renderer->pagesInStaffSpaces();
                if (!m_pages.empty()) {
                    m_naturalPageSize = QSizeF(m_pages.front().width() * BASE_STAFF_SPACE,
                                               m_pages.front().height() * BASE_STAFF_SPACE);
                }
            }
        }
    }

    if (m_needsPaint) {
        m_needsPaint = false;
        paintPicture();
    }

    setImplicitSize(m_contentSize.width(), m_contentSize.height());

    if (m_contentSize != oldContentSize) {
        emit contentSizeChanged();
    }

    if (m_naturalSize != oldNaturalSize || m_naturalPageSize != oldNaturalPageSize) {
        emit naturalSizeChanged();
    }

    if (isValid() != wasValid) {
        emit isValidChanged();
    }

    if (m_error != oldError) {
        emit errorChanged();
    }

    if (hasEffect() != hadEffect || hasHandle() != hadHandle) {
        emit effectChanged();
    }

    update();
}

void EngravingPreview::paintPicture()
{
    m_image = QImage();
    m_contentSize = QSizeF();
    m_curves.clear();

    if (!m_renderer || !m_renderer->isLoaded() || !m_error.isEmpty()) {
        return;
    }

    //! NOTE The picture is always made at the size it is shown, never scaled
    const qreal pixelRatio = window() ? window()->effectiveDevicePixelRatio() : 1.0;
    m_image = m_renderer->picture(BASE_STAFF_SPACE * m_zoom, pixelRatio);
    m_contentSize = m_image.deviceIndependentSize();

    if (m_showsControlPoints) {
        for (const EngravingPreviewRenderer::Item& item : m_renderer->items()) {
            if (!item.curve.empty()) {
                m_curves.push_back(item.curve);
            }
        }
    }
}

//---------------------------------------------------------
//   Painting
//---------------------------------------------------------

//! NOTE The picture, always made at the size it is shown, is in the middle of the item
double EngravingPreview::pixelsPerStaffSpace() const
{
    return BASE_STAFF_SPACE * m_zoom;
}

QRectF EngravingPreview::pictureRect() const
{
    return QRectF((width() - m_contentSize.width()) / 2.0, (height() - m_contentSize.height()) / 2.0,
                  m_contentSize.width(), m_contentSize.height());
}

QPointF EngravingPreview::toItem(const muse::PointF& inStaffSpaces) const
{
    return pictureRect().topLeft() + QPointF(inStaffSpaces.x(), inStaffSpaces.y()) * pixelsPerStaffSpace();
}

bool EngravingPreview::isHandleShown() const
{
    return m_showsHandles && m_isDraggable && !m_activeSid.isEmpty() && m_effect.hasHandle;
}

void EngravingPreview::placeHandle()
{
    if (!m_effect.hasHandle) {
        return;
    }

    m_handle.place(toItem(m_effect.anchor), QPointF(m_effect.direction.x(), m_effect.direction.y()),
                   m_effect.staffSpacesPerUnit * pixelsPerStaffSpace());
}

PreviewOverlay EngravingPreview::overlay()
{
    PreviewOverlay result;
    result.accent = m_accentColor;

    const double pixels = pixelsPerStaffSpace();

    if (m_showsHandles && !m_activeSid.isEmpty()) {
        const double growth = WASH_GROWTH * pixels;
        for (const muse::RectF& box : m_effect.changed) {
            const QRectF rect(toItem(box.topLeft()), QSizeF(box.width() * pixels, box.height() * pixels));
            result.washes.push_back(rect.adjusted(-growth, -growth, growth, growth));
        }
    }

    if (isHandleShown()) {
        placeHandle();

        QPointF at = m_handle.position();
        if (isDragging()) {
            //! NOTE The handle is where the dragged value takes the anchor
            at += m_handle.direction() * (m_draggedNumber - m_pressedNumber) * m_effect.staffSpacesPerUnit * pixels;
        }
        result.handle = PreviewOverlay::Handle { at, m_handle.direction() };
    }

    for (const std::vector<muse::PointF>& curve : m_curves) {
        QPolygonF polygon;
        for (const muse::PointF& point : curve) {
            polygon << toItem(point);
        }

        result.controlPolygons.push_back(polygon);
    }

    return result;
}

void EngravingPreview::paint(QPainter* painter)
{
    if (m_image.isNull() || width() <= 0.0 || height() <= 0.0) {
        return;
    }

    //! NOTE The washes mark what the option changes: behind the music, so that it is not smeared
    const PreviewOverlay marks = overlay();
    marks.paintBelow(painter);
    painter->drawImage(pictureRect(), m_image);
    marks.paintAbove(painter);

    //! NOTE The edges of the pages, which are as white as the paper around them
    if (!m_pages.empty()) {
        painter->save();
        painter->setPen(QPen(PAGE_EDGE_COLOR, 1.0));
        painter->setBrush(Qt::NoBrush);
        for (const muse::RectF& page : m_pages) {
            const QRectF rect(toItem(page.topLeft()), QSizeF(page.width(), page.height()) * pixelsPerStaffSpace());
            painter->drawRect(rect.adjusted(-0.5, -0.5, 0.5, 0.5));
        }
        painter->restore();
    }
}

//---------------------------------------------------------
//   Dragging the handle
//---------------------------------------------------------

double EngravingPreview::currentNumberOf(StyleId id) const
{
    const INotationPtr notation = globalContext()->currentNotation();
    PropertyValue value = notation->elements()->msScore()->style().styleV(id);
    for (const EngravingPreviewRenderer::Override& override : currentOverrides()) {
        if (override.id == id) {
            value = override.value;
        }
    }

    return value.type() == P_TYPE::SPATIUM ? value.value<Spatium>().val() : value.toReal();
}

QVariant EngravingPreview::valueOf(StyleId id, double number) const
{
    return MStyle::valueType(id) == P_TYPE::INT ? QVariant(static_cast<int>(std::lround(number))) : QVariant(number);
}

void EngravingPreview::setDragValue(const QVariant& value)
{
    if (m_dragValue == value) {
        return;
    }

    m_dragValue = value;
    emit dragValueChanged();

    invalidateLayout();
}

void EngravingPreview::finishDrag()
{
    m_dragValue = QVariant();
    emit isDraggingChanged();
    emit dragValueChanged();

    invalidateLayout();
}

void EngravingPreview::hoverMoveEvent(QHoverEvent* event)
{
    placeHandle();

    if (isHandleShown() && m_handle.isOver(event->position())) {
        setCursor(std::abs(m_handle.direction().x()) > 0.5 ? Qt::SizeHorCursor : Qt::SizeVerCursor);
    } else {
        unsetCursor();
    }
}

void EngravingPreview::hoverLeaveEvent(QHoverEvent*)
{
    unsetCursor();
}

void EngravingPreview::mousePressEvent(QMouseEvent* event)
{
    const StyleId id = m_activeStyleId;
    if (!isHandleShown() || id == StyleId::NOSTYLE) {
        event->ignore();
        return;
    }

    placeHandle();
    m_handle.setRange(m_dragMinimum, m_dragMaximum, m_dragStep);

    const double current = currentNumberOf(id);
    if (!m_handle.press(event->position(), current)) {
        event->ignore();
        return;
    }

    forceActiveFocus();
    m_pressedNumber = current;
    m_draggedNumber = current;
    m_dragValue = valueOf(id, current);
    emit isDraggingChanged();
    emit dragValueChanged();
}

void EngravingPreview::mouseMoveEvent(QMouseEvent* event)
{
    const std::optional<double> value = m_handle.move(event->position());
    if (value) {
        m_draggedNumber = *value;
        setDragValue(valueOf(m_activeStyleId, *value));
    }
}

void EngravingPreview::mouseReleaseEvent(QMouseEvent* event)
{
    if (!isDragging()) {
        return;
    }

    const std::optional<double> value = m_handle.release(event->position(), boundingRect());
    if (value) {
        emit valueDropped(valueOf(m_activeStyleId, *value));
    }

    finishDrag();
}

void EngravingPreview::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape && isDragging()) {
        m_handle.cancel();
        finishDrag();
        return;
    }

    QQuickPaintedItem::keyPressEvent(event);
}
