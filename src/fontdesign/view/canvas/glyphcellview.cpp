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
#include "glyphcellview.h"

#include <QPainter>
#include <QPainterPath>

using namespace mu::fontdesign;

GlyphCellView::GlyphCellView(QQuickItem* parent)
    : muse::uicomponents::QuickPaintedView(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void GlyphCellView::paint(QPainter* painter)
{
    FontDesignProjectPtr project = fontDesignService()->currentProject();
    if (!project) {
        return;
    }

    const GlyphItem* glyph = project->glyph(static_cast<char32_t>(m_codepoint));
    if (!glyph || glyph->outline.isEmpty()) {
        return;
    }

    QPainterPath path = glyph->outline.toPainterPath().toQPainterPath();
    QRectF bbox = path.boundingRect();
    if (bbox.isEmpty()) {
        return;
    }

    const double padding = 6.0;
    const double availW = width() - 2 * padding;
    const double availH = height() - 2 * padding;
    if (availW <= 0 || availH <= 0) {
        return;
    }

    // 适配 bbox，且限制放大倍数，避免小字形被过度放大
    const double maxScale = availH / (project->upem() * 0.8);
    double scale = std::min({ availW / bbox.width(), availH / bbox.height(), maxScale });

    // y 向上 → 屏幕 y 向下翻转；bbox 中心对齐单元格中心
    QTransform transform;
    transform.translate(width() / 2.0, height() / 2.0);
    transform.scale(scale, -scale);
    transform.translate(-bbox.center().x(), -bbox.center().y());

    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillPath(transform.map(path), m_color);
}

int GlyphCellView::codepoint() const
{
    return m_codepoint;
}

void GlyphCellView::setCodepoint(int code)
{
    if (m_codepoint == code) {
        return;
    }

    m_codepoint = code;
    emit codepointChanged();
    update();
}

QColor GlyphCellView::color() const
{
    return m_color;
}

void GlyphCellView::setColor(const QColor& color)
{
    if (m_color == color) {
        return;
    }

    m_color = color;
    emit colorChanged();
    update();
}
