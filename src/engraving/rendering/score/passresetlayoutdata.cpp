/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "passresetlayoutdata.h"

#include "dom/engravingitem.h"
#include "dom/measurebase.h"
#include "dom/score.h"
#include "dom/system.h"

#include "layoutcontext.h"

#include "types/types.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

static bool shouldZeroPosOnRangeReset(const EngravingItem* item)
{
    switch (item->type()) {
    case ElementType::STEM:
    case ElementType::HOOK:
    case ElementType::BEAM:
    case ElementType::ARTICULATION:
    case ElementType::ORNAMENT:
    case ElementType::TREMOLO_SINGLECHORD:
    case ElementType::TREMOLO_TWOCHORD:
    case ElementType::ARPEGGIO:
    case ElementType::FERMATA:
    case ElementType::FINGERING:
    case ElementType::ACCIDENTAL:
        return true;
    default:
        return false;
    }
}

static void resetLayoutData(EngravingItem* item)
{
    if (item->ldata()) {
        item->mutldata()->reset();
        if (shouldZeroPosOnRangeReset(item)) {
            // LayoutData::reset() deliberately leaves m_pos. Stem/hook/beam leftover pos
            // makes close-to-note articulations (staccato/tenuto) setPos from stale bboxes.
            item->setPos(PointF());
        }
    }

    for (EngravingItem* ch : item->childrenItems()) {
        resetLayoutData(ch);
    }
}

void PassResetLayoutData::doRun(Score* score, LayoutContext& ctx)
{
    if (ctx.state().isLayoutAll()) {
        resetLayoutData(score->rootItem());
        return;
    }

    // Range layout rebuilds the whole current system (and may reuse later systems on
    // the page), but the dirty tick range is often just one item. Reset every measure
    // on the current system, plus System-owned spanner segments, and zero leftover pos
    // on note-attached marks so slur/hairpin/articulation layout matches a full system layout.
    if (System* system = ctx.mutState().curSystem()) {
        resetLayoutData(system);
        for (MeasureBase* mb : system->measures()) {
            resetLayoutData(mb);
        }
    }

    MeasureBase* m = ctx.mutState().nextMeasure();
    while (m && m->tick() <= ctx.state().endTick()) {
        resetLayoutData(m);
        m = m->next();
    }
}
