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

    code: "glissando-note-line"
    legacyCodes: ["glissando-note-line"]
    title: qsTrc("notation/engravingoptions", "Glissandos and note-anchored lines")
    group: qsTrc("notation/engravingoptions", "Lines")

    EoSection {
        title: qsTrc("notation/engravingoptions", "Glissandos")
        music: "c'2 \\glissando c''2"

        EoChoice {
            sid: "glissandoType"
            title: qsTrc("notation", "Line type:")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Straight") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Wavy") }
            ]
        }

        EoChoice {
            sid: "glissandoLineStyle"
            title: qsTrc("EditStyleBase", "Line style:")
            hint: qsTrc("notation/engravingoptions", "Only for straight glissandos.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Solid") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Dashed") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Dotted") }
            ]
            enabled: page.value("glissandoType") === 0
        }

        EoNumber {
            sid: "glissandoLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            hint: qsTrc("notation/engravingoptions",
                        "Only for straight glissandos; a wavy one takes its weight from the music font.")
            step: 0.01
            enabled: page.value("glissandoType") === 0
        }

        EoNumber {
            sid: "glissandoDashLineLen"
            musicStyle: ({ glissandoLineStyle: 1 })
            title: qsTrc("notation/engravingoptions", "Dash length")
            hint: qsTrc("notation/engravingoptions",
                        "Length of each dash as a multiple of the line thickness. Only with the line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            enabled: page.value("glissandoType") === 0
            visible: page.value("glissandoLineStyle") === 1
        }

        EoNumber {
            sid: "glissandoDashGapLen"
            music: "c'2 \\glissando g''2"
            musicStyle: ({ glissandoLineStyle: 1 })
            title: qsTrc("notation/engravingoptions", "Gap length")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between two dashes as a multiple of the line thickness; it is evened out slightly so " +
                        "that the line ends with a full dash. Only with the line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            enabled: page.value("glissandoType") === 0
            visible: page.value("glissandoLineStyle") === 1
        }

        EoToggle {
            sid: "glissandoShowText"
            title: qsTrc("notation/engravingoptions", "Show text")
            hint: qsTrc("notation/engravingoptions",
                        "Writes the text below along the line, wherever the line is long enough to hold it.")
        }

        EoText {
            sid: "glissandoText"
            title: qsTrc("notation/engravingoptions", "Text")
            hint: qsTrc("notation/engravingoptions",
                        "The word written along the line, ‘gliss.’ by default. Only with ‘Show text’ on.")
            enabled: page.value("glissandoShowText") === true
        }

        EoNumber {
            sid: "glissandoTextDistance"
            title: qsTrc("notation/engravingoptions", "Text distance above a straight line")
            hint: qsTrc("notation/engravingoptions", "How far the text is raised above a straight glissando line.")
            max: 2
        }

        EoNumber {
            sid: "glissandoTextDistanceWavy"
            musicStyle: ({ glissandoType: 1 })
            title: qsTrc("notation/engravingoptions", "Text distance above a wavy line")
            hint: qsTrc("notation/engravingoptions",
                        "How far the text is raised above the middle of a wavy glissando line.")
            max: 2
        }

        EoNumber {
            sid: "minStraightGlissandoLength"
            music: "c'16 \\glissando g'16 r8 r4 r2"
            title: qsTrc("notation", "Min. length (straight):")
            hint: qsTrc("notation/engravingoptions",
                        "The two notes are spaced apart far enough for a straight glissando of at least this length. " +
                        "Note-anchored lines use it too.")
            step: 0.1
        }

        EoNumber {
            sid: "minWigglyGlissandoLength"
            music: "c'16 \\glissando g'16 r8 r4 r2"
            musicStyle: ({ glissandoType: 1 })
            title: qsTrc("notation", "Min. length (wavy):")
            hint: qsTrc("notation/engravingoptions",
                        "The two notes are spaced apart far enough for a wavy glissando of at least this length.")
            step: 0.1
        }

        EoChoice {
            sid: "glissandoStyle"
            title: qsTrc("notation/engravingoptions", "Playback")
            hint: qsTrc("notation/engravingoptions",
                        "Which notes sound between the two ends of a glissando: every semitone, only the white or " +
                        "the black keys, the notes of the scale as written, or a continuous slide. For playback " +
                        "only; nothing changes on the page.")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Chromatic") },
                { value: 1, title: qsTrc("notation/engravingoptions", "White keys") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Black keys") },
                { value: 3, title: qsTrc("notation/engravingoptions", "Diatonic") },
                { value: 4, title: qsTrc("notation/engravingoptions", "Portamento") }
            ]
        }

        EoChoice {
            sid: "glissandoStyleHarp"
            title: qsTrc("notation/engravingoptions", "Playback on harp staves")
            hint: qsTrc("notation/engravingoptions",
                        "‘Diatonic’ plays the strings as the current harp pedal diagram sets them; ‘Chromatic’ plays " +
                        "every semitone. For playback only.")
            choices: [
                { value: 3, title: qsTrc("notation/engravingoptions", "Diatonic") },
                { value: 0, title: qsTrc("notation/engravingoptions", "Chromatic") }
            ]
        }

        EoLink {
            title: qsTrc("notation", "Edit glissando text style")
            target: "glissando"
        }
    }

    EoSection {
        title: qsTrc("notation/engravingoptions", "Note-anchored lines")
        music: "c'2 \\noteLine g'2"

        EoChoice {
            sid: "noteLineStyle"
            title: qsTrc("EditStyleBase", "Line style:")
            choices: [
                { value: 0, title: qsTrc("notation/engravingoptions", "Solid") },
                { value: 1, title: qsTrc("notation/engravingoptions", "Dashed") },
                { value: 2, title: qsTrc("notation/engravingoptions", "Dotted") }
            ]
        }

        EoNumber {
            sid: "noteLineWidth"
            title: qsTrc("EditStyleBase", "Line thickness:")
            step: 0.01
        }

        EoNumber {
            sid: "noteLineDashLineLen"
            musicStyle: ({ noteLineStyle: 1 })
            title: qsTrc("notation/engravingoptions", "Dash length")
            hint: qsTrc("notation/engravingoptions",
                        "Length of each dash as a multiple of the line thickness. Only with the line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("noteLineStyle") === 1
        }

        EoNumber {
            sid: "noteLineDashGapLen"
            musicStyle: ({ noteLineStyle: 1 })
            title: qsTrc("notation/engravingoptions", "Gap length")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between two dashes as a multiple of the line thickness; it is evened out slightly so " +
                        "that the line ends with a full dash. Only with the line style ‘Dashed’.")
            min: 0.1
            max: 100
            step: 0.1
            visible: page.value("noteLineStyle") === 1
        }

        EoNumber {
            sid: "noteAnchoredLineNoteDistance"
            title: qsTrc("notation/engravingoptions", "Gap between the line and its notes")
            hint: qsTrc("notation/engravingoptions",
                        "Gap between each end of the line and the notehead, augmentation dots or accidental next to " +
                        "it. Glissandos use it too.")
            max: 2
        }

        EoNumber {
            sid: "noteAnchoredLineSameLineSlant"
            music: "c''4 \\noteLine cis''4 r2"
            title: qsTrc("notation/engravingoptions", "Slant between notes at the same staff position")
            hint: qsTrc("notation/engravingoptions",
                        "When both notes sit on the same line or space, one end of the line is lowered and the other " +
                        "raised by this much, so that the line still shows whether the pitch rises or falls. " +
                        "Glissandos use it too.")
            max: 1
        }

        EoNumber {
            sid: "noteAnchoredLineTabSlant"
            music: "\\tab c'4 \\noteLine d'4 r2"
            title: qsTrc("notation/engravingoptions", "Slant on tablature staves")
            hint: qsTrc("notation/engravingoptions",
                        "On tablature one end of the line is lowered and the other raised by this fraction of the " +
                        "distance between two strings, so that a slide along one string is not drawn flat. " +
                        "Glissandos use it too.")
            max: 1
        }

        EoLink {
            title: qsTrc("notation", "Edit note-anchored line text style")
            target: "note-line"
        }
    }
}
