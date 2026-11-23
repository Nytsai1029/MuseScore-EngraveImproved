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

#include <optional>

#include <QPointF>
#include <QRectF>

namespace mu::notation {
//! NOTE The handle of a preview: a point on the picture which is dragged to change a value, as far along a
//! line as the value changes the picture. It knows nothing of the picture or of QML, only of the pointer:
//! where the handle is, which way it goes, and how many pixels make a unit of the value
class PreviewHandle
{
public:
    //! The handle sits at `position` (pixels), and the value grows by one when the pointer goes
    //! `pixelsPerUnit` pixels along `direction` (a unit vector)
    void place(const QPointF& position, const QPointF& direction, double pixelsPerUnit);

    //! The values the handle gives: between `minimum` and `maximum`, in steps
    void setRange(double minimum, double maximum, double step);

    const QPointF& position() const { return m_position; }
    const QPointF& direction() const { return m_direction; }

    //! Whether a pointer at `point` is over the handle
    bool isOver(const QPointF& point) const;

    //! The pointer is pressed over the handle, which is where the value `current` is
    bool press(const QPointF& point, double current);

    //! The value under the pointer, while the handle is dragged
    std::optional<double> move(const QPointF& point) const;

    //! The pointer is released: the value to keep, or nothing if it is released outside of `bounds`
    std::optional<double> release(const QPointF& point, const QRectF& bounds);

    void cancel();
    bool isDragging() const { return m_isDragging; }

private:
    QPointF m_position;
    QPointF m_direction;
    double m_pixelsPerUnit = 1.0;

    double m_minimum = -1e9;
    double m_maximum = 1e9;
    double m_step = 0.01;

    bool m_isDragging = false;
    QPointF m_pressedAt;
    double m_valueAtPress = 0.0;
};
}
