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

#include <QObject>

#include "modularity/ioc.h"
#include "iinteractive.h"

#include "engraving/iengravingfontsprovider.h"

#include "../ifontdesignservice.h"
#include "../internal/project/glyphoutline.h"

namespace mu::fontdesign {
//! 「从字体添加字形」对话框的后端（Dorico 式组合来源）：
//! 三来源——已装雕版字体 / 任意字体文件 / 系统文本字体字符。
//! 载入的轮廓按 upem 缩放后经 IFontDesignEditSurface 追加进当前字形。
class AddGlyphSourceModel : public QObject, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(bool hasLoadedGlyph READ hasLoadedGlyph NOTIFY loadedChanged)
    Q_PROPERTY(QString loadedInfo READ loadedInfo NOTIFY loadedChanged)

    muse::Inject<IFontDesignService> fontDesignService = { this };
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<mu::engraving::IEngravingFontsProvider> engravingFonts = { this };

public:
    explicit AddGlyphSourceModel(QObject* parent = nullptr);

    bool hasLoadedGlyph() const { return !m_outline.isEmpty(); }
    QString loadedInfo() const { return m_info; }

    Q_INVOKABLE QStringList engravingFontNames() const;
    Q_INVOKABLE QStringList textFontFamilies() const;
    //! 当前字形码位的 hex（对话框默认值）
    Q_INVOKABLE QString currentCodepointHex() const;

    //! 以下均返回错误信息，空串 = 成功
    Q_INVOKABLE QString loadFromEngravingFont(const QString& name, const QString& codepointHex);
    Q_INVOKABLE QString loadFromFile(const QString& filePath, const QString& codepointHex);
    Q_INVOKABLE QString loadFromTextCharacter(const QString& family, const QString& text);
    Q_INVOKABLE QString addToCurrentGlyph();

    //! 预览用：轮廓（字体单位）与来源 upem
    const GlyphOutline& loadedOutline() const { return m_outline; }
    double loadedUpem() const { return m_sourceUpem; }

signals:
    void loadedChanged();

private:
    QString loadFromFontPath(const muse::io::path_t& path, const QString& codepointHex, const QString& sourceLabel);
    bool parseCodepoint(const QString& hex, char32_t& out) const;
    void setLoaded(GlyphOutline&& outline, double sourceUpem, const QString& info);

    GlyphOutline m_outline;
    double m_sourceUpem = 1000.0;
    QString m_info;
};
}
