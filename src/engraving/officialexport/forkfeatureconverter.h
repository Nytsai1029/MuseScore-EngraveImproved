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
#pragma once

#include <functional>

#include "officialexportreport.h"

namespace mu::engraving {
class EngravingItem;
class LayoutSnapshot;
class Score;

//! Turns the working copy of a score into one that official MuseScore Studio can hold.
//! Runs once, on the fork's own layout and before any other layout: what the fork decided by itself and
//! official can store outright (breaks, beam positions, stem and curve directions) is stored, what official
//! has another form for is rewritten in that form, and the fork-only settings are taken out and reported.
//! Where the items end up is not its business: OfficialLayoutBaker pins that afterwards.
class ForkFeatureConverter
{
public:
    ForkFeatureConverter(Score* score, LayoutSnapshot& target, OfficialExportReport& report);

    void convert();

    //! Every item of the score, spanners and what multimeasure rests hide included
    static void forEachItem(Score* score, const std::function<void(EngravingItem*)>& func);

private:
    void pinBreaks();
    void pinBeams();
    void splitSlurs();
    void pinDirections();
    void stripStyle();
    void stripItems();
    void stripItem(EngravingItem* item);

    Score* m_score = nullptr;
    LayoutSnapshot& m_target;
    OfficialExportReport& m_report;
    bool m_maskAllText = false;
    bool m_combinedAccidentals = false;
};
}
