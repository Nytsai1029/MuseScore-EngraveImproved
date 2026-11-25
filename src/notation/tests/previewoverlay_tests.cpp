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
#include <gtest/gtest.h>

#include <QImage>
#include <QPainter>

#include "notation/view/engravingoptions/previewhandle.h"
#include "notation/view/engravingoptions/previewoverlay.h"

using namespace mu::notation;

namespace {
const QColor ACCENT(0x20, 0x80, 0xE0);

bool isAccent(const QColor& color)
{
    return std::abs(color.red() - ACCENT.red()) < 12 && std::abs(color.green() - ACCENT.green()) < 12
           && std::abs(color.blue() - ACCENT.blue()) < 12 && color.alpha() > 240;
}

QImage paintOnWhite(const PreviewOverlay& overlay, const QSize& size = QSize(200, 120))
{
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);

    QPainter painter(&image);
    overlay.paintBelow(&painter);
    overlay.paintAbove(&painter);
    return image;
}
}

/**
 * @TEST_CASE handleIsDraggedAlongItsDirection
 * @details The value follows the pointer along the direction, one unit for a given number of pixels
 */
TEST(Notation_PreviewHandleTests, handleIsDraggedAlongItsDirection)
{
    PreviewHandle handle;
    handle.place(QPointF(50, 40), QPointF(1, 0), 20.0);
    handle.setRange(0.0, 5.0, 0.25);

    // [THEN] Away from the handle nothing is pressed
    EXPECT_TRUE(handle.isOver(QPointF(53, 43)));
    EXPECT_FALSE(handle.isOver(QPointF(80, 40)));
    EXPECT_FALSE(handle.press(QPointF(80, 40), 1.0));
    EXPECT_FALSE(handle.isDragging());
    EXPECT_FALSE(handle.move(QPointF(90, 40)).has_value());

    // [WHEN] The handle is pressed and moved 40 px to the right, and 10 px down
    ASSERT_TRUE(handle.press(QPointF(50, 40), 1.0));
    EXPECT_TRUE(handle.isDragging());
    const std::optional<double> value = handle.move(QPointF(90, 50));

    // [THEN] Only the way along the direction counts: two units more
    ASSERT_TRUE(value);
    EXPECT_DOUBLE_EQ(*value, 3.0);

    // [THEN] A value is a number of steps, and stays in the range
    EXPECT_DOUBLE_EQ(*handle.move(QPointF(57, 40)), 1.25);
    EXPECT_DOUBLE_EQ(*handle.move(QPointF(500, 40)), 5.0);
    EXPECT_DOUBLE_EQ(*handle.move(QPointF(-500, 40)), 0.0);
}

/**
 * @TEST_CASE handleOfAVerticalValueMovesWithItsDirection
 * @details A handle which goes up when the value grows gives more value when pulled up
 */
TEST(Notation_PreviewHandleTests, handleOfAVerticalValueMovesWithItsDirection)
{
    PreviewHandle handle;
    handle.place(QPointF(50, 40), QPointF(0, -1), 10.0);
    handle.setRange(-10.0, 10.0, 0.5);

    ASSERT_TRUE(handle.press(QPointF(50, 40), 0.0));
    EXPECT_DOUBLE_EQ(*handle.move(QPointF(50, 25)), 1.5);
    EXPECT_DOUBLE_EQ(*handle.move(QPointF(50, 60)), -2.0);
}

/**
 * @TEST_CASE releaseKeepsTheValueOnlyInsideTheItem
 * @details A release outside of the item, and a cancel, give nothing
 */
TEST(Notation_PreviewHandleTests, releaseKeepsTheValueOnlyInsideTheItem)
{
    const QRectF bounds(0, 0, 200, 100);

    PreviewHandle handle;
    handle.place(QPointF(50, 40), QPointF(1, 0), 20.0);
    handle.setRange(0.0, 5.0, 0.25);

    ASSERT_TRUE(handle.press(QPointF(50, 40), 1.0));
    const std::optional<double> kept = handle.release(QPointF(90, 40), bounds);
    ASSERT_TRUE(kept);
    EXPECT_DOUBLE_EQ(*kept, 3.0);
    EXPECT_FALSE(handle.isDragging());

    ASSERT_TRUE(handle.press(QPointF(50, 40), 1.0));
    EXPECT_FALSE(handle.release(QPointF(250, 40), bounds).has_value());
    EXPECT_FALSE(handle.isDragging());

    ASSERT_TRUE(handle.press(QPointF(50, 40), 1.0));
    handle.cancel();
    EXPECT_FALSE(handle.isDragging());
    EXPECT_FALSE(handle.move(QPointF(90, 40)).has_value());
}

/**
 * @TEST_CASE handleIsPaintedInTheAccentColor
 * @details The dot of the handle is the accent colour, and what is far from it is not painted
 */
TEST(Notation_PreviewOverlayTests, handleIsPaintedInTheAccentColor)
{
    PreviewOverlay overlay;
    overlay.accent = ACCENT;
    overlay.handle = PreviewOverlay::Handle { QPointF(100, 60), QPointF(1, 0) };

    const QImage image = paintOnWhite(overlay);

    EXPECT_TRUE(isAccent(image.pixelColor(100, 60)));
    // the line goes on in front of the dot (a line a pixel and a half wide is not whole in one pixel)
    EXPECT_GT(image.pixelColor(110, 60).blue(), image.pixelColor(110, 60).red());
    EXPECT_EQ(image.pixelColor(20, 20), QColor(Qt::white));
    EXPECT_EQ(image.pixelColor(100, 90), QColor(Qt::white));
}

/**
 * @TEST_CASE washIsSoft
 * @details Behind the items which change there is a pale wash of the accent, not the accent itself
 */
TEST(Notation_PreviewOverlayTests, washIsSoft)
{
    PreviewOverlay overlay;
    overlay.accent = ACCENT;
    overlay.washes.push_back(QRectF(40, 30, 60, 30));

    const QImage image = paintOnWhite(overlay);

    const QColor inside = image.pixelColor(70, 45);
    EXPECT_NE(inside, QColor(Qt::white));
    EXPECT_FALSE(isAccent(inside));
    EXPECT_GT(inside.blue(), inside.red());
    EXPECT_EQ(image.pixelColor(10, 10), QColor(Qt::white));
}

/**
 * @TEST_CASE overlappingWashesStayWashed
 * @details Where two washes overlap is still washed, with the alpha of one wash (winding fill, not odd-even)
 */
TEST(Notation_PreviewOverlayTests, overlappingWashesStayWashed)
{
    PreviewOverlay overlay;
    overlay.accent = ACCENT;
    overlay.washes.push_back(QRectF(40, 30, 60, 30));
    overlay.washes.push_back(QRectF(80, 30, 60, 30));

    const QImage image = paintOnWhite(overlay);

    const QColor alone = image.pixelColor(70, 45);
    const QColor overlap = image.pixelColor(90, 45);
    EXPECT_EQ(overlap, alone);
}

/**
 * @TEST_CASE controlPointsAreSquaresOfTheAccent
 * @details The four points of a curve are painted in the accent colour
 */
TEST(Notation_PreviewOverlayTests, controlPointsAreSquaresOfTheAccent)
{
    PreviewOverlay overlay;
    overlay.accent = ACCENT;
    overlay.controlPolygons.push_back(
        QPolygonF({ QPointF(20, 80), QPointF(60, 30), QPointF(120, 30), QPointF(160, 80) }));

    const QImage image = paintOnWhite(overlay);

    for (const QPointF& point : overlay.controlPolygons.front()) {
        EXPECT_TRUE(isAccent(image.pixelColor(point.toPoint()))) << point.x() << "," << point.y();
    }
}

/**
 * @TEST_CASE washRectForKeepsSmallMarksReadable
 * @details A tiny mark (a staccato dot, a barline) gets a wash at least large enough to see, centred on it
 */
TEST(Notation_PreviewOverlayTests, washRectForKeepsSmallMarksReadable)
{
    const double pixelsPerSp = 10.0;
    const QRectF dot(50.0, 40.0, 2.0, 2.0);
    const QRectF wash = PreviewOverlay::washRectFor(dot, pixelsPerSp);

    EXPECT_GE(wash.width(), 8.0);
    EXPECT_GE(wash.height(), 8.0);
    EXPECT_NEAR(wash.center().x(), dot.center().x(), 0.01);
    EXPECT_NEAR(wash.center().y(), dot.center().y(), 0.01);
}
