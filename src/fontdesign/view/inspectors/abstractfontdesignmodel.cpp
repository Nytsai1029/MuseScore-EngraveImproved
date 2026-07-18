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
#include "abstractfontdesignmodel.h"

using namespace mu::fontdesign;

AbstractFontDesignModel::AbstractFontDesignModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void AbstractFontDesignModel::init()
{
    fontDesignService()->currentProjectChanged().onNotify(this, [this]() {
        attachToProject();
        reload();
    });

    attachToProject();
    reload();
}

FontDesignProjectPtr AbstractFontDesignModel::project() const
{
    return fontDesignService()->currentProject();
}

const GlyphItem* AbstractFontDesignModel::currentGlyphItem() const
{
    FontDesignProjectPtr proj = project();
    return proj ? proj->glyph(proj->currentGlyph()) : nullptr;
}

void AbstractFontDesignModel::pushCommand(std::unique_ptr<UndoCommand> cmd)
{
    if (FontDesignProjectPtr proj = project()) {
        proj->undoStack().push(std::move(cmd));
    }
}

void AbstractFontDesignModel::attachToProject()
{
    FontDesignProjectPtr proj = project();
    if (proj.get() == m_attachedProject) {
        return;
    }

    m_attachedProject = proj.get();

    if (proj) {
        proj->changed().onNotify(this, [this]() {
            reload();
        });
        proj->currentGlyphChanged().onNotify(this, [this]() {
            reload();
        });
    }
}
