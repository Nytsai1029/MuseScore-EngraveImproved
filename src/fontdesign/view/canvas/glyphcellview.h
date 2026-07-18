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
#pragma once

#include <QColor>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "uicomponents/view/quickpaintedview.h"

#include "../../ifontdesignservice.h"

namespace mu::fontdesign {
//! 字形浏览器的单元格渲染：直接绘制项目内的轮廓数据，不依赖系统字体库
class GlyphCellView : public muse::uicomponents::QuickPaintedView, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(int codepoint READ codepoint WRITE setCodepoint NOTIFY codepointChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    muse::Inject<IFontDesignService> fontDesignService = { this };

public:
    explicit GlyphCellView(QQuickItem* parent = nullptr);

    void paint(QPainter* painter) override;

    int codepoint() const;
    void setCodepoint(int code);

    QColor color() const;
    void setColor(const QColor& color);

signals:
    void codepointChanged();
    void colorChanged();

private:
    int m_codepoint = 0;
    QColor m_color;
};
}
