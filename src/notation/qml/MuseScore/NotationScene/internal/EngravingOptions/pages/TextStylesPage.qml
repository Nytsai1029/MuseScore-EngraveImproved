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

    code: "text-styles"
    title: qsTrc("EditStyleBase", "Text styles")
    group: qsTrc("notation/engravingoptions", "Text")
    view: EoTextStylesView {}

    //! NOTE: the music which the preview shows for the values of each text style (a style which no music can show
    //! yet, as the header, the footer and the user styles, has none)
    property list<EoTextStyleMusic> musics: [
        EoTextStyleMusic {
            styleCode: "title"
            music: "\\title \"Sonata\" c''1"
        },
        EoTextStyleMusic {
            styleCode: "measure-number"
            music: "c''2 c''2 | c''1 | c''1"
            musicStyle: ({ showMeasureNumber: true, measureNumberSystem: false, measureNumberInterval: 1 })
        },
        EoTextStyleMusic {
            styleCode: "multimeasure-rest-range"
            music: "c''1 | R1 | R1 | R1 | c''1"
            musicStyle: ({ createMultiMeasureRests: true, mmRestShowMeasureNumberRange: true })
        },
        EoTextStyleMusic {
            styleCode: "tempo"
            music: "c''4 \\tempo \"Allegro\" e''4 g''4 e''4"
        },
        EoTextStyleMusic {
            styleCode: "tempo-change"
            music: "c''4 \\startTempoChange rit e''4 g''4 e''4 \\stopTempoChange | c''1"
        },
        EoTextStyleMusic {
            styleCode: "repeat-play-count"
            music: "\\bar \".|:\" c''2 c''2 \\bar \":|.\" \\playCount 3 c''2 c''2"
        },
        EoTextStyleMusic {
            styleCode: "repeat-text-left"
            music: "\\segno c''2 c''2 | c''2 c''2"
        },
        EoTextStyleMusic {
            styleCode: "repeat-text-right"
            music: "c''2 c''2 | c''2 c''2 \\jump \"D.C.\""
        },
        EoTextStyleMusic {
            styleCode: "rehearsal-mark"
            music: "c''1 | c''4 \\mark \"B\" e''4 g''4 e''4"
        },
        EoTextStyleMusic {
            styleCode: "system"
            music: "c''4 \\systemText \"Tutti\" e''4 g''4 e''4"
        },
        EoTextStyleMusic {
            styleCode: "staff"
            music: "c''4^\"pizz.\" e''4 g''4 e''4"
        },
        EoTextStyleMusic {
            styleCode: "expression"
            music: "c'4 \\p \\expression \"dolce\" e'4 g'4 e'4"
        },
        EoTextStyleMusic {
            styleCode: "hairpin"
            music: "c''4 \\cresc d''4 e''4 f''4 \\!"
        },
        EoTextStyleMusic {
            styleCode: "lyrics-odd-lines"
            music: "c''4_\"Glo-\"_\"Sing\" d''4_\"ri-\"_\"a\" e''2_\"a\"_\"song\""
        },
        EoTextStyleMusic {
            styleCode: "lyrics-even-lines"
            music: "c''4_\"Glo-\"_\"Sing\" d''4_\"ri-\"_\"a\" e''2_\"a\"_\"song\""
        },
        EoTextStyleMusic {
            styleCode: "chord-symbols"
            music: "c''2 \\chordSymbol \"C7(b9)\" g'2 \\chordSymbol \"Fmaj7\""
        },
        EoTextStyleMusic {
            styleCode: "chord-symbols-alternate"
            music: "c''2 \\chordSymbol \"C7(b9)\" alternate g'2 \\chordSymbol \"Fmaj7\" alternate"
        },
        EoTextStyleMusic {
            styleCode: "roman-numeral-analysis"
            music: "c'4 \\romanNumeral \"I\" f'4 \\romanNumeral \"IV\" g'4 \\romanNumeral \"V7\" c'4 \\romanNumeral " +
                   "\"I\""
        },
        EoTextStyleMusic {
            styleCode: "nashville-number"
            music: "c''4 \\nashville \"1\" f''4 \\nashville \"4\" g''4 \\nashville \"5\" c''4 \\nashville \"1\""
        },
        EoTextStyleMusic {
            styleCode: "tuplet"
            music: "\\tuplet 3/2 { c''8 d''8 e''8 } f''4 g''2"
        },
        EoTextStyleMusic {
            styleCode: "sticking"
            music: "c''4 \\sticking \"R\" d''4 \\sticking \"L\" e''2 \\sticking \"R\""
        },
        EoTextStyleMusic {
            styleCode: "fingering"
            music: "c''4-1 d''4-2 e''4-3 f''4-4"
        },
        EoTextStyleMusic {
            styleCode: "tab-fret-number"
            music: "\\tab e'4 g'4 a'4 g'4"
        },
        EoTextStyleMusic {
            styleCode: "hammer-ons-pull-offs-and-tapping"
            music: "\\hopoOn e'4( g'4 a'4 g'4) \\hopoOff"
        },
        EoTextStyleMusic {
            styleCode: "string-tunings"
            music: "\\tab common e'1 \\stringTunings"
        },
        EoTextStyleMusic {
            styleCode: "fretboard-diagram-fingering"
            music: "c''2 \\fretDiagram \"x32010\" fingers \"032010\" r2"
            musicStyle: ({ fretShowFingerings: true })
        },
        EoTextStyleMusic {
            styleCode: "fretboard-diagram-fret-number"
            music: "c''2 \\fretDiagram \"x35553\" at 3 r2"
        },
        EoTextStyleMusic {
            styleCode: "harp-pedal-diagram"
            music: "c''4 \\harpPedals e''4 g''4 e''4"
        },
        EoTextStyleMusic {
            styleCode: "harp-pedal-text-diagram"
            music: "c''4 \\harpPedalsText e''4 g''4 e''4"
        },
        EoTextStyleMusic {
            styleCode: "text-line"
            music: "c''4 \\startTextLine \"sul pont.\" d''4 e''4 \\stopTextLine f''4"
        },
        EoTextStyleMusic {
            styleCode: "volta"
            music: "c''2 c''2 | \\volta \"1.\" 1 c''2 c''2 \\bar \":|.\" \\volta \"2.\" 1 c''2 c''2 \\bar \"|.\""
        },
        EoTextStyleMusic {
            styleCode: "ottava"
            music: "c''4 \\startOttava 8va d''4 e''4 \\stopOttava f''4"
            musicStyle: ({ ottavaNumbersOnly: false })
        },
        EoTextStyleMusic {
            styleCode: "glissando"
            music: "c'2 \\glissando c''2"
        },
        EoTextStyleMusic {
            styleCode: "pedal"
            music: "c'4 \\startPedal d'4 e'4 \\stopPedal f'4"
        },
        EoTextStyleMusic {
            styleCode: "bend"
            music: "\\tab a'4 \\bend b'4 r2"
        },
        EoTextStyleMusic {
            styleCode: "let-ring"
            music: "e'4 \\startLetRing g'4 a'4 \\stopLetRing g'4"
        },
        EoTextStyleMusic {
            styleCode: "palm-mute"
            music: "e'4 \\startPalmMute g'4 a'4 \\stopPalmMute g'4"
        }
    ]
}
