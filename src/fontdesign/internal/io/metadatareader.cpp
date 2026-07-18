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
#include "metadatareader.h"

#include <set>

#include <QFile>

#include "serialization/json.h"

#include "log.h"

using namespace mu::fontdesign;
using namespace muse;

//! NOTE muse::JsonValue 的 toArray()/toObject() 不做类型检查，
//! 对缺失键/类型不符的值取容器内容会触发 picojson 断言（debug 下直接 abort），
//! 因此这里的一切容器访问都必须先判断类型。

static JsonObject objectValue(const JsonObject& obj, const std::string& key)
{
    JsonValue val = obj.value(key);
    return val.isObject() ? val.toObject() : JsonObject();
}

static std::string stringValue(const JsonObject& obj, const std::string& key)
{
    return obj.value(key).toStdString();
}

static std::vector<std::string> stringListValue(const JsonObject& obj, const std::string& key)
{
    std::vector<std::string> result;

    JsonValue val = obj.value(key);
    if (!val.isArray()) {
        return result;
    }

    JsonArray arr = val.toArray();
    for (size_t i = 0; i < arr.size(); ++i) {
        JsonValue item = arr.at(i);
        if (item.isString()) {
            result.push_back(item.toStdString());
        }
    }

    return result;
}

static char32_t codepointValue(const JsonObject& obj, const std::string& key)
{
    return SmuflDatabase::codepointFromString(stringValue(obj, key));
}

Ret MetadataReader::read(const io::path_t& path, const SmuflDatabase& db,
                         FontMetadata& out,
                         std::map<char32_t, std::map<AnchorId, PointF>>& anchorsByCode)
{
    QFile file(path.toQString());
    if (!file.open(QIODevice::ReadOnly)) {
        return make_ret(Ret::Code::UnknownError, std::string("failed to open metadata: ") + path.toStdString());
    }

    std::string err;
    JsonDocument doc = JsonDocument::fromJson(ByteArray::fromQByteArray(file.readAll()), &err);
    if (!err.empty() || !doc.isObject()) {
        return make_ret(Ret::Code::UnknownError, std::string("failed to parse metadata: ") + err);
    }

    JsonObject root = doc.rootObject();

    out.fontName = stringValue(root, "fontName");

    JsonValue versionVal = root.value("fontVersion");
    if (versionVal.isNumber()) {
        out.fontVersion = versionVal.toDouble();
    } else if (versionVal.isString()) {
        out.fontVersion = std::atof(versionVal.toStdString().c_str());
    }

    if (root.value("designSize").isNumber()) {
        out.designSize = root.value("designSize").toInt();
    }

    JsonValue sizeRangeVal = root.value("sizeRange");
    if (sizeRangeVal.isArray()) {
        JsonArray range = sizeRangeVal.toArray();
        if (range.size() == 2) {
            out.sizeRange = std::make_pair(range.at(0).toInt(), range.at(1).toInt());
        }
    }

    JsonObject engravingDefaults = objectValue(root, "engravingDefaults");
    if (engravingDefaults.isValid()) {
        for (const std::string& key : engravingDefaults.keys()) {
            JsonValue val = engravingDefaults.value(key);
            if (key == "textFontFamily") {
                // SMuFL 允许 string 或 string 数组（优先字体族列表）
                if (val.isString()) {
                    out.textFontFamily = val.toStdString();
                } else if (val.isArray()) {
                    JsonArray arr = val.toArray();
                    std::string joined;
                    for (size_t i = 0; i < arr.size(); ++i) {
                        if (!arr.at(i).isString()) {
                            continue;
                        }
                        if (!joined.empty()) {
                            joined += ", ";
                        }
                        joined += arr.at(i).toStdString();
                    }
                    out.textFontFamily = joined;
                }
            } else if (val.isNumber()) {
                out.engravingDefaults[key] = val.toDouble();
            }
        }
    }

    // 先解析 optionalGlyphs：锚点条目里的可选字形名要靠它解析码位
    JsonObject optionalGlyphs = objectValue(root, "optionalGlyphs");
    if (optionalGlyphs.isValid()) {
        for (const std::string& name : optionalGlyphs.keys()) {
            JsonObject entry = objectValue(optionalGlyphs, name);
            if (!entry.isValid()) {
                continue;
            }

            OptionalGlyphInfo info;
            info.codepoint = codepointValue(entry, "codepoint");
            info.classes = stringListValue(entry, "classes");
            info.description = stringValue(entry, "description");
            out.optionalGlyphs[name] = std::move(info);
        }
    }

    JsonObject alternates = objectValue(root, "glyphsWithAlternates");
    if (alternates.isValid()) {
        for (const std::string& baseName : alternates.keys()) {
            JsonObject baseEntry = objectValue(alternates, baseName);
            if (!baseEntry.isValid()) {
                continue;
            }

            JsonValue arrVal = baseEntry.value("alternates");
            if (!arrVal.isArray()) {
                continue;
            }

            JsonArray arr = arrVal.toArray();
            std::vector<AlternateInfo> list;
            for (size_t i = 0; i < arr.size(); ++i) {
                JsonValue itemVal = arr.at(i);
                if (!itemVal.isObject()) {
                    continue;
                }

                JsonObject entry = itemVal.toObject();
                AlternateInfo info;
                info.name = stringValue(entry, "name");
                info.codepoint = codepointValue(entry, "codepoint");
                list.push_back(std::move(info));
            }
            out.alternates[baseName] = std::move(list);
        }
    }

    JsonObject ligatures = objectValue(root, "ligatures");
    if (ligatures.isValid()) {
        for (const std::string& name : ligatures.keys()) {
            JsonObject entry = objectValue(ligatures, name);
            if (!entry.isValid()) {
                continue;
            }

            LigatureInfo info;
            info.codepoint = codepointValue(entry, "codepoint");
            info.componentGlyphs = stringListValue(entry, "componentGlyphs");
            info.description = stringValue(entry, "description");
            out.ligatures[name] = std::move(info);
        }
    }

    JsonObject sets = objectValue(root, "sets");
    if (sets.isValid()) {
        for (const std::string& setId : sets.keys()) {
            JsonObject entry = objectValue(sets, setId);
            if (!entry.isValid()) {
                continue;
            }

            SetInfo info;
            info.description = stringValue(entry, "description");
            info.type = stringValue(entry, "type");

            JsonValue glyphsVal = entry.value("glyphs");
            if (glyphsVal.isArray()) {
                JsonArray glyphs = glyphsVal.toArray();
                for (size_t i = 0; i < glyphs.size(); ++i) {
                    JsonValue itemVal = glyphs.at(i);
                    if (!itemVal.isObject()) {
                        continue;
                    }

                    JsonObject glyphEntry = itemVal.toObject();
                    SetGlyphInfo glyphInfo;
                    glyphInfo.alternateFor = stringValue(glyphEntry, "alternateFor");
                    glyphInfo.codepoint = codepointValue(glyphEntry, "codepoint");
                    glyphInfo.name = stringValue(glyphEntry, "name");
                    glyphInfo.description = stringValue(glyphEntry, "description");
                    info.glyphs.push_back(std::move(glyphInfo));
                }
            }

            out.sets[setId] = std::move(info);
        }
    }

    JsonObject glyphsWithAnchors = objectValue(root, "glyphsWithAnchors");
    if (glyphsWithAnchors.isValid()) {
        const std::map<std::string, AnchorId>& anchorIds = anchorIdsByName();

        for (const std::string& glyphName : glyphsWithAnchors.keys()) {
            JsonObject anchors = objectValue(glyphsWithAnchors, glyphName);
            if (!anchors.isValid()) {
                continue;
            }

            char32_t code = 0;
            auto optIt = out.optionalGlyphs.find(glyphName);
            if (optIt != out.optionalGlyphs.end()) {
                code = optIt->second.codepoint;
            } else if (const SmuflDatabase::GlyphInfo* info = db.infoByName(glyphName)) {
                code = info->codepoint;
            }

            if (code == 0) {
                out.passthroughAnchors.set(glyphName, anchors);
                continue;
            }

            JsonObject unknownAnchors;
            std::map<AnchorId, PointF>& glyphAnchors = anchorsByCode[code];

            for (const std::string& anchorName : anchors.keys()) {
                JsonValue coordsVal = anchors.value(anchorName);

                auto idIt = anchorIds.find(anchorName);
                if (idIt == anchorIds.end() || !coordsVal.isArray()) {
                    unknownAnchors.set(anchorName, coordsVal);
                    continue;
                }

                JsonArray coords = coordsVal.toArray();
                if (coords.size() == 2) {
                    glyphAnchors[idIt->second] = PointF(coords.at(0).toDouble(), coords.at(1).toDouble());
                }
            }

            if (!unknownAnchors.empty()) {
                out.passthroughAnchors.set(glyphName, unknownAnchors);
            }
        }
    }

    // glyphAdvanceWidths / glyphBBoxes 导出时由字体数据自动生成，读入时忽略
    static const std::set<std::string> knownKeys {
        "fontName", "fontVersion", "designSize", "sizeRange",
        "engravingDefaults", "glyphAdvanceWidths", "glyphBBoxes",
        "glyphsWithAnchors", "glyphsWithAlternates", "ligatures",
        "optionalGlyphs", "sets",
    };

    for (const std::string& key : root.keys()) {
        if (knownKeys.find(key) == knownKeys.end()) {
            out.passthrough.set(key, root.value(key));
        }
    }

    return make_ok();
}
