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
#include <vector>

#include <QColor>
#include <QImage>
#include <QPointF>
#include <QPolygonF>
#include <QRectF>
#include <QVector>

class QPainter;

namespace mu::notation {
//! NOTE What a preview draws over its picture to show what an option does: a wash behind the items which the
//! option changes, the handle which is dragged to change it, and the control points of slurs and ties.
//! All of it in the pixels of the item, so it can be painted without a score
class PreviewOverlay
{
public:
    struct Handle {
        QPointF position;
        QPointF direction;      // a unit vector
    };

    //! NOTE The ink that a probe adds or moves into new positions, tinted in the accent color and placed in
    //! the before-picture's logical pixel frame
    struct Ghost {
        QImage image;
        QPointF offset;         // logical pixels from the before-picture's top-left corner

        bool isNull() const { return image.isNull(); }
    };

    QVector<QRectF> washes;
    std::optional<Handle> handle;
    QVector<QPolygonF> controlPolygons;     // start, bezier 1, bezier 2, end of each curve

    QColor accent;

    //! NOTE The wash behind one changed item box (in item pixels): padded around the box and kept large enough
    //! to read on tiny marks such as staccato dots, tenuto dashes and single barlines
    static QRectF washRectFor(const QRectF& itemRect, double pixelsPerStaffSpace);

    //! NOTE Builds the accent-tinted ghost of pixels present in `after` (aligned by `probeFrameShiftLogical`)
    //! but not in `before`, clipped to `afterRegionsLogical` so carried-along items stay clean
    static Ghost probedGhost(const QImage& before, const QImage& after, const QPointF& probeFrameShiftLogical,
                             const std::vector<QRectF>& afterRegionsLogical, double pixelsPerStaffSpace,
                             const QColor& accent);

    //! NOTE The washes, painted behind the music so that it is not smeared
    void paintBelow(QPainter* painter) const;
    //! NOTE The control polygons and the handle, painted over the music
    void paintAbove(QPainter* painter) const;

private:
    void paintWashes(QPainter* painter) const;
    void paintControlPolygons(QPainter* painter) const;
    void paintHandle(QPainter* painter) const;
};
}
