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
#include "previewhandle.h"

#include <algorithm>
#include <cmath>

using namespace mu::notation;

//! NOTE How near the pointer must be to the middle of the handle, in pixels
static constexpr double GRAB_RADIUS = 9.0;

void PreviewHandle::place(const QPointF& position, const QPointF& direction, double pixelsPerUnit)
{
    m_position = position;
    m_direction = direction;
    m_pixelsPerUnit = pixelsPerUnit > 0.0 ? pixelsPerUnit : 1.0;
}

void PreviewHandle::setRange(double minimum, double maximum, double step)
{
    m_minimum = minimum;
    m_maximum = maximum;
    m_step = step > 0.0 ? step : 0.01;
}

bool PreviewHandle::isOver(const QPointF& point) const
{
    const QPointF away = point - m_position;
    return std::hypot(away.x(), away.y()) <= GRAB_RADIUS;
}

bool PreviewHandle::press(const QPointF& point, double current)
{
    if (!isOver(point)) {
        return false;
    }

    m_isDragging = true;
    m_pressedAt = point;
    m_valueAtPress = current;
    return true;
}

std::optional<double> PreviewHandle::move(const QPointF& point) const
{
    if (!m_isDragging) {
        return std::nullopt;
    }

    const QPointF moved = point - m_pressedAt;
    const double along = moved.x() * m_direction.x() + moved.y() * m_direction.y();
    const double value = m_valueAtPress + along / m_pixelsPerUnit;

    const double stepped = std::round(value / m_step) * m_step;
    return std::clamp(stepped, m_minimum, m_maximum);
}

std::optional<double> PreviewHandle::release(const QPointF& point, const QRectF& bounds)
{
    const std::optional<double> value = move(point);
    m_isDragging = false;

    return bounds.contains(point) ? value : std::nullopt;
}

void PreviewHandle::cancel()
{
    m_isDragging = false;
}
