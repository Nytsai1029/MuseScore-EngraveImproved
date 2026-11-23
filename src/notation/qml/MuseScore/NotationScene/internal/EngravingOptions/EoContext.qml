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

//! NOTE: the situation which a preview shows (a single note or a chord, stems up or down, …), chosen beside the
//! picture of the option. It is not an option: it has no style value, and the page binds the music of its
//! section to the `value`
EoOption {
    kind: "context"

    //! NOTE: a list of { value, title }
    property var choices: []

    //! NOTE: the choice which is made; the page says which one is first
    property var value: null
}
