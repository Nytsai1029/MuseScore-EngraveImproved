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

#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "context/iglobalcontext.h"
#include "iglobalconfiguration.h"

namespace mu::appshell {
//! NOTE What the toolbar of the Atelier skin shows about the current score
class AtelierContextModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool hasProject READ hasProject NOTIFY projectChanged)
    Q_PROPERTY(QString projectTitle READ projectTitle NOTIFY projectChanged)

    //! NOTE Empty for the full score
    Q_PROPERTY(QString partName READ partName NOTIFY notationChanged)

    Q_PROPERTY(bool devModeEnabled READ devModeEnabled CONSTANT)

    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<muse::IGlobalConfiguration> globalConfiguration = { this };

public:
    explicit AtelierContextModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    bool hasProject() const;
    QString projectTitle() const;
    QString partName() const;
    bool devModeEnabled() const;

signals:
    void projectChanged();
    void notationChanged();

private:
    void listenProject();
};
}
