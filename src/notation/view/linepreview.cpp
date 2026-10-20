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
#include "linepreview.h"

#include <algorithm>
#include <cmath>

#include "draw/types/brush.h"
#include "draw/types/pen.h"

#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/page.h"
#include "engraving/dom/score.h"
#include "engraving/dom/select.h"
#include "engraving/dom/system.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;
using namespace muse::draw;

//! NOTE The margin of a card around its line, in sp
static constexpr double CARD_MARGIN_SP = 1.5;

//! NOTE In pixels of the screen, at any zoom
static constexpr double CARD_RADIUS = 8.0;
static constexpr int CARD_SHADOW_STEPS = 3;

static const Color CARD_SHADOW_COLOR(0, 0, 0, 12);
static const Color CARD_EDGE_COLOR(0, 0, 0, 46);

std::vector<const System*> mu::notation::wholeSelectedLines(const Score* score)
{
    if (!score) {
        return {};
    }

    const Selection& selection = score->selection();
    if (!selection.isRange()) {
        return {};
    }

    const Fraction start = selection.tickStart();
    const Fraction end = selection.tickEnd();
    if (start < Fraction(0, 1) || end <= start) {
        return {};
    }

    std::vector<const System*> lines;

    for (const System* system : score->systems()) {
        const Measure* first = system->firstMeasure();
        const Measure* last = system->lastMeasure();

        //! NOTE A system of a frame has no measures
        if (!first || !last || last->endTick() <= start) {
            continue;
        }

        if (first->tick() >= end) {
            break;
        }

        if (first->tick() < start || last->endTick() > end) {
            return {};
        }

        lines.push_back(system);
    }

    return lines;
}

RectF mu::notation::linePreviewCardRect(const System* line)
{
    if (!line) {
        return RectF();
    }

    const staff_idx_t firstIdx = line->firstVisibleStaff();
    const staff_idx_t lastIdx = line->lastVisibleStaff();
    if (firstIdx == muse::nidx || lastIdx == muse::nidx) {
        return RectF();
    }

    const SysStaff* firstStaff = line->staff(firstIdx);
    const SysStaff* lastStaff = line->staff(lastIdx);

    //! NOTE The skylines have what is above the first staff and below the last one (tempo, dynamics, lyrics, …),
    //! from the top of the staff
    double top = firstStaff->bbox().top();
    if (firstStaff->skyline().north().valid()) {
        top += std::min(0.0, firstStaff->skyline().north().max());
    }

    double bottom = lastStaff->bbox().bottom();
    if (lastStaff->skyline().south().valid()) {
        bottom = std::max(bottom, lastStaff->bbox().top() + lastStaff->skyline().south().max());
    }

    //! NOTE The system has the names and the brackets of the staves on the left;
    //! the skylines have what sticks out on the sides (e.g. the number of the first measure)
    const RectF box = line->ldata()->bbox();
    double left = box.left();
    double right = box.right();

    for (staff_idx_t staffIdx = firstIdx; staffIdx <= lastIdx; ++staffIdx) {
        const SysStaff* staff = line->staff(staffIdx);
        if (!staff->show()) {
            continue;
        }

        for (const SkylineLine* skylineLine : { &staff->skyline().north(), &staff->skyline().south() }) {
            for (const ShapeElement& element : skylineLine->elements()) {
                left = std::min(left, element.left());
                right = std::max(right, element.right());
            }
        }
    }

    const PointF origin = line->canvasPos();
    RectF rect(origin.x() + left, origin.y() + top, right - left, bottom - top);

    const double margin = CARD_MARGIN_SP * line->spatium();
    rect.adjust(-margin, -margin, margin, margin);

    if (const Page* page = line->page()) {
        rect = rect.intersected(page->canvasBoundingRect());
    }

    return rect;
}

void mu::notation::paintLinePreview(Painter* painter, const INotationPtr& notation,
                                    const std::vector<const System*>& lines)
{
    if (!painter || !notation || lines.empty()) {
        return;
    }

    const double scaling = painter->worldTransform().m11();
    if (scaling <= 0.0) {
        return;
    }

    const double px = 1.0 / scaling;
    const double radius = CARD_RADIUS * px;

    //! NOTE The drawing of the score as it will be printed sets it, and the next drawing of the view
    //! would be the one which sets it back
    const bool wasPdfPrinting = MScore::pdfPrinting;

    for (const System* line : lines) {
        const RectF card = linePreviewCardRect(line);
        if (!card.isValid()) {
            continue;
        }

        painter->save();
        painter->setNoPen();

        //! NOTE A soft shadow, so that the card is seen on a white page too
        for (int i = CARD_SHADOW_STEPS; i > 0; --i) {
            const double spread = i * px;
            painter->setBrush(Brush(CARD_SHADOW_COLOR));
            painter->drawRoundedRect(card.adjusted(-spread, -spread + px, spread, spread + px),
                                     radius + spread, radius + spread);
        }

        painter->setBrush(Brush(Color::WHITE));
        painter->drawRoundedRect(card, radius, radius);

        //! NOTE The line as the preview draws it, inside the round corners
        const double inset = radius * (1.0 - std::sqrt(0.5));
        const RectF content = card.adjusted(inset, inset, -inset, -inset);

        painter->setClipRect(content);
        notation->painting()->paintView(painter, content, true);

        painter->restore();

        painter->save();
        painter->setBrush(BrushStyle::NoBrush);
        painter->setPen(Pen(CARD_EDGE_COLOR, px));
        painter->drawRoundedRect(card, radius, radius);
        painter->restore();
    }

    MScore::pdfPrinting = wasPdfPrinting;
}
