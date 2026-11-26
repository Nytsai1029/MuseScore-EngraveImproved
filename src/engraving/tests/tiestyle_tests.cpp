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

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include "compat/scoreaccess.h"
#include "dom/chord.h"
#include "dom/factory.h"
#include "dom/keysig.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/page.h"
#include "dom/segment.h"
#include "dom/slur.h"
#include "dom/staff.h"
#include "dom/system.h"
#include "dom/tie.h"
#include "dom/timesig.h"
#include "draw/types/painterpath.h"
#include "draw/types/transform.h"
#include "officialexport/officiallayoutbaker.h"
#include "rendering/score/slurtielayout.h"
#include "style/style.h"

#include "utils/scorerw.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;
using muse::draw::PainterPath;
using muse::draw::Transform;

namespace {
const String REF_SPANS_SCORE(u"tiestyle_data/referenceSpans.mscx");
const String SIG_GAPS_SCORE(u"tiestyle_data/signatureGaps.mscx");

std::vector<TieSegment*> collectTieSegments(Score* score)
{
    std::vector<TieSegment*> result;
    for (Page* page : score->pages()) {
        for (System* system : page->systems()) {
            for (SpannerSegment* seg : system->spannerSegments()) {
                if (seg && seg->isTieSegment()) {
                    result.push_back(static_cast<TieSegment*>(seg));
                }
            }
        }
    }
    return result;
}

double tieSpanInSp(const TieSegment* seg)
{
    const PointF d = seg->ups(Grip::END).pos() - seg->ups(Grip::START).pos();
    return std::hypot(d.x(), d.y()) / seg->spatium();
}

void relayout(MasterScore* score)
{
    score->setLayoutAll();
    score->doLayout();
}

// Reconstructs the pre-change flat lens outline directly from a segment's final normalized
// control points so FLAT mode can be verified element-by-element against the original formula.
PainterPath buildLegacyFlatLensPath(const SlurTieSegment* seg, const PointF& midThicknessVec)
{
    const PointF start = seg->ups(Grip::START).pos();
    const PointF end = seg->ups(Grip::END).pos();
    const PointF diff = end - start;
    const double angle = std::atan(diff.y() / diff.x());

    Transform toLocal;
    toLocal.rotateRadians(-angle);
    const PointF endLocal = toLocal.map(diff);
    const PointF c1Local = toLocal.map(seg->ups(Grip::BEZIER1).pos() - start);
    const PointF c2Local = toLocal.map(seg->ups(Grip::BEZIER2).pos() - start);

    auto legacyNormal = [](const PointF& from, const PointF& to) -> PointF {
        const PointF d = to - from;
        const double len = std::hypot(d.x(), d.y());
        if (len < 1e-9) {
            return PointF(0.0, -1.0);
        }
        const PointF n(d.y() / len, -d.x() / len);
        return n.y() > 0.0 ? -n : n;
    };

    const Staff* staff = seg->staff();
    const double mag = staff ? staff->staffMag(seg->slurTie()->tick()) : 1.0;
    const double endHalfWidth = 0.5 * seg->endWidth() * mag;
    const PointF startOff = legacyNormal(PointF(), c1Local) * endHalfWidth;
    const PointF endOff = legacyNormal(c2Local, endLocal) * endHalfWidth;
    const PointF widened = midThicknessVec + PointF(0.0, endHalfWidth);

    PainterPath path;
    path.setFillRule(PainterPath::FillRule::WindingFill);
    path.moveTo(startOff);
    path.cubicTo(c1Local - widened, c2Local - widened, endLocal + endOff);
    path.lineTo(endLocal - endOff);
    path.cubicTo(c2Local + widened, c1Local + widened, -startOff);
    path.closeSubpath();

    Transform toSystem;
    toSystem.translate(start.x(), start.y());
    toSystem.rotateRadians(angle);
    return toSystem.map(path);
}

void expectPathsEqual(const PainterPath& actual, const PainterPath& expected)
{
    ASSERT_EQ(actual.elementCount(), expected.elementCount());
    for (size_t i = 0; i < actual.elementCount(); ++i) {
        const PainterPath::Element a = actual.elementAt(i);
        const PainterPath::Element e = expected.elementAt(i);
        EXPECT_EQ(a.type, e.type) << "element " << i;
        EXPECT_NEAR(a.x, e.x, 1e-9) << "element " << i;
        EXPECT_NEAR(a.y, e.y, 1e-9) << "element " << i;
    }
}

double maxProjectionAlong(const PainterPath& path, const PointF& origin, const PointF& unitDir)
{
    double maxProj = -1e30;
    for (size_t i = 0; i < path.elementCount(); ++i) {
        const PointF p(path.elementAt(i));
        const PointF rel = p - origin;
        const double proj = rel.x() * unitDir.x() + rel.y() * unitDir.y();
        maxProj = std::max(maxProj, proj);
    }
    return maxProj;
}

TieSegment* makeStandaloneTieSegment(MasterScore* score, double spanSp, bool up = true)
{
    const double sp = score->style().spatium();
    Tie* tie = Factory::createTie(score->dummy());
    tie->setUp(up);
    tie->setStyleType(SlurStyleType::Solid);
    tie->fixupSegments(1);

    TieSegment* seg = tie->frontSegment();
    seg->setSpannerSegmentType(SpannerSegmentType::SINGLE);
    seg->ups(Grip::START).p = PointF(0.0, 0.0);
    seg->ups(Grip::END).p = PointF(spanSp * sp, 0.0);
    return seg;
}

SlurSegment* makeStandaloneSlurSegment(MasterScore* score, double spanSp, bool multiBezier = false)
{
    const double sp = score->style().spatium();
    Slur* slur = Factory::createSlur(score->dummy());
    slur->setUp(true);
    slur->setAutoplace(false);
    slur->setStyleType(SlurStyleType::Solid);
    if (multiBezier) {
        slur->setMultiBezierEnabled(true);
        slur->setMultiBezierKnotCount(2);
    }
    slur->fixupSegments(1);

    SlurSegment* seg = slur->frontSegment();
    seg->setSpannerSegmentType(SpannerSegmentType::SINGLE);
    seg->setAutoplace(false);
    seg->ups(Grip::START).p = PointF(0.0, 0.0);
    seg->ups(Grip::END).p = PointF(spanSp * sp, 0.0);
    return seg;
}
}

class Engraving_TieStyleTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_useRead302 = MScore::useRead302InTestMode;
        MScore::useRead302InTestMode = false;
    }

    void TearDown() override
    {
        MScore::useRead302InTestMode = m_useRead302;
    }

private:
    bool m_useRead302 = true;
};

TEST_F(Engraving_TieStyleTests, defaultsKeepTheCurve)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(REF_SPANS_SCORE));
    ASSERT_NE(score, nullptr);
    const double sp = score->style().spatium();

    // Verify default style values match the formula sampled at 2, 5, 8 sp.
    EXPECT_FALSE(score->style().styleB(Sid::tieCurveByReferenceSpans));
    EXPECT_DOUBLE_EQ(score->style().styleS(Sid::tieShortSpan).val(), 2.0);
    EXPECT_DOUBLE_EQ(score->style().styleS(Sid::tieMediumSpan).val(), 5.0);
    EXPECT_DOUBLE_EQ(score->style().styleS(Sid::tieLongSpan).val(), 8.0);
    EXPECT_DOUBLE_EQ(score->style().styleS(Sid::tieShortHeight).val(), 0.6);
    EXPECT_DOUBLE_EQ(score->style().styleS(Sid::tieMediumHeight).val(), 0.9);
    EXPECT_DOUBLE_EQ(score->style().styleS(Sid::tieLongHeight).val(), 1.09);
    EXPECT_DOUBLE_EQ(score->style().styleS(Sid::tieShortControlDistance).val(), 1.2);
    EXPECT_DOUBLE_EQ(score->style().styleS(Sid::tieMediumControlDistance).val(), 3.0);
    EXPECT_DOUBLE_EQ(score->style().styleS(Sid::tieLongControlDistance).val(), 4.8);

    // 1. Exact check at 2, 5, 8 sp and intermediate / extrapolated spans.
    double maxAtRefsSp = 0.0;
    double maxOtherSp = 0.0;
    for (double lengthSp : { 1.5, 2.0, 3.5, 5.0, 6.5, 8.0, 10.0 }) {
        score->style().set(Sid::tieCurveByReferenceSpans, false);
        TieSegment* seg = makeStandaloneTieSegment(score.get(), lengthSp);
        SlurTieLayout::computeBezier(seg);
        const PointF b1Formula = seg->ups(Grip::BEZIER1).pos();
        const PointF b2Formula = seg->ups(Grip::BEZIER2).pos();

        score->style().set(Sid::tieCurveByReferenceSpans, true);
        SlurTieLayout::computeBezier(seg);
        const PointF b1Ref = seg->ups(Grip::BEZIER1).pos();
        const PointF b2Ref = seg->ups(Grip::BEZIER2).pos();

        const double d1 = std::hypot(b1Ref.x() - b1Formula.x(), b1Ref.y() - b1Formula.y()) / sp;
        const double d2 = std::hypot(b2Ref.x() - b2Formula.x(), b2Ref.y() - b2Formula.y()) / sp;
        const double delta = std::max(d1, d2);
        const bool atReference = std::abs(lengthSp - 2.0) < 0.25
                                 || std::abs(lengthSp - 5.0) < 0.25
                                 || std::abs(lengthSp - 8.0) < 0.25;
        if (atReference) {
            maxAtRefsSp = std::max(maxAtRefsSp, delta);
            EXPECT_LE(delta, 0.02) << "span=" << lengthSp << "sp";
        } else {
            maxOtherSp = std::max(maxOtherSp, delta);
            EXPECT_LE(delta, 0.08) << "span=" << lengthSp << "sp";
        }
        delete seg->tie();
    }

    // 2. Fixture score with natural ties across short, medium, and long durations.
    score->style().set(Sid::tieCurveByReferenceSpans, false);
    relayout(score.get());
    const std::vector<TieSegment*> tiesOff = collectTieSegments(score.get());
    ASSERT_GE(tiesOff.size(), 5u);

    std::vector<std::pair<PointF, PointF> > formulaControls;
    std::vector<double> spans;
    for (const TieSegment* seg : tiesOff) {
        formulaControls.emplace_back(seg->ups(Grip::BEZIER1).pos(), seg->ups(Grip::BEZIER2).pos());
        spans.push_back(tieSpanInSp(seg));
    }

    score->style().set(Sid::tieCurveByReferenceSpans, true);
    relayout(score.get());
    const std::vector<TieSegment*> tiesOn = collectTieSegments(score.get());
    ASSERT_EQ(tiesOn.size(), tiesOff.size());

    double maxFixtureSp = 0.0;
    for (size_t i = 0; i < tiesOn.size(); ++i) {
        const PointF b1 = tiesOn[i]->ups(Grip::BEZIER1).pos();
        const PointF b2 = tiesOn[i]->ups(Grip::BEZIER2).pos();
        const PointF f1 = formulaControls[i].first;
        const PointF f2 = formulaControls[i].second;
        const double d1 = std::hypot(b1.x() - f1.x(), b1.y() - f1.y()) / sp;
        const double d2 = std::hypot(b2.x() - f2.x(), b2.y() - f2.y()) / sp;
        const double delta = std::max(d1, d2);
        maxFixtureSp = std::max(maxFixtureSp, delta);

        const double span = spans[i];
        const bool nearRef = std::abs(span - 2.0) < 0.35
                             || std::abs(span - 5.0) < 0.35
                             || std::abs(span - 8.0) < 0.35;
        std::cout << "  fixture tie " << i << ": span=" << span << "sp, delta=" << delta << "sp" << std::endl;
        EXPECT_LE(delta, nearRef ? 0.02 : 0.08) << "tie " << i << " span=" << span << "sp";
    }

    std::cout << "[TieStyle] defaultsKeepTheCurve: maxAtRefs=" << maxAtRefsSp
              << "sp, maxOther=" << maxOtherSp << "sp, maxFixture=" << maxFixtureSp << "sp" << std::endl;
}

TEST_F(Engraving_TieStyleTests, referenceSpansShapeTheCurve)
{
    std::unique_ptr<MasterScore> score(compat::ScoreAccess::createMasterScore(nullptr));
    ASSERT_NE(score, nullptr);
    const double sp = score->style().spatium();
    score->style().set(Sid::tieCurveByReferenceSpans, true);

    TieSegment* shortTie = makeStandaloneTieSegment(score.get(), 2.0);
    TieSegment* mediumTie = makeStandaloneTieSegment(score.get(), 5.0);
    TieSegment* longTie = makeStandaloneTieSegment(score.get(), 8.0);

    SlurTieLayout::computeBezier(shortTie);
    SlurTieLayout::computeBezier(mediumTie);
    SlurTieLayout::computeBezier(longTie);

    const PointF shortB1Before = shortTie->ups(Grip::BEZIER1).pos();
    const PointF mediumB1Before = mediumTie->ups(Grip::BEZIER1).pos();
    const PointF longB1Before = longTie->ups(Grip::BEZIER1).pos();
    const PointF longB2Before = longTie->ups(Grip::BEZIER2).pos();

    // Raising tieMediumHeight raises medium tie control points and leaves short tie untouched.
    score->style().set(Sid::tieMediumHeight, Spatium(1.4));
    SlurTieLayout::computeBezier(shortTie);
    SlurTieLayout::computeBezier(mediumTie);
    EXPECT_NEAR(shortTie->ups(Grip::BEZIER1).pos().y(), shortB1Before.y(), 1e-9);
    EXPECT_NEAR(mediumTie->ups(Grip::BEZIER1).pos().y(), -1.4 * sp, 1e-6);
    EXPECT_LT(mediumTie->ups(Grip::BEZIER1).pos().y(), mediumB1Before.y());

    // Increasing tieLongControlDistance moves long tie control points apart symmetrically.
    score->style().set(Sid::tieLongControlDistance, Spatium(6.0));
    SlurTieLayout::computeBezier(longTie);
    const PointF longB1After = longTie->ups(Grip::BEZIER1).pos();
    const PointF longB2After = longTie->ups(Grip::BEZIER2).pos();
    EXPECT_NEAR(longB2After.x() - longB1After.x(), 6.0 * sp, 1e-6);
    EXPECT_NEAR(0.5 * (longB1After.x() + longB2After.x()), 4.0 * sp, 1e-6);
    EXPECT_NEAR(longB1Before.x() - longB1After.x(), longB2After.x() - longB2Before.x(), 1e-6);

    // Min / max shoulder height clamps still apply.
    score->style().set(Sid::tieLongHeight, Spatium(5.0));
    SlurTieLayout::computeBezier(longTie);
    EXPECT_NEAR(-longTie->ups(Grip::BEZIER1).pos().y(), score->style().styleMM(Sid::tieMaxShoulderHeight), 1e-6);

    score->style().set(Sid::tieShortHeight, Spatium(0.05));
    SlurTieLayout::computeBezier(shortTie);
    EXPECT_NEAR(-shortTie->ups(Grip::BEZIER1).pos().y(), score->style().styleMM(Sid::tieMinShoulderHeight), 1e-6);

    // Control distance is clamped to [0, L].
    score->style().set(Sid::tieLongControlDistance, Spatium(20.0));
    SlurTieLayout::computeBezier(longTie);
    EXPECT_NEAR(longTie->ups(Grip::BEZIER2).pos().x() - longTie->ups(Grip::BEZIER1).pos().x(), 8.0 * sp, 1e-6);

    // Duplicate spans must not divide by zero; the later entry wins.
    score->style().set(Sid::tieShortSpan, Spatium(5.0));
    score->style().set(Sid::tieMediumSpan, Spatium(5.0));
    score->style().set(Sid::tieLongSpan, Spatium(8.0));
    score->style().set(Sid::tieShortHeight, Spatium(0.5));
    score->style().set(Sid::tieMediumHeight, Spatium(1.1));
    score->style().set(Sid::tieLongHeight, Spatium(1.4));
    SlurTieLayout::computeBezier(mediumTie);
    EXPECT_TRUE(std::isfinite(mediumTie->ups(Grip::BEZIER1).pos().y()));
    EXPECT_NEAR(-mediumTie->ups(Grip::BEZIER1).pos().y(), 1.1 * sp, 1e-6);

    // All three spans equal: last (long) entry wins, shorter spans scale proportionally.
    score->style().set(Sid::tieLongSpan, Spatium(5.0));
    score->style().set(Sid::tieLongHeight, Spatium(1.2));
    SlurTieLayout::computeBezier(mediumTie);
    EXPECT_NEAR(-mediumTie->ups(Grip::BEZIER1).pos().y(), 1.2 * sp, 1e-6);

    delete shortTie->tie();
    delete mediumTie->tie();
    delete longTie->tie();
}

TEST_F(Engraving_TieStyleTests, roundEndsAreRound)
{
    std::unique_ptr<MasterScore> score(compat::ScoreAccess::createMasterScore(nullptr));
    ASSERT_NE(score, nullptr);
    score->style().set(Sid::tieEndWidth, Spatium(0.16));
    score->style().set(Sid::slurEndWidth, Spatium(0.16));

    // 1. Solid tie: FLAT matches the exact legacy lens path; ROUND extends along both end tangents.
    TieSegment* tieSeg = makeStandaloneTieSegment(score.get(), 5.0);
    score->style().set(Sid::tieEndStyle, int(SlurTieEndStyle::FLAT));
    SlurTieLayout::computeBezier(tieSeg);
    const PainterPath flatTie = tieSeg->ldata()->path();
    const PainterPath expectedFlatTie = buildLegacyFlatLensPath(tieSeg, PointF(0.0, tieSeg->ldata()->midThickness()));
    expectPathsEqual(flatTie, expectedFlatTie);

    score->style().set(Sid::tieEndStyle, int(SlurTieEndStyle::ROUND));
    SlurTieLayout::computeBezier(tieSeg);
    const PainterPath roundTie = tieSeg->ldata()->path();
    EXPECT_GT(roundTie.elementCount(), flatTie.elementCount());

    const double tieEndHalfW = 0.5 * tieSeg->endWidth();
    const PointF start = tieSeg->ups(Grip::START).pos();
    const PointF c1 = tieSeg->ups(Grip::BEZIER1).pos();
    const PointF c2 = tieSeg->ups(Grip::BEZIER2).pos();
    const PointF end = tieSeg->ups(Grip::END).pos();
    const PointF startOut = (start - c1) / std::hypot(start.x() - c1.x(), start.y() - c1.y());
    const PointF endOut = (end - c2) / std::hypot(end.x() - c2.x(), end.y() - c2.y());

    EXPECT_NEAR(maxProjectionAlong(roundTie, start, startOut) - maxProjectionAlong(flatTie, start, startOut),
                tieEndHalfW, 1e-4);
    EXPECT_NEAR(maxProjectionAlong(roundTie, end, endOut) - maxProjectionAlong(flatTie, end, endOut),
                tieEndHalfW, 1e-4);
    EXPECT_TRUE(roundTie.boundingRect().contains(flatTie.boundingRect()));

    // Dotted tie is untouched by tieEndStyle.
    tieSeg->tie()->setStyleType(SlurStyleType::Dotted);
    score->style().set(Sid::tieEndStyle, int(SlurTieEndStyle::FLAT));
    SlurTieLayout::computeBezier(tieSeg);
    const PainterPath dottedFlat = tieSeg->ldata()->path();
    score->style().set(Sid::tieEndStyle, int(SlurTieEndStyle::ROUND));
    SlurTieLayout::computeBezier(tieSeg);
    expectPathsEqual(tieSeg->ldata()->path(), dottedFlat);

    // 2. Solid slur (single-cubic lens): FLAT equals legacy path; ROUND extends by endHalfWidth.
    SlurSegment* slurSeg = makeStandaloneSlurSegment(score.get(), 6.0, false);
    score->style().set(Sid::slurEndStyle, int(SlurTieEndStyle::FLAT));
    SlurTieLayout::computeBezier(slurSeg);
    const PainterPath flatSlur = slurSeg->ldata()->path();
    const PainterPath expectedFlatSlur
        = buildLegacyFlatLensPath(slurSeg, PointF(0.0, slurSeg->ldata()->midThickness()));
    expectPathsEqual(flatSlur, expectedFlatSlur);

    score->style().set(Sid::slurEndStyle, int(SlurTieEndStyle::ROUND));
    SlurTieLayout::computeBezier(slurSeg);
    const PainterPath roundSlur = slurSeg->ldata()->path();
    EXPECT_GT(roundSlur.elementCount(), flatSlur.elementCount());
    EXPECT_TRUE(roundSlur.boundingRect().contains(flatSlur.boundingRect()));

    const double slurEndHalfW = 0.5 * slurSeg->endWidth();
    const PointF sStart = slurSeg->ups(Grip::START).pos();
    const PointF sC1 = slurSeg->ups(Grip::BEZIER1).pos();
    const PointF sC2 = slurSeg->ups(Grip::BEZIER2).pos();
    const PointF sEnd = slurSeg->ups(Grip::END).pos();
    const PointF sStartOut = (sStart - sC1) / std::hypot(sStart.x() - sC1.x(), sStart.y() - sC1.y());
    const PointF sEndOut = (sEnd - sC2) / std::hypot(sEnd.x() - sC2.x(), sEnd.y() - sC2.y());
    EXPECT_NEAR(maxProjectionAlong(roundSlur, sStart, sStartOut) - maxProjectionAlong(flatSlur, sStart, sStartOut),
                slurEndHalfW, 1e-4);
    EXPECT_NEAR(maxProjectionAlong(roundSlur, sEnd, sEndOut) - maxProjectionAlong(flatSlur, sEnd, sEndOut),
                slurEndHalfW, 1e-4);

    // 3. Solid multi-bezier slur (envelope builder): shares appendRoundCap cleanly.
    SlurSegment* multiSlur = makeStandaloneSlurSegment(score.get(), 8.0, true);
    score->style().set(Sid::slurEndStyle, int(SlurTieEndStyle::FLAT));
    SlurTieLayout::computeBezier(multiSlur);
    const PainterPath flatMulti = multiSlur->ldata()->path();

    score->style().set(Sid::slurEndStyle, int(SlurTieEndStyle::ROUND));
    SlurTieLayout::computeBezier(multiSlur);
    const PainterPath roundMulti = multiSlur->ldata()->path();
    EXPECT_GT(roundMulti.elementCount(), flatMulti.elementCount());
    EXPECT_TRUE(roundMulti.boundingRect().contains(flatMulti.boundingRect()));

    delete tieSeg->tie();
    delete slurSeg->slur();
    delete multiSlur->slur();
}

TEST_F(Engraving_TieStyleTests, gapAtSignatures)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(SIG_GAPS_SCORE));
    ASSERT_NE(score, nullptr);

    // Identify ties by their start tick:
    //  0: measure 1 beat 1 -> beat 2 (no signature crossed)
    //  1: measure 1 beat 3 -> measure 2 beat 1 (crosses D-major KeySig)
    //  2: measure 2 beat 3 -> measure 3 beat 1 (crosses 3/4 TimeSig)
    //  3: measure 3 beat 3 -> measure 4 beat 1 (crosses system break with courtesy KeySig + TimeSig)
    auto findTieAtTick = [&](const Fraction& tick,
                             SpannerSegmentType segType = SpannerSegmentType::SINGLE) -> TieSegment* {
        for (TieSegment* seg : collectTieSegments(score.get())) {
            if (seg->tie()->tick() == tick && seg->spannerSegmentType() == segType) {
                return seg;
            }
        }
        return nullptr;
    };

    const Fraction tickInternal(0, 1);
    const Fraction tickKeyChange(1, 2);
    const Fraction tickTimeChange(3, 2);
    const Fraction tickSystemBreak(5, 2);

    // Defaults off: no tie gets any mask.
    EXPECT_FALSE(score->style().styleB(Sid::tieGapAtKeySig));
    EXPECT_FALSE(score->style().styleB(Sid::tieGapAtTimeSig));
    for (const TieSegment* seg : collectTieSegments(score.get())) {
        EXPECT_TRUE(seg->ldata()->mask().empty());
    }

    // Key signature gap only.
    score->style().set(Sid::tieGapAtKeySig, true);
    relayout(score.get());

    TieSegment* internalTie = findTieAtTick(tickInternal);
    TieSegment* keyChangeTie = findTieAtTick(tickKeyChange);
    TieSegment* timeChangeTie = findTieAtTick(tickTimeChange);
    ASSERT_NE(internalTie, nullptr);
    ASSERT_NE(keyChangeTie, nullptr);
    ASSERT_NE(timeChangeTie, nullptr);

    EXPECT_TRUE(internalTie->ldata()->mask().empty());
    EXPECT_TRUE(timeChangeTie->ldata()->mask().empty());
    ASSERT_EQ(keyChangeTie->ldata()->mask().size(), 1u);

    Measure* m2 = score->firstMeasure()->nextMeasure();
    ASSERT_NE(m2, nullptr);
    Segment* keySeg = m2->findSegment(SegmentType::KeySig, m2->tick());
    ASSERT_NE(keySeg, nullptr);
    const KeySig* keySig = toKeySig(keySeg->element(0));
    ASSERT_NE(keySig, nullptr);

    const double keyPad = score->style().styleS(Sid::tieGapAtKeySigPadding).val() * keySig->spatium();
    const RectF expectedKeyMask = keySig->shape().translated(keySig->pagePos()).padded(keyPad).bbox()
                                  .translated(-keyChangeTie->pagePos());
    const RectF actualKeyMask = keyChangeTie->ldata()->mask().bbox();
    EXPECT_NEAR(actualKeyMask.left(), expectedKeyMask.left(), 1e-6);
    EXPECT_NEAR(actualKeyMask.right(), expectedKeyMask.right(), 1e-6);
    EXPECT_NEAR(actualKeyMask.top(), expectedKeyMask.top(), 1e-6);
    EXPECT_NEAR(actualKeyMask.bottom(), expectedKeyMask.bottom(), 1e-6);

    // Raising padding widens the mask rectangle accordingly.
    score->style().set(Sid::tieGapAtKeySigPadding, Spatium(0.30));
    relayout(score.get());
    keyChangeTie = findTieAtTick(tickKeyChange);
    ASSERT_NE(keyChangeTie, nullptr);
    ASSERT_EQ(keyChangeTie->ldata()->mask().size(), 1u);
    const double widerPad = 0.30 * keySig->spatium();
    EXPECT_NEAR(keyChangeTie->ldata()->mask().bbox().width(),
                keySig->shape().bbox().width() + 2.0 * widerPad, 1e-6);

    // Time signature gap only: key tie's old mask is cleared, time tie gets masked.
    score->style().set(Sid::tieGapAtKeySig, false);
    score->style().set(Sid::tieGapAtTimeSig, true);
    relayout(score.get());

    internalTie = findTieAtTick(tickInternal);
    keyChangeTie = findTieAtTick(tickKeyChange);
    timeChangeTie = findTieAtTick(tickTimeChange);
    ASSERT_NE(internalTie, nullptr);
    ASSERT_NE(keyChangeTie, nullptr);
    ASSERT_NE(timeChangeTie, nullptr);

    EXPECT_TRUE(internalTie->ldata()->mask().empty());
    EXPECT_TRUE(keyChangeTie->ldata()->mask().empty());
    ASSERT_EQ(timeChangeTie->ldata()->mask().size(), 1u);

    Measure* m3 = m2->nextMeasure();
    ASSERT_NE(m3, nullptr);
    Segment* timeSeg = m3->findSegment(SegmentType::TimeSig, m3->tick());
    ASSERT_NE(timeSeg, nullptr);
    const TimeSig* timeSig = toTimeSig(timeSeg->element(0));
    ASSERT_NE(timeSig, nullptr);

    const double timePad = score->style().styleS(Sid::tieGapAtTimeSigPadding).val() * timeSig->spatium();
    const RectF expectedTimeMask = timeSig->shape().translated(timeSig->pagePos()).padded(timePad).bbox()
                                   .translated(-timeChangeTie->pagePos());
    const RectF actualTimeMask = timeChangeTie->ldata()->mask().bbox();
    EXPECT_NEAR(actualTimeMask.left(), expectedTimeMask.left(), 1e-6);
    EXPECT_NEAR(actualTimeMask.right(), expectedTimeMask.right(), 1e-6);
    EXPECT_NEAR(actualTimeMask.top(), expectedTimeMask.top(), 1e-6);
    EXPECT_NEAR(actualTimeMask.bottom(), expectedTimeMask.bottom(), 1e-6);

    // Both options on: mid-system signatures are masked, and an outgoing tie extended past the
    // system-end barline across courtesy KeySigAnnounce + TimeSigAnnounce masks both of them.
    score->style().set(Sid::tieGapAtKeySig, true);
    relayout(score.get());

    keyChangeTie = findTieAtTick(tickKeyChange);
    timeChangeTie = findTieAtTick(tickTimeChange);
    TieSegment* courtesyTie = findTieAtTick(tickSystemBreak, SpannerSegmentType::BEGIN);
    ASSERT_NE(keyChangeTie, nullptr);
    ASSERT_NE(timeChangeTie, nullptr);
    ASSERT_NE(courtesyTie, nullptr);
    EXPECT_FALSE(keyChangeTie->ldata()->mask().empty());
    EXPECT_FALSE(timeChangeTie->ldata()->mask().empty());

    // By default a system-break tie stops at EndBarLine before the courtesy signatures; extend
    // its end grip across the courtesy KeySigAnnounce and TimeSigAnnounce to verify both mask.
    EXPECT_TRUE(courtesyTie->ldata()->mask().empty());
    Measure* m3After = score->firstMeasure()->nextMeasure()->nextMeasure();
    ASSERT_NE(m3After, nullptr);
    ASSERT_NE(m3After->findSegment(SegmentType::KeySigAnnounce, m3After->endTick()), nullptr);
    ASSERT_NE(m3After->findSegment(SegmentType::TimeSigAnnounce, m3After->endTick()), nullptr);
    courtesyTie->ups(Grip::END).off = PointF(8.0 * score->style().spatium(), 0.0);
    relayout(score.get());
    courtesyTie = findTieAtTick(tickSystemBreak, SpannerSegmentType::BEGIN);
    ASSERT_NE(courtesyTie, nullptr);
    EXPECT_EQ(courtesyTie->ldata()->mask().size(), 2u);

    // Official export reports TieSignatureGaps when ties are masked.
    {
        std::unique_ptr<MasterScore> exportCopy(ScoreRW::readScore(SIG_GAPS_SCORE));
        ASSERT_NE(exportCopy, nullptr);
        exportCopy->style().set(Sid::tieGapAtKeySig, true);
        relayout(exportCopy.get());
        const OfficialExportReport report = OfficialLayoutBaker::bake(exportCopy.get());
        EXPECT_GE(report.dropped.count(DroppedFeature::TieSignatureGaps), size_t(1));
    }

    // Turning both options off clears every existing tie mask.
    score->style().set(Sid::tieGapAtKeySig, false);
    score->style().set(Sid::tieGapAtTimeSig, false);
    relayout(score.get());
    for (const TieSegment* seg : collectTieSegments(score.get())) {
        EXPECT_TRUE(seg->ldata()->mask().empty());
    }
}
