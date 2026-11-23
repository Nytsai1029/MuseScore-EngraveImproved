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

    //! A renderer which has the music, and has laid it out
    static std::unique_ptr<EngravingPreviewRenderer> laidOut(const QString& music, const MStyle& style,
                                                             const std::vector<EngravingPreviewRenderer::Override>&
                                                             overrides = {},
                                                             double systemWidthSp = 0.0)
    {
        auto renderer = std::make_unique<EngravingPreviewRenderer>(nullptr);
        const muse::Ret ret = renderer->setMusic(music);
        EXPECT_TRUE(ret) << music.toStdString() << ": " << ret.text();
        if (!ret) {
            return nullptr;
        }

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
    "SystemPage",
    "NotesPage", "AccidentalsPage", "StemsPage", "BeamsPage", "TremolosPage",
    "RestsPage", "TupletsPage", "ArpeggiosPage", "TiesPage", "SlursPage"
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
        "mmRestReferenceWidth",
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

        "slurTieShortLengthLimit",
        "stemMinInnerLengthNoBeam",
        "stemMinStaffOverlapFourBeamsWide",
        "stemSlashMultiFlagAngleFactor",
        "tieAdjacentTieInset",
        "tieArcStaffLineShortLength",
        "tieDotsPlacement",
        "tieMaxShoulderHeight",
        "tieOutsideInsetChord",
        "tremoloTwoNoteMinStemGap",
        "tupletBracketMinArmLengthRatio",
        "tupletExtendToEndOfDurationPadding",
        "tupletMaxSlope",
        "tupletNumberRhythmicCenterStemShift",
    };

    return OPTIONS;
}

//! NOTE The options whose music cannot fit 44 × 20 staff spaces, each with the reason. Two systems
//! after a `\break` are spread over the height of the page, so they stand about 28 staff spaces high
const std::map<QString, const char*>& optionsWhichMusicCannotFitThePane()
{
    static const std::map<QString, const char*> OPTIONS {
        { "angleHangingSlursAwayFromStaff", "two systems after a \\break" },
        { "dividerLeft",                    "two systems after a \\break" },
        { "dividerLeftSym",                 "two systems after a \\break" },
        { "dividerLeftX",                   "two systems after a \\break" },
        { "dividerLeftY",                   "two systems after a \\break" },
        { "dividerRight",                   "two systems after a \\break" },
        { "dividerRightSym",                "two systems after a \\break" },
        { "dividerRightX",                  "two systems after a \\break" },
        { "dividerRightY",                  "two systems after a \\break" },
        { "slurContinuedEndOffset",         "two systems after a \\break" },
        { "slurContinuedMaxHeightDiff",     "two systems after a \\break" },
        { "slurContinuedSingleChordRise",   "two systems after a \\break" },
        { "slurHangingEndHeight",           "two systems after a \\break" },
        { "slurTieExtendToSystemEndBarline", "two systems after a \\break" },
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

    const QVariant current = styleValueToUi(id, EngravingPreviewRenderer::valueOf(style, overrides, id));
    int index = 0;
    for (int i = 0; i < values.size(); ++i) {
        if (values[i] == current.toString() || values[i].toDouble() == current.toDouble()) {
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

    size_t checked = 0;
    for (const char* name : EFFECT_TEST_PAGES) {
        const SourceBlock page = readPageSource(pagesDir + name + ".qml");
        ASSERT_EQ(page.type, QString("EoPage")) << name;

        for (const SourceBlock& section : page.children) {
            for (const auto& entry : optionsOf(section)) {
                const SourceBlock& option = *entry.first;
                const SourceBlock* grid = entry.second;
                const QString sid = option.sid();
                const auto [music, systemWidth] = musicInfoOf(option, grid, section, page);
                if (sid.isEmpty() || music.isEmpty()) {
                    continue;
                }

                const StyleId id = MStyle::styleIdx(muse::String::fromQString(sid));
                const std::vector<EngravingPreviewRenderer::Override> overrides
                    = musicStyleOf(option, grid, section, page);
                const std::optional<PropertyValue> probe = option.type == "EoChoice"
                                                           ? choiceProbeOf(option, style, overrides, id)
                                                           : EngravingPreviewRenderer::probeOf(style, overrides, id,
                                                                                             dragMaximumOf(option));
                if (!probe) {
                    continue;
                }

                auto renderer = laidOut(music, style, overrides);
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
 *          and 20 high
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
                const auto [music, systemWidth] = musicInfoOf(option, grid, section, page);
                if (sid.isEmpty() || music.isEmpty()) {
                    continue;
                }
                if (optionsWhichMusicCannotFitThePane().count(sid)) {
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
 * @TEST_CASE aMusicWithABreakHasASystemWidth
 * @details A music which ends a system is laid out with a width in staff spaces, or the break makes the
 *          system as wide as the page can be
 */
TEST_F(Notation_EngravingPreviewTests, aMusicWithABreakHasASystemWidth)
{
    const QString pagesDir = QString::fromUtf8(ENGRAVING_OPTIONS_QML_DIR) + "/pages/";

    for (const char* name : EFFECT_TEST_PAGES) {
        const SourceBlock page = readPageSource(pagesDir + name + ".qml");

        for (const SourceBlock& section : page.children) {
            for (const auto& entry : optionsOf(section)) {
                const SourceBlock& option = *entry.first;
                const auto [music, systemWidth] = musicInfoOf(option, entry.second, section, page);
                if (music.contains("\\break")) {
                    EXPECT_GT(systemWidth, 0.0) << name << ": " << option.sid().toStdString()
                                                << " has a \\break but no systemWidth";
                }
            }
        }
    }
}
