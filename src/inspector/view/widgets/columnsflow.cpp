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
#include "columnsflow.h"

#include <algorithm>

#include <QMatrix4x4>

using namespace mu::inspector;

//! NOTE The sizes are compared with a tolerance, to not depend on the rounding of the fractional sizes
static constexpr qreal COLUMNS_FLOW_SIZE_TOLERANCE = 0.5;

namespace mu::inspector {
class ColumnOffset : public QQuickTransform
{
public:
    ColumnOffset(const QPointF& offset, QObject* parent)
        : QQuickTransform(parent), m_offset(offset)
    {
    }

    void applyTo(QMatrix4x4* matrix) const override
    {
        matrix->translate(m_offset.x(), m_offset.y());
    }

private:
    QPointF m_offset;
};
}

ColumnsFlow::ColumnsFlow(QObject* parent)
    : QObject(parent)
{
}

ColumnsFlow::~ColumnsFlow()
{
    clearOffsets();
}

QQuickItem* ColumnsFlow::target() const
{
    return m_target;
}

void ColumnsFlow::setTarget(QQuickItem* target)
{
    if (m_target == target) {
        return;
    }

    m_target = target;
    emit targetChanged();

    scheduleRelayout();
}

qreal ColumnsFlow::columnWidth() const
{
    return m_columnWidth;
}

void ColumnsFlow::setColumnWidth(qreal width)
{
    if (qFuzzyIsNull(m_columnWidth - width)) {
        return;
    }

    m_columnWidth = width;
    emit columnWidthChanged();
    emit contentWidthChanged();

    scheduleRelayout();
}

qreal ColumnsFlow::columnHeight() const
{
    return m_columnHeight;
}

void ColumnsFlow::setColumnHeight(qreal height)
{
    if (qFuzzyIsNull(m_columnHeight - height)) {
        return;
    }

    m_columnHeight = height;
    emit columnHeightChanged();

    scheduleRelayout();
}

qreal ColumnsFlow::columnSpacing() const
{
    return m_columnSpacing;
}

void ColumnsFlow::setColumnSpacing(qreal spacing)
{
    if (qFuzzyIsNull(m_columnSpacing - spacing)) {
        return;
    }

    m_columnSpacing = spacing;
    emit columnSpacingChanged();
    emit contentWidthChanged();

    scheduleRelayout();
}

int ColumnsFlow::columnCount() const
{
    return m_columnCount;
}

qreal ColumnsFlow::contentWidth() const
{
    return m_columnCount * m_columnWidth + (m_columnCount - 1) * m_columnSpacing;
}

qreal ColumnsFlow::contentHeight() const
{
    return m_contentHeight;
}

void ColumnsFlow::scheduleRelayout()
{
    if (m_isRelayoutScheduled) {
        return;
    }

    m_isRelayoutScheduled = true;
    QMetaObject::invokeMethod(this, &ColumnsFlow::relayout, Qt::QueuedConnection);
}

void ColumnsFlow::relayout()
{
    m_isRelayoutScheduled = false;

    clearOffsets();

    FlowState state;

    if (m_target) {
        if (m_columnHeight > 0.0 && m_columnWidth > 0.0) {
            flowItem(m_target, 0.0, state);
        } else {
            watchItem(m_target);
            state.contentHeight = m_target->height();
        }
    }

    setColumnCount(state.column + 1);
    setContentHeight(state.contentHeight);
}

void ColumnsFlow::flowItem(QQuickItem* item, qreal top, FlowState& state)
{
    watchItem(item);

    const qreal height = item->height();
    const bool fitsIntoCurrentColumn = top + height - state.columnTop <= m_columnHeight + COLUMNS_FLOW_SIZE_TOLERANCE;

    if (!fitsIntoCurrentColumn) {
        const bool fitsIntoNewColumn = height <= m_columnHeight + COLUMNS_FLOW_SIZE_TOLERANCE;

        if (!fitsIntoNewColumn) {
            const QList<QQuickItem*> parts = partsOfItem(item);
            if (!parts.empty()) {
                for (QQuickItem* part : parts) {
                    flowItem(part, top + part->y(), state);
                }

                return;
            }
        }

        if (!state.isColumnEmpty) {
            ++state.column;
            state.columnTop = top;
            state.isColumnEmpty = true;
        }
    }

    placeItem(item, top, state);
}

void ColumnsFlow::placeItem(QQuickItem* item, qreal top, FlowState& state)
{
    const QPointF offset(state.column * (m_columnWidth + m_columnSpacing), -state.columnTop);

    if (!offset.isNull()) {
        ColumnOffset* transform = new ColumnOffset(offset, this);
        transform->appendToItem(item);
        m_offsets.push_back(transform);
    }

    state.isColumnEmpty = false;
    state.contentHeight = std::max(state.contentHeight, top + item->height() - state.columnTop);
}

//! NOTE Returns the parts into which the item can be split (from top to bottom), if any
QList<QQuickItem*> ColumnsFlow::partsOfItem(QQuickItem* item)
{
    //! NOTE: the item itself stays where it is, so it must not be visible and must not clip its parts
    if (item->clip() || (item->flags() & QQuickItem::ItemHasContents)) {
        return {};
    }

    //! NOTE: the items of the columns and the grids are placed one under another (row by row),
    //! so they can be split between any of them
    const bool isColumnOrGrid = item->inherits("QQuickColumn") || item->inherits("QQuickColumnLayout")
                                || item->inherits("QQuickGrid") || item->inherits("QQuickGridLayout");

    QList<QQuickItem*> parts;

    for (QQuickItem* child : item->childItems()) {
        watchItem(child);

        if (!child->isVisible() || child->width() <= 0.0 || child->height() <= 0.0) {
            continue;
        }

        //! NOTE: skip the helpers which show nothing (like the mouse areas)
        const bool showsSomething = (child->flags() & QQuickItem::ItemHasContents) || !child->childItems().empty();
        if (!isColumnOrGrid && !showsSomething) {
            continue;
        }

        parts.push_back(child);
    }

    //! NOTE: any other item can be split only if it is just a wrapper of its single child
    //! (otherwise, its children may be placed side by side or over each other)
    if (!isColumnOrGrid && parts.size() != 1) {
        return {};
    }

    std::stable_sort(parts.begin(), parts.end(), [](const QQuickItem* part1, const QQuickItem* part2) {
        return part1->y() < part2->y();
    });

    return parts;
}

void ColumnsFlow::watchItem(QQuickItem* item)
{
    connect(item, &QQuickItem::yChanged, this, &ColumnsFlow::scheduleRelayout, Qt::UniqueConnection);
    connect(item, &QQuickItem::heightChanged, this, &ColumnsFlow::scheduleRelayout, Qt::UniqueConnection);
    connect(item, &QQuickItem::visibleChanged, this, &ColumnsFlow::scheduleRelayout, Qt::UniqueConnection);
    connect(item, &QQuickItem::childrenChanged, this, &ColumnsFlow::scheduleRelayout, Qt::UniqueConnection);
}

void ColumnsFlow::clearOffsets()
{
    for (const QPointer<QQuickTransform>& offset : m_offsets) {
        delete offset.data();
    }

    m_offsets.clear();
}

void ColumnsFlow::setColumnCount(int count)
{
    if (m_columnCount == count) {
        return;
    }

    m_columnCount = count;
    emit columnCountChanged();
    emit contentWidthChanged();
}

void ColumnsFlow::setContentHeight(qreal height)
{
    if (qFuzzyIsNull(m_contentHeight - height)) {
        return;
    }

    m_contentHeight = height;
    emit contentHeightChanged();
}
