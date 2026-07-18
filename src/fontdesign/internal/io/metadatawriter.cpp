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
#include "metadatawriter.h"

#include <cmath>
#include <cstdio>
#include <map>
#include <set>

#include <QFile>
#include <QPainterPath>

#include "../fontdesigntypes.h"
#include "../project/fontdesignproject.h"

#include "log.h"

using namespace mu::fontdesign;
using namespace muse;

namespace {
struct Writer {
    std::string out;
    int indent = 0;

    void nl()
    {
        out += "\n";
        out.append(indent * 4, ' ');
    }

    void key(const std::string& k)
    {
        string(k);
        out += ": ";
    }

    void string(const std::string& s)
    {
        out += '"';
        for (char c : s) {
            switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += c;
                }
            }
        }
        out += '"';
    }

    void number(double v)
    {
        if (std::abs(v - std::round(v)) < 1e-9 && std::abs(v) < 1e15) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%lld", static_cast<long long>(std::llround(v)));
            out += buf;
            return;
        }

        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.5f", v);
        std::string s(buf);

        while (!s.empty() && s.back() == '0') {
            s.pop_back();
        }
        if (!s.empty() && s.back() == '.') {
            s.pop_back();
        }

        out += s;
    }

    void coords(double x, double y)
    {
        out += "[";
        indent += 1;
        nl();
        number(x);
        out += ",";
        nl();
        number(y);
        indent -= 1;
        nl();
        out += "]";
    }

    //! 透传子树（passthrough 的 JsonValue）
    void jsonValue(const JsonValue& val)
    {
        if (val.isBool()) {
            out += val.toBool() ? "true" : "false";
        } else if (val.isNumber()) {
            number(val.toDouble());
        } else if (val.isString()) {
            string(val.toStdString());
        } else if (val.isArray()) {
            JsonArray arr = val.toArray();
            if (arr.empty()) {
                out += "[]";
                return;
            }
            out += "[";
            indent += 1;
            for (size_t i = 0; i < arr.size(); ++i) {
                if (i > 0) {
                    out += ",";
                }
                nl();
                jsonValue(arr.at(i));
            }
            indent -= 1;
            nl();
            out += "]";
        } else if (val.isObject()) {
            JsonObject obj = val.toObject();
            std::vector<std::string> keys = obj.keys();
            if (keys.empty()) {
                out += "{}";
                return;
            }
            out += "{";
            indent += 1;
            bool first = true;
            for (const std::string& k : keys) {
                if (!first) {
                    out += ",";
                }
                first = false;
                nl();
                key(k);
                jsonValue(obj.value(k));
            }
            indent -= 1;
            nl();
            out += "}";
        } else {
            out += "null";
        }
    }
};

//! 元数据用的字形键名：SMuFL 名称，无名者 uniXXXX
std::string metadataGlyphName(const GlyphItem& glyph)
{
    if (!glyph.smuflName.empty()) {
        return glyph.smuflName;
    }

    char buf[16];
    std::snprintf(buf, sizeof(buf), "uni%04X", static_cast<unsigned int>(glyph.codepoint));
    return buf;
}

std::string codepointString(char32_t code)
{
    char buf[16];
    std::snprintf(buf, sizeof(buf), "U+%04X", static_cast<unsigned int>(code));
    return buf;
}
}

std::string MetadataWriter::toJsonText(const FontDesignProject& project)
{
    const FontMetadata& metadata = project.metadata();
    const double spatium = project.spatium();

    Writer w;
    w.out += "{";
    w.indent = 1;

    bool firstSection = true;
    auto section = [&w, &firstSection](const std::string& name) {
        if (!firstSection) {
            w.out += ",";
        }
        firstSection = false;
        w.nl();
        w.key(name);
    };

    section("fontName");
    w.string(metadata.fontName);

    section("fontVersion");
    w.number(metadata.fontVersion);

    if (metadata.designSize.has_value()) {
        section("designSize");
        w.number(metadata.designSize.value());
    }

    if (metadata.sizeRange.has_value()) {
        section("sizeRange");
        w.coords(metadata.sizeRange->first, metadata.sizeRange->second);
    }

    if (!metadata.engravingDefaults.empty() || !metadata.textFontFamily.empty()) {
        section("engravingDefaults");
        w.out += "{";
        w.indent += 1;
        bool first = true;
        auto entry = [&w, &first](const std::string& k) {
            if (!first) {
                w.out += ",";
            }
            first = false;
            w.nl();
            w.key(k);
        };

        if (!metadata.textFontFamily.empty()) {
            entry("textFontFamily");
            // SMuFL：textFontFamily 为字体族名数组
            std::vector<std::string> families;
            std::string current;
            for (char c : metadata.textFontFamily) {
                if (c == ',') {
                    // trim
                    size_t start = current.find_first_not_of(" \t");
                    size_t end = current.find_last_not_of(" \t");
                    if (start != std::string::npos) {
                        families.push_back(current.substr(start, end - start + 1));
                    }
                    current.clear();
                } else {
                    current.push_back(c);
                }
            }
            size_t start = current.find_first_not_of(" \t");
            size_t end = current.find_last_not_of(" \t");
            if (start != std::string::npos) {
                families.push_back(current.substr(start, end - start + 1));
            }

            w.out += "[";
            w.indent += 1;
            for (size_t i = 0; i < families.size(); ++i) {
                if (i > 0) {
                    w.out += ",";
                }
                w.nl();
                w.string(families[i]);
            }
            w.indent -= 1;
            w.nl();
            w.out += "]";
        }
        for (const auto& pair : metadata.engravingDefaults) {
            entry(pair.first);
            w.number(pair.second);
        }
        w.indent -= 1;
        w.nl();
        w.out += "}";
    }

    // glyphAdvanceWidths / glyphBBoxes：按当前字体数据自动生成（仅命名字形）
    {
        section("glyphAdvanceWidths");
        w.out += "{";
        w.indent += 1;
        bool first = true;
        for (const auto& pair : project.glyphs()) {
            const GlyphItem& glyph = pair.second;
            if (glyph.smuflName.empty()) {
                continue;
            }
            if (!first) {
                w.out += ",";
            }
            first = false;
            w.nl();
            w.key(glyph.smuflName);
            w.number(glyph.advance / spatium);
        }
        w.indent -= 1;
        w.nl();
        w.out += "}";
    }

    {
        section("glyphBBoxes");
        w.out += "{";
        w.indent += 1;
        bool first = true;
        for (const auto& pair : project.glyphs()) {
            const GlyphItem& glyph = pair.second;
            if (glyph.smuflName.empty() || glyph.outline.isEmpty()) {
                continue;
            }

            // Qt 的 boundingRect 为曲线精确包围盒（区别于控制点包围盒）
            QRectF bbox = glyph.outline.toPainterPath().toQPainterPath().boundingRect();

            if (!first) {
                w.out += ",";
            }
            first = false;
            w.nl();
            w.key(glyph.smuflName);
            w.out += "{";
            w.indent += 1;
            w.nl();
            w.key("bBoxNE");
            w.coords((bbox.x() + bbox.width()) / spatium, (bbox.y() + bbox.height()) / spatium);
            w.out += ",";
            w.nl();
            w.key("bBoxSW");
            w.coords(bbox.x() / spatium, bbox.y() / spatium);
            w.indent -= 1;
            w.nl();
            w.out += "}";
        }
        w.indent -= 1;
        w.nl();
        w.out += "}";
    }

    // glyphsWithAnchors：字形项上的结构化锚点 + 无法解析而透传的锚点，
    //! 按字形名合并到同一对象——同名字形若同时含两者，绝不能产生重复键
    //! （重复键会在重新加载时静默覆盖已知锚点）。
    {
        std::map<std::string, const GlyphItem*> byName;
        for (const auto& pair : project.glyphs()) {
            if (!pair.second.anchors.empty()) {
                byName[metadataGlyphName(pair.second)] = &pair.second;
            }
        }

        // 全部字形名（结构化 + 透传）的有序并集
        std::set<std::string> allNames;
        for (const auto& pair : byName) {
            allNames.insert(pair.first);
        }
        if (metadata.passthroughAnchors.isValid()) {
            for (const std::string& name : metadata.passthroughAnchors.keys()) {
                allNames.insert(name);
            }
        }

        if (!allNames.empty()) {
            section("glyphsWithAnchors");
            w.out += "{";
            w.indent += 1;
            bool first = true;

            for (const std::string& name : allNames) {
                if (!first) {
                    w.out += ",";
                }
                first = false;
                w.nl();
                w.key(name);
                w.out += "{";
                w.indent += 1;

                bool firstAnchor = true;

                auto byNameIt = byName.find(name);
                if (byNameIt != byName.end()) {
                    for (const auto& anchorPair : byNameIt->second->anchors) {
                        if (!firstAnchor) {
                            w.out += ",";
                        }
                        firstAnchor = false;
                        w.nl();
                        w.key(anchorNameById(anchorPair.first));
                        w.coords(anchorPair.second.x(), anchorPair.second.y());
                    }
                }

                // 透传该字形的未识别锚点（键在 reader 侧已保证与结构化锚点不重名）
                if (metadata.passthroughAnchors.isValid() && metadata.passthroughAnchors.contains(name)) {
                    JsonObject extra = metadata.passthroughAnchors.value(name).toObject();
                    if (extra.isValid()) {
                        for (const std::string& anchorName : extra.keys()) {
                            if (!firstAnchor) {
                                w.out += ",";
                            }
                            firstAnchor = false;
                            w.nl();
                            w.key(anchorName);
                            w.jsonValue(extra.value(anchorName));
                        }
                    }
                }

                w.indent -= 1;
                w.nl();
                w.out += "}";
            }

            w.indent -= 1;
            w.nl();
            w.out += "}";
        }
    }

    if (!metadata.alternates.empty()) {
        section("glyphsWithAlternates");
        w.out += "{";
        w.indent += 1;
        bool first = true;
        for (const auto& pair : metadata.alternates) {
            if (!first) {
                w.out += ",";
            }
            first = false;
            w.nl();
            w.key(pair.first);
            w.out += "{";
            w.indent += 1;
            w.nl();
            w.key("alternates");
            w.out += "[";
            w.indent += 1;
            bool firstAlt = true;
            for (const AlternateInfo& alt : pair.second) {
                if (!firstAlt) {
                    w.out += ",";
                }
                firstAlt = false;
                w.nl();
                w.out += "{";
                w.indent += 1;
                w.nl();
                w.key("codepoint");
                w.string(codepointString(alt.codepoint));
                w.out += ",";
                w.nl();
                w.key("name");
                w.string(alt.name);
                w.indent -= 1;
                w.nl();
                w.out += "}";
            }
            w.indent -= 1;
            w.nl();
            w.out += "]";
            w.indent -= 1;
            w.nl();
            w.out += "}";
        }
        w.indent -= 1;
        w.nl();
        w.out += "}";
    }

    if (!metadata.ligatures.empty()) {
        section("ligatures");
        w.out += "{";
        w.indent += 1;
        bool first = true;
        for (const auto& pair : metadata.ligatures) {
            if (!first) {
                w.out += ",";
            }
            first = false;
            w.nl();
            w.key(pair.first);
            w.out += "{";
            w.indent += 1;
            w.nl();
            w.key("codepoint");
            w.string(codepointString(pair.second.codepoint));
            w.out += ",";
            w.nl();
            w.key("componentGlyphs");
            w.out += "[";
            w.indent += 1;
            bool firstComp = true;
            for (const std::string& comp : pair.second.componentGlyphs) {
                if (!firstComp) {
                    w.out += ",";
                }
                firstComp = false;
                w.nl();
                w.string(comp);
            }
            w.indent -= 1;
            w.nl();
            w.out += "]";
            if (!pair.second.description.empty()) {
                w.out += ",";
                w.nl();
                w.key("description");
                w.string(pair.second.description);
            }
            w.indent -= 1;
            w.nl();
            w.out += "}";
        }
        w.indent -= 1;
        w.nl();
        w.out += "}";
    }

    if (!metadata.optionalGlyphs.empty()) {
        section("optionalGlyphs");
        w.out += "{";
        w.indent += 1;
        bool first = true;
        for (const auto& pair : metadata.optionalGlyphs) {
            if (!first) {
                w.out += ",";
            }
            first = false;
            w.nl();
            w.key(pair.first);
            w.out += "{";
            w.indent += 1;
            w.nl();
            w.key("classes");
            if (pair.second.classes.empty()) {
                w.out += "[]";
            } else {
                w.out += "[";
                w.indent += 1;
                bool firstClass = true;
                for (const std::string& cls : pair.second.classes) {
                    if (!firstClass) {
                        w.out += ",";
                    }
                    firstClass = false;
                    w.nl();
                    w.string(cls);
                }
                w.indent -= 1;
                w.nl();
                w.out += "]";
            }
            w.out += ",";
            w.nl();
            w.key("codepoint");
            w.string(codepointString(pair.second.codepoint));
            if (!pair.second.description.empty()) {
                w.out += ",";
                w.nl();
                w.key("description");
                w.string(pair.second.description);
            }
            w.indent -= 1;
            w.nl();
            w.out += "}";
        }
        w.indent -= 1;
        w.nl();
        w.out += "}";
    }

    if (!metadata.sets.empty()) {
        section("sets");
        w.out += "{";
        w.indent += 1;
        bool first = true;
        for (const auto& pair : metadata.sets) {
            if (!first) {
                w.out += ",";
            }
            first = false;
            w.nl();
            w.key(pair.first);
            w.out += "{";
            w.indent += 1;
            w.nl();
            w.key("description");
            w.string(pair.second.description);
            w.out += ",";
            w.nl();
            w.key("glyphs");
            w.out += "[";
            w.indent += 1;
            bool firstGlyph = true;
            for (const SetGlyphInfo& glyph : pair.second.glyphs) {
                if (!firstGlyph) {
                    w.out += ",";
                }
                firstGlyph = false;
                w.nl();
                w.out += "{";
                w.indent += 1;
                w.nl();
                w.key("alternateFor");
                w.string(glyph.alternateFor);
                w.out += ",";
                w.nl();
                w.key("codepoint");
                w.string(codepointString(glyph.codepoint));
                if (!glyph.description.empty()) {
                    w.out += ",";
                    w.nl();
                    w.key("description");
                    w.string(glyph.description);
                }
                w.out += ",";
                w.nl();
                w.key("name");
                w.string(glyph.name);
                w.indent -= 1;
                w.nl();
                w.out += "}";
            }
            w.indent -= 1;
            w.nl();
            w.out += "]";
            w.out += ",";
            w.nl();
            w.key("type");
            w.string(pair.second.type);
            w.indent -= 1;
            w.nl();
            w.out += "}";
        }
        w.indent -= 1;
        w.nl();
        w.out += "}";
    }

    // 未识别的顶层键透传
    if (metadata.passthrough.isValid()) {
        for (const std::string& k : metadata.passthrough.keys()) {
            section(k);
            w.jsonValue(metadata.passthrough.value(k));
        }
    }

    w.indent = 0;
    w.nl();
    w.out += "}";
    w.out += "\n";

    return w.out;
}

Ret MetadataWriter::write(const FontDesignProject& project, const io::path_t& path)
{
    std::string json = toJsonText(project);

    QFile file(path.toQString());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return make_ret(Ret::Code::UnknownError, std::string("failed to open for writing: ") + path.toStdString());
    }

    qint64 written = file.write(json.data(), static_cast<qint64>(json.size()));
    file.close();

    if (written != static_cast<qint64>(json.size())) {
        return make_ret(Ret::Code::UnknownError, std::string("failed to write: ") + path.toStdString());
    }

    LOGI() << "metadata saved: " << path.toStdString();

    return make_ok();
}
