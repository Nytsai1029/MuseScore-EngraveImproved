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
#include "outlinepreview.h"

#include <QPainter>
#include <QPainterPath>

#include "../addglyphsourcemodel.h"

using namespace mu::fontdesign;

OutlinePreview::OutlinePreview(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
}

QObject* OutlinePreview::sourceModel() const
{
    return m_model;
}

void OutlinePreview::setSourceModel(QObject* model)
{
    AddGlyphSourceModel* casted = qobject_cast<AddGlyphSourceModel*>(model);
    if (m_model == casted) {
        return;
    }

    if (m_model) {
        disconnect(m_model, nullptr, this, nullptr);
    }
    m_model = casted;
    if (m_model) {
        connect(m_model, &AddGlyphSourceModel::loadedChanged, this, [this]() {
            update();
        });
    }
    emit sourceModelChanged();
    update();
}

void OutlinePreview::setFillColor(const QColor& color)
{
    if (m_fillColor != color) {
        m_fillColor = color;
        emit fillColorChanged();
        update();
    }
}

void OutlinePreview::paint(QPainter* painter)
{
    if (!m_model || !m_model->hasLoadedGlyph()) {
        return;
    }

    QPainterPath path = m_model->loadedOutline().toPainterPath().toQPainterPath();
    const QRectF bounds = path.boundingRect();
    if (bounds.isEmpty()) {
        return;
    }

    constexpr double margin = 8.0;
    const double sx = (width() - 2 * margin) / bounds.width();
    const double sy = (height() - 2 * margin) / bounds.height();
    const double s = std::min(sx, sy);

    //! 字体单位 y 向上 → 视图 y 向下
    QTransform t;
    t.translate(width() / 2.0, height() / 2.0);
    t.scale(s, -s);
    t.translate(-bounds.center().x(), -bounds.center().y());

    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillPath(t.map(path), m_fillColor);
}
