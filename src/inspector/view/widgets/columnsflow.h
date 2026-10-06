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

#include <QList>
#include <QObject>
#include <QPointer>
#include <QQuickItem>

namespace mu::inspector {
//! NOTE Shows the content of the target item in several columns, when it is higher than a column.
//! The content keeps its own (vertical) layout: the blocks which do not fit into a column are only shown shifted
//! to the next one, so the content does not have to know about the columns.
//! The content can be split between the items of its (nested) Columns
class ColumnsFlow : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem * target READ target WRITE setTarget NOTIFY targetChanged)

    Q_PROPERTY(qreal columnWidth READ columnWidth WRITE setColumnWidth NOTIFY columnWidthChanged)
    Q_PROPERTY(qreal columnHeight READ columnHeight WRITE setColumnHeight NOTIFY columnHeightChanged)
    Q_PROPERTY(qreal columnSpacing READ columnSpacing WRITE setColumnSpacing NOTIFY columnSpacingChanged)

    Q_PROPERTY(int columnCount READ columnCount NOTIFY columnCountChanged)
    Q_PROPERTY(qreal contentWidth READ contentWidth NOTIFY contentWidthChanged)
    Q_PROPERTY(qreal contentHeight READ contentHeight NOTIFY contentHeightChanged)

public:
    explicit ColumnsFlow(QObject* parent = nullptr);
    ~ColumnsFlow() override;

    QQuickItem* target() const;
    void setTarget(QQuickItem* target);

    qreal columnWidth() const;
    void setColumnWidth(qreal width);

    qreal columnHeight() const;
    void setColumnHeight(qreal height);

    qreal columnSpacing() const;
    void setColumnSpacing(qreal spacing);

    int columnCount() const;
    qreal contentWidth() const;
    qreal contentHeight() const;

signals:
    void targetChanged();
    void columnWidthChanged();
    void columnHeightChanged();
    void columnSpacingChanged();

    void columnCountChanged();
    void contentWidthChanged();
    void contentHeightChanged();

private slots:
    void scheduleRelayout();
    void relayout();

private:
    struct FlowState {
        int column = 0;
        qreal columnTop = 0.0;
        bool isColumnEmpty = true;
        qreal contentHeight = 0.0;
    };

    void flowItem(QQuickItem* item, qreal top, FlowState& state);
    void placeItem(QQuickItem* item, qreal top, FlowState& state);
    QList<QQuickItem*> partsOfItem(QQuickItem* item);

    void watchItem(QQuickItem* item);
    void clearOffsets();

    void setColumnCount(int count);
    void setContentHeight(qreal height);

    QPointer<QQuickItem> m_target;

    qreal m_columnWidth = 0.0;
    qreal m_columnHeight = 0.0;
    qreal m_columnSpacing = 0.0;

    int m_columnCount = 1;
    qreal m_contentHeight = 0.0;

    QList<QPointer<QQuickTransform> > m_offsets;
    bool m_isRelayoutScheduled = false;
};
}
