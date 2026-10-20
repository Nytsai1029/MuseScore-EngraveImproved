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

#include "actions/iactionsdispatcher.h"
#include "context/iglobalcontext.h"
#include "notation/inotationconfiguration.h"

namespace mu::notation {
//! NOTE What the tools of the Engrave mode need: the step of the nudge, the placement state of the selection,
//! and the layout aids which are not actions (the staff spacers)
class EngraveModeModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(double nudgeStep READ nudgeStep WRITE setNudgeStep NOTIFY nudgeStepChanged)
    Q_PROPERTY(bool isSnappedToGrid READ isSnappedToGrid WRITE setIsSnappedToGrid NOTIFY isSnappedToGridChanged)
    Q_PROPERTY(
        bool isLedgerGuidesVisible READ isLedgerGuidesVisible WRITE setIsLedgerGuidesVisible NOTIFY isLedgerGuidesVisibleChanged)

    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionStateChanged)
    Q_PROPERTY(bool isAutoplaceOn READ isAutoplaceOn NOTIFY selectionStateChanged)

    //! NOTE One or more lines (systems) are selected whole, so they can be shown as they will be printed
    Q_PROPERTY(bool isWholeLineSelected READ isWholeLineSelected NOTIFY selectionStateChanged)

    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<INotationConfiguration> configuration = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };

public:
    explicit EngraveModeModel(QObject* parent = nullptr);

    enum SpacerKind {
        SpacerDown,
        SpacerUp,
        SpacerFixedDown
    };
    Q_ENUM(SpacerKind)

    double nudgeStep() const;
    bool isSnappedToGrid() const;
    bool isLedgerGuidesVisible() const;

    bool hasSelection() const;
    bool isAutoplaceOn() const;
    bool isWholeLineSelected() const;

    Q_INVOKABLE void load();

    Q_INVOKABLE void toggleAutoplace();
    Q_INVOKABLE void addSpacer(SpacerKind kind);

public slots:
    void setNudgeStep(double stepSp);
    void setIsSnappedToGrid(bool isSnapped);
    void setIsLedgerGuidesVisible(bool visible);

signals:
    void nudgeStepChanged();
    void isSnappedToGridChanged();
    void isLedgerGuidesVisibleChanged();
    void selectionStateChanged();

private:
    void onCurrentNotationChanged();
};
}
