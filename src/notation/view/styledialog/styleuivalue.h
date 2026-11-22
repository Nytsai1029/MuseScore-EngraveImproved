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

#include <QVariant>

#include "engraving/style/style.h"

#include "notationtypes.h"

namespace mu::notation {
//! NOTE A style value as the interface has it: a distance in staff spaces is a plain number,
//! any other value is what PropertyValue makes of it for QML
inline QVariant styleValueToUi(StyleId id, const PropertyValue& value)
{
    if (engraving::MStyle::valueType(id) == engraving::P_TYPE::SPATIUM) {
        return value.value<engraving::Spatium>().val();
    }

    return value.toQVariant();
}

inline PropertyValue styleValueFromUi(StyleId id, const QVariant& uiValue)
{
    if (engraving::MStyle::valueType(id) == engraving::P_TYPE::SPATIUM) {
        return engraving::Spatium(uiValue.toDouble());
    }

    return PropertyValue::fromQVariant(uiValue, engraving::MStyle::valueType(id));
}
}
