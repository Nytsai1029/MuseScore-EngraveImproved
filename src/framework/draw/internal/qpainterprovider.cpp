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
#include "qpainterprovider.h"

#include <QFontDatabase>
#include <QFontMetricsF>
#include <QGlyphRun>
#include <QPaintEngine>
#include <QPainter>
#include <QPainterPath>
#include <QPixmapCache>
#include <QRawFont>
#include <QStaticText>
#include <QTextLayout>
#include <QTextLine>

#include "draw/utils/drawlogger.h"
#include "types/transform.h"
#include "types/painterpath.h"

#include "log.h"

using namespace muse::draw;

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

bool familyHasMatchingItalicStyle(const QString& family, bool bold)
{
    for (const QString& style : QFontDatabase::styles(family)) {
        if (fontStyleNameHasItalic(style) && fontStyleNameHasBold(style) == bold) {
            return true;
        }
    }

    return false;
}

QString matchingUprightStyleName(const QString& family, bool bold)
{
    const QStringList styles = QFontDatabase::styles(family);
    if (styles.empty()) {
        return QString();
    }

    QStringList preferred;
    if (bold) {
        preferred << "Bold";
    } else {
        preferred << "Regular" << "Roman" << "Book";
    }

    for (const QString& preferredStyle : preferred) {
        for (const QString& style : styles) {
            if (style.compare(preferredStyle, Qt::CaseInsensitive) == 0) {
                return style;
            }
        }
    }

    for (const QString& style : styles) {
        if (fontStyleNameHasBold(style) == bold && !fontStyleNameHasItalic(style)) {
            return style;
        }
    }

    return QString();
}

bool isPdfPainter(const QPainter* painter)
{
    return painter && painter->paintEngine() && painter->paintEngine()->type() == QPaintEngine::Pdf;
}

bool requestedItalicResolvesToBoldStyle(const Font& font, const QFont& qfont)
{
    if (font.bold() || !fontStyleNameHasItalic(qfont.styleName())) {
        return false;
    }

    return fontStyleNameHasBold(qfont.styleName());
}

bool needsSyntheticPdfItalic(const Font& font, const QFont& qfont)
{
    if (!font.italic()) {
        return false;
    }

    const QString requestedFamily = font.family().id().toQString();
    if (requestedFamily.isEmpty() || fontStyleNameHasItalic(requestedFamily)) {
        return false;
    }

    if (qfont.family().compare(requestedFamily, Qt::CaseInsensitive) != 0) {
        return false;
    }

    if (!QFontDatabase::hasFamily(requestedFamily)) {
        return false;
    }

    return !familyHasMatchingItalicStyle(requestedFamily, font.bold())
           || requestedItalicResolvesToBoldStyle(font, qfont);
}

bool isCenturyOsMtStd(const Font& font)
{
    return font.family().id().toQString().compare("Century OS MT Std", Qt::CaseInsensitive) == 0;
}

bool needsPdfRegularPathWorkaround(const Font& font, const QFont& qfont)
{
    if (!isCenturyOsMtStd(font) || font.italic() || font.underline() || font.strike()) {
        return false;
    }

    if (fontStyleNameHasItalic(qfont.styleName())) {
        return false;
    }

    // Qt 6.10's PDF engine caches this family by the first face it writes. If the
    // regular face is written first, later italic text is embedded as CenturyOSMTStd.
    return true;
}

void drawTextPath(QPainter* painter, const QPainterPath& path)
{
    painter->save();
    const QPen oldPen = painter->pen();
    const QBrush textBrush = oldPen.style() == Qt::NoPen ? painter->brush() : oldPen.brush();
    painter->setPen(Qt::NoPen);
    painter->setBrush(textBrush);
    painter->drawPath(path);
    painter->restore();
}

void drawTextAsPath(QPainter* painter, const QPointF& point, const QString& text)
{
    QPainterPath path;
    path.addText(point, painter->font(), text);

    drawTextPath(painter, path);
}

QPointF alignedTextPoint(const QRectF& rect, int flags, double lineWidth, double textHeight,
                         const QFontMetricsF& fontMetrics)
{
    double x = rect.left();
    if (flags & AlignHCenter) {
        x += (rect.width() - lineWidth) / 2.0;
    } else if (flags & AlignRight) {
        x = rect.right() - lineWidth;
    }

    double y = rect.top() + fontMetrics.ascent();
    if (flags & AlignVCenter) {
        y = rect.top() + ((rect.height() - textHeight) / 2.0) + fontMetrics.ascent();
    } else if (flags & AlignBottom) {
        y = rect.bottom() - textHeight + fontMetrics.ascent();
    }

    return QPointF(x, y);
}

void drawTextAsPath(QPainter* painter, const QRectF& rect, int flags, const QString& text)
{
    if (flags & (TextWordWrap | TextWrapAnywhere)) {
        painter->drawText(rect, flags, text);
        return;
    }

    const QFontMetricsF fontMetrics(painter->font());
    QString normalizedText = text;
    normalizedText.replace("\r\n", "\n");
    normalizedText.replace('\r', '\n');
    const QStringList lines = normalizedText.split('\n');

    double textHeight = 0.0;
    if (!lines.empty()) {
        textHeight = fontMetrics.height() + (fontMetrics.lineSpacing() * (lines.size() - 1));
    }

    QPainterPath path;
    double y = alignedTextPoint(rect, flags, 0.0, textHeight, fontMetrics).y();
    for (const QString& line : lines) {
        const double lineWidth = fontMetrics.horizontalAdvance(line);
        QPointF point = alignedTextPoint(rect, flags, lineWidth, textHeight, fontMetrics);
        point.setY(y);
        path.addText(point, painter->font(), line);
        y += fontMetrics.lineSpacing();
    }

    painter->save();
    if (!(flags & TextDontClip)) {
        painter->setClipRect(rect, Qt::IntersectClip);
    }
    drawTextPath(painter, path);
    painter->restore();
}

void setSyntheticItalicBaseFont(QPainter* painter, bool bold)
{
    QFont qfont = painter->font();
    const QString styleName = matchingUprightStyleName(qfont.family(), bold);
    if (!styleName.isEmpty()) {
        qfont.setStyleName(styleName);
    }
    qfont.setBold(bold);
    qfont.setItalic(false);
    qfont.setStyle(QFont::StyleNormal);
    painter->setFont(qfont);
}

void drawTextWithSyntheticItalic(QPainter* painter, const QPointF& point, const QString& text, bool bold)
{
    painter->save();
    setSyntheticItalicBaseFont(painter, bold);
    painter->translate(point);
    painter->shear(-0.25, 0.0);
    painter->drawText(QPointF(0.0, 0.0), text);
    painter->restore();
}

void drawTextWithSyntheticItalic(QPainter* painter, const QRectF& rect, int flags, const QString& text, bool bold)
{
    painter->save();
    setSyntheticItalicBaseFont(painter, bold);
    painter->translate(rect.topLeft());
    painter->shear(-0.25, 0.0);
    painter->drawText(QRectF(QPointF(0.0, 0.0), rect.size()), flags, text);
    painter->restore();
}
}

QPainterProvider::QPainterProvider(QPainter* painter, bool ownsPainter)
    : m_painter(painter), m_ownsPainter(ownsPainter), m_drawObjectsLogger(new DrawObjectsLogger())
{
    if (painter->isActive()) {
        m_font = Font::fromQFont(m_painter->font(), Font::Type::Undefined);
        m_pen = Pen::fromQPen(m_painter->pen());
        m_brush = Brush::fromQBrush(m_painter->brush());
        m_transform = Transform::fromQTransform(m_painter->transform());
    }
}

QPainterProvider::~QPainterProvider()
{
    if (m_ownsPainter) {
        delete m_painter;
    }

    delete m_drawObjectsLogger;
}

IPaintProviderPtr QPainterProvider::make(QPaintDevice* dp)
{
    return std::make_shared<QPainterProvider>(new QPainter(dp), true);
}

IPaintProviderPtr QPainterProvider::make(QPainter* qp, bool ownsPainter)
{
    return std::make_shared<QPainterProvider>(qp, ownsPainter);
}

QPainter* QPainterProvider::qpainter() const
{
    return m_painter;
}

void QPainterProvider::beginTarget(const std::string&)
{
}

void QPainterProvider::beforeEndTargetHook(Painter*)
{
}

bool QPainterProvider::endTarget(bool endDraw)
{
    if (endDraw) {
        return m_painter->end();
    }
    return true;
}

bool QPainterProvider::isActive() const
{
    return m_painter->isActive();
}

void QPainterProvider::beginObject(const std::string& name)
{
    UNUSED(name)
    //m_drawObjectsLogger->beginObject(name);
}

void QPainterProvider::endObject()
{
    //m_drawObjectsLogger->endObject();
}

void QPainterProvider::setAntialiasing(bool arg)
{
    m_painter->setRenderHint(QPainter::Antialiasing, arg);
    m_painter->setRenderHint(QPainter::TextAntialiasing, arg);
}

void QPainterProvider::setCompositionMode(CompositionMode mode)
{
    auto toQPainter = [](CompositionMode mode) {
        switch (mode) {
        case CompositionMode::SourceOver: return QPainter::CompositionMode_SourceOver;
        case CompositionMode::HardLight: return QPainter::CompositionMode_HardLight;
        }
        return QPainter::CompositionMode_SourceOver;
    };
    m_painter->setCompositionMode(toQPainter(mode));
}

void QPainterProvider::setWindow(const RectF& window)
{
    // no need set
    UNUSED(window);
}

void QPainterProvider::setViewport(const RectF& viewport)
{
    // no need set
    UNUSED(viewport);
}

void QPainterProvider::setFont(const Font& font)
{
    if (m_font != font) {
        m_painter->setFont(font.toQFont());
        m_font = font;
    }
}

const Font& QPainterProvider::font() const
{
    return m_font;
}

void QPainterProvider::setPen(const Pen& pen)
{
    m_pen = pen;
    m_painter->setPen(Pen::toQPen(m_pen));
}

void QPainterProvider::setNoPen()
{
    m_pen = Pen(PenStyle::NoPen);
    m_painter->setPen(Pen::toQPen(m_pen));
}

const Pen& QPainterProvider::pen() const
{
    return m_pen;
}

void QPainterProvider::setBrush(const Brush& brush)
{
    m_brush = brush;
    m_painter->setBrush(Brush::toQBrush(m_brush));
}

const Brush& QPainterProvider::brush() const
{
    return m_brush;
}

void QPainterProvider::save()
{
    m_painter->save();
}

void QPainterProvider::restore()
{
    m_painter->restore();
    m_font = Font::fromQFont(m_painter->font(), Font::Type::Undefined);
    m_pen = Pen::fromQPen(m_painter->pen());
    m_brush = Brush::fromQBrush(m_painter->brush());
    m_transform = Transform::fromQTransform(m_painter->transform());
}

void QPainterProvider::setTransform(const Transform& transform)
{
    m_transform = transform;
    m_painter->setTransform(Transform::toQTransform(m_transform));
}

const Transform& QPainterProvider::transform() const
{
    return m_transform;
}

// drawing functions

void QPainterProvider::drawPath(const PainterPath& path)
{
    m_painter->drawPath(PainterPath::toQPainterPath(path));
}

void QPainterProvider::drawPolygon(const PointF* points, size_t pointCount, PolygonMode mode)
{
    static_assert(sizeof(QPointF) == sizeof(PointF), "sizeof(QPointF) and sizeof(PointF) must be equal");

    const QPointF* qpoints = reinterpret_cast<const QPointF*>(points);

    switch (mode) {
    case PolygonMode::OddEven: {
        m_painter->drawPolygon(qpoints, int(pointCount), Qt::OddEvenFill);
    } break;
    case PolygonMode::Winding: {
        m_painter->drawPolygon(qpoints, int(pointCount), Qt::WindingFill);
    } break;
    case PolygonMode::Convex: {
        m_painter->drawConvexPolygon(qpoints, int(pointCount));
    } break;
    case PolygonMode::Polyline: {
        m_painter->drawPolyline(qpoints, int(pointCount));
    } break;
    }
}

void QPainterProvider::drawText(const PointF& point, const String& text)
{
    QPointF p = point.toQPointF();
    QString t = text.toQString();

    if (isPdfPainter(m_painter) && needsPdfRegularPathWorkaround(m_font, m_painter->font())) {
        drawTextAsPath(m_painter, p, t);
        return;
    }

    if (isPdfPainter(m_painter) && needsSyntheticPdfItalic(m_font, m_painter->font())) {
        drawTextWithSyntheticItalic(m_painter, p, t, m_font.bold());
        return;
    }

    m_painter->drawText(p, t);
}

void QPainterProvider::drawText(const RectF& rect, int flags, const String& text)
{
    QRectF r = rect.toQRectF();
    QString t = text.toQString();

    if (isPdfPainter(m_painter) && needsPdfRegularPathWorkaround(m_font, m_painter->font())) {
        drawTextAsPath(m_painter, r, flags, t);
        return;
    }

    if (isPdfPainter(m_painter) && needsSyntheticPdfItalic(m_font, m_painter->font())) {
        drawTextWithSyntheticItalic(m_painter, r, flags, t, m_font.bold());
        return;
    }

    m_painter->drawText(r, flags, t);
}

void QPainterProvider::drawTextWorkaround(const Font& f, const PointF& pos, const String& text)
{
    m_painter->save();
    double mm = m_painter->worldTransform().m11();
    double dx = m_painter->worldTransform().dx();
    double dy = m_painter->worldTransform().dy();
    // diagonal elements will now be changed to 1.0
    m_painter->setWorldTransform(QTransform(1.0, 0.0, 0.0, 1.0, dx, dy));

    // correction factor for bold text drawing, due to the change of the diagonal elements
    double factor = 1.0 / mm;
    QFont fnew(f.toQFont(), m_painter->device());
    fnew.setPointSizeF(f.pointSizeF() / factor);
    QRawFont fRaw = QRawFont::fromFont(fnew);
    QTextLayout textLayout(text, f.toQFont(), m_painter->device());
    textLayout.beginLayout();
    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid()) {
            break;
        }
    }
    textLayout.endLayout();
    // glyphruns with correct positions, but potentially wrong glyphs
    // (see bug https://musescore.org/en/node/117191 regarding positions and DPI)
    QList<QGlyphRun> glyphruns = textLayout.glyphRuns();
    double offset = 0;
    // glyphrun drawing has an offset equal to the max ascent of the text fragment
    for (int i = 0; i < glyphruns.length(); i++) {
        double value = glyphruns.at(i).rawFont().ascent() / factor;
        if (value > offset) {
            offset = value;
        }
    }
    for (int i = 0; i < glyphruns.length(); i++) {
        QVector<QPointF> positions1 = glyphruns.at(i).positions();
        QVector<QPointF> positions2;
        // calculate the new positions for the scaled geometry
        for (int j = 0; j < positions1.length(); j++) {
            QPointF newPoint = positions1.at(j) / factor;
            positions2.append(newPoint);
        }
        QGlyphRun glyphrun2 = glyphruns.at(i);
        glyphrun2.setPositions(positions2);
        // change the glyphs with the correct glyphs
        // and account for glyph substitution
        if (glyphrun2.rawFont().familyName() != fnew.family()) {
            QFont f2(fnew);
            f2.setFamily(glyphrun2.rawFont().familyName());
            glyphrun2.setRawFont(QRawFont::fromFont(f2));
        } else {
            glyphrun2.setRawFont(fRaw);
        }
        m_painter->drawGlyphRun(QPointF(pos.x() / factor, pos.y() / factor - offset), glyphrun2);
        positions2.clear();
    }
    // Restore the QPainter to its former state
    m_painter->setWorldTransform(QTransform(mm, 0.0, 0.0, mm, dx, dy));
    m_painter->restore();
}

void QPainterProvider::drawSymbol(const PointF& point, char32_t ucs4Code)
{
    static QHash<char32_t, QString> cache;
    if (!cache.contains(ucs4Code)) {
        cache[ucs4Code] = QString::fromUcs4(&ucs4Code, 1);
    }

    drawText(point, cache.value(ucs4Code));
}

void QPainterProvider::drawPixmap(const PointF& point, const Pixmap& pm)
{
    QString key = QString::number(pm.key());
    QPixmap pixmap;
    if (!QPixmapCache::find(key, &pixmap)) {
        pixmap.loadFromData(pm.data().toQByteArrayNoCopy());
        QPixmapCache::insert(key, pixmap);
    }

    m_painter->drawPixmap(QPointF(point.x(), point.y()), pixmap);
}

void QPainterProvider::drawTiledPixmap(const RectF& rect, const Pixmap& pm, const PointF& offset)
{
    QString key = QString::number(pm.key());
    QPixmap pixmap;
    if (!QPixmapCache::find(key, &pixmap)) {
        pixmap.loadFromData(pm.data().toQByteArrayNoCopy());
        QPixmapCache::insert(key, pixmap);
    }

    m_painter->drawTiledPixmap(rect.toQRectF(), pixmap, QPointF(offset.x(), offset.y()));
}

void QPainterProvider::drawPixmap(const PointF& point, const QPixmap& pm)
{
    m_painter->drawPixmap(QPointF(point.x(), point.y()), pm);
}

void QPainterProvider::drawTiledPixmap(const RectF& rect, const QPixmap& pm, const PointF& offset)
{
    m_painter->drawTiledPixmap(rect.toQRectF(), pm, QPointF(offset.x(), offset.y()));
}

bool QPainterProvider::hasClipping() const
{
    return m_painter->hasClipping();
}

void QPainterProvider::setClipRect(const RectF& rect)
{
    m_painter->setClipRect(rect.toQRectF());
}

void QPainterProvider::setMask(const RectF& background, const std::vector<RectF>& maskRects)
{
    if (maskRects.empty()) {
        m_painter->setClipPath(QPainterPath(), Qt::NoClip);
        return;
    }

    QPainterPath backgroundPath;
    backgroundPath.addRect(background.toQRectF());

    QPainterPath exclusionRegion;
    exclusionRegion.setFillRule(Qt::WindingFill);
    for (const RectF& rect : maskRects) {
        exclusionRegion.addRect(rect.toQRectF());
    }

    QPainterPath mask = backgroundPath.subtracted(exclusionRegion);

    m_painter->setClipPath(mask);
}

void QPainterProvider::setClipping(bool enable)
{
    m_painter->setClipping(enable);
}
