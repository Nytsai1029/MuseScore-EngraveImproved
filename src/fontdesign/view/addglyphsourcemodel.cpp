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
#include "addglyphsourcemodel.h"

#include <QFile>
#include <QFontDatabase>
#include <QPainterPath>
#include <QRawFont>
#include <QTemporaryFile>
#include <QTransform>

#include "translation.h"

#include "../ifontdesigneditsurface.h"
#include "../internal/io/fontfacereader.h"
#include "../internal/project/projectcommands.h"

using namespace mu::fontdesign;
using namespace muse;

AddGlyphSourceModel::AddGlyphSourceModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

QStringList AddGlyphSourceModel::engravingFontNames() const
{
    QStringList names;
    for (const auto& font : engravingFonts()->fonts()) {
        const QString name = QString::fromStdString(font->name());
        if (!names.contains(name)) {
            names << name;
        }
    }
    return names;
}

QStringList AddGlyphSourceModel::textFontFamilies() const
{
    return QFontDatabase::families();
}

QString AddGlyphSourceModel::currentCodepointHex() const
{
    FontDesignProjectPtr project = fontDesignService()->currentProject();
    if (!project || project->currentGlyph() == 0) {
        return QStringLiteral("E000");
    }
    return QString::number(static_cast<uint>(project->currentGlyph()), 16).toUpper();
}

bool AddGlyphSourceModel::parseCodepoint(const QString& hex, char32_t& out) const
{
    QString cleaned = hex.trimmed();
    cleaned.remove(QStringLiteral("U+"), Qt::CaseInsensitive);
    cleaned.remove(QStringLiteral("0x"), Qt::CaseInsensitive);

    if (cleaned.isEmpty()) {
        FontDesignProjectPtr project = fontDesignService()->currentProject();
        if (project && project->currentGlyph() != 0) {
            out = project->currentGlyph();
            return true;
        }
        return false;
    }

    bool ok = false;
    const uint value = cleaned.toUInt(&ok, 16);
    if (!ok || value == 0 || value > 0x10FFFF) {
        return false;
    }
    out = static_cast<char32_t>(value);
    return true;
}

void AddGlyphSourceModel::setLoaded(GlyphOutline&& outline, double sourceUpem, const QString& info)
{
    m_outline = std::move(outline);
    m_sourceUpem = sourceUpem > 0 ? sourceUpem : 1000.0;
    m_info = info;
    emit loadedChanged();
}

QString AddGlyphSourceModel::loadFromFontPath(const io::path_t& path, const QString& codepointHex,
                                              const QString& sourceLabel)
{
    char32_t code = 0;
    if (!parseCodepoint(codepointHex, code)) {
        return qtrc("fontdesign", "Enter a valid codepoint (e.g. E050)");
    }

    //! 内置雕版字体是 qrc 路径（":/fonts/…"），FreeType 只读文件系统：经临时文件桥接
    io::path_t readPath = path;
    QTemporaryFile tempFile;
    if (path.toQString().startsWith(u":")) {
        QFile res(path.toQString());
        if (!res.open(QIODevice::ReadOnly)) {
            return qtrc("fontdesign", "Unable to read the font resource");
        }
        if (!tempFile.open()) {
            return qtrc("fontdesign", "Unable to create a temporary file");
        }
        tempFile.write(res.readAll());
        tempFile.flush();
        readPath = io::path_t(tempFile.fileName());
    }

    FontFaceReader::FaceData face;
    Ret ret = FontFaceReader::read(readPath, face);
    if (!ret) {
        return QString::fromStdString(ret.text());
    }

    for (FontFaceReader::FaceGlyph& glyph : face.glyphs) {
        if (glyph.codepoint == code) {
            if (glyph.outline.isEmpty()) {
                return qtrc("fontdesign", "The glyph exists but has an empty outline");
            }
            const QString info = qtrc("fontdesign", "%1 · U+%2 · %3 contours")
                                 .arg(sourceLabel,
                                      QString::number(static_cast<uint>(code), 16).toUpper(),
                                      QString::number(glyph.outline.contours().size()));
            setLoaded(std::move(glyph.outline), face.upem, info);
            return QString();
        }
    }

    return qtrc("fontdesign", "U+%1 is not present in this font")
           .arg(QString::number(static_cast<uint>(code), 16).toUpper());
}

QString AddGlyphSourceModel::loadFromEngravingFont(const QString& name, const QString& codepointHex)
{
    for (const auto& font : engravingFonts()->fonts()) {
        if (QString::fromStdString(font->name()) == name) {
            return loadFromFontPath(font->fontPath(), codepointHex, name);
        }
    }
    return qtrc("fontdesign", "Unknown engraving font");
}

QString AddGlyphSourceModel::loadFromFile(const QString& filePath, const QString& codepointHex)
{
    if (filePath.trimmed().isEmpty()) {
        return qtrc("fontdesign", "Choose a font file first");
    }
    return loadFromFontPath(io::path_t(filePath), codepointHex, io::path_t(filePath).toQString().section(u'/', -1));
}

QString AddGlyphSourceModel::loadFromTextCharacter(const QString& family, const QString& text)
{
    const QString trimmed = text.trimmed();
    if (family.isEmpty() || trimmed.isEmpty()) {
        return qtrc("fontdesign", "Choose a font family and enter a character");
    }

    FontDesignProjectPtr project = fontDesignService()->currentProject();
    const double upem = project ? project->upem() : 1000.0;

    QFont qfont(family);
    qfont.setPixelSize(static_cast<int>(upem));
    QRawFont raw = QRawFont::fromFont(qfont);
    if (!raw.isValid()) {
        return qtrc("fontdesign", "Unable to load the text font");
    }

    const QVector<quint32> indexes = raw.glyphIndexesForString(trimmed);
    if (indexes.isEmpty() || indexes.first() == 0) {
        return qtrc("fontdesign", "The character is not present in this font");
    }

    QPainterPath glyphPath = raw.pathForGlyph(indexes.first());
    if (glyphPath.isEmpty()) {
        return qtrc("fontdesign", "The character has an empty outline");
    }

    //! QRawFont 路径为文本坐标（基线原点，y 向下）→ 翻为 y 向上
    QTransform flip;
    flip.scale(1.0, -1.0);
    GlyphOutline outline = GlyphOutline::fromQPainterPath(flip.map(glyphPath));
    if (outline.isEmpty()) {
        return qtrc("fontdesign", "The character has an empty outline");
    }

    const QString info = qtrc("fontdesign", "%1 · “%2” · %3 contours")
                         .arg(family, trimmed.left(2), QString::number(outline.contours().size()));
    setLoaded(std::move(outline), upem, info);
    return QString();
}

QString AddGlyphSourceModel::addToCurrentGlyph()
{
    if (m_outline.isEmpty()) {
        return qtrc("fontdesign", "Load a glyph first");
    }

    FontDesignProjectPtr project = fontDesignService()->currentProject();
    if (!project) {
        return qtrc("fontdesign", "No font project is open");
    }
    if (project->currentGlyph() == 0) {
        return qtrc("fontdesign", "Select a target codepoint in the glyph browser first");
    }

    GlyphOutline outline = m_outline;
    if (std::abs(m_sourceUpem - project->upem()) > 0.5) {
        outline.scale(project->upem() / m_sourceUpem);
    }

    //! 画布在场：追加并选中（可直接移动）；兜底走整字形替换命令
    if (IFontDesignEditSurface* surface = fontDesignService()->activeEditSurface()) {
        surface->pasteOutline(outline);
        return QString();
    }

    project->undoStack().push(std::make_unique<ReplaceOutlineCommand>(project.get(), project->currentGlyph(), outline));
    return QString();
}
