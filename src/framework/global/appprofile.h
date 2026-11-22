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

namespace muse {
//! NOTE A profile is a separate set of settings and user data of the app, chosen by the MUSE_APP_PROFILE
//! environment variable. The app can then be run (by hand or by a script, see the --test-case-gui option)
//! without touching the settings, the user data, the recent files or the running instances of the user.
//! The name is made of ASCII letters, digits, '-' and '_'; any other name is ignored. Empty if no profile is chosen
inline QString appProfile()
{
    const QString name = qEnvironmentVariable("MUSE_APP_PROFILE");

    for (const QChar c : name) {
        const char16_t u = c.unicode();
        const bool isAllowed = (u >= u'a' && u <= u'z') || (u >= u'A' && u <= u'Z') || (u >= u'0' && u <= u'9')
                               || u == u'-' || u == u'_';
        if (!isAllowed) {
            return QString();
        }
    }

    return name;
}

//! NOTE What is added to a name which tells the instances of the app from other apps
//! (the application name, the name of the channel of the instances), so that a profile is an app of its own
inline QString appProfileSuffix()
{
    const QString profile = appProfile();
    return profile.isEmpty() ? QString() : QStringLiteral("-") + profile;
}
}
