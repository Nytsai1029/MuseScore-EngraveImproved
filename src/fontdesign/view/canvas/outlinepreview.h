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
#include <QQuickPaintedItem>

namespace mu::fontdesign {
class AddGlyphSourceModel;

//! 「从字体添加字形」对话框的轮廓预览：居中等比填充绘制已载入字形
class OutlinePreview : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(QObject * sourceModel READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged)
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)

public:
    explicit OutlinePreview(QQuickItem* parent = nullptr);

    void paint(QPainter* painter) override;

    QObject* sourceModel() const;
    void setSourceModel(QObject* model);

    QColor fillColor() const { return m_fillColor; }
    void setFillColor(const QColor& color);

signals:
    void sourceModelChanged();
    void fillColorChanged();

private:
    AddGlyphSourceModel* m_model = nullptr;
    QColor m_fillColor;
};
}
