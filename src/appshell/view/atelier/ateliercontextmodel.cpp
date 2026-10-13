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
#include "ateliercontextmodel.h"

using namespace mu::appshell;
using namespace mu::notation;
using namespace mu::project;

AtelierContextModel::AtelierContextModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void AtelierContextModel::load()
{
    globalContext()->currentProjectChanged().onNotify(this, [this]() {
        listenProject();
        emit projectChanged();
    });

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        emit notationChanged();
    });

    listenProject();
}

void AtelierContextModel::listenProject()
{
    INotationProjectPtr project = globalContext()->currentProject();
    if (!project) {
        return;
    }

    project->displayNameChanged().onNotify(this, [this]() {
        emit projectChanged();
    });
}

bool AtelierContextModel::hasProject() const
{
    return globalContext()->currentProject() != nullptr;
}

QString AtelierContextModel::projectTitle() const
{
    INotationProjectPtr project = globalContext()->currentProject();
    return project ? project->displayName() : QString();
}

QString AtelierContextModel::partName() const
{
    INotationPtr notation = globalContext()->currentNotation();
    IMasterNotationPtr masterNotation = globalContext()->currentMasterNotation();

    if (!notation || !masterNotation || masterNotation->notation() == notation) {
        return QString();
    }

    return notation->name();
}

bool AtelierContextModel::devModeEnabled() const
{
    return globalConfiguration()->devModeEnabled();
}
