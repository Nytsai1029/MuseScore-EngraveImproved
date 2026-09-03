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

#include <algorithm>
#include <cmath>

#include <gtest/gtest.h>

#include "dom/accidental.h"
#include "dom/chord.h"
#include "dom/ledgerline.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/note.h"

#include "style/styledef.h"

#include "utils/scorerw.h"

using namespace mu;
using namespace mu::engraving;

static const String ACCIDENTAL_DATA_DIR(u"accidental_data/");

class Engraving_AccidentalTests : public ::testing::Test
{
};

static Chord* chordAt(MasterScore* score, int measureIndex)
{
    Measure* measure = score->firstMeasure();
    for (int i = 0; i < measureIndex && measure; ++i) {
        measure = measure->nextMeasure();
    }
    if (!measure) {
        return nullptr;
    }
    return measure->findChord(measure->tick(), 0);
}

static double accidentalToNoteGap(const Note* note)
{
    const Accidental* acc = note->accidental();
    if (!acc) {
        return 0.0;
    }
    return note->pageBoundingRect().left() - acc->pageBoundingRect().right();
}

static double accidentalToLedgerGap(const Note* note)
{
    const Accidental* acc = note->accidental();
    const Chord* chord = note->chord();
    if (!acc || !chord || chord->ledgerLines().empty()) {
        return 0.0;
    }

    const double accRight = acc->pageBoundingRect().right();
    double minGap = 1e9;
    for (const LedgerLine* ledger : chord->ledgerLines()) {
        minGap = std::min(minGap, ledger->pageBoundingRect().left() - accRight);
    }
    return minGap;
}

static void relayoutWithStyle(MasterScore* score)
{
    score->styleChanged();
    score->doLayout();
}

TEST_F(Engraving_AccidentalTests, accidentalNoteDistanceMovesInStaffAccidental)
{
    MasterScore* score = ScoreRW::readScore(ACCIDENTAL_DATA_DIR + u"accidental-spacing.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Chord* chord = chordAt(score, 0);
    ASSERT_TRUE(chord);
    const Note* note = chord->upNote();
    ASSERT_TRUE(note);
    ASSERT_TRUE(note->accidental());
    EXPECT_TRUE(chord->ledgerLines().empty());

    const double defaultGap = accidentalToNoteGap(note);
    const double spatium = score->style().spatium();
    EXPECT_GT(defaultGap, 0.0);

    score->style().set(Sid::accidentalNoteDistance, Spatium(0.85));
    relayoutWithStyle(score);

    chord = chordAt(score, 0);
    ASSERT_TRUE(chord);
    EXPECT_GT(accidentalToNoteGap(chord->upNote()), defaultGap + 0.4 * spatium);

    delete score;
}

TEST_F(Engraving_AccidentalTests, flushToLedgerClosesRightmostGap)
{
    MasterScore* score = ScoreRW::readScore(ACCIDENTAL_DATA_DIR + u"accidental-spacing.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Chord* ledgerChord = chordAt(score, 1);
    ASSERT_TRUE(ledgerChord);
    const Note* ledgerNote = ledgerChord->upNote();
    ASSERT_TRUE(ledgerNote);
    ASSERT_TRUE(ledgerNote->accidental());
    ASSERT_FALSE(ledgerChord->ledgerLines().empty());

    const double defaultLedgerGap = accidentalToLedgerGap(ledgerNote);
    const double spatium = score->style().spatium();
    EXPECT_GT(defaultLedgerGap, 0.05 * spatium);

    Chord* staffChord = chordAt(score, 0);
    ASSERT_TRUE(staffChord);
    const double staffGapBefore = accidentalToNoteGap(staffChord->upNote());

    score->style().set(Sid::accidentalFlushToLedgerLine, true);
    relayoutWithStyle(score);

    ledgerChord = chordAt(score, 1);
    ASSERT_TRUE(ledgerChord);
    const double flushedLedgerGap = accidentalToLedgerGap(ledgerChord->upNote());
    EXPECT_LT(flushedLedgerGap, defaultLedgerGap - 0.05 * spatium);
    EXPECT_NEAR(flushedLedgerGap, -0.05 * spatium, 0.08 * spatium);

    staffChord = chordAt(score, 0);
    ASSERT_TRUE(staffChord);
    EXPECT_NEAR(accidentalToNoteGap(staffChord->upNote()), staffGapBefore, 0.02 * spatium);

    delete score;
}

TEST_F(Engraving_AccidentalTests, flushKeepsChordStacking)
{
    MasterScore* score = ScoreRW::readScore(ACCIDENTAL_DATA_DIR + u"accidental-spacing.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Chord* chord = chordAt(score, 2);
    ASSERT_TRUE(chord);
    ASSERT_EQ(chord->notes().size(), 2);
    ASSERT_FALSE(chord->ledgerLines().empty());

    const Note* lower = chord->downNote();
    const Note* upper = chord->upNote();
    ASSERT_TRUE(lower->accidental());
    ASSERT_TRUE(upper->accidental());

    const double stackedGapBefore = std::abs(lower->accidental()->pagePos().x() - upper->accidental()->pagePos().x());
    EXPECT_GT(stackedGapBefore, 0.15 * score->style().spatium());

    score->style().set(Sid::accidentalFlushToLedgerLine, true);
    relayoutWithStyle(score);

    chord = chordAt(score, 2);
    ASSERT_TRUE(chord);
    lower = chord->downNote();
    upper = chord->upNote();

    const double stackedGapAfter = std::abs(lower->accidental()->pagePos().x() - upper->accidental()->pagePos().x());
    EXPECT_NEAR(stackedGapAfter, stackedGapBefore, 0.15 * score->style().spatium());

    const double rightmostX = std::max(lower->accidental()->pageBoundingRect().right(),
                                       upper->accidental()->pageBoundingRect().right());
    double minLedgerGap = 1e9;
    for (const LedgerLine* ledger : chord->ledgerLines()) {
        minLedgerGap = std::min(minLedgerGap, ledger->pageBoundingRect().left() - rightmostX);
    }
    EXPECT_NEAR(minLedgerGap, -0.05 * score->style().spatium(), 0.12 * score->style().spatium());

    delete score;
}
