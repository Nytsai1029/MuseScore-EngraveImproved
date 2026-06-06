/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "font.h"
#include "global/realfn.h"

#ifndef NO_QT_SUPPORT
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFontInfo>
#include <QSet>
#include <QStandardPaths>

#include <mutex>
#endif

using namespace muse;
using namespace muse::draw;

bool Font::g_disableFontMerging = false;

#ifndef NO_QT_SUPPORT
namespace {
bool fontStyleNameHasItalic(const QString& styleName)
{
    const QString lowerStyleName = styleName.toLower();
    return lowerStyleName.contains("italic")
           || lowerStyleName.contains("oblique")
           || lowerStyleName.contains("boldita")
           || lowerStyleName.contains("bdita");
}

bool fontStyleNameHasBold(const QString& styleName)
{
    const QString lowerStyleName = styleName.toLower();
    return lowerStyleName.contains("bold")
           || lowerStyleName.contains("bdita");
}

QString matchingStyleName(const QString& family, bool bold, bool italic);

QString normalizedFontFamilyName(const QString& name)
{
    QString normalized;
    normalized.reserve(name.size());

    for (const QChar& ch : name) {
        if (ch.isLetterOrNumber()) {
            normalized.append(ch.toLower());
        }
    }

    return normalized;
}

QStringList fontSearchPaths()
{
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::FontsLocation);

    const QString homePath = QDir::homePath();
    if (!homePath.isEmpty()) {
        paths << homePath + "/Library/Fonts";
    }

    const QString userName = qEnvironmentVariable("USER");
    if (!userName.isEmpty()) {
        paths << "/Users/" + userName + "/Library/Fonts";
    }

    paths << "/Library/Fonts"
          << "/System/Library/Fonts"
          << "/System/Library/Fonts/Supplemental";
    paths.removeDuplicates();

    return paths;
}

const QStringList& localFontFiles()
{
    static const QStringList files = []() {
        QStringList result;
        const QStringList filters {
            "*.otf", "*.ttf", "*.otc", "*.ttc"
        };

        for (const QString& path : fontSearchPaths()) {
            QDir dir(path);
            if (!dir.exists()) {
                continue;
            }

            QDirIterator it(dir.absolutePath(), filters, QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                result << it.next();
            }
        }

        result.removeDuplicates();
        return result;
    }();

    return files;
}

bool hasRequestedFontStyle(const QString& family, bool bold, bool italic)
{
    if (!QFontDatabase::hasFamily(family)) {
        return false;
    }

    if (!bold && !italic) {
        return true;
    }

    return !matchingStyleName(family, bold, italic).isEmpty();
}

void ensureApplicationFontFamilyAvailable(const QString& family, bool bold, bool italic)
{
    if (family.isEmpty()) {
        return;
    }

    if (!bold && !italic && hasRequestedFontStyle(family, bold, italic)) {
        return;
    }

    const QString normalizedFamily = normalizedFontFamilyName(family);
    if (normalizedFamily.isEmpty()) {
        return;
    }

    static std::mutex mutex;
    static QSet<QString> attemptedFamilies;

    std::lock_guard<std::mutex> lock(mutex);
    if (attemptedFamilies.contains(normalizedFamily)) {
        return;
    }
    attemptedFamilies.insert(normalizedFamily);

    for (const QString& path : localFontFiles()) {
        const QString normalizedFileName = normalizedFontFamilyName(QFileInfo(path).completeBaseName());
        if (normalizedFileName.contains(normalizedFamily)) {
            QFontDatabase::addApplicationFont(path);
        }
    }
}

QString matchingStyleName(const QString& family, bool bold, bool italic)
{
    if (!bold && !italic) {
        return QString();
    }

    const QStringList styles = QFontDatabase::styles(family);
    if (styles.empty()) {
        return QString();
    }

    QStringList preferred;
    if (bold && italic) {
        preferred << "Bold Italic" << "Bold Oblique";
    } else if (bold) {
        preferred << "Bold";
    } else {
        preferred << "Italic" << "Oblique";
    }

    for (const QString& preferredStyle : preferred) {
        for (const QString& style : styles) {
            if (style.compare(preferredStyle, Qt::CaseInsensitive) == 0) {
                return style;
            }
        }
    }

    for (const QString& style : styles) {
        if (fontStyleNameHasBold(style) == bold && fontStyleNameHasItalic(style) == italic) {
            return style;
        }
    }

    return QString();
}

void applyFontStyle(QFont& qf, QFont::Weight weight, bool bold, bool italic)
{
    qf.setWeight(weight);
    qf.setBold(bold);
    qf.setItalic(italic);
}

bool applyMatchingStyleName(QFont& qf, const QString& family, QFont::Weight weight, bool bold, bool italic)
{
    const QString styleName = matchingStyleName(family, bold, italic);
    if (styleName.isEmpty()) {
        return false;
    }

    qf.setFamily(family);
    qf.setStyleName(styleName);
    applyFontStyle(qf, weight, bold, italic);
    return true;
}
}
#endif

Font::Font(const FontFamily& family, Type type)
    : m_family(family), m_type(type)
{
}

bool Font::operator ==(const Font& other) const
{
    //! NOTE At the moment, the type is entered for information,
    //! its correct installation is not guaranteed,
    //! so we do not take it when comparing it yet
    // && m_type == other.m_type

    return m_family == other.m_family
           && RealIsEqual(m_pointSizeF, other.m_pointSizeF)
           && m_weight == other.m_weight
           && m_style == other.m_style
           && RealIsEqual(m_letterSpacing, other.m_letterSpacing)
           && m_noFontMerging == other.m_noFontMerging
           && m_hinting == other.m_hinting;
}

void Font::setFamily(const FontFamily& family, Type type)
{
    m_family = family;
    m_type = type;
}

Font::FontFamily Font::family() const
{
    return m_family;
}

Font::Type Font::type() const
{
    return m_type;
}

double Font::pointSizeF() const
{
    return m_pointSizeF;
}

void Font::setPointSizeF(double s)
{
    m_pointSizeF = s;
    m_pixelSize = -1;
}

int Font::pixelSize() const
{
    return m_pixelSize;
}

void Font::setPixelSize(int s)
{
    m_pixelSize = s;
    m_pointSizeF = -1.0;
}

Font::Weight Font::weight() const
{
    return m_weight;
}

void Font::setWeight(Weight w)
{
    m_weight = w;
}

bool Font::bold() const
{
    return m_style.testFlag(Style::Bold);
}

void Font::setBold(bool arg)
{
    m_style.setFlag(Style::Bold, arg);
}

bool Font::italic() const
{
    return m_style.testFlag(Style::Italic);
}

void Font::setItalic(bool arg)
{
    m_style.setFlag(Style::Italic, arg);
}

bool Font::underline() const
{
    return m_style.testFlag(Style::Underline);
}

void Font::setUnderline(bool arg)
{
    m_style.setFlag(Style::Underline, arg);
}

bool Font::strike() const
{
    return m_style.testFlag(Style::Strike);
}

void Font::setStrike(bool arg)
{
    m_style.setFlag(Style::Strike, arg);
}

double Font::letterSpacing() const
{
    return m_letterSpacing;
}

void Font::setLetterSpacing(double spacing)
{
    m_letterSpacing = spacing;
}

void Font::setNoFontMerging(bool arg)
{
    m_noFontMerging = arg;
}

bool Font::noFontMerging() const
{
    if (g_disableFontMerging) {
        return true;
    }
    return m_noFontMerging;
}

Font::Hinting Font::hinting() const
{
    return m_hinting;
}

void Font::setHinting(Hinting hinting)
{
    m_hinting = hinting;
}

#ifndef NO_QT_SUPPORT
QFont Font::toQFont() const
{
    const QString requestedFamily = family().id().toQString();
    const bool isBold = bold() || fontStyleNameHasBold(requestedFamily);
    const bool isItalic = italic() || fontStyleNameHasItalic(requestedFamily);
    const QFont::Weight fontWeight = static_cast<QFont::Weight>(weight());

    ensureApplicationFontFamilyAvailable(requestedFamily, isBold, isItalic);

    QFont qf(requestedFamily);

    if (pointSizeF() > 0) {
        qf.setPointSizeF(pointSizeF());
    } else if (pixelSize() > 0) {
        qf.setPixelSize(pixelSize());
    }
    applyFontStyle(qf, fontWeight, isBold, isItalic);
    qf.setUnderline(underline());
    qf.setStrikeOut(strike());

    // Some PDF backends choose the regular face unless a concrete family style is set.
    const bool hasMatchingRequestedStyle = applyMatchingStyleName(qf, requestedFamily, fontWeight, isBold, isItalic);
    if ((isBold || isItalic) && !hasMatchingRequestedStyle && !QFontDatabase::hasFamily(requestedFamily)) {
        const QString fallbackFamily = QFontInfo(qf).family();
        if (!fallbackFamily.isEmpty() && fallbackFamily.compare(requestedFamily, Qt::CaseInsensitive) != 0) {
            applyMatchingStyleName(qf, fallbackFamily, fontWeight, isBold, isItalic);
        }
    }

    if (!RealIsNull(m_letterSpacing)) {
        qf.setLetterSpacing(QFont::PercentageSpacing, 100.0 + m_letterSpacing);
    }
    if (noFontMerging()) {
        qf.setStyleStrategy(QFont::NoFontMerging);
    }
    qf.setHintingPreference(static_cast<QFont::HintingPreference>(hinting()));

    return qf;
}

Font Font::fromQFont(const QFont& qf, Font::Type type)
{
    Font f(String::fromQString(qf.family()), type);
    if (qf.pointSizeF() > 0) {
        f.setPointSizeF(qf.pointSizeF());
    } else if (qf.pixelSize() > 0) {
        f.setPixelSize(qf.pixelSize());
    }
    f.setWeight(static_cast<Font::Weight>(qf.weight()));
    f.setBold(qf.bold());
    f.setItalic(qf.italic());
    f.setUnderline(qf.underline());
    f.setStrike(qf.strikeOut());
    if (qf.letterSpacingType() == QFont::PercentageSpacing) {
        f.setLetterSpacing(qf.letterSpacing() - 100.0);
    }
    if (qf.styleStrategy() == QFont::NoFontMerging) {
        f.setNoFontMerging(true);
    }
    f.setHinting(static_cast<Font::Hinting>(qf.hintingPreference()));
    return f;
}

#endif
