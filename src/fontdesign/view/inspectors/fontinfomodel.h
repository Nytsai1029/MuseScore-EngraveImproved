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
class FontInfoModel : public AbstractFontDesignModel
{
    Q_OBJECT

    Q_PROPERTY(bool hasProject READ hasProject NOTIFY infoChanged)
    Q_PROPERTY(QString fontName READ fontName WRITE setFontName NOTIFY infoChanged)
    Q_PROPERTY(double fontVersion READ fontVersion WRITE setFontVersion NOTIFY infoChanged)
    Q_PROPERTY(int designSize READ designSize WRITE setDesignSize NOTIFY infoChanged)
    Q_PROPERTY(int sizeRangeMin READ sizeRangeMin WRITE setSizeRangeMin NOTIFY infoChanged)
    Q_PROPERTY(int sizeRangeMax READ sizeRangeMax WRITE setSizeRangeMax NOTIFY infoChanged)
    Q_PROPERTY(QString textFontFamily READ textFontFamily WRITE setTextFontFamily NOTIFY infoChanged)
    Q_PROPERTY(int upem READ upem NOTIFY infoChanged)

public:
    explicit FontInfoModel(QObject* parent = nullptr);

    bool hasProject() const;
    QString fontName() const;
    double fontVersion() const;
    int designSize() const;       // 0 = 未设置
    int sizeRangeMin() const;     // 0 = 未设置
    int sizeRangeMax() const;
    QString textFontFamily() const;
    int upem() const;

    void setFontName(const QString& name);
    void setFontVersion(double version);
    void setDesignSize(int size);
    void setSizeRangeMin(int min);
    void setSizeRangeMax(int max);
    void setTextFontFamily(const QString& family);

signals:
    void infoChanged();

private:
    void reload() override;

    void commitInfo(const SetFontInfoCommand::Info& info);
};
}
