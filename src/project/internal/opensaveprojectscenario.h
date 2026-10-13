/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#ifndef MU_PROJECT_OPENSAVEPROJECTSCENARIO_H
#define MU_PROJECT_OPENSAVEPROJECTSCENARIO_H

#include "iopensaveprojectscenario.h"

#include "modularity/ioc.h"
#include "iprojectconfiguration.h"
#include "global/iinteractive.h"
#include "io/ifilesystem.h"

namespace mu::project {
class OpenSaveProjectScenario : public IOpenSaveProjectScenario
{
    INJECT(IProjectConfiguration, configuration)
    INJECT(muse::IInteractive, interactive)
    INJECT(muse::io::IFileSystem, fileSystem)

public:
    OpenSaveProjectScenario() = default;

    muse::RetVal<SaveLocation> askSaveLocation(INotationProjectPtr project, SaveMode mode) const override;

    muse::RetVal<muse::io::path_t> askLocalPath(INotationProjectPtr project, SaveMode mode) const override;
};
}

#endif // MU_PROJECT_OPENSAVEPROJECTSCENARIO_H
