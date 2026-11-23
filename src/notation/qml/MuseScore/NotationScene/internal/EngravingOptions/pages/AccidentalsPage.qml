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
        title: qsTrc("notation/engravingoptions", "Distances")

        EoNumber {
            sid: "accidentalNoteDistance"
            music: "fis'4"
            title: qsTrc("EditStyleBase", "Accidental to note distance:")
            hint: qsTrc("notation/engravingoptions", "Gap between an accidental and the notehead to its right.")
            step: 0.01
        }

        EoNumber {
            sid: "accidentalDistance"
            music: "<cis' d'>4 <ees' f'>4 cis''4 <gis' a'>4"
            title: qsTrc("EditStyleBase", "Accidental to accidental distance:")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal gap between accidentals which stand in neighbouring columns of a chord.")
            step: 0.01
        }

        EoNumber {
            sid: "accidentalVerticalClearance"
            music: "<cis' e' gis'>4 <des' fes' bes'>4 <c' eis' g' bis'>4 <fis' ais' cis'' fis''>4"
            title: qsTrc("notation/engravingoptions", "Vertical clearance between accidentals")
            hint: qsTrc("notation/engravingoptions",
                        "Two accidentals may stand in the same column when they are at least this far apart " +
                        "vertically; with less room one of them moves to the next column.")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "accidentalSharpVerticalClearance"
            music: "<fis' eis''>4"
            title: qsTrc("notation/engravingoptions", "Vertical clearance between sharps")
            hint: qsTrc("notation/engravingoptions",
                        "The same for two sharps, which may sit closer above one another than other accidentals.")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "accidentalChordVerticalClearance"
            music: "\\stemDown <c'' d'' gis''>4"
            title: qsTrc("notation/engravingoptions", "Vertical clearance from noteheads and stems")
            hint: qsTrc("notation/engravingoptions",
                        "An accidental may slide past a notehead, stem or ledger line of the chord, instead of " +
                        "stopping to the left of it, when it is at least this far above or below it.")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "accidentalNaturalExtraPadding"
            music: "<fis' a' cis''>4 c''4 bis'4"
            title: qsTrc("notation/engravingoptions", "Extra padding for naturals")
            hint: qsTrc("notation/engravingoptions",
                        "Extra horizontal space between a natural and a stem, or a natural or flat close above or " +
                        "below it, so that their upright strokes do not seem to run together.")
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "accidentalFlatLowerNotePadding"
            music: "\\stemUp <c'' des''>4"
            title: qsTrc("notation/engravingoptions", "Flat to lower notehead")
            hint: qsTrc("notation/engravingoptions",
                        "A flat may come this close to a notehead which lies lower than its own note, instead of " +
                        "keeping ‘Accidental to note distance’.")
            max: 1
            step: 0.01
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Accidentals and ledger lines")

        EoToggle {
            sid: "accidentalFlushToLedgerLine"
            music: "ais''4 <c''' eis'''>4"
            title: qsTrc("notation/editstyle/accidentals", "Keep rightmost accidental flush with ledger lines")
            hint: qsTrc("notation/engravingoptions",
                        "The accidental of a note on a ledger line is set right against the end of the ledger line, " +
                        "with no gap; when off, it keeps the distances below.")
        }

        EoNumber {
            sid: "accidentalFlushToLedgerLineInset"
            music: "ais''4"
            title: qsTrc("notation/engravingoptions", "Overlap of a flush accidental with the ledger line")
            hint: qsTrc("notation/engravingoptions",
                        "How far the accidental reaches into the end of the ledger line, so that the two join " +
                        "without a hairline gap. Only with ‘Keep rightmost accidental flush with ledger lines’ on.")
            min: -0.5
            max: 0.5
            step: 0.01
        }

        EoNumber {
            sid: "accidentalLedgerLinePadding"
            music: "fis'''4"
            title: qsTrc("notation/engravingoptions", "Smallest gap from a sharp or natural to a ledger line")
            hint: qsTrc("notation/engravingoptions",
                        "A sharp or natural never comes closer than this to the end of a ledger line which passes " +
                        "anywhere between its top and bottom, even where its outline would leave room; flats may " +
                        "reach over ledger lines. Not used while ‘Keep rightmost accidental flush with ledger lines’ " +
                        "is on.")
            min: -0.5
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "accidentalLedgerLineDistance"
            music: "<cis''' e'''>4"
            title: qsTrc("notation/engravingoptions", "Gap between an accidental and a ledger line")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal gap between an accidental and the end of a ledger line beside it. A flat keeps " +
                        "no gap from a ledger line below its own note, which its bowl may touch.")
            max: 2
            step: 0.01
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Stacking in chords")

        EoToggle {
            sid: "keepAccidentalSecondsTogether"
            music: "<cis' dis' fis' ais'>4"
            title: qsTrc("notation", "Pair accidentals on seconds")
            hint: qsTrc("notation/engravingoptions",
                        "The accidentals of two notes a second apart are placed one directly after the other, in " +
                        "neighbouring columns, instead of being separated by the usual stacking order.")
        }

        EoToggle {
            sid: "alignAccidentalOctavesAcrossSubChords"
            music: "<dis' fis' gis' fis''>4"
            title: qsTrc("notation", "Align accidentals an octave apart across subchords")
            hint: qsTrc("notation/engravingoptions",
                        "In a wide chord the accidentals are stacked in separate groups (see ‘Interval that starts a " +
                        "new stack’). With this on, groups which contain the same accidental an octave apart are " +
                        "stacked together, so that the two stand in one column.")
        }

        EoToggle {
            sid: "alignOffsetOctaveAccidentals"
            music: "<fis' g' fis''>4"
            title: qsTrc("notation", "Align accidentals on offset octaves")
            hint: qsTrc("notation/engravingoptions",
                        "Two accidentals of the same kind an octave apart are lined up in one column even when they " +
                        "would stand at different distances from the chord, for instance because one of the notes is " +
                        "displaced by a second.")
        }

        EoToggle {
            sid: "accidentalOrderFollowsNoteDisplacement"
            music: "<cis' dis' fis' gis'>4"
            title: qsTrc("notation", "Keep accidentals close to offset notes")
            hint: qsTrc("notation/engravingoptions",
                        "In a chord with noteheads on both sides of the stem, the accidentals are placed one column " +
                        "of noteheads after the other, beginning with the noteheads on the right, so that each " +
                        "accidental stays as near as possible to its own notehead.")
        }

        EoNumber {
            sid: "accidentalStackSplitInterval"
            music: "<cis' eis' dis'' fis''>4"
            title: qsTrc("notation/engravingoptions", "Interval that starts a new stack")
            hint: qsTrc("notation/engravingoptions",
                        "When two vertically neighbouring accidentals of a chord are at least this many staff " +
                        "positions apart (6 = a seventh), the lower one starts a new stack close to the notes " +
                        "instead of continuing the columns above.")
            min: 1
            max: 20
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "accidentalSmallGroupLimit"
            music: "<cis' eis' gis' bis'>4"
            title: qsTrc("notation/engravingoptions", "Largest stack placed in zigzag order")
            hint: qsTrc("notation/engravingoptions",
                        "Stacks of up to this many accidentals are placed in the classical order: top, bottom, " +
                        "second from the top, and so on. Two neighbouring stacks which together are no larger than " +
                        "this are treated as one.")
            max: 12
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "accidentalLargeGroupLimit"
            music: "<cis' eis' gis' bis' dis'' fis'' ais''>4"
            title: qsTrc("notation/engravingoptions", "Largest stack before the compact order")
            hint: qsTrc("notation/engravingoptions",
                        "Stacks of more accidentals than this are packed compactly, each column taking as many " +
                        "accidentals as fit. Stacks between the two limits use whichever order needs fewer columns.")
            max: 20
            step: 1
            decimals: 0
        }

        EoNumber {
            sid: "accidentalDisplacementSplitThreshold"
            music: "<< { <cis'' eis''>4 r r r } \\\\ { <dis'' fis''>4 r r r } >>"
            title: qsTrc("notation/engravingoptions", "Difference in position which splits a stack")
            hint: qsTrc("notation/engravingoptions",
                        "When some accidentals of a chord can come closer to the notes than the others by more than " +
                        "this (past a ledger line, or beside a notehead displaced by a second), they are placed " +
                        "first as a stack of their own and the others follow to their left.")
            max: 3
        }

        EoNumber {
            sid: "accidentalColumnAlignThreshold"
            music: "<ees' des'' ees''>4"
            title: qsTrc("notation/engravingoptions", "Tolerance for aligning a column")
            hint: qsTrc("notation/engravingoptions",
                        "Accidentals of the same kind which stand in the same column are set exactly above one " +
                        "another when their horizontal positions differ by less than this; with a larger difference " +
                        "each keeps its own position.")
            max: 3
        }

        EoNumber {
            sid: "accidentalOffsetOctaveAlignMaxDistance"
            music: "<fis' ais' fis''>4"
            title: qsTrc("notation/engravingoptions", "Largest difference for aligning offset octaves")
            hint: qsTrc("notation/engravingoptions",
                        "With ‘Align accidentals on offset octaves’ on, two accidentals an octave apart are lined up " +
                        "only when their horizontal positions differ by less than this.")
            max: 6
            step: 0.1
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Kerning")
        hint: qsTrc("notation/engravingoptions", "How accidentals may tuck under one another to save a column.")
        music: "<ees' aes'>4"

        EoToggle {
            sid: "accidentalKernFourths"
            music: "<ees' aes'>4"
            title: qsTrc("notation/engravingoptions", "Tuck flats a fourth apart")
            hint: qsTrc("notation/engravingoptions",
                        "A flat is tucked partly under the flat or natural a fourth above it instead of standing " +
                        "clear to its left.")
        }

        EoNumber {
            sid: "accidentalFlatKerningOfFourth"
            music: "<ees' aes'>4"
            title: qsTrc("notation/engravingoptions", "Gap of a flat tucked under a flat")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal gap between the lower flat and the flat a fourth above it; a negative value " +
                        "makes them overlap. Only with ‘Tuck flats a fourth apart’ on.")
            min: -1
            max: 1
            step: 0.01
        }

        EoNumber {
            sid: "accidentalNaturalKerningOfFourth"
            music: "<ees' a'!>4"
            title: qsTrc("notation/engravingoptions", "Gap of a flat tucked under a natural")
            hint: qsTrc("notation/engravingoptions",
                        "Horizontal gap between the flat and the natural a fourth above it; a negative value makes " +
                        "them overlap. Only with ‘Tuck flats a fourth apart’ on.")
            min: -1
            max: 1
            step: 0.01
        }

        EoToggle {
            sid: "accidentalNaturalSixthsShareColumn"
            music: "<e'! c''!>4"
            title: qsTrc("notation/engravingoptions", "Naturals a sixth apart share a column")
            hint: qsTrc("notation/engravingoptions",
                        "Two naturals a sixth apart stand in the same column and interlock, although their outlines " +
                        "are closer than ‘Vertical clearance between accidentals’.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Cautionary accidentals and parentheses")

        EoToggle {
            sid: "showCautionaryAccidentals"
            music: "fis''4 | f''4"
            title: qsTrc("notation/editstyle/accidentals", "Show cautionary accidentals")
            hint: qsTrc("notation/engravingoptions",
                        "A note which was altered in the previous measure gets a reminder accidental the first time " +
                        "it comes back in the next measure; not after a key change or a section break.")
        }

        EoToggle {
            sid: "cautionaryAccidentalsInParentheses"
            music: "fis''4 | f''4"
            musicStyle: ({ showCautionaryAccidentals: true })
            title: qsTrc("notation/editstyle/accidentals", "Put cautionary accidentals in parentheses")
            hint: qsTrc("notation/engravingoptions", "Only with ‘Show cautionary accidentals’ on.")
            enabled: Boolean(page.value("showCautionaryAccidentals"))
        }

        EoToggle {
            sid: "bracketedAccidentalUseSmuflSym"
            music: "cis''?4 bes'?4"
            musicStyle: ({ musicalSymbolFont: "Bravura" })
            title: qsTrc("notation/editstyle/accidentals", "Use SMuFL symbols with built-in parentheses")
            hint: qsTrc("notation/engravingoptions",
                        "A flat, natural, sharp or double accidental in parentheses is drawn with the music font’s " +
                        "ready-made symbol, in which the parentheses hug the accidental; when off, separate " +
                        "parentheses are set around it with ‘Padding inside parentheses’.")
        }

        EoNumber {
            sid: "bracketedAccidentalPadding"
            music: "cis''?4 bes'?4"
            title: qsTrc("notation/editstyle/accidentals", "Padding inside parentheses:")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between an accidental and each of its parentheses, square brackets or braces. No effect " +
                        "on an accidental drawn with a ready-made parenthesised symbol.")
            min: -10
            max: 10
            step: 0.1
            decimals: 3
        }

        EoNumber {
            sid: "bracketedAccidentalBraceYOffset"
            music: "cis''*4"
            title: qsTrc("notation/engravingoptions", "Downward shift of braces around an accidental")
            hint: qsTrc("notation/engravingoptions",
                        "Braces around an accidental are set this much lower than the accidental; parentheses and " +
                        "square brackets are not affected.")
            min: -2
            max: 2
        }
    }
}
