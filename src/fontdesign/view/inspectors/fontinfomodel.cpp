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
#include "fontinfomodel.h"

using namespace mu::fontdesign;

FontInfoModel::FontInfoModel(QObject* parent)
    : AbstractFontDesignModel(parent)
{
}

bool FontInfoModel::hasProject() const
{
    return project() != nullptr;
}

QString FontInfoModel::fontName() const
{
    return project() ? QString::fromStdString(project()->metadata().fontName) : QString();
}

double FontInfoModel::fontVersion() const
{
    return project() ? project()->metadata().fontVersion : 0.0;
}

int FontInfoModel::designSize() const
{
    return project() && project()->metadata().designSize.has_value()
           ? project()->metadata().designSize.value() : 0;
}

int FontInfoModel::sizeRangeMin() const
{
    return project() && project()->metadata().sizeRange.has_value()
           ? project()->metadata().sizeRange->first : 0;
}

int FontInfoModel::sizeRangeMax() const
{
    return project() && project()->metadata().sizeRange.has_value()
           ? project()->metadata().sizeRange->second : 0;
}

QString FontInfoModel::textFontFamily() const
{
    return project() ? QString::fromStdString(project()->metadata().textFontFamily) : QString();
}

int FontInfoModel::upem() const
{
    return project() ? static_cast<int>(project()->upem()) : 0;
}

void FontInfoModel::setFontName(const QString& name)
{
    if (!project() || name == fontName()) {
        return;
    }

    SetFontInfoCommand::Info info = SetFontInfoCommand::captureOf(*project());
    info.fontName = name.toStdString();
    commitInfo(info);
}

void FontInfoModel::setFontVersion(double version)
{
    if (!project() || qFuzzyCompare(version, fontVersion())) {
        return;
    }

    SetFontInfoCommand::Info info = SetFontInfoCommand::captureOf(*project());
    info.fontVersion = version;
    commitInfo(info);
}

void FontInfoModel::setDesignSize(int size)
{
    if (!project() || size == designSize()) {
        return;
    }

    SetFontInfoCommand::Info info = SetFontInfoCommand::captureOf(*project());
    if (size > 0) {
        info.designSize = size;
    } else {
        info.designSize.reset();
    }
    commitInfo(info);
}

void FontInfoModel::setSizeRangeMin(int min)
{
    if (!project() || min == sizeRangeMin()) {
        return;
    }

    SetFontInfoCommand::Info info = SetFontInfoCommand::captureOf(*project());
    if (min > 0 || sizeRangeMax() > 0) {
        info.sizeRange = std::make_pair(min, sizeRangeMax());
    } else {
        info.sizeRange.reset();
    }
    commitInfo(info);
}

void FontInfoModel::setSizeRangeMax(int max)
{
    if (!project() || max == sizeRangeMax()) {
        return;
    }

    SetFontInfoCommand::Info info = SetFontInfoCommand::captureOf(*project());
    if (max > 0 || sizeRangeMin() > 0) {
        info.sizeRange = std::make_pair(sizeRangeMin(), max);
    } else {
        info.sizeRange.reset();
    }
    commitInfo(info);
}

void FontInfoModel::setTextFontFamily(const QString& family)
{
    if (!project() || family == textFontFamily()) {
        return;
    }

    SetFontInfoCommand::Info info = SetFontInfoCommand::captureOf(*project());
    info.textFontFamily = family.toStdString();
    commitInfo(info);
}

void FontInfoModel::reload()
{
    emit infoChanged();
}

void FontInfoModel::commitInfo(const SetFontInfoCommand::Info& info)
{
    pushCommand(std::make_unique<SetFontInfoCommand>(project().get(), info));
}
