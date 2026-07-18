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
#include "fontdesignconfiguration.h"

#include <algorithm>

#include "settings.h"

using namespace mu::fontdesign;
using namespace muse;

static const std::string module_name("fontdesign");

static const Settings::Key LAST_OPENED_FONT_PATH(module_name, "fontdesign/lastOpenedFontPath");
static const Settings::Key RECENT_FONT_PATHS(module_name, "fontdesign/recentFontPaths");

static constexpr size_t MAX_RECENT_FONTS = 10;
static const std::string RECENT_SEPARATOR("\n");

void FontDesignConfiguration::init()
{
    settings()->setDefaultValue(LAST_OPENED_FONT_PATH, Val(std::string()));
    settings()->setDefaultValue(RECENT_FONT_PATHS, Val(std::string()));
}

muse::io::path_t FontDesignConfiguration::lastOpenedFontPath() const
{
    return settings()->value(LAST_OPENED_FONT_PATH).toPath();
}

void FontDesignConfiguration::setLastOpenedFontPath(const muse::io::path_t& path)
{
    settings()->setSharedValue(LAST_OPENED_FONT_PATH, Val(path));
}

muse::io::paths_t FontDesignConfiguration::recentFontPaths() const
{
    io::paths_t result;
    const std::string raw = settings()->value(RECENT_FONT_PATHS).toString();
    size_t start = 0;
    while (start < raw.size()) {
        size_t end = raw.find(RECENT_SEPARATOR, start);
        if (end == std::string::npos) {
            end = raw.size();
        }
        if (end > start) {
            result.push_back(io::path_t(raw.substr(start, end - start)));
        }
        start = end + RECENT_SEPARATOR.size();
    }
    return result;
}

void FontDesignConfiguration::saveRecentFontPaths(const muse::io::paths_t& paths)
{
    std::string raw;
    for (const io::path_t& p : paths) {
        if (!raw.empty()) {
            raw += RECENT_SEPARATOR;
        }
        raw += p.toStdString();
    }
    settings()->setSharedValue(RECENT_FONT_PATHS, Val(raw));
}

void FontDesignConfiguration::prependRecentFontPath(const muse::io::path_t& path)
{
    if (path.empty()) {
        return;
    }

    io::paths_t paths = recentFontPaths();
    paths.erase(std::remove(paths.begin(), paths.end(), path), paths.end());
    paths.insert(paths.begin(), path);
    if (paths.size() > MAX_RECENT_FONTS) {
        paths.resize(MAX_RECENT_FONTS);
    }
    saveRecentFontPaths(paths);
}

void FontDesignConfiguration::removeRecentFontPath(const muse::io::path_t& path)
{
    io::paths_t paths = recentFontPaths();
    paths.erase(std::remove(paths.begin(), paths.end(), path), paths.end());
    saveRecentFontPaths(paths);
}
