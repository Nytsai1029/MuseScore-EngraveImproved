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

import Muse.Ui 1.0

import ".."

EoPage {
    id: page

    code: "accidentals"
    legacyCodes: ["accidentals"]
    title: qsTrc("EditStyleBase", "Accidentals")
    group: qsTrc("notation/engravingoptions", "Notes and rests")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Accidentals: clearances and padding")

        EoNumber {
            sid: "accidentalVerticalClearance"
            title: qsTrc("notation/engravingoptions", "Vertical clearance between accidentals")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum vertical gap between two accidentals for them to share a column; with less room one of them moves to the next column.")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "accidentalSharpVerticalClearance"
            title: qsTrc("notation/engravingoptions", "Vertical clearance between sharps")
            hint: qsTrc("notation/engravingoptions",
                         "Minimum vertical gap between two sharps for them to share a column; sharps may sit closer together than other accidentals.")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "accidentalChordVerticalClearance"
            title: qsTrc("notation/engravingoptions", "Vertical clearance from noteheads and stems")
            hint: qsTrc("notation/engravingoptions",
                         "Vertical gap an accidental must keep from a notehead, stem or ledger line before it may slide past it instead of stopping beside it.")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "accidentalNaturalExtraPadding"
            title: qsTrc("notation/engravingoptions", "Extra padding for naturals")
            hint: qsTrc("notation/engravingoptions",
                         "Extra horizontal space between a natural and a neighbouring stem, natural or flat, so that their vertical strokes do not seem to merge.")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "accidentalLedgerLinePadding"
            title: qsTrc("notation/engravingoptions", "Sharp and natural to ledger line")
            hint: qsTrc("notation/engravingoptions",
                         "Gap between a sharp or natural and the end of a ledger line at its own height; flats may reach over ledger lines and are not affected.")
            min: -0.5
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "accidentalFlushToLedgerLineInset"
            title: qsTrc("notation/engravingoptions", "Flush accidental overlap into ledger line")
            hint: qsTrc("notation/engravingoptions",
                         "With ‘flush to ledger line’ enabled, how far the accidental reaches into the end of the ledger line so that the two join without a visible gap.")
            min: -0.5
            max: 0.5
            step: 0.01
        }

        EoNumber {
            sid: "accidentalFlatLowerNotePadding"
            title: qsTrc("notation/engravingoptions", "Flat to lower notehead")
            hint: qsTrc("notation/engravingoptions",
                         "Distance between a flat and a notehead that lies lower than the flat’s own note, which the flat may approach more closely than the normal accidental distance.")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "bracketedAccidentalBraceYOffset"
            title: qsTrc("notation/engravingoptions", "Vertical offset of braces")
            hint: qsTrc("notation/engravingoptions",
                         "Moves the braces that enclose an accidental down by this amount relative to the accidental.")
            min: -2
            max: 2
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Accidentals: kerning")

        EoToggle {
            sid: "accidentalKernFourths"
            title: qsTrc("notation/engravingoptions", "Tuck flats a fourth apart")
            hint: qsTrc("notation/engravingoptions",
                         "Tucks a flat beneath the flat or natural a fourth above it instead of giving it a column of its own.")
        }

        EoNumber {
            sid: "accidentalFlatKerningOfFourth"
            title: qsTrc("notation/engravingoptions", "Kerning of flats a fourth apart")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal gap between a flat and the flat a fourth above it when the lower one is tucked beneath; a negative value makes them overlap.")
            min: -1
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "accidentalNaturalKerningOfFourth"
            title: qsTrc("notation/engravingoptions", "Kerning of a flat below a natural")
            hint: qsTrc("notation/engravingoptions",
                         "Horizontal gap between a flat and the natural a fourth above it when the flat is tucked beneath; a negative value makes them overlap.")
            min: -1
            max: 1
            step: 0.01
        }

        EoToggle {
            sid: "accidentalNaturalSixthsShareColumn"
            title: qsTrc("notation/engravingoptions", "Naturals a sixth apart share a column")
            hint: qsTrc("notation/engravingoptions",
                         "Lets two naturals a sixth apart stand in the same column although their outlines interlock.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Accidentals: stacking order and alignment")

        EoNumber {
            sid: "accidentalStackSplitInterval"
            title: qsTrc("notation/engravingoptions", "Interval that starts a new stack")
            hint: qsTrc("notation/engravingoptions",
                         "Number of staff positions between vertically adjacent accidentals from which the lower one starts a new stack close to the notes (6 = a seventh).")
            min: 1
            max: 20
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "accidentalSmallGroupLimit"
            title: qsTrc("notation/engravingoptions", "Largest group in zigzag order")
            hint: qsTrc("notation/engravingoptions",
                         "Groups of up to this many accidentals are stacked in the standard order: top, bottom, second from top and so on. Neighbouring stacks no larger than this in total are also merged.")
            max: 12
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "accidentalLargeGroupLimit"
            title: qsTrc("notation/engravingoptions", "Largest group before compact order")
            hint: qsTrc("notation/engravingoptions",
                         "Groups of more accidentals than this are stacked compactly, filling each column with as many as fit. Groups between the two limits use whichever order needs fewer columns.")
            max: 20
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "accidentalDisplacementSplitThreshold"
            title: qsTrc("notation/engravingoptions", "Split threshold for displaced accidentals")
            hint: qsTrc("notation/engravingoptions",
                         "When the closest positions that the accidentals of a chord can reach differ by more than this, those that can sit further right are stacked first as a separate group.")
            max: 3
        }

        EoNumber {
            sid: "accidentalColumnAlignThreshold"
            title: qsTrc("notation/engravingoptions", "Column alignment threshold")
            hint: qsTrc("notation/engravingoptions",
                         "Accidentals of the same kind in the same column are aligned vertically when their horizontal positions differ by less than this.")
            max: 3
        }

        EoNumber {
            sid: "accidentalOffsetOctaveAlignMaxDistance"
            title: qsTrc("notation/engravingoptions", "Maximum distance for aligning offset octaves")
            hint: qsTrc("notation/engravingoptions",
                         "With ‘align offset octave accidentals’ enabled, accidentals an octave apart are aligned only when their horizontal positions differ by less than this.")
            max: 6
            step: 0.1
        }
    }
}
