/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "inotationconfiguration.h"

namespace mu::notation {
//! NOTE The state of the Engraving Options dialog which is kept between its openings — the page it
//! showed last, whether the previews draw their handles, the widths of its side columns — as the
//! settings of the application (INotationConfiguration), so that it survives a restart of the
//! application and is the same for every open dialog. A change is written to the settings at once
class EngravingOptionsState : public QObject, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(QString lastPageCode READ lastPageCode WRITE setLastPageCode NOTIFY lastPageCodeChanged)
    Q_PROPERTY(bool showsHandles READ showsHandles WRITE setShowsHandles NOTIFY showsHandlesChanged)
    Q_PROPERTY(int sidebarWidth READ sidebarWidth WRITE setSidebarWidth NOTIFY sidebarWidthChanged)
    Q_PROPERTY(int previewWidth READ previewWidth WRITE setPreviewWidth NOTIFY previewWidthChanged)

    //! NOTE The widths the columns take when nothing was kept: from the one place which has them,
    //! the member initialisers of EngravingOptionsDialogState
    Q_PROPERTY(int defaultSidebarWidth READ defaultSidebarWidth CONSTANT)
    Q_PROPERTY(int defaultPreviewWidth READ defaultPreviewWidth CONSTANT)

    muse::Inject<INotationConfiguration> configuration = { this };

public:
    explicit EngravingOptionsState(QObject* parent = nullptr);

    QString lastPageCode() const;
    void setLastPageCode(const QString& code);

    bool showsHandles() const;
    void setShowsHandles(bool shows);

    int sidebarWidth() const;
    void setSidebarWidth(int width);

    int previewWidth() const;
    void setPreviewWidth(int width);

    int defaultSidebarWidth() const;
    int defaultPreviewWidth() const;

signals:
    void lastPageCodeChanged();
    void showsHandlesChanged();
    void sidebarWidthChanged();
    void previewWidthChanged();

private:
    EngravingOptionsDialogState state() const;
    void writeState(const EngravingOptionsDialogState& state);
};
}
