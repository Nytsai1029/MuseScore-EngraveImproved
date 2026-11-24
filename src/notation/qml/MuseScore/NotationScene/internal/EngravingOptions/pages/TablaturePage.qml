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

    code: "tablature"
    legacyCodes: ["tablature-styles"]
    title: qsTrc("EditStyleBase", "Tablature")
    group: qsTrc("notation/engravingoptions", "Text")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Fret numbers")
        music: "\\tab e'''16 f'''16 g'''16 f'''16 e'''8 d'''8 e'''4 c'''4"

        EoNumber {
            sid: "tabFretPadding"
            title: qsTrc("EditStyleBase", "Padding around fret numbers:")
            hint: qsTrc("notation/engravingoptions",
                        "How much of the string line is left blank on each side of a fret number; the same space " +
                        "separates a number from its parentheses.")
            min: 0
            max: 2
            step: 0.05
        }

        EoNumber {
            sid: "tabFretPaddingScale"
            title: qsTrc("notation/engravingoptions", "Minimum gap between fret numbers")
            hint: qsTrc("notation/engravingoptions",
                        "As a multiple of the minimum gap which notes keep from each other on a standard staff: at " +
                        "1.5 tightly spaced fret numbers stand half as far apart again.")
            min: 0.5
            max: 5
            step: 0.1
            decimals: 1
        }

        EoToggle {
            sid: "preferSameStringForTranspose"
            title: qsTrc("EditStyleBase", "Prefer the same string when transposing")
            hint: qsTrc("notation/engravingoptions",
                        "When the music is transposed, each note stays on its string wherever the new fret can be " +
                        "played, instead of being fretted afresh at the lowest position.")
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Tied notes")
        music: "\\tab common e'2 g'2~ | g'2 a'2~ | a'1"

        EoChoice {
            sid: "tabShowTiedFret"
            title: qsTrc("notation/engravingoptions", "Tied fret marks")
            hint: qsTrc("notation/engravingoptions", "What is drawn for a note which is tied to the one before it.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Show ties and repeat fret marks") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Show ties only") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Show initial fret mark only") }
            ]
        }

        EoChoice {
            sid: "tabParenthesizeTiedFret"
            musicStyle: ({ tabShowTiedFret: 1 })
            title: qsTrc("notation/engravingoptions", "Parentheses indicating ties")
            hint: qsTrc("notation/engravingoptions",
                        "Where the fret number of a tied note is left out, it can still be given in parentheses as a " +
                        "reminder after a system break or a barline. Not used with ‘Show ties and repeat fret " +
                        "marks’.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Only show at the start of a system") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Always show at the start of a measure") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Never show") }
            ]
            enabled: page.value("tabShowTiedFret") !== 0
        }

        EoToggle {
            sid: "parenthesizeTiedFretIfArticulation"
            music: "\\tab common e'2~ e'2-. g'2 r2"
            musicStyle: ({ tabShowTiedFret: 1 })
            title: qsTrc("notation/engravingoptions",
                         "Always show parenthesized fret marks when articulation markings are present")
            hint: qsTrc("notation/engravingoptions",
                        "A tied note which carries an articulation, a fall or doit, a vibrato or tremolo bar, or " +
                        "which starts a glissando, gets its fret number in parentheses wherever it stands.")
            enabled: page.value("tabShowTiedFret") !== 0
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Marks shown on tablature staves")
        hint: qsTrc("notation/engravingoptions",
                    "Which marks are shown on simple and on common tablature staves; a mark which is switched off " +
                    "stays in the score and on a linked standard staff.")

        EoNote {
            title: qsTrc("EditStyleBase",
                         "Select the elements you would like displayed in common and simple tablature staves")
        }

        EoGrid {
            hint: qsTrc("notation/engravingoptions",
                        "‘Simple’ covers the simple tablature staff types and the lute tablatures; ‘Common’ is every " +
                        "other tablature staff, common and full alike.")
            cornerTitle: qsTrc("notation/engravingoptions", "Show")
            rowTitles: [
                qsTrc("notation/engravingoptions", "Slurs"),
                qsTrc("notation/engravingoptions", "Fermatas"),
                qsTrc("notation/engravingoptions", "Dynamics"),
                qsTrc("notation/engravingoptions", "Hairpins"),
                qsTrc("notation/engravingoptions", "Accents and marcatos"),
                qsTrc("notation/engravingoptions", "Staccatos"),
                qsTrc("notation/engravingoptions", "Harmonic marks"),
                qsTrc("notation/engravingoptions", "Let ring"),
                qsTrc("notation/engravingoptions", "Palm mute (P.M.)"),
                qsTrc("notation/engravingoptions", "Rasgueado"),
                qsTrc("notation/engravingoptions", "Mordents"),
                qsTrc("notation/engravingoptions", "Turns"),
                qsTrc("notation/engravingoptions", "Wah open / close"),
                qsTrc("notation/engravingoptions", "Golpe")
            ]
            columnTitles: [
                qsTrc("notation/engravingoptions", "Simple"),
                qsTrc("notation/engravingoptions", "Common")
            ]

            EoToggle {
                sid: "slurShowTabSimple"
                music: "\\tab e'4( g'4) a'4 g'4"
                hint: qsTrc("notation/engravingoptions", "Shows slurs on simple tablature staves.")
            }

            EoToggle {
                sid: "slurShowTabCommon"
                music: "\\tab common e'4( g'4) a'4 g'4"
                hint: qsTrc("notation/engravingoptions", "Shows slurs on common and full tablature staves.")
            }

            EoToggle {
                sid: "fermataShowTabSimple"
                music: "\\tab e'4 g'4 a'2 \\fermata"
                hint: qsTrc("notation/engravingoptions", "Shows fermatas on simple tablature staves.")
            }

            EoToggle {
                sid: "fermataShowTabCommon"
                music: "\\tab common e'4 g'4 a'2 \\fermata"
                hint: qsTrc("notation/engravingoptions", "Shows fermatas on common and full tablature staves.")
            }

            EoToggle {
                sid: "dynamicsShowTabSimple"
                music: "\\tab e'4 \\f g'4 a'4 g'4"
                hint: qsTrc("notation/engravingoptions", "Shows dynamics on simple tablature staves.")
            }

            EoToggle {
                sid: "dynamicsShowTabCommon"
                music: "\\tab common e'4 \\f g'4 a'4 g'4"
                hint: qsTrc("notation/engravingoptions", "Shows dynamics on common and full tablature staves.")
            }

            EoToggle {
                sid: "hairpinShowTabSimple"
                music: "\\tab e'4 \\< g'4 a'4 \\! g'4"
                hint: qsTrc("notation/engravingoptions", "Shows hairpins on simple tablature staves.")
            }

            EoToggle {
                sid: "hairpinShowTabCommon"
                music: "\\tab common e'4 \\< g'4 a'4 \\! g'4"
                hint: qsTrc("notation/engravingoptions", "Shows hairpins on common and full tablature staves.")
            }

            EoToggle {
                sid: "accentShowTabSimple"
                music: "\\tab e'4-> g'4-> a'4 g'4"
                hint: qsTrc("notation/engravingoptions", "Shows accents and marcatos on simple tablature staves.")
            }

            EoToggle {
                sid: "accentShowTabCommon"
                music: "\\tab common e'4-> g'4-> a'4 g'4"
                hint: qsTrc("notation/engravingoptions",
                            "Shows accents and marcatos on common and full tablature staves.")
            }

            EoToggle {
                sid: "staccatoShowTabSimple"
                music: "\\tab e'4-. g'4-. a'4 g'4"
                hint: qsTrc("notation/engravingoptions", "Shows staccatos on simple tablature staves.")
            }

            EoToggle {
                sid: "staccatoShowTabCommon"
                music: "\\tab common e'4-. g'4-. a'4 g'4"
                hint: qsTrc("notation/engravingoptions", "Shows staccatos on common and full tablature staves.")
            }

            EoToggle {
                sid: "harmonicMarkShowTabSimple"
                music: "\\tab e'4 \\startHarmonicMark \"A.H.\" g'4 a'4 \\stopHarmonicMark g'4"
                hint: qsTrc("notation/engravingoptions", "Shows harmonic marks on simple tablature staves.")
            }

            EoToggle {
                sid: "harmonicMarkShowTabCommon"
                music: "\\tab common e'4 \\startHarmonicMark \"A.H.\" g'4 a'4 \\stopHarmonicMark g'4"
                hint: qsTrc("notation/engravingoptions", "Shows harmonic marks on common and full tablature staves.")
            }

            EoToggle {
                sid: "letRingShowTabSimple"
                music: "\\tab e'4 \\startLetRing g'4 a'4 \\stopLetRing g'4"
                hint: qsTrc("notation/engravingoptions", "Shows let-ring lines on simple tablature staves.")
            }

            EoToggle {
                sid: "letRingShowTabCommon"
                music: "\\tab common e'4 \\startLetRing g'4 a'4 \\stopLetRing g'4"
                hint: qsTrc("notation/engravingoptions", "Shows let-ring lines on common and full tablature staves.")
            }

            EoToggle {
                sid: "palmMuteShowTabSimple"
                music: "\\tab e'4 \\startPalmMute g'4 a'4 \\stopPalmMute g'4"
                hint: qsTrc("notation/engravingoptions", "Shows palm-mute lines on simple tablature staves.")
            }

            EoToggle {
                sid: "palmMuteShowTabCommon"
                music: "\\tab common e'4 \\startPalmMute g'4 a'4 \\stopPalmMute g'4"
                hint: qsTrc("notation/engravingoptions", "Shows palm-mute lines on common and full tablature staves.")
            }

            EoToggle {
                sid: "rasgueadoShowTabSimple"
                music: "\\tab e'4 \\startRasgueado g'4 a'4 \\stopRasgueado g'4"
                hint: qsTrc("notation/engravingoptions", "Shows rasgueado lines on simple tablature staves.")
            }

            EoToggle {
                sid: "rasgueadoShowTabCommon"
                music: "\\tab common e'4 \\startRasgueado g'4 a'4 \\stopRasgueado g'4"
                hint: qsTrc("notation/engravingoptions", "Shows rasgueado lines on common and full tablature staves.")
            }

            EoToggle {
                sid: "mordentShowTabSimple"
                music: "\\tab e'4 \\mordent g'4 a'4 g'4"
                hint: qsTrc("notation/engravingoptions", "Shows mordents on simple tablature staves.")
            }

            EoToggle {
                sid: "mordentShowTabCommon"
                music: "\\tab common e'4 \\mordent g'4 a'4 g'4"
                hint: qsTrc("notation/engravingoptions", "Shows mordents on common and full tablature staves.")
            }

            EoToggle {
                sid: "turnShowTabSimple"
                music: "\\tab e'4 \\turn g'4 a'4 g'4"
                hint: qsTrc("notation/engravingoptions", "Shows turns on simple tablature staves.")
            }

            EoToggle {
                sid: "turnShowTabCommon"
                music: "\\tab common e'4 \\turn g'4 a'4 g'4"
                hint: qsTrc("notation/engravingoptions", "Shows turns on common and full tablature staves.")
            }

            EoToggle {
                sid: "wahShowTabSimple"
                music: "\\tab e'4 \\wahOpen g'4 \\wahClosed a'4 g'4"
                hint: qsTrc("notation/engravingoptions", "Shows wah open and close marks on simple tablature staves.")
            }

            EoToggle {
                sid: "wahShowTabCommon"
                music: "\\tab common e'4 \\wahOpen g'4 \\wahClosed a'4 g'4"
                hint: qsTrc("notation/engravingoptions",
                            "Shows wah open and close marks on common and full tablature staves.")
            }

            EoToggle {
                sid: "golpeShowTabSimple"
                music: "\\tab e'4 \\golpe g'4 a'4 g'4"
                hint: qsTrc("notation/engravingoptions", "Shows golpe marks on simple tablature staves.")
            }

            EoToggle {
                sid: "golpeShowTabCommon"
                music: "\\tab common e'4 \\golpe g'4 a'4 g'4"
                hint: qsTrc("notation/engravingoptions", "Shows golpe marks on common and full tablature staves.")
            }
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "String tunings")
        music: "\\tab common e'1 \\stringTunings"

        EoNumber {
            sid: "stringTuningsMeasureStartDistance"
            title: qsTrc("notation/engravingoptions", "Distance from the start of the measure")
            hint: qsTrc("notation/engravingoptions",
                        "How far to the right of the left edge of its measure the string tunings indication begins.")
            min: -5
            max: 10
            step: 0.1
            decimals: 1
        }
    }
}
