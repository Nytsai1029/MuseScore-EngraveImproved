/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "../ifontdesignconfiguration.h"

namespace mu::fontdesign {
class FontDesignConfiguration : public IFontDesignConfiguration
{
public:
    FontDesignConfiguration() = default;

    void init();

    muse::io::path_t lastOpenedFontPath() const override;
    void setLastOpenedFontPath(const muse::io::path_t& path) override;

    muse::io::paths_t recentFontPaths() const override;
    void prependRecentFontPath(const muse::io::path_t& path) override;
    void removeRecentFontPath(const muse::io::path_t& path) override;

private:
    void saveRecentFontPaths(const muse::io::paths_t& paths);
};
}
