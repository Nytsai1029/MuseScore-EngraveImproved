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

#include <QByteArray>
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

char32_t MetadataReader::codepointFromUniName(const std::string& name)
{
    size_t prefix = 0;
    if (name.compare(0, 3, "uni") == 0) {
        prefix = 3;
    } else if (!name.empty() && name[0] == 'u') {
        prefix = 1;
    } else {
        return 0;
    }

    const size_t digits = name.size() - prefix;
    if (digits < 4 || digits > 6) {
        return 0;
    }

    char32_t code = 0;
    for (size_t i = prefix; i < name.size(); ++i) {
        const char c = name[i];
        int v = 0;
        if (c >= '0' && c <= '9') {
            v = c - '0';
        } else if (c >= 'A' && c <= 'F') {
            v = c - 'A' + 10;
        } else {
            return 0;
        }
        code = code * 16 + static_cast<char32_t>(v);
    }

    return code <= 0x10FFFF ? code : 0;
}

//! 元数据里的字形名 → 码位。顺序：字体自己的 optionalGlyphs 声明 → SMuFL 规范名 →
//! 字体自带的字形名 → uniXXXX。ligatures / alternates / sets 里声明的码位不参与：
//! 它们描述的是「谁替代谁」，实测并不可靠（Leland 把带括号变音记号声明在字体里没有字形的码位上）。
static char32_t resolveGlyphName(const std::string& name, const FontMetadata& metadata, const SmuflDatabase& db,
                                 const MetadataReader::FontGlyphIndex& font)
{
    auto optIt = metadata.optionalGlyphs.find(name);
    if (optIt != metadata.optionalGlyphs.end() && optIt->second.codepoint != 0) {
        return optIt->second.codepoint;
    }

    if (const SmuflDatabase::GlyphInfo* info = db.infoByName(name)) {
        return info->codepoint;
    }

    auto fontIt = font.codepointByName.find(name);
    if (fontIt != font.codepointByName.end()) {
        return fontIt->second;
    }

    return MetadataReader::codepointFromUniName(name);
}

Ret MetadataReader::read(const io::path_t& path, const SmuflDatabase& db, const FontGlyphIndex& font, Output& output)
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

    FontMetadata& out = output.metadata;
    std::map<char32_t, std::map<AnchorId, PointF>>& anchorsByCode = output.anchorsByCode;

    out.fontName = stringValue(root, "fontName");

    JsonValue versionVal = root.value("fontVersion");
    if (versionVal.isNumber()) {
        out.fontVersion = versionVal.toDouble();
    } else if (versionVal.isString()) {
        //! QByteArray::toDouble 恒用 C locale（atof 的小数点随 LC_NUMERIC 变化）
        out.fontVersion = QByteArray::fromStdString(versionVal.toStdString()).trimmed().toDouble();
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
    for (const std::string& key : engravingDefaults.keys()) {
        JsonValue val = engravingDefaults.value(key);
        if (key == "textFontFamily" && val.isString()) {
            out.textFontFamily = val.toStdString();
        } else if (key == "textFontFamily" && val.isArray()) {
            // SMuFL 允许 string 或 string 数组（优先字体族列表）
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
        } else if (key != "textFontFamily" && val.isNumber()) {
            out.engravingDefaults[key] = val.toDouble();
        } else {
            // 类型不在预期内的值：不解释，原样透传
            out.passthroughEngravingDefaults.set(key, val);
        }
    }

    // 先解析 optionalGlyphs：锚点条目里的可选字形名要靠它解析码位
    JsonObject optionalGlyphs = objectValue(root, "optionalGlyphs");
    for (const std::string& name : optionalGlyphs.keys()) {
        if (!optionalGlyphs.value(name).isObject()) {
            continue;
        }
        JsonObject entry = optionalGlyphs.value(name).toObject();

        OptionalGlyphInfo info;
        info.codepoint = codepointValue(entry, "codepoint");
        info.classes = stringListValue(entry, "classes");
        info.description = stringValue(entry, "description");
        out.optionalGlyphs[name] = std::move(info);
    }

    JsonObject alternates = objectValue(root, "glyphsWithAlternates");
    for (const std::string& baseName : alternates.keys()) {
        JsonObject baseEntry = objectValue(alternates, baseName);

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

    JsonObject ligatures = objectValue(root, "ligatures");
    for (const std::string& name : ligatures.keys()) {
        if (!ligatures.value(name).isObject()) {
            continue;
        }
        JsonObject entry = ligatures.value(name).toObject();

        LigatureInfo info;
        info.codepoint = codepointValue(entry, "codepoint");
        info.componentGlyphs = stringListValue(entry, "componentGlyphs");
        info.description = stringValue(entry, "description");
        out.ligatures[name] = std::move(info);
    }

    JsonObject sets = objectValue(root, "sets");
    for (const std::string& setId : sets.keys()) {
        if (!sets.value(setId).isObject()) {
            continue;
        }
        JsonObject entry = sets.value(setId).toObject();

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

    //! glyphBBoxes / glyphAdvanceWidths：能对上字形的条目在写出时按当前轮廓重新生成；
    //! 原文整段留底，对不上字形的条目写出时原样带回（见 MetadataWriter）
    out.sourceGlyphBBoxes = objectValue(root, "glyphBBoxes");
    out.sourceGlyphAdvanceWidths = objectValue(root, "glyphAdvanceWidths");
    for (const std::string& name : out.sourceGlyphBBoxes.keys()) {
        output.glyphKeys.insert(name);
    }
    for (const std::string& name : out.sourceGlyphAdvanceWidths.keys()) {
        output.glyphKeys.insert(name);
    }

    JsonObject glyphsWithAnchors = objectValue(root, "glyphsWithAnchors");
    const std::map<std::string, AnchorId>& anchorIds = anchorIdsByName();

    for (const std::string& glyphName : glyphsWithAnchors.keys()) {
        if (!glyphsWithAnchors.value(glyphName).isObject()) {
            continue;
        }
        JsonObject anchors = glyphsWithAnchors.value(glyphName).toObject();
        output.glyphKeys.insert(glyphName);

        //! 名字解析不出码位、或字体里没有这个字形：整条原样透传
        const char32_t code = resolveGlyphName(glyphName, out, db, font);
        if (code == 0 || font.codepoints.count(code) == 0) {
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
            if (coords.size() != 2 || !coords.at(0).isNumber() || !coords.at(1).isNumber()) {
                // 形状不对的坐标：不猜，原样透传
                unknownAnchors.set(anchorName, coordsVal);
                continue;
            }

            glyphAnchors[idIt->second] = PointF(coords.at(0).toDouble(), coords.at(1).toDouble());
        }

        if (!unknownAnchors.empty()) {
            out.passthroughAnchors.set(glyphName, unknownAnchors);
        }
    }

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
