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

#include <gtest/gtest.h>

#include "dom/accidental.h"
#include "dom/articulation.h"
#include "dom/barline.h"
#include "dom/beam.h"
#include "dom/chord.h"
#include "dom/chordrest.h"
#include "dom/factory.h"
#include "dom/layoutbreak.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/mscore.h"
#include "dom/note.h"
#include "dom/noteval.h"
#include "dom/pitchspelling.h"
#include "dom/property.h"
#include "dom/segment.h"
#include "dom/system.h"
#include "dom/tremolosinglechord.h"

#include "engraving/compat/scoreaccess.h"
#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String NOTE_DATA_DIR("note_data/");

class Engraving_NoteTests : public ::testing::Test
{
};

//---------------------------------------------------------
///   note
///   read/write test of note
//---------------------------------------------------------

TEST_F(Engraving_NoteTests, note)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    Chord* chord = Factory::createChord(score->dummy()->segment());
    Note* note = Factory::createNote(chord);
    chord->add(note);

    // pitch
    note->setPitch(33);
    note->setTpcFromPitch();
    Note* n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->pitch(), 33);
    delete n;

    // tpc
    note->setTpc1(22);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->tpc1(), 22);
    delete n;

    note->setTpc1(23);
    note->setTpc2(23);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->tpc2(), 23);
    delete n;

    // small
    note->setSmall(true);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_TRUE(n->isSmall());
    delete n;

    // mirror
    note->setUserMirror(DirectionH::LEFT);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->userMirror(), DirectionH::LEFT);
    delete n;

    note->setUserMirror(DirectionH::RIGHT);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->userMirror(), DirectionH::RIGHT);
    delete n;

    note->setUserMirror(DirectionH::AUTO);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->userMirror(), DirectionH::AUTO);
    delete n;

    // dot position
    note->setUserDotPosition(DirectionV::UP);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(int(n->userDotPosition()), int(DirectionV::UP));
    delete n;

    note->setUserDotPosition(DirectionV::DOWN);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(int(n->userDotPosition()), int(DirectionV::DOWN));
    delete n;

    note->setUserDotPosition(DirectionV::AUTO);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(int(n->userDotPosition()), int(DirectionV::AUTO));
    delete n;
    // headGroup
    for (int i = 0; i < int(NoteHeadGroup::HEAD_GROUPS); ++i) {
        note->setHeadGroup(NoteHeadGroup(i));
        n = toNote(ScoreRW::writeReadElement(note));
        EXPECT_EQ(int(n->headGroup()), i);
        delete n;
    }

    // headType
    for (int i = 0; i < int(NoteHeadType::HEAD_TYPES); ++i) {
        note->setHeadType(NoteHeadType(i));
        n = toNote(ScoreRW::writeReadElement(note));
        EXPECT_EQ(int(n->headType()), i);
        delete n;
    }

    // user velocity
    note->setUserVelocity(71);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->userVelocity(), 71);
    delete n;

    // tuning
    note->setTuning(1.3);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->tuning(), 1.3);
    delete n;

    // fret
    note->setFret(9);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->fret(), 9);
    delete n;

    // string
    note->setString(3);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->string(), 3);
    delete n;

    // ghost
    note->setGhost(true);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_TRUE(n->ghost());
    delete n;

    //================================================
    //   test setProperty(int, QVariant)
    //================================================

    // pitch
    note->setProperty(Pid::PITCH, 32);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->pitch(), 32);
    delete n;

    // tpc
    note->setProperty(Pid::TPC1, 21);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->tpc1(), 21);
    delete n;

    note->setProperty(Pid::TPC1, 22);
    note->setProperty(Pid::TPC2, 22);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->tpc2(), 22);
    delete n;

    // small
    note->setProperty(Pid::SMALL, false);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_TRUE(!n->isSmall());
    delete n;

    note->setProperty(Pid::SMALL, true);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_TRUE(n->isSmall());
    delete n;

    // mirror
    note->setProperty(Pid::MIRROR_HEAD, int(DirectionH::LEFT));
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->userMirror(), DirectionH::LEFT);
    delete n;

    note->setProperty(Pid::MIRROR_HEAD, int(DirectionH::RIGHT));
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->userMirror(), DirectionH::RIGHT);
    delete n;

    note->setProperty(Pid::MIRROR_HEAD, int(DirectionH::AUTO));
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->userMirror(), DirectionH::AUTO);
    delete n;

    // dot position
    note->setProperty(Pid::DOT_POSITION, PropertyValue::fromValue(DirectionV(DirectionV::UP)));
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(int(n->userDotPosition()), int(DirectionV::UP));
    delete n;

    note->setProperty(Pid::DOT_POSITION, PropertyValue::fromValue(DirectionV(DirectionV::DOWN)));
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(int(n->userDotPosition()), int(DirectionV::DOWN));
    delete n;

    note->setProperty(Pid::DOT_POSITION, PropertyValue::fromValue(DirectionV(DirectionV::AUTO)));
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(int(n->userDotPosition()), int(DirectionV::AUTO));
    delete n;

    // headGroup
    for (int i = 0; i < int(NoteHeadGroup::HEAD_GROUPS); ++i) {
        note->setProperty(Pid::HEAD_GROUP, i);
        n = toNote(ScoreRW::writeReadElement(note));
        EXPECT_EQ(int(n->headGroup()), i);
        delete n;
    }

    // headType
    for (int i = 0; i < int(NoteHeadType::HEAD_TYPES); ++i) {
        note->setProperty(Pid::HEAD_TYPE, i);
        n = toNote(ScoreRW::writeReadElement(note));
        EXPECT_EQ(int(n->headType()), i);
        delete n;
    }

    // user velocity
    note->setProperty(Pid::USER_VELOCITY, 38);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->userVelocity(), 38);
    delete n;

    // tuning
    note->setProperty(Pid::TUNING, 2.4);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->tuning(), 2.4);
    delete n;

    // fret
    note->setProperty(Pid::FRET, 7);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->fret(), 7);
    delete n;

    // string
    note->setProperty(Pid::STRING, 4);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_EQ(n->string(), 4);
    delete n;

    // ghost
    note->setProperty(Pid::GHOST, false);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_TRUE(!n->ghost());
    delete n;

    note->setProperty(Pid::GHOST, true);
    n = toNote(ScoreRW::writeReadElement(note));
    EXPECT_TRUE(n->ghost());
    delete n;

    delete chord;

    delete score;
}

//---------------------------------------------------------
///   grace
///   read/write test of grace notes
//---------------------------------------------------------

TEST_F(Engraving_NoteTests, grace)
{
    MasterScore* score = ScoreRW::readScore(NOTE_DATA_DIR + u"grace.mscx");
    score->doLayout();
    Chord* chord = score->firstMeasure()->findChord(Fraction(0, 1), 0);
    Note* note = chord->upNote();

    // create
    score->setGraceNote(chord, note->pitch(), NoteType::APPOGGIATURA, Constants::DIVISION / 2);
    Chord* gc = chord->graceNotes().front();
    Note* gn = gc->notes().front();
//      Note* n = toNote(ScoreRW::writeReadElement(gn));
//      QCOMPARE(n->noteType(), NoteType::APPOGGIATURA);
//      delete n;

    // tie
    score->select(gn);
    score->cmdAddTie();
//      n = toNote(ScoreRW::writeReadElement(gn));
//      QVERIFY(n->tieFor() != 0);
//      delete n;

    // tremolo
    score->startCmd(TranslatableString::untranslatable("Engraving note tests"));
    TremoloSingleChord* tr = Factory::createTremoloSingleChord(gc);
    tr->setTremoloType(TremoloType::R16);
    tr->setParent(gc);
    tr->setTrack(gc->track());
    score->undoAddElement(tr);
    score->endCmd();
//      Chord* c = static_cast<Chord*>(ScoreRW::writeReadElement(gc));
//      QVERIFY(c->tremolo() != 0);
//      delete c;

    // articulation
    score->startCmd(TranslatableString::untranslatable("Engraving note tests"));
    Articulation* ar = Factory::createArticulation(gc);
    ar->setSymId(SymId::articAccentAbove);
    ar->setParent(gc);
    ar->setTrack(gc->track());
    score->undoAddElement(ar);
    score->endCmd();
//      c = static_cast<Chord*>(ScoreRW::writeReadElement(gc));
//      QVERIFY(c->articulations().size() == 1);
//      delete c;

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"grace-test.mscx", NOTE_DATA_DIR + u"grace-ref.mscx"));
}

//---------------------------------------------------------
///   graceSlashSave
///   read/write test of grace notes
//---------------------------------------------------------

TEST_F(Engraving_NoteTests, graceAfterSlashSave)
{
    MasterScore* score = ScoreRW::readScore(NOTE_DATA_DIR + u"grace.mscx");
    score->doLayout();
    Chord* chord = score->firstMeasure()->findChord(Fraction(0, 1), 0);
    Note* note = chord->upNote();

    // create
    score->setGraceNote(chord, note->pitch(), NoteType::GRACE8_AFTER, Constants::DIVISION / 2);
    Chord* gc = chord->graceNotes().front();
    gc->undoChangeProperty(Pid::SHOW_STEM_SLASH, true);

    EXPECT_TRUE(gc->showStemSlash());

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"graceAfterSlashSave-test.mscx", NOTE_DATA_DIR + u"graceAfterSlashSave-ref.mscx"));
}

//---------------------------------------------------------
///   tpc
///   test of note tpc values
//---------------------------------------------------------

TEST_F(Engraving_NoteTests, tpc)
{
    MasterScore* score = ScoreRW::readScore(NOTE_DATA_DIR + u"tpc.mscx");

    score->inputState().setTrack(0);
    score->inputState().setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->inputState().setDuration(DurationType::V_QUARTER);
    score->inputState().setNoteEntryMode(true);
    int octave = 5 * 7;
    score->cmdAddPitch(octave + 1, false, false);
    score->cmdAddPitch(octave + 2, false, false);
    score->cmdAddPitch(octave + 3, false, false);
    score->cmdAddPitch(octave + 4, false, false);
    score->cmdAddPitch(octave + 5, false, false);
    score->cmdAddPitch(octave + 6, false, false);
    score->cmdAddPitch(octave + 7, false, false);
    score->cmdAddPitch(octave + 8, false, false);

    score->cmdConcertPitchChanged(true);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"tpc-test.mscx", NOTE_DATA_DIR + u"tpc-ref.mscx"));
}

//---------------------------------------------------------
///   tpcTranspose
///   test of note tpc values & transposition
//---------------------------------------------------------

TEST_F(Engraving_NoteTests, tpcTranspose)
{
    MasterScore* score = ScoreRW::readScore(NOTE_DATA_DIR + u"tpc-transpose.mscx");

    score->startCmd(TranslatableString::untranslatable("Engraving note tests"));
    Measure* m = score->firstMeasure();
    score->select(m, SelectType::SINGLE, 0);
    score->changeAccidental(AccidentalType::FLAT);
    score->endCmd();

    score->startCmd(TranslatableString::untranslatable("Engraving note tests"));
    m = m->nextMeasure();
    score->select(m, SelectType::SINGLE, 0);
    score->upDown(false, UpDownMode::CHROMATIC);
    score->endCmd();

    score->startCmd(TranslatableString::untranslatable("Engraving note tests"));
    score->cmdConcertPitchChanged(true);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"tpc-transpose-test.mscx", NOTE_DATA_DIR + u"tpc-transpose-ref.mscx"));
}

//---------------------------------------------------------
///   tpcTranspose2
///   more tests of note tpc values & transposition
//---------------------------------------------------------

TEST_F(Engraving_NoteTests, tpcTranspose2)
{
    MasterScore* score = ScoreRW::readScore(NOTE_DATA_DIR + u"tpc-transpose2.mscx");

    score->inputState().setTrack(0);
    score->inputState().setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->inputState().setDuration(DurationType::V_QUARTER);
    score->inputState().setNoteEntryMode(true);
    int octave = 5 * 7;
    score->cmdAddPitch(octave + 3, false, false);

    score->startCmd(TranslatableString::untranslatable("Engraving note tests"));
    score->cmdConcertPitchChanged(true);
    score->endCmd();

    printf("================\n");

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"tpc-transpose2-test.mscx", NOTE_DATA_DIR + u"tpc-transpose2-ref.mscx"));
}

//---------------------------------------------------------
///   noteLimits
//---------------------------------------------------------

TEST_F(Engraving_NoteTests, noteLimits)
{
    MasterScore* score = ScoreRW::readScore(NOTE_DATA_DIR + u"empty.mscx");

    score->inputState().setTrack(0);
    score->inputState().setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->inputState().setDuration(DurationType::V_QUARTER);
    score->inputState().setNoteEntryMode(true);

    // over 127 shouldn't crash
    score->cmdAddPitch(140, false, false);
    // below 0 shouldn't crash
    score->cmdAddPitch(-40, false, false);

    // stack chords
    score->cmdAddPitch(42, false, false);
    for (int i = 1; i < 20; i++) {
        score->cmdAddPitch(42 + i * 7, true, false);
    }

    // interval below
    score->cmdAddPitch(42, false, false);
    for (int i = 0; i < 20; i++) {
        std::vector<Note*> nl = score->selection().noteList();
        score->startCmd(TranslatableString::untranslatable("Engraving note tests"));
        score->addInterval(-8, nl);
        score->endCmd();
    }

    // interval above
    score->cmdAddPitch(42, false, false);
    for (int i = 0; i < 20; i++) {
        std::vector<Note*> nl = score->selection().noteList();
        score->startCmd(TranslatableString::untranslatable("Engraving note tests"));
        score->addInterval(8, nl);
        score->endCmd();
    }
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"notelimits-test.mscx", NOTE_DATA_DIR + u"notelimits-ref.mscx"));
}

TEST_F(Engraving_NoteTests, tpcDegrees)
{
    EXPECT_EQ(tpc2degree(Tpc::TPC_C,   Key::C),   0);
    //QCOMPARE(tpc2degree(Tpc::TPC_E_S, Key::C),   3);
    EXPECT_EQ(tpc2degree(Tpc::TPC_B,   Key::C),   6);
    EXPECT_EQ(tpc2degree(Tpc::TPC_F_S, Key::C_S), 3);
    EXPECT_EQ(tpc2degree(Tpc::TPC_B,   Key::C_S), 6);
    EXPECT_EQ(tpc2degree(Tpc::TPC_B_B, Key::C_S), 6);
    //QCOMPARE(tpc2degree(Tpc::TPC_B_S, Key::C_S), 7);
}

TEST_F(Engraving_NoteTests, alteredUnison)
{
    MasterScore* score = ScoreRW::readScore(NOTE_DATA_DIR + u"altered-unison.mscx");
    Measure* m = score->firstMeasure();
    Chord* c = m->findChord(Fraction(0, 1), 0);
    EXPECT_TRUE(c->downNote()->accidental() && c->downNote()->accidental()->accidentalType() == AccidentalType::FLAT);
    EXPECT_TRUE(c->upNote()->accidental() && c->upNote()->accidental()->accidentalType() == AccidentalType::NATURAL);
    c = m->findChord(Fraction(1, 4), 0);
    EXPECT_TRUE(c->downNote()->accidental() && c->downNote()->accidental()->accidentalType() == AccidentalType::NATURAL);
    EXPECT_TRUE(c->upNote()->accidental() && c->upNote()->accidental()->accidentalType() == AccidentalType::SHARP);
}

//---------------------------------------------------------
///   LongNoteAfterShort_183746
///    Put a small 128th rest
///    Then put a long Breve note
///    This breve will get spread out across multiple measures
///    Verifies that the resulting notes are tied over at least 3 times (to span 3 measures) and have total duration the same as a breve,
///    regardless of how the breve was divided up.
//---------------------------------------------------------

TEST_F(Engraving_NoteTests, LongNoteAfterShort_183746)
{
    Score* score = ScoreRW::readScore(NOTE_DATA_DIR + "empty.mscx");
    score->doLayout();

    score->inputState().setTrack(0);
    score->inputState().setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->inputState().setDuration(DurationType::V_128TH);
    score->inputState().setNoteEntryMode(true);

    score->cmdEnterRest(DurationType::V_128TH);

    score->inputState().setDuration(DurationType::V_BREVE);
    score->cmdAddPitch(47, 0, 0);

    Segment* s = score->tick2segment(TDuration(DurationType::V_128TH).ticks());
    EXPECT_TRUE(s && s->segmentType() == SegmentType::ChordRest);
    EXPECT_TRUE(s->tick() == Fraction(1, 128));

    EngravingItem* e = s->firstElementForNavigation(0);
    EXPECT_TRUE(e && e->isNote());

    std::vector<Note*> nl = toNote(e)->tiedNotes();
    EXPECT_TRUE(nl.size() >= 3);   // the breve must be divided across at least 3 measures
    Fraction totalTicks = Fraction(0, 1);
    for (Note* n : nl) {
        totalTicks += n->chord()->durationTypeTicks();
    }
    Fraction breveTicks = TDuration(DurationType::V_BREVE).ticks();
    EXPECT_TRUE(totalTicks == breveTicks);   // total duration same as a breve
}

static Chord* chordAtMeasureStart(Measure* measure)
{
    Chord* chord = measure->findChord(measure->tick(), 0);
    EXPECT_TRUE(chord);
    return chord;
}

TEST_F(Engraving_NoteTests, graceBeforeBarlineRoundtrip)
{
    MasterScore* score = ScoreRW::readScore(NOTE_DATA_DIR + u"grace-before-barline.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Measure* m2 = score->firstMeasure()->nextMeasure();
    ASSERT_TRUE(m2);
    Chord* chord = chordAtMeasureStart(m2);
    ASSERT_TRUE(chord);
    Note* graceNote = score->setGraceNote(chord, 74, NoteType::ACCIACCATURA, Constants::DIVISION / 2);
    ASSERT_TRUE(graceNote);
    Chord* gc = graceNote->chord();
    gc->undoChangeProperty(Pid::GRACE_BEFORE_BARLINE, true);

    EXPECT_TRUE(gc->graceBeforeBarline());
    EXPECT_TRUE(gc->placeGraceNotesBeforeBarline());

    const String saveName(u"/tmp/musescore-graceBeforeBarline-test.mscx");
    ASSERT_TRUE(ScoreRW::saveScore(score, saveName));

    const bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;
    MasterScore* restored = ScoreRW::readScore(saveName, true);
    MScore::useRead302InTestMode = useRead302;
    ASSERT_TRUE(restored);

    Measure* restoredM2 = restored->firstMeasure()->nextMeasure();
    ASSERT_TRUE(restoredM2);
    Chord* restoredChord = chordAtMeasureStart(restoredM2);
    ASSERT_TRUE(restoredChord);
    ASSERT_FALSE(restoredChord->graceNotesBefore().empty());
    EXPECT_TRUE(restoredChord->graceNotesBefore().front()->graceBeforeBarline());

    delete restored;
    delete score;
}

TEST_F(Engraving_NoteTests, graceBeforeBarlineGroupAndLayout)
{
    MasterScore* score = ScoreRW::readScore(NOTE_DATA_DIR + u"grace-before-barline.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();
    ASSERT_TRUE(m2);
    Chord* chord = chordAtMeasureStart(m2);
    ASSERT_TRUE(chord);

    Note* grace1 = score->setGraceNote(chord, 74, NoteType::ACCIACCATURA, Constants::DIVISION / 2);
    Note* grace2 = score->setGraceNote(chord, 76, NoteType::ACCIACCATURA, Constants::DIVISION / 2);
    ASSERT_TRUE(grace1);
    ASSERT_TRUE(grace2);

    Chord* gc1 = grace1->chord();
    Chord* gc2 = grace2->chord();
    gc1->undoChangeProperty(Pid::GRACE_BEFORE_BARLINE, true);

    EXPECT_TRUE(gc1->graceBeforeBarline());
    EXPECT_TRUE(gc2->graceBeforeBarline());

    score->doLayout();

    Segment* barlineSeg = m1->findSegmentR(SegmentType::EndBarLine, m1->ticks());
    ASSERT_TRUE(barlineSeg);
    EngravingItem* barline = barlineSeg->element(0);
    ASSERT_TRUE(barline);

    const double barlineX = barline->pagePos().x();
    const double measure1X = m1->pagePos().x();
    EXPECT_LT(gc1->pagePos().x(), barlineX);
    EXPECT_LT(gc2->pagePos().x(), barlineX);
    EXPECT_GT(gc1->pagePos().x(), measure1X);
    EXPECT_GT(gc2->pagePos().x(), measure1X);
    EXPECT_TRUE(gc1->placeGraceNotesBeforeBarline());
    EXPECT_EQ(chord->graceNotesBefore().appendedSegment(), barlineSeg);

    delete score;
}

static Chord* lastChordInMeasure(Measure* measure)
{
    for (Segment* s = measure->last(); s; s = s->prev()) {
        if (!s->isChordRestType()) {
            continue;
        }
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            return toChord(e);
        }
    }
    return nullptr;
}

TEST_F(Engraving_NoteTests, graceGroupInternalSpacingDoesNotChangeNextMeasureLeading)
{
    MasterScore* score = ScoreRW::readScore(NOTE_DATA_DIR + u"grace-before-barline.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();
    ASSERT_TRUE(m2);
    Chord* principal = chordAtMeasureStart(m2);
    ASSERT_TRUE(principal);

    Note* added1 = score->setGraceNote(principal, 74, NoteType::ACCIACCATURA, Constants::DIVISION / 2);
    Note* added2 = score->setGraceNote(principal, 76, NoteType::ACCIACCATURA, Constants::DIVISION / 2);
    ASSERT_TRUE(added1);
    ASSERT_TRUE(added2);
    score->doLayout();

    GraceNotesGroup& gnb = principal->graceNotesBefore();
    ASSERT_EQ(gnb.size(), 2u);
    Chord* leftmost = gnb.front();
    Chord* second = gnb.at(1);
    Note* leftmostNote = leftmost->notes().front();
    Note* secondNote = second->notes().front();
    ASSERT_TRUE(leftmostNote);
    ASSERT_TRUE(secondNote);

    EXPECT_EQ(secondNote->prevChordOnStaff(), leftmost);
    EXPECT_EQ(secondNote->prevNoteDistanceLeadingItem(), second);
    EXPECT_EQ(leftmostNote->prevNoteDistanceLeadingItem(), principal->segment());

    const double gapBefore = second->pagePos().x() - leftmost->pagePos().x();

    second->undoChangeProperty(Pid::LEADING_SPACE, Spatium(1.0));
    score->doLayout();

    EXPECT_GT(second->pagePos().x() - leftmost->pagePos().x(), gapBefore);
    EXPECT_DOUBLE_EQ(principal->segment()->extraLeadingSpace().val(), 0.0);
    EXPECT_DOUBLE_EQ(second->extraLeadingSpace().val(), 1.0);

    leftmost->undoChangeProperty(Pid::GRACE_BEFORE_BARLINE, true);
    score->doLayout();

    EXPECT_EQ(leftmostNote->prevNoteDistanceLeadingItem(), leftmost);

    Chord* prevMeasureChord = lastChordInMeasure(m1);
    ASSERT_TRUE(prevMeasureChord);
    const double distToPrev = leftmost->pagePos().x() - prevMeasureChord->pagePos().x();
    const double distToPrevStem = leftmostNote->prevNoteDistance().val();

    leftmost->undoChangeProperty(Pid::LEADING_SPACE, Spatium(1.0));
    score->doLayout();

    EXPECT_GT(leftmost->pagePos().x() - prevMeasureChord->pagePos().x(), distToPrev);
    EXPECT_GT(leftmostNote->prevNoteDistance().val(), distToPrevStem);
    EXPECT_DOUBLE_EQ(principal->segment()->extraLeadingSpace().val(), 0.0);
    EXPECT_DOUBLE_EQ(leftmost->extraLeadingSpace().val(), 1.0);

    Segment* barlineSeg = m1->findSegmentR(SegmentType::EndBarLine, m1->ticks());
    ASSERT_TRUE(barlineSeg);
    EXPECT_DOUBLE_EQ(barlineSeg->extraLeadingSpace().val(), 0.0);

    const double innerGap = second->pagePos().x() - leftmost->pagePos().x();
    second->undoChangeProperty(Pid::LEADING_SPACE, Spatium(2.0));
    score->doLayout();
    EXPECT_GT(second->pagePos().x() - leftmost->pagePos().x(), innerGap);
    EXPECT_DOUBLE_EQ(principal->segment()->extraLeadingSpace().val(), 0.0);
    EXPECT_DOUBLE_EQ(barlineSeg->extraLeadingSpace().val(), 0.0);

    EngravingItem* barline = barlineSeg->element(0);
    ASSERT_TRUE(barline);
    second->undoChangeProperty(Pid::LEADING_SPACE, Spatium(20.0));
    leftmost->undoChangeProperty(Pid::LEADING_SPACE, Spatium(20.0));
    score->doLayout();
    EXPECT_LT(leftmost->pagePos().x(), barline->pagePos().x());
    EXPECT_LT(second->pagePos().x(), barline->pagePos().x());
    EXPECT_DOUBLE_EQ(principal->segment()->extraLeadingSpace().val(), 0.0);

    delete score;
}

TEST_F(Engraving_NoteTests, graceSecondSpacingClearsWhenAutoplaceOff)
{
    MasterScore* score = ScoreRW::readScore(NOTE_DATA_DIR + u"grace-before-barline.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Measure* m2 = score->firstMeasure()->nextMeasure();
    ASSERT_TRUE(m2);
    Chord* principal = chordAtMeasureStart(m2);
    ASSERT_TRUE(principal);

    Note* added1 = score->setGraceNote(principal, 71, NoteType::ACCIACCATURA, Constants::DIVISION / 2);
    Note* added2 = score->setGraceNote(principal, 72, NoteType::ACCIACCATURA, Constants::DIVISION / 2);
    ASSERT_TRUE(added1);
    ASSERT_TRUE(added2);

    Chord* leftmost = added2->chord();
    Note* dyad = Factory::createNote(leftmost);
    dyad->setPitch(73);
    dyad->setTpcFromPitch();
    leftmost->add(dyad);
    score->doLayout();

    GraceNotesGroup& gnb = principal->graceNotesBefore();
    ASSERT_EQ(gnb.size(), 2u);
    Chord* second = gnb.at(1);
    ASSERT_EQ(gnb.front(), leftmost);
    ASSERT_EQ(leftmost->notes().size(), 2u);
    const double gapWithAutoplace = second->pagePos().x() - leftmost->pagePos().x();

    for (Chord* grace : gnb) {
        for (Note* note : grace->notes()) {
            note->undoChangeProperty(Pid::AUTOPLACE, false);
        }
    }
    score->doLayout();

    EXPECT_LE(second->pagePos().x() - leftmost->pagePos().x(), gapWithAutoplace);
    EXPECT_DOUBLE_EQ(principal->segment()->extraLeadingSpace().val(), 0.0);

    delete score;
}

static void addLineBreak(Score* score, Measure* measure)
{
    LayoutBreak* lb = Factory::createLayoutBreak(measure);
    lb->setLayoutBreakType(LayoutBreakType::LINE);
    lb->setTrack(0);
    lb->setParent(measure);
    score->undoAddElement(lb);
}

TEST_F(Engraving_NoteTests, graceBeforeBarlineAcrossSystem)
{
    MasterScore* score = ScoreRW::readScore(NOTE_DATA_DIR + u"grace-before-barline.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();
    ASSERT_TRUE(m2);
    Chord* principal = chordAtMeasureStart(m2);
    ASSERT_TRUE(principal);

    Note* added1 = score->setGraceNote(principal, 74, NoteType::ACCIACCATURA, Constants::DIVISION / 2);
    Note* added2 = score->setGraceNote(principal, 76, NoteType::ACCIACCATURA, Constants::DIVISION / 2);
    ASSERT_TRUE(added1);
    ASSERT_TRUE(added2);
    principal->graceNotesBefore().front()->undoChangeProperty(Pid::GRACE_BEFORE_BARLINE, true);
    addLineBreak(score, m1);
    score->doLayout();

    ASSERT_TRUE(m1->system());
    ASSERT_TRUE(m2->system());
    EXPECT_NE(m1->system(), m2->system());

    GraceNotesGroup& gnb = principal->graceNotesBefore();
    ASSERT_EQ(gnb.size(), 2u);
    Chord* leftmost = gnb.front();
    Chord* second = gnb.at(1);

    Segment* barlineSeg = m1->findSegmentR(SegmentType::EndBarLine, m1->ticks());
    ASSERT_TRUE(barlineSeg);
    EngravingItem* barline = barlineSeg->element(0);
    ASSERT_TRUE(barline);

    EXPECT_LT(leftmost->pagePos().x(), barline->pagePos().x());
    EXPECT_LT(second->pagePos().x(), barline->pagePos().x());
    EXPECT_GT(leftmost->pagePos().x(), m1->pagePos().x());
    EXPECT_GT(second->pagePos().x(), m1->pagePos().x());

    const double yGrace = leftmost->pagePos().y();
    EXPECT_LT(std::abs(yGrace - m1->system()->staffYpage(0)), std::abs(yGrace - m2->system()->staffYpage(0)));

    if (Beam* beam = leftmost->beam()) {
        EXPECT_EQ(beam->system(), m1->system());
    }

    for (Chord* grace : gnb) {
        for (Note* note : grace->notes()) {
            note->undoChangeProperty(Pid::AUTOPLACE, false);
        }
    }
    score->doLayout();

    Chord* prevMeasureChord = lastChordInMeasure(m1);
    ASSERT_TRUE(prevMeasureChord);
    const double distToPrev = leftmost->pagePos().x() - prevMeasureChord->pagePos().x();
    const double distToPrevStem = leftmost->notes().front()->prevNoteDistance().val();

    second->undoChangeProperty(Pid::LEADING_SPACE, Spatium(1.0));
    leftmost->undoChangeProperty(Pid::LEADING_SPACE, Spatium(1.0));
    score->doLayout();

    EXPECT_DOUBLE_EQ(principal->segment()->extraLeadingSpace().val(), 0.0);
    EXPECT_DOUBLE_EQ(barlineSeg->extraLeadingSpace().val(), 0.0);
    EXPECT_DOUBLE_EQ(second->extraLeadingSpace().val(), 1.0);
    EXPECT_DOUBLE_EQ(leftmost->extraLeadingSpace().val(), 1.0);
    EXPECT_GT(leftmost->pagePos().x() - prevMeasureChord->pagePos().x(), distToPrev);
    EXPECT_GT(leftmost->notes().front()->prevNoteDistance().val(), distToPrevStem);

    const String saveName(u"/tmp/musescore-graceBeforeBarline-system-test.mscx");
    ASSERT_TRUE(ScoreRW::saveScore(score, saveName));

    const bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;
    MasterScore* restored = ScoreRW::readScore(saveName, true);
    MScore::useRead302InTestMode = useRead302;
    ASSERT_TRUE(restored);
    restored->doLayout();

    Measure* restoredM1 = restored->firstMeasure();
    Measure* restoredM2 = restoredM1->nextMeasure();
    ASSERT_TRUE(restoredM2);
    Chord* restoredPrincipal = chordAtMeasureStart(restoredM2);
    ASSERT_TRUE(restoredPrincipal);
    GraceNotesGroup& restoredGnb = restoredPrincipal->graceNotesBefore();
    ASSERT_EQ(restoredGnb.size(), 2u);
    EXPECT_DOUBLE_EQ(restoredGnb.front()->extraLeadingSpace().val(), 1.0);
    EXPECT_DOUBLE_EQ(restoredGnb.at(1)->extraLeadingSpace().val(), 1.0);

    Chord* restoredPrev = lastChordInMeasure(restoredM1);
    ASSERT_TRUE(restoredPrev);
    EXPECT_GT(restoredGnb.front()->pagePos().x() - restoredPrev->pagePos().x(), distToPrev);

    delete restored;
    delete score;
}
