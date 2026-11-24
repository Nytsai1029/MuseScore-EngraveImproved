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
import QtQuick 2.15

import "pages"

//! NOTE: the pages of the Engraving Options dialog, in the order of its list.
//! A page is added here and nowhere else
QtObject {
    id: root

    //! NOTE: the EngravingOptionsModel
    property var model: null

    readonly property list<QtObject> pages: [
        ScorePage { model: root.model },
        VerticalSpacingPage { model: root.model },
        SpacingPage { model: root.model },
        SizesPage { model: root.model },
        SystemPage { model: root.model },
        HeaderAndFooterPage { model: root.model },
        MeasureNumberPage { model: root.model },
        NotesPage { model: root.model },
        AccidentalsPage { model: root.model },
        StemsPage { model: root.model },
        BeamsPage { model: root.model },
        TremolosPage { model: root.model },
        RestsPage { model: root.model },
        TupletsPage { model: root.model },
        ArpeggiosPage { model: root.model },
        TiesPage { model: root.model },
        SlursPage { model: root.model },
        ClefsKeyAndTimeSignaturesPage { model: root.model },
        BarlinesPage { model: root.model },
        RepeatsPage { model: root.model },
        VoltasPage { model: root.model },
        ArticulationsAndOrnamentsPage { model: root.model },
        FermatasPage { model: root.model },
        DynamicsHairpinsPage { model: root.model },
        FingeringPage { model: root.model },
        ParenthesesPage { model: root.model },
        LyricsPage { model: root.model },
        LinesPage { model: root.model },
        OttavaPage { model: root.model },
        PedalPage { model: root.model },
        TrillPage { model: root.model },
        GlissandoNoteLinePage { model: root.model },
        TextLinePage { model: root.model },
        LetRingAndPalmMutePage { model: root.model },
        BendPage { model: root.model },
        HammerOnsPullOffsAndTappingPage { model: root.model },
        StaffTextPage { model: root.model },
        TempoTextPage { model: root.model },
        RehearsalMarksPage { model: root.model },
        ChordSymbolsPage { model: root.model },
        FretboardDiagramsPage { model: root.model },
        FiguredBassPage { model: root.model },
        TablaturePage { model: root.model },
        AllValuesPage { model: root.model }
    ]

    //! NOTE: the page with this code, or which stands for the page of the style dialog with this code; -1 if none
    function indexOfCode(code) {
        if (!Boolean(code)) {
            return -1
        }

        for (var i = 0; i < pages.length; ++i) {
            if (pages[i].code === code || pages[i].legacyCodes.indexOf(code) >= 0) {
                return i
            }
        }

        return -1
    }
}
