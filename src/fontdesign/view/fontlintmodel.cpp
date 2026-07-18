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
#include "fontlintmodel.h"

#include <map>
#include <set>

#include "translation.h"

#include "../internal/project/outlinegeometry.h"
#include "../internal/smufldatabase.h"

using namespace mu::fontdesign;
using namespace muse;

//! MuseScore 排版实际依赖的核心 SMuFL 字形（缺失时乐谱会明显残缺）
static const std::vector<char32_t> CORE_GLYPHS = {
    0xE0A0, 0xE0A2, 0xE0A3, 0xE0A4,                     // noteheads: doubleWhole/whole/half/black
    0xE050, 0xE05C, 0xE062,                             // clefs: G/C/F
    0xE260, 0xE261, 0xE262, 0xE263, 0xE264,             // accidentals: b/nat/#/x/bb
    0xE4E3, 0xE4E4, 0xE4E5, 0xE4E6, 0xE4E7,             // rests: whole..16th
    0xE240, 0xE241, 0xE242, 0xE243,                     // flags: 8thUp/8thDown/16thUp/16thDown
    0xE080, 0xE081, 0xE082, 0xE083, 0xE084,             // timeSig 0-4
    0xE085, 0xE086, 0xE087, 0xE088, 0xE089,             // timeSig 5-9
    0xE1E7,                                             // augmentationDot
};

static QString codeHex(char32_t code)
{
    return QStringLiteral("U+") + QString::number(static_cast<uint>(code), 16).toUpper();
}

FontLintModel::FontLintModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

int FontLintModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_items.size());
}

QVariant FontLintModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= static_cast<int>(m_items.size())) {
        return QVariant();
    }
    const Item& item = m_items[index.row()];
    switch (role) {
    case SeverityRole: return item.severity;
    case MessageRole: return item.message;
    case CodepointRole: return static_cast<uint>(item.codepoint);
    default: return QVariant();
    }
}

QHash<int, QByteArray> FontLintModel::roleNames() const
{
    return {
        { SeverityRole, "severity" },
        { MessageRole, "message" },
        { CodepointRole, "codepoint" },
    };
}

void FontLintModel::addItem(const QString& severity, const QString& message, char32_t codepoint)
{
    Item item;
    item.severity = severity;
    item.message = message;
    item.codepoint = codepoint;
    m_items.push_back(item);
}

void FontLintModel::run()
{
    beginResetModel();
    m_items.clear();

    FontDesignProjectPtr project = fontDesignService()->currentProject();
    if (!project) {
        m_summary = qtrc("fontdesign", "No font project is open");
        endResetModel();
        emit resultsChanged();
        return;
    }

    const SmuflDatabase& db = fontDesignService()->smuflDatabase();
    auto glyphLabel = [&db](char32_t code) {
        QString label = codeHex(code);
        if (const SmuflDatabase::GlyphInfo* info = db.infoByCodepoint(code)) {
            label += QStringLiteral(" ") + QString::fromStdString(info->name);
        }
        return label;
    };

    // 1. 字体名规则（安装约定）
    const std::string& fontName = project->metadata().fontName;
    if (fontName.empty()) {
        addItem("error", qtrc("fontdesign", "Font name is empty"));
    } else {
        if (fontName.find("Text") != std::string::npos) {
            addItem("warning", qtrc("fontdesign", "Font name contains “Text” — the installer skips such folders (reserved for text companion fonts)"));
        }
        if (fontName.find('/') != std::string::npos || fontName.find('\\') != std::string::npos
            || fontName.find(':') != std::string::npos) {
            addItem("error", qtrc("fontdesign", "Font name contains invalid path characters"));
        }
    }

    // 2. 核心字形覆盖
    for (char32_t code : CORE_GLYPHS) {
        const GlyphItem* glyph = project->glyph(code);
        if (!glyph || glyph->outline.isEmpty()) {
            addItem("warning", qtrc("fontdesign", "Missing core glyph %1").arg(glyphLabel(code)), code);
        }
    }

    // 3/4/5. 逐字形检查：空轮廓 / advance / 环绕方向
    int wrongDirectionGlyphs = 0;
    for (const auto& pair : project->glyphs()) {
        const GlyphItem& glyph = pair.second;

        if (glyph.outline.isEmpty()) {
            addItem("info", qtrc("fontdesign", "%1 has no outline").arg(glyphLabel(glyph.codepoint)), glyph.codepoint);
            continue;
        }

        if (glyph.advance <= 0) {
            addItem("warning", qtrc("fontdesign", "%1 has no advance width").arg(glyphLabel(glyph.codepoint)),
                    glyph.codepoint);
        }

        int wrongContours = 0;
        const auto& contours = glyph.outline.contours();
        for (int ci = 0; ci < static_cast<int>(contours.size()); ++ci) {
            if (!outlinegeom::contourDirectionIsCorrect(contours, ci)) {
                ++wrongContours;
            }
        }
        if (wrongContours > 0) {
            ++wrongDirectionGlyphs;
            addItem("warning",
                    qtrc("fontdesign", "%1 has %2 contour(s) with wrong winding direction — holes may render filled (use “Correct path directions”)")
                    .arg(glyphLabel(glyph.codepoint)).arg(wrongContours),
                    glyph.codepoint);
        }
    }

    // 6. optionalGlyphs PUA 冲突
    std::map<char32_t, std::vector<std::string>> optionalByCode;
    for (const auto& pair : project->metadata().optionalGlyphs) {
        optionalByCode[pair.second.codepoint].push_back(pair.first);
    }
    for (const auto& pair : optionalByCode) {
        if (pair.second.size() > 1) {
            QString names;
            for (const std::string& n : pair.second) {
                if (!names.isEmpty()) {
                    names += QStringLiteral(", ");
                }
                names += QString::fromStdString(n);
            }
            addItem("error", qtrc("fontdesign", "Optional glyphs share codepoint %1: %2")
                    .arg(codeHex(pair.first), names), pair.first);
        }
        if (pair.first < 0xF400 && pair.first >= 0xE000) {
            addItem("warning", qtrc("fontdesign", "Optional glyph %1 lies in the recommended range (optional glyphs start at U+F400)")
                    .arg(codeHex(pair.first)), pair.first);
        }
    }

    // 汇总
    int errors = 0;
    int warnings = 0;
    for (const Item& item : m_items) {
        if (item.severity == QLatin1String("error")) {
            ++errors;
        } else if (item.severity == QLatin1String("warning")) {
            ++warnings;
        }
    }
    if (m_items.empty()) {
        m_summary = qtrc("fontdesign", "All checks passed — %1 glyphs")
                    .arg(QString::number(project->glyphs().size()));
    } else {
        m_summary = qtrc("fontdesign", "%1 errors · %2 warnings · %3 notes")
                    .arg(errors).arg(warnings).arg(static_cast<int>(m_items.size()) - errors - warnings);
    }

    endResetModel();
    emit resultsChanged();
}

void FontLintModel::goToGlyph(int row)
{
    if (row < 0 || row >= static_cast<int>(m_items.size())) {
        return;
    }
    const char32_t code = m_items[row].codepoint;
    if (code == 0) {
        return;
    }
    if (FontDesignProjectPtr project = fontDesignService()->currentProject()) {
        project->setCurrentGlyph(code);
    }
}
