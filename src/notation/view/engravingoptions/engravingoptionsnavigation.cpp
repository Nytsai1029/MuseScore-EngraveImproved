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
#include "engravingoptionsnavigation.h"

#include <algorithm>
#include <cmath>

#include <QMetaObject>
#include <QQuickItem>
#include <QRectF>
#include <QVariant>

using namespace mu::notation;
using namespace muse::ui;

namespace {
constexpr double HORIZONTAL_MARGIN = 16.0;
constexpr double VERTICAL_MARGIN = 20.0;
constexpr double SCROLL_EPSILON = 0.5;

bool isDescendantOf(const QQuickItem* item, const QQuickItem* ancestor)
{
    for (const QQuickItem* current = item; current; current = current->parentItem()) {
        if (current == ancestor) {
            return true;
        }
    }
    return false;
}

double readReal(const QObject* object, const char* name)
{
    const QVariant value = object->property(name);
    return value.isValid() ? value.toDouble() : 0.0;
}
}

EngravingOptionsNavigation::EngravingOptionsNavigation(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

QObject* EngravingOptionsNavigation::section() const
{
    return m_section;
}

void EngravingOptionsNavigation::setSection(QObject* section)
{
    if (m_section == section) {
        return;
    }

    m_section = section;
    m_lastControlKey = nullptr;

    const bool hadActive = m_activeItem != nullptr || m_isHighlighted;
    m_activeItem = nullptr;
    m_isHighlighted = false;

    emit sectionChanged();
    if (hadActive) {
        emit activeItemChanged();
    }

    connectController();
    scheduleSync();
}

QQuickItem* EngravingOptionsNavigation::activeItem() const
{
    return m_activeItem;
}

bool EngravingOptionsNavigation::isHighlighted() const
{
    return m_isHighlighted;
}

void EngravingOptionsNavigation::connectController()
{
    if (m_controllerConnected || !navigationController()) {
        return;
    }

    m_controllerConnected = true;
    navigationController()->navigationChanged().onNotify(this, [this]() {
        scheduleSync();
    });
    navigationController()->highlightChanged().onNotify(this, [this]() {
        scheduleSync();
    });
}

void EngravingOptionsNavigation::scheduleSync()
{
    if (m_syncPending) {
        return;
    }

    m_syncPending = true;
    //! NOTE Coalesce on the next event-loop tick so NavigationController::navigateTo has already set
    //! isHighlight(true) and any newly shown QML items have settled their layout geometry
    QMetaObject::invokeMethod(this, [this]() {
        m_syncPending = false;
        syncActiveControl();
    }, Qt::QueuedConnection);
}

void EngravingOptionsNavigation::syncActiveControl()
{
    if (!m_section || !navigationController()) {
        return;
    }

    const auto* trackedSection = dynamic_cast<const INavigationSection*>(m_section.data());
    if (!trackedSection) {
        return;
    }

    INavigationControl* control = navigationController()->activeControl();
    INavigationPanel* panel = control ? control->panel() : nullptr;
    INavigationSection* controlSection = panel ? panel->section() : nullptr;

    if (!control || controlSection != trackedSection) {
        const bool changed = m_activeItem != nullptr || m_isHighlighted || m_lastControlKey != nullptr;
        m_lastControlKey = nullptr;
        m_activeItem = nullptr;
        m_isHighlighted = false;
        if (changed) {
            emit activeItemChanged();
        }
        return;
    }

    QQuickItem* item = control->visualItem();
    const bool highlighted = navigationController()->isHighlight();
    const bool controlChanged = (m_lastControlKey != control) || (m_activeItem.data() != item);
    const bool highlightTurnedOn = !m_isHighlighted && highlighted;
    const bool highlightChanged = m_isHighlighted != highlighted;

    m_lastControlKey = control;
    m_activeItem = item;
    m_isHighlighted = highlighted;

    if (controlChanged || highlightChanged) {
        emit activeItemChanged();
    }

    if (item && (controlChanged || highlightTurnedOn)) {
        emit controlActivated(item, highlighted);
    }
}

double EngravingOptionsNavigation::scrollOffsetToReveal(double currentOffset, double minOffset, double maxOffset,
                                                        double viewportSize, double controlStart, double controlEnd,
                                                        double containerStart, double containerEnd, double margin,
                                                        bool revealContainerWhenVisible)
{
    if (viewportSize <= 0.0 || maxOffset <= minOffset) {
        return currentOffset;
    }

    const double pad = std::max(0.0, std::min(margin, viewportSize * 0.25));
    const bool controlVisible = (controlStart >= 0.0 && controlEnd <= viewportSize);

    if (!revealContainerWhenVisible && controlVisible) {
        return currentOffset;
    }

    const bool containerFits = (containerEnd - containerStart) <= (viewportSize - 2.0 * pad);
    const double start = containerFits ? containerStart : controlStart;
    const double end = containerFits ? containerEnd : controlEnd;

    double delta = 0.0;
    if (start < pad && end > viewportSize - pad) {
        if (start < 0.0) {
            delta = start - pad;
        } else if (end > viewportSize) {
            delta = end - (viewportSize - pad);
        }
    } else if (start < pad) {
        delta = start - pad;
    } else if (end > viewportSize - pad) {
        delta = end - (viewportSize - pad);
    }

    if (std::abs(delta) < SCROLL_EPSILON) {
        return currentOffset;
    }

    return std::clamp(currentOffset + delta, minOffset, maxOffset);
}

void EngravingOptionsNavigation::ensureVisible(QQuickItem* controlItem, QQuickItem* containerItem,
                                               bool revealContainerWhenVisible) const
{
    if (!controlItem || !controlItem->isVisible() || controlItem->width() <= 0.0 || controlItem->height() <= 0.0) {
        return;
    }

    for (QQuickItem* ancestor = controlItem->parentItem(); ancestor; ancestor = ancestor->parentItem()) {
        if (!ancestor->inherits("QQuickFlickable")) {
            continue;
        }

        if (!ancestor->isVisible() || ancestor->width() <= 0.0 || ancestor->height() <= 0.0) {
            continue;
        }

        const QRectF controlRect = controlItem->mapRectToItem(
            ancestor, QRectF(0.0, 0.0, controlItem->width(), controlItem->height()));

        QRectF containerRect = controlRect;
        if (containerItem && containerItem->isVisible() && containerItem->width() > 0.0
            && containerItem->height() > 0.0 && isDescendantOf(containerItem, ancestor)) {
            containerRect = containerItem->mapRectToItem(
                ancestor, QRectF(0.0, 0.0, containerItem->width(), containerItem->height()));
        }

        const double contentWidth = readReal(ancestor, "contentWidth");
        if (contentWidth > 0.0) {
            const double contentX = readReal(ancestor, "contentX");
            const double originX = readReal(ancestor, "originX");
            const double leftMargin = readReal(ancestor, "leftMargin");
            const double rightMargin = readReal(ancestor, "rightMargin");
            const double minX = originX - leftMargin;
            const double maxX = std::max(minX, originX + contentWidth + rightMargin - ancestor->width());

            const double targetX = scrollOffsetToReveal(contentX, minX, maxX, ancestor->width(),
                                                        controlRect.left(), controlRect.right(),
                                                        containerRect.left(), containerRect.right(),
                                                        HORIZONTAL_MARGIN, revealContainerWhenVisible);
            if (std::abs(targetX - contentX) >= SCROLL_EPSILON) {
                ancestor->setProperty("contentX", targetX);
            }
        }

        const double contentHeight = readReal(ancestor, "contentHeight");
        if (contentHeight > 0.0) {
            const double contentY = readReal(ancestor, "contentY");
            const double originY = readReal(ancestor, "originY");
            const double topMargin = readReal(ancestor, "topMargin");
            const double bottomMargin = readReal(ancestor, "bottomMargin");
            const double minY = originY - topMargin;
            const double maxY = std::max(minY, originY + contentHeight + bottomMargin - ancestor->height());

            const double targetY = scrollOffsetToReveal(contentY, minY, maxY, ancestor->height(),
                                                        controlRect.top(), controlRect.bottom(),
                                                        containerRect.top(), containerRect.bottom(),
                                                        VERTICAL_MARGIN, revealContainerWhenVisible);
            if (std::abs(targetY - contentY) >= SCROLL_EPSILON) {
                ancestor->setProperty("contentY", targetY);
            }
        }
    }
}
