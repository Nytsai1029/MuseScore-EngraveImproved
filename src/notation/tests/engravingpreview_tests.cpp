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

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <vector>

#include <QImage>
#include <QString>

#include "draw/painter.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mscore.h"
#include "engraving/style/style.h"
#include "engraving/types/types.h"

#include "notation/view/engravingoptions/engravingpreviewrenderer.h"

#include "view/styledialog/styleuivalue.h"

#include "engravingoptionspagesource.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace mu::notation::tests;

namespace {
const QString TWO_MEASURES("c''2 d''4 e''8 f'' | g''2 a''2");

//! NOTE The style which a score gets when nothing is set, as the preview takes it from the current score
MStyle baseStyle()
{
    std::unique_ptr<MasterScore> score(compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr));
    return score->style();
}

int count(const std::vector<EngravingPreviewRenderer::Item>& items, ElementType type)
{
    return static_cast<int>(std::count_if(items.begin(), items.end(),
                                          [type](const EngravingPreviewRenderer::Item& item) {
        return item.type == type;
    }));
}

bool same(const std::vector<EngravingPreviewRenderer::Item>& a, const std::vector<EngravingPreviewRenderer::Item>& b)
{
    if (a.size() != b.size()) {
        return false;
    }

    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i].type != b[i].type || a[i].track != b[i].track || a[i].tick != b[i].tick || a[i].box != b[i].box
            || a[i].curve != b[i].curve) {
            return false;
        }
    }

    return true;
}
}

class Notation_EngravingPreviewTests : public ::testing::Test
{
protected:
    //! NOTE The tests read as the application does, not with the reader of old files which test mode takes
    void SetUp() override
    {
        m_useRead302 = MScore::useRead302InTestMode;
        MScore::useRead302InTestMode = false;
    }

    void TearDown() override
    {
        MScore::useRead302InTestMode = m_useRead302;
    }

    //! A renderer which has the music, and has laid it out (on a whole page if `showsPage`)
    static std::unique_ptr<EngravingPreviewRenderer> laidOut(const QString& music, const MStyle& style,
                                                             const std::vector<EngravingPreviewRenderer::Override>&
                                                             overrides = {},
                                                             double systemWidthSp = 0.0, bool showsPage = false)
    {
        auto renderer = std::make_unique<EngravingPreviewRenderer>(nullptr);
        const muse::Ret ret = renderer->setMusic(music);
        EXPECT_TRUE(ret) << music.toStdString() << ": " << ret.text();
        if (!ret) {
            return nullptr;
        }

        renderer->setShowsPage(showsPage);
        renderer->layout(style, overrides, systemWidthSp);
        return renderer;
    }

    bool m_useRead302 = false;
};

/**
 * @TEST_CASE musicIsLaidOut
 * @details Music of a size a preview can show, and the reason when the music is not valid
 */
TEST_F(Notation_EngravingPreviewTests, musicIsLaidOut)
{
    auto renderer = laidOut(TWO_MEASURES, baseStyle());
    ASSERT_TRUE(renderer);

    const muse::SizeF size = renderer->sizeInStaffSpaces();
    EXPECT_GE(size.width(), 6.0);
    EXPECT_LE(size.width(), 200.0);
    EXPECT_GE(size.height(), 4.0);
    EXPECT_LE(size.height(), 40.0);
    EXPECT_TRUE(renderer->error().isEmpty());

    EngravingPreviewRenderer broken(nullptr);
    EXPECT_FALSE(broken.setMusic("c'4 x"));
    EXPECT_FALSE(broken.isLoaded());
    EXPECT_FALSE(broken.error().isEmpty());
}

/**
 * @TEST_CASE aPageIsShownWhole
 * @details With showsPage the music is laid out on the page as the style has it, and the picture is the whole
 *          page: the size of the paper, with the music below the top margin, and the margins of the music on the
 *          page change it. Music of several pages shows them all, side by side, with the items of each
 */
TEST_F(Notation_EngravingPreviewTests, aPageIsShownWhole)
{
    const MStyle style = baseStyle();
    auto renderer = laidOut(TWO_MEASURES, style, {}, 0.0, true);
    ASSERT_TRUE(renderer);
    EXPECT_TRUE(renderer->showsPage());

    const double spatium = style.spatium();
    const muse::SizeF size = renderer->sizeInStaffSpaces();
    EXPECT_NEAR(size.width(), style.styleD(Sid::pageWidth) * DPI / spatium, 0.01);
    EXPECT_NEAR(size.height(), style.styleD(Sid::pageHeight) * DPI / spatium, 0.01);

    const std::vector<EngravingPreviewRenderer::Item> items = renderer->items();
    const auto staffLines = std::find_if(items.begin(), items.end(), [](const EngravingPreviewRenderer::Item& item) {
        return item.type == ElementType::STAFF_LINES;
    });
    ASSERT_NE(staffLines, items.end());
    EXPECT_GT(staffLines->box.top(), style.styleD(Sid::pageOddTopMargin) * DPI / spatium);
    EXPECT_GT(staffLines->box.left(), style.styleD(Sid::pageOddLeftMargin) * DPI / spatium - 0.01);

    EXPECT_FALSE(renderer->effectOf(style, {}, Sid::staffUpperBorder, PropertyValue(Spatium(10.0))).isEmpty());

    const std::vector<muse::RectF> pages = renderer->pagesInStaffSpaces();
    ASSERT_EQ(pages.size(), 1u);
    EXPECT_NEAR(pages.front().width(), size.width(), 0.01);

    auto alone = laidOut(TWO_MEASURES, style);
    ASSERT_TRUE(alone);
    EXPECT_LT(alone->sizeInStaffSpaces().height(), size.height() / 4.0);
    EXPECT_TRUE(alone->pagesInStaffSpaces().empty());

    auto twoPages = laidOut("c''1 \\pageBreak e''1", style, {}, 0.0, true);
    ASSERT_TRUE(twoPages);
    const std::vector<muse::RectF> both = twoPages->pagesInStaffSpaces();
    ASSERT_EQ(both.size(), 2u);
    EXPECT_GT(both[1].left(), both[0].right() - 0.01);
    EXPECT_NEAR(twoPages->sizeInStaffSpaces().width(), both[1].right() - both[0].left(), 0.01);
    EXPECT_EQ(count(twoPages->items(), ElementType::NOTE), 2);
}

/**
 * @TEST_CASE itemsAreTheNotesAndTheTie
 * @details The items of the first page have what the music has, in the same order whenever it is laid out
 */
TEST_F(Notation_EngravingPreviewTests, itemsAreTheNotesAndTheTie)
{
    auto renderer = laidOut("c''2~ c''2 e''4", baseStyle());
    ASSERT_TRUE(renderer);

    const std::vector<EngravingPreviewRenderer::Item> items = renderer->items();
    EXPECT_EQ(count(items, ElementType::NOTE), 3);
    EXPECT_EQ(count(items, ElementType::TIE_SEGMENT), 1);

    for (const EngravingPreviewRenderer::Item& item : items) {
        EXPECT_GE(item.box.left(), -0.01) << int(item.type);
        EXPECT_GE(item.box.top(), -0.01) << int(item.type);

        if (item.type == ElementType::TIE_SEGMENT) {
            EXPECT_EQ(item.curve.size(), 4u);
        } else {
            EXPECT_TRUE(item.curve.empty());
        }
    }

    for (size_t i = 1; i < items.size(); ++i) {
        EXPECT_FALSE(std::make_tuple(items[i].type, items[i].track, items[i].tick)
                     < std::make_tuple(items[i - 1].type, items[i - 1].track, items[i - 1].tick));
    }

    renderer->layout(baseStyle(), {});
    EXPECT_TRUE(same(items, renderer->items()));
}

/**
 * @TEST_CASE overrideChangesTheLayout
 * @details A value which the preview sets is the one the layout uses
 */
/**
 * @TEST_CASE theBracketOfAGrandStaffIsInThePicture
 * @details `\staves 2` gives the two staves a brace; the drawn bracket item is inside
 *          the picture's box — the picture contains everything which is drawn on the page
 */
TEST_F(Notation_EngravingPreviewTests, theBracketOfAGrandStaffIsInThePicture)
{
    auto renderer = laidOut("\\staves 2 c''1 | c''1", baseStyle());
    ASSERT_TRUE(renderer);

    const std::vector<EngravingPreviewRenderer::Item> items = renderer->items();
    EXPECT_GE(count(items, ElementType::BRACKET), 1);

    const muse::SizeF size = renderer->sizeInStaffSpaces();
    for (const auto& item : items) {
        if (item.type == ElementType::BRACKET) {
            EXPECT_GE(item.box.left(), -0.01);
            EXPECT_LE(item.box.right(), size.width() + 0.01);
        }
    }
}

TEST_F(Notation_EngravingPreviewTests, overrideChangesTheLayout)
{
    const MStyle style = baseStyle();

    auto plain = laidOut(TWO_MEASURES, style);
    auto wide = laidOut(TWO_MEASURES, style, { { Sid::minMeasureWidth, Spatium(40) } });
    ASSERT_TRUE(plain);
    ASSERT_TRUE(wide);

    EXPECT_GT(wide->sizeInStaffSpaces().width(), plain->sizeInStaffSpaces().width());
    EXPECT_GE(wide->sizeInStaffSpaces().width(), 80.0);
}

/**
 * @TEST_CASE sizeDoesNotDependOnTheStaffSize
 * @details The size in staff spaces is the same whatever the size of the staff space is
 */
TEST_F(Notation_EngravingPreviewTests, sizeDoesNotDependOnTheStaffSize)
{
    const MStyle style = baseStyle();
    MStyle doubled = style;
    doubled.set(Sid::spatium, style.spatium() * 2.0);

    auto normal = laidOut(TWO_MEASURES, style);
    auto large = laidOut(TWO_MEASURES, doubled);
    ASSERT_TRUE(normal);
    ASSERT_TRUE(large);

    const muse::SizeF a = normal->sizeInStaffSpaces();
    const muse::SizeF b = large->sizeInStaffSpaces();
    EXPECT_NEAR(a.width(), b.width(), a.width() * 0.02);
    EXPECT_NEAR(a.height(), b.height(), a.height() * 0.02);
}

/**
 * @TEST_CASE givenSystemWidthIsNotExceeded
 * @details With a width given, the systems are not wider
 */
TEST_F(Notation_EngravingPreviewTests, givenSystemWidthIsNotExceeded)
{
    auto renderer = laidOut(TWO_MEASURES + " | c''1 | d''1", baseStyle(), {}, 60.0);
    ASSERT_TRUE(renderer);

    EXPECT_LE(renderer->sizeInStaffSpaces().width(), 60.5);
    EXPECT_GT(renderer->sizeInStaffSpaces().width(), 0.0);
}

/**
 * @TEST_CASE twoSystemsHaveRoomInThePane
 * @details The system after a break stands close under the one before, whatever distance the style gives the systems
 *          of a page, so that a picture of two systems has room in the pane (20 staff spaces high)
 */
TEST_F(Notation_EngravingPreviewTests, twoSystemsHaveRoomInThePane)
{
    MStyle style = baseStyle();
    style.set(Sid::minSystemDistance, Spatium(12.0));
    style.set(Sid::maxSystemDistance, Spatium(20.0));

    auto renderer = laidOut("c''1 \\break c''1", style, {}, 16.0);
    ASSERT_TRUE(renderer);

    EXPECT_GT(renderer->sizeInStaffSpaces().height(), 8.0 + 5.0);
    EXPECT_LE(renderer->sizeInStaffSpaces().height(), 20.0);
}

/**
 * @TEST_CASE onlyWhatIsDrawnIsInThePicture
 * @details A time signature across the staves is drawn on the first staff only: the one which the second staff
 *          carries for it is neither an item of the picture nor in its size
 */
TEST_F(Notation_EngravingPreviewTests, onlyWhatIsDrawnIsInThePicture)
{
    auto renderer = laidOut("\\instruments 2 \\time 3/4 c''2.", baseStyle(),
                            { { Sid::timeSigPlacement, PropertyValue(TimeSigPlacement::ACROSS_STAVES) } });
    ASSERT_TRUE(renderer);

    size_t timeSigs = 0;
    for (const EngravingPreviewRenderer::Item& item : renderer->items()) {
        if (item.type == ElementType::TIMESIG) {
            ++timeSigs;
            EXPECT_EQ(item.track, 0);
        }
    }

    EXPECT_EQ(timeSigs, 1u);
    EXPECT_LE(renderer->sizeInStaffSpaces().height(), 20.0);
}

/**
 * @TEST_CASE paintsInk
 * @details The picture of the laid out music has the size it says, and ink in it
 */
TEST_F(Notation_EngravingPreviewTests, paintsInk)
{
    auto renderer = laidOut(TWO_MEASURES, baseStyle());
    ASSERT_TRUE(renderer);

    constexpr double PIXELS_PER_STAFF_SPACE = 7.0;
    const muse::SizeF size = renderer->sizeInStaffSpaces() * PIXELS_PER_STAFF_SPACE;
    const QSize imageSize(static_cast<int>(std::ceil(size.width())), static_cast<int>(std::ceil(size.height())));
    ASSERT_GT(imageSize.width(), 0);
    ASSERT_GT(imageSize.height(), 0);

    const QImage image = renderer->picture(PIXELS_PER_STAFF_SPACE);
    ASSERT_EQ(image.size(), imageSize);

    size_t inkPixels = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(image.pixel(x, y)) == 255) {
                ++inkPixels;
            }
        }
    }

    EXPECT_GT(inkPixels, 100u);
}

/**
 * @TEST_CASE theMusicIsPaintedInsideItsPicture
 * @details The picture is painted from the box it is measured by, and the box has all that is printed: a layout
 *          break, which is not printed but stands above the first system, does not move the music down (the bottom
 *          of the second system was cut off), and the brace of a grand staff is in the picture
 */
TEST_F(Notation_EngravingPreviewTests, theMusicIsPaintedInsideItsPicture)
{
    constexpr double PIXELS_PER_STAFF_SPACE = 10.0;

    for (const char* music : { "c''1 \\cresc \\break d''2 e''2 \\!", "\\staves 2 c''1" }) {
        auto renderer = laidOut(music, baseStyle(), {}, 16.0);
        ASSERT_TRUE(renderer) << music;

        const QImage image = renderer->picture(PIXELS_PER_STAFF_SPACE);
        ASSERT_FALSE(image.isNull()) << music;

        QRect ink;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                if (qAlpha(image.pixel(x, y)) > 128) {
                    ink |= QRect(x, y, 1, 1);
                }
            }
        }

        //! NOTE The picture has a margin of half a staff space (5 pixels) around what is printed; boxes of text are a
        //! little taller than its ink
        EXPECT_GE(ink.left(), 2) << music;
        EXPECT_LE(ink.left(), 9) << music;
        EXPECT_GE(ink.top(), 2) << music;
        EXPECT_LE(ink.top(), 9) << music;
        EXPECT_GE(ink.right(), image.width() - 12) << music;
        EXPECT_LE(ink.right(), image.width() - 3) << music;
        EXPECT_GE(ink.bottom(), image.height() - 12) << music;
        EXPECT_LE(ink.bottom(), image.height() - 3) << music;
    }
}

/**
 * @TEST_CASE pictureMatchesTheItemsAtEveryScale
 * @details The regression of the glyph-size bug: the ink fills the boxes of the items at every scale and
 *          pixel ratio; the clef reaches above and below the staff, so a clef which a wrong DPI makes a
 *          different size of fails it
 */
TEST_F(Notation_EngravingPreviewTests, pictureMatchesTheItemsAtEveryScale)
{
    auto renderer = laidOut("b'1", baseStyle());
    ASSERT_TRUE(renderer);

    for (double pixelsPerStaffSpace : { 6.0, 10.0, 24.0 }) {
        for (double ratio : { 1.0, 2.0 }) {
            const QImage image = renderer->picture(pixelsPerStaffSpace, ratio);
            ASSERT_FALSE(image.isNull()) << pixelsPerStaffSpace << " @" << ratio;

            const double pixels = pixelsPerStaffSpace * ratio;

            QRectF expected;
            for (const EngravingPreviewRenderer::Item& item : renderer->items()) {
                const QRectF box(item.box.left() * pixels, item.box.top() * pixels,
                                 item.box.width() * pixels, item.box.height() * pixels);
                expected = expected.isNull() ? box : expected.united(box);
            }
            ASSERT_FALSE(expected.isNull());

            QRect ink;
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    if (qAlpha(image.pixel(x, y)) > 32) {
                        ink = ink.isNull() ? QRect(x, y, 1, 1) : ink.united(QRect(x, y, 1, 1));
                    }
                }
            }
            ASSERT_FALSE(ink.isNull());

            constexpr double WITHIN = 1.5;  // device pixels on each side
            EXPECT_NEAR(ink.left(), expected.left(), WITHIN) << pixelsPerStaffSpace << " @" << ratio;
            EXPECT_NEAR(ink.top(), expected.top(), WITHIN) << pixelsPerStaffSpace << " @" << ratio;
            EXPECT_NEAR(ink.right() + 1, expected.right(), WITHIN) << pixelsPerStaffSpace << " @" << ratio;
            EXPECT_NEAR(ink.bottom() + 1, expected.bottom(), WITHIN) << pixelsPerStaffSpace << " @" << ratio;
        }
    }
}

/**
 * @TEST_CASE spacingEffectHasAHorizontalHandle
 * @details A value which spreads the notes moves items sideways, and says how far for a unit of the value
 */
TEST_F(Notation_EngravingPreviewTests, spacingEffectHasAHorizontalHandle)
{
    const MStyle style = baseStyle();
    auto renderer = laidOut("c''8 d'' e'' f''", style);
    ASSERT_TRUE(renderer);

    const double current = style.styleS(Sid::minNoteDistance).val();
    const EngravingPreviewRenderer::Effect effect
        = renderer->effectOf(style, {}, Sid::minNoteDistance, PropertyValue(Spatium(current + 1.0)));

    EXPECT_FALSE(effect.isEmpty());
    ASSERT_TRUE(effect.hasHandle);
    EXPECT_DOUBLE_EQ(std::abs(effect.direction.x()), 1.0);
    EXPECT_DOUBLE_EQ(effect.direction.y(), 0.0);
    EXPECT_GT(effect.staffSpacesPerUnit, 0.2);
    EXPECT_LT(effect.staffSpacesPerUnit, 20.0);
}

/**
 * @TEST_CASE aScaleIsProbedInBothDimensions
 * @details The probe of a scale steps its width and its height alike, so that a scale whose two are kept the same
 *          stays so, and turns back below its maximum
 */
TEST_F(Notation_EngravingPreviewTests, aScaleIsProbedInBothDimensions)
{
    const MStyle style = baseStyle();
    const ScaleF current = style.styleV(Sid::timeSigNormalScale).value<ScaleF>();

    const std::optional<PropertyValue> up
        = EngravingPreviewRenderer::probeOf(style, {}, Sid::timeSigNormalScale, 10.0);
    ASSERT_TRUE(up);
    EXPECT_GT(up->value<ScaleF>().width(), current.width());
    EXPECT_DOUBLE_EQ(up->value<ScaleF>().width() - current.width(), up->value<ScaleF>().height() - current.height());

    const std::optional<PropertyValue> down
        = EngravingPreviewRenderer::probeOf(style, {}, Sid::timeSigNormalScale, current.width());
    ASSERT_TRUE(down);
    EXPECT_LT(down->value<ScaleF>().width(), current.width());
    EXPECT_LT(down->value<ScaleF>().height(), current.height());
}

/**
 * @TEST_CASE aPlainNumberIsProbedByAPartOfItself
 * @details A ratio of 30 changed by a quarter changes nothing which can be seen: a plain number is probed by half of
 *          itself, a small one still by a quarter, and a value in staff spaces by a quarter of one
 */
TEST_F(Notation_EngravingPreviewTests, aPlainNumberIsProbedByAPartOfItself)
{
    const MStyle style = baseStyle();

    const double ratio = style.styleD(Sid::parenthesisNoteHeightToThicknessRatio);
    const std::optional<PropertyValue> ratioProbe
        = EngravingPreviewRenderer::probeOf(style, {}, Sid::parenthesisNoteHeightToThicknessRatio, 1000.0);
    ASSERT_TRUE(ratioProbe);
    EXPECT_DOUBLE_EQ(ratioProbe->toReal(), ratio * 1.5);

    const std::optional<PropertyValue> smallProbe = EngravingPreviewRenderer::probeOf(
        style, { { Sid::parenthesisNoteHeightToThicknessRatio, PropertyValue(0.2) } },
        Sid::parenthesisNoteHeightToThicknessRatio, 1000.0);
    ASSERT_TRUE(smallProbe);
    EXPECT_DOUBLE_EQ(smallProbe->toReal(), 0.2 + 0.25);

    const Spatium distance = style.styleS(Sid::parenthesisNotePadding);
    const std::optional<PropertyValue> distanceProbe
        = EngravingPreviewRenderer::probeOf(style, {}, Sid::parenthesisNotePadding, 1000.0);
    ASSERT_TRUE(distanceProbe);
    EXPECT_DOUBLE_EQ(distanceProbe->value<Spatium>().val(), distance.val() + 0.25);
}

/**
 * @TEST_CASE effectOfATieValueIsTheTie
 * @details A value of the tie changes the tie and nothing else
 */
TEST_F(Notation_EngravingPreviewTests, effectOfATieValueIsTheTie)
{
    const MStyle style = baseStyle();
    auto renderer = laidOut("c''2~ c''2", style);
    ASSERT_TRUE(renderer);

    std::vector<muse::RectF> tieBoxes;
    for (const EngravingPreviewRenderer::Item& item : renderer->items()) {
        if (item.type == ElementType::TIE_SEGMENT) {
            tieBoxes.push_back(item.box);
        }
    }
    ASSERT_EQ(tieBoxes.size(), 1u);

    const double current = style.styleS(Sid::tieMidWidth).val();
    const EngravingPreviewRenderer::Effect effect
        = renderer->effectOf(style, {}, Sid::tieMidWidth, PropertyValue(Spatium(current + 0.25)));

    ASSERT_EQ(effect.changed.size(), 1u);
    EXPECT_EQ(effect.changed.front(), tieBoxes.front());
}

/**
 * @TEST_CASE effectOfAValueTheMusicDoesNotUseIsNothing
 * @details A value of lyrics changes nothing in music without lyrics
 */
TEST_F(Notation_EngravingPreviewTests, effectOfAValueTheMusicDoesNotUseIsNothing)
{
    const MStyle style = baseStyle();
    auto renderer = laidOut(TWO_MEASURES, style);
    ASSERT_TRUE(renderer);

    const double current = style.styleS(Sid::lyricsMinDistance).val();
    const EngravingPreviewRenderer::Effect effect
        = renderer->effectOf(style, {}, Sid::lyricsMinDistance, PropertyValue(Spatium(current + 1.0)));

    EXPECT_TRUE(effect.isEmpty());
    EXPECT_FALSE(effect.hasHandle);
}

/**
 * @TEST_CASE effectLeavesTheLayoutAsItWas
 * @details The picture after asking what a value does is the one of the real values
 */
TEST_F(Notation_EngravingPreviewTests, effectLeavesTheLayoutAsItWas)
{
    const MStyle style = baseStyle();
    auto renderer = laidOut(TWO_MEASURES, style);
    ASSERT_TRUE(renderer);

    const std::vector<EngravingPreviewRenderer::Item> before = renderer->items();
    const muse::SizeF sizeBefore = renderer->sizeInStaffSpaces();

    renderer->effectOf(style, {}, Sid::minMeasureWidth, PropertyValue(Spatium(40)));

    EXPECT_TRUE(same(before, renderer->items()));
    EXPECT_EQ(renderer->sizeInStaffSpaces(), sizeBefore);
}

namespace {
EngravingPreviewRenderer::Item itemAt(ElementType type, int track, int tick, double left, double top,
                                      double width = 1.0, double height = 1.0)
{
    EngravingPreviewRenderer::Item item;
    item.type = type;
    item.track = track;
    item.tick = tick;
    item.box = muse::RectF(left, top, width, height);
    return item;
}

std::vector<EngravingPreviewRenderer::Item> staffLinesAndNotes(int tick0)
{
    return {
        itemAt(ElementType::NOTE, 0, tick0, 0.0, 0.0),
        itemAt(ElementType::NOTE, 0, tick0 + 10, 4.0, 0.0),
        itemAt(ElementType::NOTE, 0, tick0 + 20, 8.0, 0.0),
        itemAt(ElementType::STAFF_LINES, -1, -1, 0.0, 0.0, 12.0, 4.0),
    };
}
}

/**
 * @TEST_CASE carriedAlongItemsAreNotMarked
 * @details Of a run of items which all moved by as much, only the first is marked; the ones which the
 *          change only carried along are not, and the staff lines are not
 */
TEST_F(Notation_EngravingPreviewTests, carriedAlongItemsAreNotMarked)
{
    const std::vector<EngravingPreviewRenderer::Item> before = staffLinesAndNotes(0);
    std::vector<EngravingPreviewRenderer::Item> after = staffLinesAndNotes(0);
    for (EngravingPreviewRenderer::Item& item : after) {
        if (item.type == ElementType::NOTE) {
            item.box.translate(2.0, 0.0);
        }
    }

    const EngravingPreviewRenderer::MarkedChanges marked
        = EngravingPreviewRenderer::markedChanges(before, after);

    ASSERT_EQ(marked.pairs.size(), 1u);
    EXPECT_DOUBLE_EQ(marked.pairs.front().first.box.left(), 0.0);
    EXPECT_TRUE(marked.boxes.empty());
}

/**
 * @TEST_CASE aVerticalMoveIsAlwaysMarked
 * @details An item which the change pushes down is marked however its neighbours moved
 */
TEST_F(Notation_EngravingPreviewTests, aVerticalMoveIsAlwaysMarked)
{
    const std::vector<EngravingPreviewRenderer::Item> before = staffLinesAndNotes(0);
    std::vector<EngravingPreviewRenderer::Item> after = before;
    after[1].box.translate(0.0, 1.0);

    const EngravingPreviewRenderer::MarkedChanges marked
        = EngravingPreviewRenderer::markedChanges(before, after);

    ASSERT_EQ(marked.pairs.size(), 1u);
    EXPECT_DOUBLE_EQ(marked.pairs.front().first.box.left(), 4.0);
}

/**
 * @TEST_CASE theFirstOfACarriedRunStaysMarked
 * @details An item whose neighbour to the left did not move with it is the one the change really moves
 */
TEST_F(Notation_EngravingPreviewTests, theFirstOfACarriedRunStaysMarked)
{
    const std::vector<EngravingPreviewRenderer::Item> before = staffLinesAndNotes(0);
    std::vector<EngravingPreviewRenderer::Item> after = before;
    after[1].box.translate(2.0, 0.0);
    after[2].box.translate(3.0, 0.0);    // a different dx: it is not carried by the first

    const EngravingPreviewRenderer::MarkedChanges marked
        = EngravingPreviewRenderer::markedChanges(before, after);

    ASSERT_EQ(marked.pairs.size(), 2u);
    EXPECT_DOUBLE_EQ(marked.pairs[0].first.box.left(), 4.0);
    EXPECT_DOUBLE_EQ(marked.pairs[1].first.box.left(), 8.0);
}

/**
 * @TEST_CASE staffLinesAreMarkedOnlyIfNothingElseIs
 * @details The staff lines are dropped when music is marked, kept when they alone changed
 */
TEST_F(Notation_EngravingPreviewTests, staffLinesAreMarkedOnlyIfNothingElseIs)
{
    std::vector<EngravingPreviewRenderer::Item> before = staffLinesAndNotes(0);
    std::vector<EngravingPreviewRenderer::Item> after = before;

    after[3].box.setHeight(4.5);    // the lines alone
    EngravingPreviewRenderer::MarkedChanges marked = EngravingPreviewRenderer::markedChanges(before, after);
    ASSERT_EQ(marked.pairs.size(), 1u);
    EXPECT_EQ(marked.pairs.front().first.type, ElementType::STAFF_LINES);

    after = before;
    after[1].box.translate(0.0, 1.0);
    after[3].box.setHeight(4.5);    // the lines too, but a note changed
    marked = EngravingPreviewRenderer::markedChanges(before, after);
    ASSERT_EQ(marked.pairs.size(), 1u);
    EXPECT_EQ(marked.pairs.front().first.type, ElementType::NOTE);
}

/**
 * @TEST_CASE onlyTheDotIsMarked
 * @details The distance of the dot marks the dot: the notes it pushes are carried along, the staff lines
 *          are not marked
 */
TEST_F(Notation_EngravingPreviewTests, onlyTheDotIsMarked)
{
    const MStyle style = baseStyle();
    auto renderer = laidOut("c''4. d''8 e''8", style);
    ASSERT_TRUE(renderer);

    const std::vector<EngravingPreviewRenderer::Item> before = renderer->items();
    const double current = style.styleS(Sid::dotNoteDistance).val();
    renderer->layout(style, { { Sid::dotNoteDistance, Spatium(current + 0.6) } });
    const std::vector<EngravingPreviewRenderer::Item> after = renderer->items();

    const EngravingPreviewRenderer::MarkedChanges marked
        = EngravingPreviewRenderer::markedChanges(before, after);

    EXPECT_TRUE(marked.boxes.empty());
    ASSERT_FALSE(marked.pairs.empty());
    for (const auto& pair : marked.pairs) {
        EXPECT_EQ(pair.first.type, ElementType::NOTEDOT);
    }
}

/**
 * @TEST_CASE theStaffLinesAreMarked
 * @details An option of the lines themselves marks them: nothing else differs. The bar lines share the
 *          width of the staff lines, and only their box has the width in it (the box of the staff lines
 *          is the staff, not the strokes)
 */
TEST_F(Notation_EngravingPreviewTests, theStaffLinesAreMarked)
{
    const MStyle style = baseStyle();
    auto renderer = laidOut(TWO_MEASURES, style);
    ASSERT_TRUE(renderer);

    const std::vector<EngravingPreviewRenderer::Item> before = renderer->items();
    renderer->layout(style, { { Sid::staffLineWidth, Spatium(style.styleS(Sid::staffLineWidth).val() * 3.0) } });
    const std::vector<EngravingPreviewRenderer::Item> after = renderer->items();

    const EngravingPreviewRenderer::MarkedChanges marked
        = EngravingPreviewRenderer::markedChanges(before, after);

    EXPECT_TRUE(marked.boxes.empty());
    ASSERT_FALSE(marked.pairs.empty());
    for (const auto& pair : marked.pairs) {
        EXPECT_TRUE(pair.first.type == ElementType::STAFF_LINES || pair.first.type == ElementType::BAR_LINE)
            << int(pair.first.type);
    }
}

namespace {
//! The pages whose every option is meant to show what it does
const char* const EFFECT_TEST_PAGES[] = {
    "ScorePage", "VerticalSpacingPage", "SpacingPage", "SizesPage", "SystemPage", "HeaderAndFooterPage",
    "MeasureNumberPage",
    "NotesPage", "AccidentalsPage", "StemsPage", "BeamsPage", "TremolosPage",
    "RestsPage", "TupletsPage", "ArpeggiosPage", "TiesPage", "SlursPage",
    "BarlinesPage", "RepeatsPage", "VoltasPage", "ClefsKeyAndTimeSignaturesPage", "ArticulationsAndOrnamentsPage",
    "FermatasPage", "DynamicsHairpinsPage", "FingeringPage", "ParenthesesPage", "LyricsPage",
    "LinesPage", "OttavaPage", "PedalPage", "TrillPage", "GlissandoNoteLinePage",
    "TextLinePage", "LetRingAndPalmMutePage", "BendPage",
    "HammerOnsPullOffsAndTappingPage", "StaffTextPage"
};

//! NOTE The options whose value nothing in the layout reads, each with the code fact which shows it
const std::map<QString, const char*>& optionsWhichAreDead()
{
    static const std::map<QString, const char*> OPTIONS {
        { "slurMinDistance",
          "only written when a pre-4.3 file is read (style.cpp:473); no layout code reads it" },
        { "tieMinDistance",
          "only written when a pre-4.3 file is read (style.cpp:472); no layout code reads it" },
    };

    return OPTIONS;
}

//! NOTE The options whose situation is not written yet. This list must only shrink: an option which starts
//! to show an effect fails the test as a stale entry, so take it off the list.
const std::set<QString>& optionsWhoseMusicCannotShowItYet()
{
    static const std::set<QString> OPTIONS {
        "accidentalColumnAlignThreshold",
        "accidentalDisplacementSplitThreshold",
        "accidentalDistance",
        "accidentalFlushToLedgerLineInset",
        "accidentalLargeGroupLimit",
        "accidentalNaturalKerningOfFourth",
        "accidentalNaturalSixthsShareColumn",
        "accidentalOffsetOctaveAlignMaxDistance",
        "accidentalSharpVerticalClearance",
        "accidentalSmallGroupLimit",
        "arpeggioAccidentalDistanceMin",
        "arpeggioHiddenInStdIfTab",     // needs a standard staff linked to a tablature staff
        "arpeggioLineWidth",
        // the custom-positioning deficit pass (beamtremololayout.cpp:1031) needs
        // useDefaultBeamSlantRules=false + beamCustomPositioningRules=true AND a group where an end
        // stem's deficit exceeds the allowance; every group tried had zero deficit
        "beamCustomEndStemMaxShortening",
        // the ledger-group branch of the same pass (beamtremololayout.cpp:1060): same switches, plus a
        // group of notes all on ledger lines whose deep-end deficit exceeds the tolerance
        "beamCustomLedgerGroupMaxShortening",
        // applyCustomLedgerGroupPosition (beamtremololayout.cpp:388): same switches, plus a ledger-line
        // group whose base lands off the line grid with a toward-notes snap shortening it past the bound
        "beamCustomLedgerGroupSnapShortening",
        // offsetBeamToRemoveCollisions (beamtremololayout.cpp:484): the reduction binds only when the
        // loop stops with the beam at an inner stem's anchor + reduction; for a slope above three quarter
        // spaces every group tried snapped the beam above all inner tips instead (the small-slant case is
        // shown by "\stemUp c'8[ e'8 f'8 f'8]")
        // two barlines side by side (around a measure with nothing in it) and a "partial" tie which hangs from a
        // barline (into a second volta): the music writes neither, and the ties across repeat barlines and into
        // voltas which it writes are laid out whole
        "barBarlineDistance",
        "barlineToLineStartDistance",
        // pairs which the layout never sets side by side: a clef change stands before the barline, key and time
        // signatures after it in the order clef, key, time (Segment types in segment.h), and a key or time
        // signature is followed by notes
        "clefClefDistance",
        "keyBarlineDistance",
        "keyClefDistance",
        "keyKeyDistance",
        "timesigBarlineDistance",
        "timesigClefDistance",
        "timesigTimesigDistance",
        // the music which shows these is not found yet: the note to key and time signature pairs (the dot, flag and
        // rest pairs show with the barline paddings at 0), the kerning clearances, the default padding, stem to stem,
        // equal duration spacing and the least start of a system
        "kerningVerticalClearance",
        "kerningVerticalClearanceAccidental",
        "kerningVerticalClearanceParenthesis",
        "keepEqualDurationSpacing",
        "noteKeyDistance",
        "noteTimesigDistance",
        "spacingDefaultPadding",
        "stemStemDistance",
        "systemHeaderMinStartOfSystemDistance",
        "beamInnerStemShorteningLargeSlant",
        "beamInnerStemShorteningMediumSlant",
        "bracketedAccidentalBraceYOffset",
        "combineVoice",
        "dotFlagVerticalClearance",
        "flagStemCutoutDown",
        "flagStemCutoutUp",
        "flagStemMaxHeight",
        "fullMeasureRestMinHorizontalClearance",
        "laissezVibParenthesisPadding",
        "minHangingTieLength",
        "minMMRestWidth",
        "mmRestNumberMaskHBar",
        "restChordMinClearanceWholeHalf",
        "slurArcClearanceMin",
        "slurCollisionBalanceConstrained",
        "slurCollisionBalanceFree",
        "slurCollisionMaxStep",
        "slurHangingEndHeight",
        "slurItemClearance",
        "slurShoulderLengthLong",
        "slurShoulderLengthMedium",
        "slurShoulderLengthShort",
        // avoidCollisions (slurtielayout.cpp:1621, 1639) applies the slant reduction only to a slur with a
        // collision in its middle third and none in its left third, rising more than the reduction angle, and
        // longer than this; the probe moves the limit by 0.25 sp, so the slur must also be 8 to 8.25 sp long:
        // none of 660 slurs tried (four to six notes, spacing in steps of 0.01) met all of it at once
        "slurSlantReductionMinLength",

        "slurTieShortLengthLimit",
        "stemMinInnerLengthNoBeam",
        "stemMinStaffOverlapFourBeamsWide",
        "stemSlashMultiFlagAngleFactor",
        "tieAdjacentTieInset",
        "tieArcStaffLineShortLength",
        "tieDotsPlacement",
        "tieMaxShoulderHeight",
        "tieOutsideInsetChord",
        // adjustTieForStaffLines (slurtielayout.cpp:3000-3017) treats a tie as small only on an inner note of a
        // chord with outside placement whose arc needs a staff-line correction, and the tie must also be 0.7 to
        // 0.95 sp high for the probe to matter: none of 233 ties tried (inner ties of five chords, spacing in steps
        // of 0.04, minimum tie height 0.5 to 1.0 sp) met both
        "tieSmallMaxHeight",
        "tremoloTwoNoteMinStemGap",
        "tupletBracketMinArmLengthRatio",
        "tupletExtendToEndOfDurationPadding",
        "tupletMaxSlope",
        "tupletNumberRhythmicCenterStemShift",
        // the measure number and the measure number range are moved by their minimum distance only where the
        // skyline under them comes nearer (Autoplace::autoplaceMeasureElement, autoplace.cpp:129): no tried
        // measure (high notes under left and centred numbers, a high multimeasure rest number) brought them together
        "measureNumberMinDistance",
        "mmRestRangeMinDistance",
        // playback only: the swing of the score is read by Staff::swing (staff.cpp:1010) for playing, by nothing in
        // the layout
        "swingRatio",
        "swingUnit",
        // playback only: which notes a glissando plays (Glissando::pitchSteps, spannersmetaparser.cpp)
        "glissandoStyle",
        "glissandoStyleHarp",
        // concert pitch shows only on a transposing instrument, and the music is always written for a piano
        "concertPitch",
        // only where the bottom staff of the score is one of the staves chosen to show system markings (the
        // preview's score has no such staves)
        "systemObjectsBelowBottomStaff",
        // a frame below a system: the music writes a frame only before the first measure (`\title`)
        "paddingToNotationAbove",
        "systemFrameDistance",
        // the gap between a bracketed (braced) group and a staff outside it in the same system (VerticalGapData::
        // addSpaceAroundNormalBracket): the music has two staves at most, so the group is the whole system, and
        // between systems the larger factor for systems wins
        "spreadCurlyBracket",
        "spreadSquareBracket",
        // a jump or marker shown on a staff below the top one (a staff chosen to show system markings): the music
        // has no such staves
        "repeatTextBarlinePadding",
        // the text of a play count keeps clear of the notes under it, but no music tried (high notes up to the end
        // repeat barline, spacing 0.8 to 1.2) moved it by this
        "repeatPlayCountMinDistance",
        // the least distance between systems decides how many systems fit on a page, so it shows only on a page
        // which is full to within a step of it: none of 228 pages tried (7 to 10 systems, music top margin in steps
        // of 0.25 sp) was
        "minSystemDistance",
        // a note's parentheses are spaced against its ledger lines (ParenthesisLayout::getParentShape adds them),
        // but the parenthesis does not move however large this is: an engine finding, not fixed yet
        "parenthesisLedgerLinePadding",
        // the number of dashes changes only where a gap passes a multiple of this (16 sp): a probe of a quarter of a
        // staff space shows it only on a gap within a quarter of 16 or 32 sp, which any change of the spacing would
        // move; dragging it in the pane over the long gap of its music shows it
        "lyricsDashMaxDistance",
    };

    return OPTIONS;
}

//! NOTE The options whose music cannot fit 44 × 20 staff spaces, each with the reason. The systems of a picture stand
//! 5 staff spaces apart, but two systems of a grand staff are still about 37 staff spaces high. This list must only
//! shrink: an option whose music fits fails the test as a stale entry
const std::map<QString, const char*>& optionsWhichMusicCannotFitThePane()
{
    static const std::map<QString, const char*> OPTIONS {
        { "dividerLeft",                    "two systems of a grand staff after a \\break" },
        { "dividerLeftSym",                 "two systems of a grand staff after a \\break" },
        { "dividerLeftX",                   "two systems of a grand staff after a \\break" },
        { "dividerLeftY",                   "two systems of a grand staff after a \\break" },
        { "dividerRight",                   "two systems of a grand staff after a \\break" },
        { "dividerRightSym",                "two systems of a grand staff after a \\break" },
        { "dividerRightX",                  "two systems of a grand staff after a \\break" },
        { "dividerRightY",                  "two systems of a grand staff after a \\break" },
        { "timeSigVSMarginCentered",        "two systems after a \\break, each with a time signature above" },
        { "timeSigVSMarginNonCentered",     "two systems after a \\break, each with a time signature above" },
    };

    return OPTIONS;
}

//! The style values which the music of an option sets: those of the page, the section and the option,
//! the nearer one wins
std::vector<EngravingPreviewRenderer::Override> musicStyleOf(const SourceBlock& option, const SourceBlock* grid,
                                                             const SourceBlock& section, const SourceBlock& page)
{
    std::map<QString, QString> merged;
    for (const SourceBlock* block : { &page, &section, grid, &option }) {
        if (!block) {
            continue;
        }
        for (const auto& [name, value] : block->musicStyle()) {
            merged[name] = value;
        }
    }

    std::vector<EngravingPreviewRenderer::Override> overrides;
    for (const auto& [name, text] : merged) {
        const StyleId id = MStyle::styleIdx(muse::String::fromQString(name));
        EXPECT_NE(id, StyleId::NOSTYLE) << name.toStdString();

        const QVariant plain = text == "true" || text == "false" ? QVariant(text == "true")
                               : text.startsWith('"') ? QVariant(SourceBlock::unquoted(text))
                               : text.contains('.') ? QVariant(text.toDouble()) : QVariant(text.toInt());
        const PropertyValue value = styleValueFromUi(id, plain);

        overrides.push_back({ id, value });
    }

    return overrides;
}

//! The upper bound the pane gives the probe and the handle: `max`, in engine units (percents are hundredths)
//! The value a choice option is probed with, as the pane does: the choice after the current one, wrapping
std::optional<PropertyValue> choiceProbeOf(const SourceBlock& option, const MStyle& style,
                                           const std::vector<EngravingPreviewRenderer::Override>& overrides, StyleId id)
{
    const QStringList values = option.choiceValues();
    if (values.size() < 2) {
        return std::nullopt;
    }

    //! NOTE A value is the current one if it is written so, or if both are numbers and equal: a word as `true` is no
    //! number (its toDouble() would be 0, as false is)
    const QVariant current = styleValueToUi(id, EngravingPreviewRenderer::valueOf(style, overrides, id));
    int index = 0;
    for (int i = 0; i < values.size(); ++i) {
        bool isNumber = false;
        const double number = values[i].toDouble(&isNumber);
        if (values[i] == current.toString() || (isNumber && number == current.toDouble())) {
            index = i;
            break;
        }
    }

    const QString text = values[(index + 1) % values.size()];
    const QVariant plain = text == "true" || text == "false" ? QVariant(text == "true")
                           : QVariant(text).toString().contains('"') ? QVariant(SourceBlock::unquoted(text))
                           : text.contains('.') ? QVariant(text.toDouble()) : QVariant(text.toInt());
    return styleValueFromUi(id, plain);
}

//! The value a text option is shown with: its text with a sample added, as EoMusic.probeValueOf has it
PropertyValue textProbeOf(const MStyle& style, const std::vector<EngravingPreviewRenderer::Override>& overrides,
                          StyleId id)
{
    static const QString TEXT_PROBE_SAMPLE = "Text";
    const QVariant current = styleValueToUi(id, EngravingPreviewRenderer::valueOf(style, overrides, id));
    return styleValueFromUi(id, current.toString() + TEXT_PROBE_SAMPLE);
}
}

/**
 * @TEST_CASE previewShowsWhatTheOptionDoes
 * @details For every option of the pages of notes and rests whose music is a plain text, changing the value
 *          by the visible step the preview takes changes the picture
 */
TEST_F(Notation_EngravingPreviewTests, previewShowsWhatTheOptionDoes)
{
    const MStyle style = baseStyle();
    const QString pagesDir = QString::fromUtf8(ENGRAVING_OPTIONS_QML_DIR) + "/pages/";

    //! NOTE EO_EFFECT_ONLY=<sid> checks that option only: a quick loop while its music is being written
    const QString only = qEnvironmentVariable("EO_EFFECT_ONLY");

    size_t checked = 0;
    for (const char* name : EFFECT_TEST_PAGES) {
        const SourceBlock page = readPageSource(pagesDir + name + ".qml");
        ASSERT_EQ(page.type, QString("EoPage")) << name;

        for (const SourceBlock& section : page.children) {
            for (const auto& entry : optionsOf(section)) {
                const SourceBlock& option = *entry.first;
                const SourceBlock* grid = entry.second;
                const QString sid = option.sid();
                const auto [music, systemWidth, showsPage] = musicInfoOf(option, grid, section, page);
                if (sid.isEmpty() || music.isEmpty() || (!only.isEmpty() && sid != only)) {
                    continue;
                }

                const StyleId id = MStyle::styleIdx(muse::String::fromQString(sid));
                const std::vector<EngravingPreviewRenderer::Override> overrides
                    = musicStyleOf(option, grid, section, page);
                const std::optional<PropertyValue> probe
                    = option.type == "EoChoice" ? choiceProbeOf(option, style, overrides, id)
                      : option.type == "EoText" ? std::optional<PropertyValue>(textProbeOf(style, overrides, id))
                      : EngravingPreviewRenderer::probeOf(style, overrides, id, dragMaximumOf(option));
                if (!probe) {
                    continue;
                }

                auto renderer = laidOut(music, style, overrides, systemWidth, showsPage);
                if (!renderer) {
                    ADD_FAILURE() << name << ": the music of " << sid.toStdString() << " is not valid: "
                                  << music.toStdString();
                    continue;
                }

                ++checked;
                const bool dead = optionsWhichAreDead().count(sid);
                const bool notYet = optionsWhoseMusicCannotShowItYet().count(sid);
                if (dead || notYet) {
                    EXPECT_TRUE(renderer->effectOf(style, overrides, id, *probe, systemWidth).isEmpty())
                        << name << ": " << sid.toStdString() << " is on the allow-list but shows an effect"
                        << (dead ? " — its dead-option reason is stale" : " — take it off the not-yet list");
                    continue;
                }

                EXPECT_FALSE(renderer->effectOf(style, overrides, id, *probe, systemWidth).isEmpty())
                    << name << ": the music of " << sid.toStdString() << " does not show it: " << music.toStdString();
            }
        }
    }

    EXPECT_GT(checked, 0u);
}

/**
 * @TEST_CASE theMusicOfEveryOptionFitsThePane
 * @details For every option with a sid on a page with music, the picture its music lays out — with its
 *          musicStyle and the first value of its contexts — fits the pane: at most 44 staff spaces wide
 *          and 20 high; an option on the list of those which cannot fit does not
 */
TEST_F(Notation_EngravingPreviewTests, theMusicOfEveryOptionFitsThePane)
{
    const MStyle style = baseStyle();
    const QString pagesDir = QString::fromUtf8(ENGRAVING_OPTIONS_QML_DIR) + "/pages/";

    size_t checked = 0;
    for (const char* name : EFFECT_TEST_PAGES) {
        const SourceBlock page = readPageSource(pagesDir + name + ".qml");
        ASSERT_EQ(page.type, QString("EoPage")) << name;

        for (const SourceBlock& section : page.children) {
            for (const auto& entry : optionsOf(section)) {
                const SourceBlock& option = *entry.first;
                const SourceBlock* grid = entry.second;
                const QString sid = option.sid();
                const auto [music, systemWidth, showsPage] = musicInfoOf(option, grid, section, page);
                //! NOTE A whole page is fitted to the pane, whatever its size
                if (sid.isEmpty() || music.isEmpty() || showsPage) {
                    continue;
                }

                auto renderer = laidOut(music, style, musicStyleOf(option, grid, section, page), systemWidth);
                if (!renderer) {
                    ADD_FAILURE() << name << ": the music of " << sid.toStdString() << " is not valid: "
                                  << music.toStdString();
                    continue;
                }

                ++checked;
                const muse::SizeF size = renderer->sizeInStaffSpaces();
                if (optionsWhichMusicCannotFitThePane().count(sid)) {
                    EXPECT_TRUE(size.width() > 44.0 + 0.01 || size.height() > 20.0 + 0.01)
                        << name << ": " << sid.toStdString() << " fits the pane — take it off the list of the "
                        << "options whose music cannot fit";
                    continue;
                }

                EXPECT_LE(size.width(), 44.0 + 0.01) << name << ": " << sid.toStdString()
                                                     << " is " << size.width() << " staff spaces wide: "
                                                        << music.toStdString();
                EXPECT_LE(size.height(), 20.0 + 0.01) << name << ": " << sid.toStdString()
                                                      << " is " << size.height() << " staff spaces high: "
                                                         << music.toStdString();
            }
        }
    }

    EXPECT_GT(checked, 0u);
}

//! Whether every box a change marks is the box of an item of the type — nothing else is marked (as it is
//! when the pictures of the two layouts have a different origin)
bool marksOnly(const EngravingPreviewRenderer& renderer, const EngravingPreviewRenderer::Effect& effect,
               ElementType type)
{
    const std::vector<EngravingPreviewRenderer::Item> items = renderer.items();
    for (const muse::RectF& box : effect.changed) {
        const bool found = std::any_of(items.begin(), items.end(),
                                       [&box, type](const EngravingPreviewRenderer::Item& item) {
            return item.box == box && item.type == type;
        });
        if (!found) {
            return false;
        }
    }

    return true;
}

/**
 * @TEST_CASE theEdgesOfAPictureDoNotMarkItems
 * @details A change which moves the top edge of the picture (the slur's width moves the top of the slur,
 *          hence the top of the page) marks only the slur — not the clef, the stems or the barline, as it
 *          did when the items of the two layouts were compared in their own picture frames
 */
TEST_F(Notation_EngravingPreviewTests, theEdgesOfAPictureDoNotMarkItems)
{
    const MStyle style = baseStyle();
    auto renderer = laidOut("c''4( d'' e'' f'')", style);
    ASSERT_TRUE(renderer);

    const double current = style.styleS(Sid::slurEndWidth).val();
    const EngravingPreviewRenderer::Effect effect = renderer->effectOf(style, {}, Sid::slurEndWidth,
                                                                       PropertyValue(Spatium(current + 0.2)));

    ASSERT_FALSE(effect.changed.empty());
    EXPECT_TRUE(marksOnly(*renderer, effect, ElementType::SLUR_SEGMENT))
        << "an item is marked which a wider slur cannot move";
}

/**
 * @TEST_CASE aNumberAboveTheStaffMarksOnlyItself
 * @details Moving the number of a multi-measure rest moves the top edge of the picture; only the rest is
 *          marked — the number is drawn inside it — and its handle is on the moved edge, where the
 *          number is (the other items did not move on the page)
 */
TEST_F(Notation_EngravingPreviewTests, aNumberAboveTheStaffMarksOnlyItself)
{
    const MStyle style = baseStyle();

    //! NOTE the H-bar is not broken around the number, so only the number moves
    const std::vector<EngravingPreviewRenderer::Override> overrides {
        { Sid::createMultiMeasureRests, true },
        { Sid::mmRestNumberMaskHBar, false },
    };
    auto renderer = laidOut("R1 | R1 | R1 | R1 | c''1", style, overrides);
    ASSERT_TRUE(renderer);

    const double current = style.styleS(Sid::mmRestNumberPos).val();
    const EngravingPreviewRenderer::Effect effect = renderer->effectOf(style, overrides, Sid::mmRestNumberPos,
                                                                       PropertyValue(Spatium(current - 1.0)));

    ASSERT_FALSE(effect.changed.empty());
    EXPECT_TRUE(marksOnly(*renderer, effect, ElementType::MMREST))
        << "an item is marked which the position of the number cannot move";

    ASSERT_TRUE(effect.hasHandle);
    const muse::PointF anchor = effect.anchor;
    for (const EngravingPreviewRenderer::Item& item : renderer->items()) {
        if (item.type == ElementType::MMREST && item.box.contains(anchor)) {
            EXPECT_LE(anchor.y(), item.box.top() + 1.0) << "the handle is not at the number's edge";
            SUCCEED();
            return;
        }
    }

    FAIL() << "the handle is not on the rest";
}

/**
 * @TEST_CASE aSignHighAboveTheStaffMovesAlone
 * @details A time signature above the staff reaches higher than the first system usually stands below the top of
 *          the page; when its position changes, the system stays where it is, and only the time signature is marked
 */
TEST_F(Notation_EngravingPreviewTests, aSignHighAboveTheStaffMovesAlone)
{
    const MStyle style = baseStyle();
    const std::vector<EngravingPreviewRenderer::Override> overrides {
        { Sid::timeSigPlacement, PropertyValue(TimeSigPlacement::ABOVE_STAVES) },
    };
    auto renderer = laidOut("\\time 3/4 c''2.", style, overrides);
    ASSERT_TRUE(renderer);

    const double current = style.styleS(Sid::timeSigAboveY).val();
    const EngravingPreviewRenderer::Effect effect = renderer->effectOf(style, overrides, Sid::timeSigAboveY,
                                                                       PropertyValue(Spatium(current + 0.25)));

    ASSERT_FALSE(effect.changed.empty());
    EXPECT_TRUE(marksOnly(*renderer, effect, ElementType::TIMESIG))
        << "an item is marked which the position of the time signature cannot move";
}

/**
 * @TEST_CASE aMusicWithABreakHasASystemWidth
 * @details A music which ends a system is laid out with a width in staff spaces, or the break makes the
 *          system as wide as the page can be — unless it is shown on a whole page, whose systems are as wide as it
 */
TEST_F(Notation_EngravingPreviewTests, aMusicWithABreakHasASystemWidth)
{
    const QString pagesDir = QString::fromUtf8(ENGRAVING_OPTIONS_QML_DIR) + "/pages/";

    for (const char* name : EFFECT_TEST_PAGES) {
        const SourceBlock page = readPageSource(pagesDir + name + ".qml");

        for (const SourceBlock& section : page.children) {
            for (const auto& entry : optionsOf(section)) {
                const SourceBlock& option = *entry.first;
                const auto [music, systemWidth, showsPage] = musicInfoOf(option, entry.second, section, page);
                //! NOTE The systems of a page are as wide as the page
                if (music.contains("\\break") && !showsPage) {
                    EXPECT_GT(systemWidth, 0.0) << name << ": " << option.sid().toStdString()
                                                << " has a \\break but no systemWidth";
                }
            }
        }
    }
}
