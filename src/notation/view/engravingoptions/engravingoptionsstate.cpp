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
#include "engravingoptionsstate.h"

using namespace mu::notation;

EngravingOptionsState::EngravingOptionsState(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

EngravingOptionsDialogState EngravingOptionsState::state() const
{
    return configuration()->engravingOptionsDialogState();
}

void EngravingOptionsState::writeState(const EngravingOptionsDialogState& state)
{
    configuration()->setEngravingOptionsDialogState(state);
}

QString EngravingOptionsState::lastPageCode() const
{
    return state().lastPageCode;
}

void EngravingOptionsState::setLastPageCode(const QString& code)
{
    EngravingOptionsDialogState state = this->state();
    if (state.lastPageCode == code) {
        return;
    }

    state.lastPageCode = code;
    writeState(state);
    emit lastPageCodeChanged();
}

bool EngravingOptionsState::showsHandles() const
{
    return state().showsHandles;
}

void EngravingOptionsState::setShowsHandles(bool shows)
{
    EngravingOptionsDialogState state = this->state();
    if (state.showsHandles == shows) {
        return;
    }

    state.showsHandles = shows;
    writeState(state);
    emit showsHandlesChanged();
}

int EngravingOptionsState::sidebarWidth() const
{
    return state().sidebarWidth;
}

void EngravingOptionsState::setSidebarWidth(int width)
{
    EngravingOptionsDialogState state = this->state();
    if (state.sidebarWidth == width) {
        return;
    }

    state.sidebarWidth = width;
    writeState(state);
    emit sidebarWidthChanged();
}

int EngravingOptionsState::previewWidth() const
{
    return state().previewWidth;
}

void EngravingOptionsState::setPreviewWidth(int width)
{
    EngravingOptionsDialogState state = this->state();
    if (state.previewWidth == width) {
        return;
    }

    state.previewWidth = width;
    writeState(state);
    emit previewWidthChanged();
}

int EngravingOptionsState::defaultSidebarWidth() const
{
    return EngravingOptionsDialogState().sidebarWidth;
}

int EngravingOptionsState::defaultPreviewWidth() const
{
    return EngravingOptionsDialogState().previewWidth;
}
