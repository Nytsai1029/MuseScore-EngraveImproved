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

#include "abstractfontdesignmodel.h"

namespace mu::fontdesign {
class GlyphPropertiesModel : public AbstractFontDesignModel
{
    Q_OBJECT

    Q_PROPERTY(bool hasGlyph READ hasGlyph NOTIFY glyphChanged)
    Q_PROPERTY(QString glyphName READ glyphName NOTIFY glyphChanged)
    Q_PROPERTY(QString codepointHex READ codepointHex NOTIFY glyphChanged)
    Q_PROPERTY(double advanceSp READ advanceSp WRITE setAdvanceSp NOTIFY glyphChanged)
    Q_PROPERTY(QString bboxText READ bboxText NOTIFY glyphChanged)
    Q_PROPERTY(QString classesText READ classesText NOTIFY glyphChanged)

public:
    explicit GlyphPropertiesModel(QObject* parent = nullptr);

    bool hasGlyph() const;
    QString glyphName() const;
    QString codepointHex() const;
    double advanceSp() const;
    void setAdvanceSp(double sp);
    QString bboxText() const;
    QString classesText() const;

signals:
    void glyphChanged();

private:
    void reload() override;
};
}
