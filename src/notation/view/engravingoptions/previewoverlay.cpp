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
#include "previewoverlay.h"

#include <cmath>

#include <QPainter>
#include <QPainterPath>
#include <QPen>

using namespace mu::notation;

static constexpr double WASH_OPACITY = 0.26;
static constexpr double WASH_GROWTH = 0.20;
static constexpr double WASH_MIN_SIZE = 0.85;
static constexpr double WASH_RADIUS = 3.0;
static constexpr double HANDLE_RADIUS = 4.5;
static constexpr double HANDLE_RING = 1.5;
static constexpr double HANDLE_LINE = 16.0;
static constexpr double ARROW_HEAD = 4.5;
static constexpr double CONTROL_POINT_SIZE = 4.0;

QRectF PreviewOverlay::washRectFor(const QRectF& itemRect, double pixelsPerStaffSpace)
{
    const double growth = WASH_GROWTH * pixelsPerStaffSpace;
    const double minSize = WASH_MIN_SIZE * pixelsPerStaffSpace;
    QRectF rect = itemRect.adjusted(-growth, -growth, growth, growth);
    if (rect.width() < minSize) {
        const double pad = (minSize - rect.width()) * 0.5;
        rect.adjust(-pad, 0.0, pad, 0.0);
    }
    if (rect.height() < minSize) {
        const double pad = (minSize - rect.height()) * 0.5;
        rect.adjust(0.0, -pad, 0.0, pad);
    }
    return rect;
}

void PreviewOverlay::paintBelow(QPainter* painter) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    paintWashes(painter);

    painter->restore();
}

void PreviewOverlay::paintAbove(QPainter* painter) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    paintControlPolygons(painter);
    paintHandle(painter);

    painter->restore();
}

void PreviewOverlay::paintWashes(QPainter* painter) const
{
    //! NOTE One fill of the union of the rounded rectangles: overlapping washes do not add up. The winding
    //! rule: where they overlap is still filled (odd-even would leave a hole there)
    QPainterPath shape;
    shape.setFillRule(Qt::WindingFill);
    for (const QRectF& rect : washes) {
        shape.addRoundedRect(rect, WASH_RADIUS, WASH_RADIUS);
    }

    QColor wash = accent;
    wash.setAlphaF(WASH_OPACITY);

    painter->fillPath(shape, wash);
}

void PreviewOverlay::paintControlPolygons(QPainter* painter) const
{
    QPen dashed(QColor(0x8A, 0x8A, 0x8F), 1.0, Qt::DashLine);
    dashed.setCosmetic(true);

    for (const QPolygonF& polygon : controlPolygons) {
        painter->setPen(dashed);
        painter->setBrush(Qt::NoBrush);
        painter->drawPolyline(polygon);

        painter->setPen(Qt::NoPen);
        painter->setBrush(accent);
        for (const QPointF& point : polygon) {
            painter->drawRect(QRectF(point.x() - CONTROL_POINT_SIZE / 2.0, point.y() - CONTROL_POINT_SIZE / 2.0,
                                     CONTROL_POINT_SIZE, CONTROL_POINT_SIZE));
        }
    }
}

//! The dot, with a line which goes on in the direction the value moves the picture, ending in an arrow head
void PreviewOverlay::paintHandle(QPainter* painter) const
{
    if (!handle) {
        return;
    }

    const QPointF at = handle->position;
    const QPointF along = handle->direction;
    const QPointF across(-along.y(), along.x());

    const QPointF tip = at + along * HANDLE_LINE;
    QPen line(accent, 1.5);
    line.setCapStyle(Qt::RoundCap);
    painter->setPen(line);
    painter->drawLine(at, tip);
    painter->drawLine(tip, tip - along * ARROW_HEAD + across * ARROW_HEAD * 0.6);
    painter->drawLine(tip, tip - along * ARROW_HEAD - across * ARROW_HEAD * 0.6);

    painter->setPen(QPen(Qt::white, HANDLE_RING));
    painter->setBrush(accent);
    painter->drawEllipse(at, HANDLE_RADIUS, HANDLE_RADIUS);
}
