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
#include "smufldatabase.h"

#include <algorithm>
#include <cstdlib>

#include <QFile>

#include "serialization/json.h"

#include "log.h"

using namespace mu::fontdesign;
using namespace muse;

static bool readJsonFile(const QString& path, JsonObject& out)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        LOGE() << "failed to open: " << path;
        return false;
    }

    std::string err;
    JsonDocument doc = JsonDocument::fromJson(ByteArray::fromQByteArray(file.readAll()), &err);
    if (!err.empty()) {
        LOGE() << "failed to parse " << path << ": " << err;
        return false;
    }

    out = doc.rootObject();
    return out.isValid();
}

void SmuflDatabase::init(const std::string& dir)
{
    if (m_inited) {
        return;
    }

    const QString base = QString::fromStdString(dir);

    JsonObject glyphNamesObj;
    if (readJsonFile(base + "/glyphnames.json", glyphNamesObj)) {
        for (const std::string& name : glyphNamesObj.keys()) {
            JsonObject entry = glyphNamesObj.value(name).toObject();
            if (!entry.isValid()) {
                continue;
            }

            GlyphInfo info;
            info.name = name;
            info.codepoint = codepointFromString(entry.value("codepoint").toStdString());
            info.description = entry.value("description").toStdString();

            if (info.codepoint == 0) {
                continue;
            }

            m_namesByCode[info.codepoint] = name;
            m_glyphsByName[name] = std::move(info);
        }
    }

    JsonObject rangesObj;
    if (readJsonFile(base + "/ranges.json", rangesObj)) {
        for (const std::string& rangeId : rangesObj.keys()) {
            JsonObject entry = rangesObj.value(rangeId).toObject();
            if (!entry.isValid()) {
                continue;
            }

            Range range;
            range.id = rangeId;
            range.description = entry.value("description").toStdString();
            range.start = codepointFromString(entry.value("range_start").toStdString());
            range.end = codepointFromString(entry.value("range_end").toStdString());

            JsonValue glyphsVal = entry.value("glyphs");
            if (glyphsVal.isArray()) {
                JsonArray glyphs = glyphsVal.toArray();
                for (size_t i = 0; i < glyphs.size(); ++i) {
                    range.glyphNames.push_back(glyphs.at(i).toStdString());
                }
            }

            m_ranges.push_back(std::move(range));
        }

        std::sort(m_ranges.begin(), m_ranges.end(), [](const Range& r1, const Range& r2) {
            return r1.start < r2.start;
        });
    }

    JsonObject classesObj;
    if (readJsonFile(base + "/classes.json", classesObj)) {
        for (const std::string& className : classesObj.keys()) {
            m_classNames.push_back(className);

            JsonValue glyphsVal = classesObj.value(className);
            if (!glyphsVal.isArray()) {
                continue;
            }

            JsonArray glyphs = glyphsVal.toArray();
            for (size_t i = 0; i < glyphs.size(); ++i) {
                m_classesByGlyph[glyphs.at(i).toStdString()].push_back(className);
            }
        }
    }

    m_inited = true;

    LOGI() << "SMuFL database loaded: " << m_glyphsByName.size() << " glyphs, "
           << m_ranges.size() << " ranges, " << m_classNames.size() << " classes";
}

bool SmuflDatabase::isInited() const
{
    return m_inited;
}

const std::vector<SmuflDatabase::Range>& SmuflDatabase::ranges() const
{
    return m_ranges;
}

const SmuflDatabase::GlyphInfo* SmuflDatabase::infoByName(const std::string& name) const
{
    auto it = m_glyphsByName.find(name);
    return it != m_glyphsByName.end() ? &it->second : nullptr;
}

const SmuflDatabase::GlyphInfo* SmuflDatabase::infoByCodepoint(char32_t code) const
{
    auto it = m_namesByCode.find(code);
    return it != m_namesByCode.end() ? infoByName(it->second) : nullptr;
}

const std::vector<std::string>& SmuflDatabase::classNames() const
{
    return m_classNames;
}

std::vector<std::string> SmuflDatabase::classesOfGlyph(const std::string& glyphName) const
{
    auto it = m_classesByGlyph.find(glyphName);
    return it != m_classesByGlyph.end() ? it->second : std::vector<std::string>();
}

char32_t SmuflDatabase::codepointFromString(const std::string& str)
{
    if (str.size() < 3 || (str[0] != 'U' && str[0] != 'u') || str[1] != '+') {
        return 0;
    }

    return static_cast<char32_t>(std::strtoul(str.c_str() + 2, nullptr, 16));
}
