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
#include "glyphpropertiesmodel.h"

#include <QPainterPath>

#include "../../internal/smufldatabase.h"

using namespace mu::fontdesign;

GlyphPropertiesModel::GlyphPropertiesModel(QObject* parent)
    : AbstractFontDesignModel(parent)
{
}

bool GlyphPropertiesModel::hasGlyph() const
{
    return currentGlyphItem() != nullptr;
}

QString GlyphPropertiesModel::glyphName() const
{
    const GlyphItem* glyph = currentGlyphItem();
    if (!glyph) {
        return QString();
    }

    return glyph->smuflName.empty() ? QStringLiteral("—") : QString::fromStdString(glyph->smuflName);
}

QString GlyphPropertiesModel::codepointHex() const
{
    const GlyphItem* glyph = currentGlyphItem();
    if (!glyph) {
        return QString();
    }

    return QString("U+%1").arg(QString::number(glyph->codepoint, 16).toUpper().rightJustified(4, '0'));
}

double GlyphPropertiesModel::advanceSp() const
{
    const GlyphItem* glyph = currentGlyphItem();
    return glyph && project() ? glyph->advance / project()->spatium() : 0.0;
}

void GlyphPropertiesModel::setAdvanceSp(double sp)
{
    const GlyphItem* glyph = currentGlyphItem();
    if (!glyph || !project() || qFuzzyCompare(sp, advanceSp())) {
        return;
    }

    pushCommand(std::make_unique<SetAdvanceCommand>(project().get(), glyph->codepoint, sp * project()->spatium()));
}

QString GlyphPropertiesModel::bboxText() const
{
    const GlyphItem* glyph = currentGlyphItem();
    if (!glyph || !project() || glyph->outline.isEmpty()) {
        return QStringLiteral("—");
    }

    QRectF bbox = glyph->outline.toPainterPath().toQPainterPath().boundingRect();
    double sp = project()->spatium();

    return QString("SW (%1, %2)  NE (%3, %4) sp")
           .arg(QString::number(bbox.x() / sp, 'f', 3),
                QString::number(bbox.y() / sp, 'f', 3),
                QString::number((bbox.x() + bbox.width()) / sp, 'f', 3),
                QString::number((bbox.y() + bbox.height()) / sp, 'f', 3));
}

QString GlyphPropertiesModel::classesText() const
{
    const GlyphItem* glyph = currentGlyphItem();
    if (!glyph || glyph->smuflName.empty()) {
        return QString();
    }

    std::vector<std::string> classes = fontDesignService()->smuflDatabase().classesOfGlyph(glyph->smuflName);

    QStringList list;
    for (const std::string& cls : classes) {
        list << QString::fromStdString(cls);
    }

    return list.join(", ");
}

void GlyphPropertiesModel::reload()
{
    emit glyphChanged();
}
