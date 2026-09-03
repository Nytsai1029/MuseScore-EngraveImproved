/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#if (defined (_MSCVER) || defined (_MSC_VER))
#pragma warning(disable: 4459) // _t hides global declaration
#endif

#include <algorithm>
#include <limits>

#include <gtest/gtest.h>

#include "draw/fontmetrics.h"

#include "dom/chordrest.h"
#include "dom/dynamic.h"
#include "dom/factory.h"
#include "dom/lyrics.h"
#include "dom/masterscore.h"
#include "dom/page.h"
#include "dom/segment.h"
#include "dom/stafftext.h"
#include "dom/textedit.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String TEXTBASE_DATA_DIR("textbase_data/");

class Engraving_TextBaseTests : public ::testing::Test
{
public:
    Dynamic* addDynamic(MasterScore* score);
    StaffText* addStaffText(MasterScore* score);
};

Dynamic* Engraving_TextBaseTests::addDynamic(MasterScore* score)
{
    Dynamic* dynamic = new Dynamic(score->dummy()->segment());
    dynamic->setXmlText("<sym>dynamicForte</sym>");
    ChordRest* chordRest = score->firstSegment(SegmentType::ChordRest)->nextChordRest(0);
    EditData ed;
    ed.dropElement = dynamic;
    chordRest->drop(ed);
    return dynamic;
}

TEST_F(Engraving_TextBaseTests, createDynamic)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    addDynamic(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"createDynamic.mscx", TEXTBASE_DATA_DIR + u"createDynamic-ref.mscx"));
}

TEST_F(Engraving_TextBaseTests, dynamicAddTextBefore)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    Dynamic* dynamic = addDynamic(score);
    EditData ed;
    dynamic->startEdit(ed);
    score->startCmd(TranslatableString::untranslatable("Edit dynamic text (test)"));
    score->undo(new InsertText(dynamic->cursor(), String(u"poco ")), &ed);
    score->endCmd();
    dynamic->endEdit(ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"dynamicAddTextBefore.mscx", TEXTBASE_DATA_DIR + u"dynamicAddTextBefore-ref.mscx"));
}

TEST_F(Engraving_TextBaseTests, dynamicAddTextAfter)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    Dynamic* dynamic = addDynamic(score);
    EditData ed;
    ed.s = String(u" ma non troppo");
    dynamic->startEdit(ed);
    score->startCmd(TranslatableString::untranslatable("Edit dynamic text (test)"));
    dynamic->cursor()->moveCursorToEnd();
    dynamic->edit(ed);
    score->endCmd();
    dynamic->endEdit(ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"dynamicAddTextAfter.mscx", TEXTBASE_DATA_DIR + u"dynamicAddTextAfter-ref.mscx"));
}

TEST_F(Engraving_TextBaseTests, dynamicAddTextNoItalic)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    Dynamic* dynamic = addDynamic(score);
    EditData ed;
    dynamic->startEdit(ed);
    score->startCmd(TranslatableString::untranslatable("Edit dynamic text (test)"));
    dynamic->setProperty(Pid::FONT_STYLE, PropertyValue::fromValue(0));
    score->undo(new InsertText(dynamic->cursor(), String(u"moderately ")), &ed);
    score->endCmd();
    dynamic->endEdit(ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"dynamicAddTextNoItalic.mscx", TEXTBASE_DATA_DIR + u"dynamicAddTextNoItalic-ref.mscx"));
}

StaffText* Engraving_TextBaseTests::addStaffText(MasterScore* score)
{
    StaffText* staffText = new StaffText(score->dummy()->segment());
    ChordRest* chordRest = score->firstSegment(SegmentType::ChordRest)->nextChordRest(0);
    EditData ed;
    ed.dropElement = staffText;
    chordRest->drop(ed);
    return staffText;
}

TEST_F(Engraving_TextBaseTests, getFontStyleProperty)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    StaffText* staffText = addStaffText(score);
    EditData ed;
    staffText->startEdit(ed);
    score->undo(new InsertText(staffText->cursor(), String(u"normal ")), &ed);
    staffText->setProperty(Pid::FONT_STYLE, PropertyValue::fromValue(static_cast<int>(FontStyle::Bold)));
    score->undo(new InsertText(staffText->cursor(), String(u"bold")), &ed);
    staffText->cursor()->moveCursorToStart();
    EXPECT_EQ(staffText->getProperty(Pid::FONT_STYLE), PropertyValue::fromValue(0));
    staffText->cursor()->movePosition(TextCursor::MoveOperation::NextWord, TextCursor::MoveMode::KeepAnchor);
    EXPECT_EQ(staffText->getProperty(Pid::FONT_STYLE), PropertyValue::fromValue(0));
    staffText->cursor()->movePosition(TextCursor::MoveOperation::End, TextCursor::MoveMode::KeepAnchor);
    EXPECT_EQ(staffText->getProperty(Pid::FONT_STYLE), PropertyValue::fromValue(0));
    staffText->cursor()->movePosition(TextCursor::MoveOperation::WordLeft, TextCursor::MoveMode::MoveAnchor);
    EXPECT_EQ(staffText->getProperty(Pid::FONT_STYLE), PropertyValue::fromValue(static_cast<int>(FontStyle::Normal)));
    staffText->cursor()->movePosition(TextCursor::MoveOperation::NextWord, TextCursor::MoveMode::KeepAnchor);
    EXPECT_EQ(staffText->getProperty(Pid::FONT_STYLE), PropertyValue::fromValue(static_cast<int>(FontStyle::Bold)));
    staffText->endEdit(ed);
    EXPECT_EQ(staffText->getProperty(Pid::FONT_STYLE), PropertyValue::fromValue(0));
}

TEST_F(Engraving_TextBaseTests, undoChangeFontStyleProperty)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    StaffText* staffText = addStaffText(score);
    staffText->setXmlText(u"normal <b>bold</b> <u>underline</u> <i>italic</i>");
    score->renderer()->layoutItem(staffText);
    score->startCmd(TranslatableString::untranslatable("Engraving text base tests"));
    staffText->undoChangeProperty(Pid::FONT_STYLE, PropertyValue::fromValue(0), PropertyFlags::UNSTYLED);
    score->endCmd();
    EXPECT_EQ(staffText->xmlText(), u"normal <b>bold</b> <u>underline</u> <i>italic</i>");
    score->startCmd(TranslatableString::untranslatable("Engraving text base tests"));
    staffText->undoChangeProperty(Pid::FONT_STYLE, PropertyValue::fromValue(static_cast<int>(FontStyle::Bold)),
                                  PropertyFlags::UNSTYLED);
    score->endCmd();
    EXPECT_EQ(staffText->xmlText(), u"<b>normal bold <u>underline</u> <i>italic</i></b>");
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_EQ(staffText->xmlText(), u"normal <b>bold</b> <u>underline</u> <i>italic</i>");
    score->undoStack()->redo(&ed);
    EXPECT_EQ(staffText->xmlText(), u"<b>normal bold <u>underline</u> <i>italic</i></b>");
    score->startCmd(TranslatableString::untranslatable("Engraving text base tests"));
    staffText->undoChangeProperty(Pid::FONT_STYLE, PropertyValue::fromValue(
                                      static_cast<int>(FontStyle::Italic + FontStyle::Bold)), PropertyFlags::UNSTYLED);
    score->endCmd();
    EXPECT_EQ(staffText->xmlText(), u"<b><i>normal bold <u>underline</u> italic</i></b>");
    score->startCmd(TranslatableString::untranslatable("Engraving text base tests"));
    staffText->undoChangeProperty(Pid::FONT_STYLE,
                                  PropertyValue::fromValue(
                                      static_cast<int>(FontStyle::Italic + FontStyle::Bold + FontStyle::Underline)),
                                  PropertyFlags::UNSTYLED);
    score->endCmd();
    EXPECT_EQ(staffText->xmlText(), u"<b><i><u>normal bold underline italic</u></i></b>");
    score->startCmd(TranslatableString::untranslatable("Engraving text base tests"));
    staffText->undoChangeProperty(Pid::FONT_STYLE, PropertyValue::fromValue(0), PropertyFlags::UNSTYLED);
    score->endCmd();
    EXPECT_EQ(staffText->xmlText(), u"normal bold underline italic");
}

TEST_F(Engraving_TextBaseTests, undoChangeFontSizeEmptyLines)   // Testcase for Issue #19571
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    StaffText* staffText = addStaffText(score);

    String originalText = u"<font size=\"25\"/>Line 1\nLine 2\n\n\nLine 5"; // Two lines of text, two empty lines, one line of text.
    String expectedText = u"<font size=\"30\"/>Line 1\nLine 2\n\n\nLine 5"; // The font size should change for all lines, including the empty ones.

    staffText->setXmlText(originalText);
    LOGD("Text is initially: %s", muPrintable(staffText->xmlText().replace(u"\n", u"\\n")));
    score->renderer()->layoutItem(staffText);

    score->startCmd(TranslatableString::untranslatable("Engraving text base tests"));
    staffText->undoChangeProperty(Pid::FONT_SIZE, PropertyValue::fromValue(30.0), PropertyFlags::UNSTYLED); // Change font size from 25 to 30
    score->endCmd();

    LOGD(" 1.: Text is now : %s", muPrintable(staffText->xmlText().replace(u"\n", u"\\n")));
    EXPECT_EQ(staffText->xmlText(), expectedText); // The font size should change for all lines, including the empty ones.

    EditData ed;
    score->undoStack()->undo(&ed);
    LOGD(" 2.: Text is now : %s", muPrintable(staffText->xmlText().replace(u"\n", u"\\n")));
    EXPECT_EQ(staffText->xmlText(), originalText);

    score->undoStack()->redo(&ed);
    LOGD(" 3.: Text is now : %s", muPrintable(staffText->xmlText().replace(u"\n", u"\\n")));
    EXPECT_EQ(staffText->xmlText(), expectedText);
}

TEST_F(Engraving_TextBaseTests, musicalSymbolsNotBold)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    StaffText* staffText = addStaffText(score);
    staffText->setXmlText(u"<b>Allegro <sym>metNoteQuarterUp</sym> = 120</b>");
    score->renderer()->layoutItem(staffText);
    auto fragmentList = staffText->fragmentList();
    EXPECT_TRUE(fragmentList.front().font(staffText).bold());
    EXPECT_TRUE(!std::next(fragmentList.begin())->font(staffText).bold());
}

TEST_F(Engraving_TextBaseTests, musicalSymbolsNotItalic)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    Dynamic* dynamic = addDynamic(score);
    dynamic->setXmlText(u"molto <sym>dynamicForte</sym>");
    score->renderer()->layoutItem(dynamic);
    auto fragmentList = dynamic->fragmentList();
    EXPECT_TRUE(fragmentList.front().font(dynamic).italic());
    EXPECT_TRUE(!std::next(fragmentList.begin())->font(dynamic).italic());
}

TEST_F(Engraving_TextBaseTests, letterSpacingAppliedToTextFonts)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    StaffText* staffText = addStaffText(score);
    staffText->setPlainText(u"Spacing");
    staffText->setProperty(Pid::TEXT_LETTER_SPACING, PropertyValue::fromValue(25.0));

    EXPECT_DOUBLE_EQ(staffText->propertyDefault(Pid::TEXT_LETTER_SPACING).toDouble(), 0.0);
    EXPECT_DOUBLE_EQ(staffText->font().letterSpacing(), 25.0);

    score->renderer()->layoutItem(staffText);
    auto fragmentList = staffText->fragmentList();
    ASSERT_FALSE(fragmentList.empty());
    EXPECT_DOUBLE_EQ(fragmentList.front().font(staffText).letterSpacing(), 25.0);
}

static PointF expectedTextDragOrigin(const TextBase* text)
{
    const TextBase::LayoutData* data = text->ldata();
    double left = std::numeric_limits<double>::infinity();
    double baseline = 0.0;
    for (const TextBlock& block : data->blocks) {
        for (const TextFragment& fragment : block.fragments()) {
            if (fragment.text.empty()) {
                continue;
            }
            const muse::draw::FontMetrics fm(fragment.font(text));
            left = std::min(left, fragment.pos.x() + fm.tightBoundingRect(fragment.text).left());
            baseline = block.y();
        }
    }
    return text->canvasPos() + PointF(left, baseline);
}

TEST_F(Engraving_TextBaseTests, dragAlignmentGuideLines)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    StaffText* staffText = addStaffText(score);
    staffText->setPlainText(u"Align");
    score->doLayout();

    const std::vector<LineF> guides = staffText->dragAlignmentGuideLines();
    ASSERT_EQ(guides.size(), 2);

    const PointF origin = expectedTextDragOrigin(staffText);
    EXPECT_NEAR(guides.at(0).y1(), origin.y(), 1e-6);
    EXPECT_NEAR(guides.at(0).y2(), origin.y(), 1e-6);
    EXPECT_NEAR(guides.at(1).x1(), origin.x(), 1e-6);
    EXPECT_NEAR(guides.at(1).x2(), origin.x(), 1e-6);

    const Page* page = toPage(staffText->findAncestor(ElementType::PAGE));
    ASSERT_TRUE(page);
    const RectF pageRect = page->canvasBoundingRect();
    EXPECT_NEAR(guides.at(0).x1(), pageRect.left(), 1e-6);
    EXPECT_NEAR(guides.at(0).x2(), pageRect.right(), 1e-6);
    EXPECT_NEAR(guides.at(1).y1(), pageRect.top(), 1e-6);
    EXPECT_NEAR(guides.at(1).y2(), pageRect.bottom(), 1e-6);

    EXPECT_TRUE(score->showAlignmentGuides());
    score->setShowAlignmentGuides(false);
    EXPECT_FALSE(score->showAlignmentGuides());
    EXPECT_EQ(staffText->dragAlignmentGuideLines().size(), 2);
}

TEST_F(Engraving_TextBaseTests, dynamicDragAlignmentGuideUsesInkNotCollisionBox)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    Dynamic* dynamic = addDynamic(score);
    score->doLayout();

    const std::vector<LineF> guides = dynamic->dragAlignmentGuideLines();
    ASSERT_EQ(guides.size(), 2);

    const PointF origin = expectedTextDragOrigin(dynamic);
    EXPECT_NEAR(guides.at(1).x1(), origin.x(), 1e-6);
    EXPECT_NEAR(guides.at(0).y1(), origin.y(), 1e-6);

    const double collisionLeft = dynamic->canvasPos().x() + dynamic->ldata()->bbox().left();
    if (std::abs(origin.x() - collisionLeft) > 1e-4) {
        EXPECT_GT(std::abs(guides.at(1).x1() - collisionLeft), 1e-4);
    }
}

TEST_F(Engraving_TextBaseTests, lyricsHaveNoDragAlignmentGuideLines)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ChordRest* chordRest = score->firstSegment(SegmentType::ChordRest)->nextChordRest(0);
    ASSERT_TRUE(chordRest);
    Lyrics* lyrics = Factory::createLyrics(chordRest);
    lyrics->setPlainText(u"la");
    chordRest->add(lyrics);
    score->doLayout();

    EXPECT_TRUE(lyrics->dragAlignmentGuideLines().empty());
}
