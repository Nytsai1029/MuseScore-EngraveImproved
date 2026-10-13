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
#include "engravemodemodel.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/spacer.h"

using namespace mu::notation;
using namespace mu::engraving;

EngraveModeModel::EngraveModeModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void EngraveModeModel::load()
{
    configuration()->engraveNudgeStepChanged().onNotify(this, [this]() {
        emit nudgeStepChanged();
    });

    configuration()->isSnappedToGridChanged().onNotify(this, [this]() {
        emit isSnappedToGridChanged();
    });

    configuration()->isLedgerGuidesVisibleChanged().onNotify(this, [this]() {
        emit isLedgerGuidesVisibleChanged();
    });

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    });

    onCurrentNotationChanged();
}

void EngraveModeModel::onCurrentNotationChanged()
{
    INotationPtr notation = globalContext()->currentNotation();
    if (notation) {
        notation->interaction()->selectionChanged().onNotify(this, [this]() {
            emit selectionStateChanged();
        });

        //! NOTE The automatic placement of what is selected changes without a change of the selection
        notation->undoStack()->stackChanged().onNotify(this, [this]() {
            emit selectionStateChanged();
        });
    }

    emit selectionStateChanged();
}

double EngraveModeModel::nudgeStep() const
{
    return configuration()->engraveNudgeStep();
}

void EngraveModeModel::setNudgeStep(double stepSp)
{
    if (qFuzzyCompare(nudgeStep(), stepSp)) {
        return;
    }

    configuration()->setEngraveNudgeStep(stepSp);
}

bool EngraveModeModel::isSnappedToGrid() const
{
    return configuration()->isSnappedToGrid(muse::Orientation::Horizontal)
           && configuration()->isSnappedToGrid(muse::Orientation::Vertical);
}

void EngraveModeModel::setIsSnappedToGrid(bool isSnapped)
{
    if (isSnappedToGrid() == isSnapped) {
        return;
    }

    configuration()->setIsSnappedToGrid(muse::Orientation::Horizontal, isSnapped);
    configuration()->setIsSnappedToGrid(muse::Orientation::Vertical, isSnapped);
}

bool EngraveModeModel::isLedgerGuidesVisible() const
{
    return configuration()->isLedgerGuidesVisible();
}

void EngraveModeModel::setIsLedgerGuidesVisible(bool visible)
{
    if (isLedgerGuidesVisible() == visible) {
        return;
    }

    configuration()->setIsLedgerGuidesVisible(visible);
}

bool EngraveModeModel::hasSelection() const
{
    INotationPtr notation = globalContext()->currentNotation();
    return notation && !notation->interaction()->selection()->isNone();
}

bool EngraveModeModel::isAutoplaceOn() const
{
    INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return false;
    }

    const std::vector<EngravingItem*>& elements = notation->interaction()->selection()->elements();
    if (elements.empty()) {
        return false;
    }

    for (const EngravingItem* element : elements) {
        if (element && !element->autoplace()) {
            return false;
        }
    }

    return true;
}

void EngraveModeModel::toggleAutoplace()
{
    dispatcher()->dispatch("toggle-autoplace");
}

void EngraveModeModel::addSpacer(SpacerKind kind)
{
    INotationPtr notation = globalContext()->currentNotation();
    if (!notation || !gpaletteScore) {
        return;
    }

    SpacerType type = SpacerType::DOWN;
    switch (kind) {
    case SpacerDown: type = SpacerType::DOWN;
        break;
    case SpacerUp: type = SpacerType::UP;
        break;
    case SpacerFixedDown: type = SpacerType::FIXED;
        break;
    }

    //! NOTE The same element as in the Layout palette
    std::shared_ptr<Spacer> spacer = Factory::makeSpacer(gpaletteScore->dummy()->measure());
    spacer->setSpacerType(type);
    spacer->setGap(Spatium(3));

    notation->interaction()->applyPaletteElement(spacer.get());
}
