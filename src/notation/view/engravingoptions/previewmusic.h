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
#pragma once

#include <QString>

#include "types/bytearray.h"
#include "types/retval.h"

namespace mu::notation {
//! NOTE The music which a preview of the Engraving Options dialog shows is written in the page which has the
//! option, as text, in a small subset of LilyPond's note entry (octaves are absolute):
//!
//!     "c''2~ c''2 | <c' e' g'>4( d'8 e'8) r4"
//!
//! The text is turned into a score file in memory (see EngravingPreviewRenderer::setMusic).
//!
//! Tokens are separated by white space; `|` ends a measure; a measure is as long as what is in it, and has a time
//! signature only where `\time` asks for one.
//!
//! - A note is a pitch `c d e f g a b`, then `is` (sharp) or `es` (flat) any number of times (`as` and `es`
//!   alone are a-flat and e-flat), then the octave: `c` is C3, `c'` middle C, `c''`, `c,`.
//!   Then `!` (the accidental is shown although it is not needed), `?` (shown in parentheses) or `*` (in braces).
//!   A chord is `<c' e' g'>` (`<c'~ e' g'>`: only the C is tied to the next chord), a rest `r`, a rest as long
//!   as the measure `R` (with the duration of the measure).
//! - The duration follows the pitch or chord: `1 2 4 8 … 1024`, then dots. It is the one before if left out
//!   (the first is a quarter).
//! - After the duration, in any order: `~` (tie to the next chord), `(` and `)` (slur), `[` and `]` (beam from this
//!   chord to that one), `-.` staccato, `->` accent, `--` tenuto, `-^` marcato, `-!` staccatissimo, `-1` to `-5`
//!   fingering, `:8` `:16` `:32` tremolo (`:z` a buzz roll), `_"text"` a lyric syllable (a `-` at the end of the text
//!   joins it to the next syllable of the verse, `__` draws an extension line to the last note before it; each further
//!   `_"text"` of the chord is the next verse), `^"text"` a staff text above the chord (`<sup>…</sup>`, `<sub>`, `<b>`,
//!   `<i>` format the letters between them, as in the text of a score).
//! - After a note, as words: `\fermata`, `\arpeggio` (also `\arpeggioUp|Down|Bracket|UpStraight|DownStraight`),
//!   `\laissezVibrer`, a dynamic (`\ppp \pp \p \mp \mf \f \ff \fff \sf \sfz \fp`; `\atEndOfPrevious` after it puts it
//!   before its beat), `\expression "text"` (as dolce after a p), `\sticking "text"` (the hand of a percussion note, as
//!   R), `\chordSymbol "text"` (a chord symbol, as C7(b9)/E; the text in parentheses, as (F), puts the symbol in
//!   parentheses), `\<` and `\>` (a hairpin begins), `\cresc` and `\dim` (a line with the word begins), `\niente` after
//!   any of these four (it begins or ends at niente, a small circle), `\diagonal y` after them (its end is y staff
//!   spaces lower, as if dragged) and `\!` (it ends). The ornaments `\trill` `\turn` `\mordent` `\prall` (with `!`, as
//!   `\turn!`, the accidentals of the notes they play are shown, also those of the key), `\upbow` `\downbow`,
//!   `\luteFinger thumb|1|2|3` (the right hand in lute music), the lines `\fall` `\doit` `\plop` `\scoop` (curved;
//!   `\fallStraight`, `\fallWavy` and so on), `\breathe` and `\caesura` (after the chord), and
//!   `\handBracket left|right` (the hand which plays the chord, a bracket left of its lowest note).
//!   `\glissando` and `\noteLine` (a note-anchored line) go from the notes of the chord to those of the next one.
//! - Lines over chords: `\start<Name>` after the chord with which the line begins, `\stop<Name>` after the last
//!   chord it lasts through (unlike `\!` of a hairpin, which stands at the chord where it ends). The names:
//!   `Ottava` (with `8va` `8vb` `15ma` `15mb` `22ma` `22mb` after `\startOttava`), `Pedal`, `Trill` (`upprall`
//!   `downprall` `prallprall` may follow `\startTrill`; `\startTrill!` shows the accidental of the upper note,
//!   also one of the key), `Vibrato` (`wide` `sawtooth` `sawtoothWide` may follow), `TextLine` and
//!   `SystemTextLine` (the text in quotes follows), `LetRing`, `PalmMute`, `TempoChange` (`accel` `allarg` `rall`
//!   `rit` follows). After the start of a line which has hooks, `\hooks begin end` gives the hooks of its two ends
//!   (`none` `90` `45` `90t`) and `\noLine` shows only its texts (as Ped. and * of a pedal).
//! - Before a note: `\stemUp` `\stemDown` `\stemNeutral` (until changed), `\noBeam`, `\slash` (slash noteheads, as in
//!   rhythm slash notation) and `\small` (a small cue chord or rest) for this chord only, `\parenthesize` (its notes in
//!   parentheses) and `\parenthesizeChord` (the chord as a whole),
//!   `\grace` `\acciaccatura` `\appoggiatura` (the next chord, or all the chords of `{ ... }` which follow, beamed
//!   together, are grace notes of the one after them), `\slurSolid|Dotted|Dashed|WideDashed` and
//!   `\tieSolid|Dotted|Dashed|WideDashed` (the line of the slurs and ties which begin after it, until changed),
//!   `\tremolo 16 { c'2 e'2 }` (a tremolo between two chords), `\ambitus` (an ambitus before the next note, of the
//!   highest and lowest notes of the music), `\tuplet 3/2 { ... }`, `\clef treble|bass|alto|tenor`, `\key N` (N sharps,
//!   negative: flats), `\time 3/4` (at the beginning of a measure the key and the time signature are on all the staves,
//!   as in a score), `\break` (the system ends after this measure, as if a `|` had come before it), `\pageBreak` (the
//!   page ends after it).
//! - `\bar "type"` where a barline stands (it ends the measure as `|` does, or, after a `|`, gives the barline of the
//!   measure before): `||` `|.` `.|` `!` (dashed) `;` (dotted) `.` (heavy) `..`, and the repeats `.|:` `:|.` `:|.|:`;
//!   `\playCount N` after a `\bar` which ends a repeat: it is played N times. `\volta "1." N` at the beginning of
//!   a measure: a volta with the text over N measures. `\measureRepeat N` at the beginning of a measure: N measures
//!   (1, 2 or 4) which repeat the N before them, after which the music goes on. In a measure: the markers `\segno`
//!   `\coda` (at its beginning), `\fine` `\toCoda` and a jump `\jump "D.C."` (at its end).
//! - Two voices of a whole measure, of the same length: `<< { ... } \\ { ... } >>`.
//! - Two staves (a grand staff) with `\staves 2` at the beginning. `\staff 1|2` says on which staff the notes which
//!   follow are written (at the start of each voice it is staff 1 again), and `\crossStaff down|up|off` shows the
//!   chords which follow on the neighbouring staff. A slur, a tie and a hairpin stay on one staff.
//!   `\tab` at the beginning makes the current staff a six-string tablature, `\lines N` gives it N lines
//!   (1 to 5) instead of five. `\name "long" "short"` at the beginning names the instrument,
//!   `\smallStaff` makes the current staff a small staff, `\spanBarlines` joins the barlines of two staves,
//!   `\hideStavesIndividually` lets the style hide an empty staff of the two although the other has notes.
//!   `\group` with `bracket`, `square`, `line`, `brace` or `none` chooses the bracket over the two staves
//!   (a grand staff has a brace, `none` none at all).
//!   `\instruments 2` at the beginning, instead of `\staves 2`: two instruments of one staff each (with no
//!   bracket, unless `\group` gives one), written on as the two staves of a grand staff are.
//! - `\instrument id` at the beginning: the instrument of the current staff, by its id in the instrument list (as
//!   `voice`), instead of a piano.
//! - `\title "text"` at the beginning: the title of the score, in a frame above the music (as the work title, for
//!   the headers and footers too).
//! - Before the first chord of a beam group: `\beamAt y1 y2` writes the y its ends have, in staff
//!   spaces — which the file carries as a beam the user moved, so it is laid out like one —
//!   and `\beamMove -1|0|1` writes which staff a cross-staff beam counts as belonging to.
//! - `\slurUp`, `\slurDown`, `\slurAuto` give the slurs which begin after them a direction; `\slurFlat` makes them
//!   flat in the middle. `\tapping` and `\tapping left` mark the note before them as tapped (right hand unless
//!   `left` follows), with the half slur a tap has.
//!
//! What is not in it is an error, and so is what is not written as above, with the place in the text.
struct PreviewMusic {
    //! The score file (4.60 .mscx text) of the music. If the music is not valid, `ret` says what is wrong
    //! and at which character of the text
    static muse::RetVal<muse::ByteArray> toMscx(const QString& notation);
};
}
