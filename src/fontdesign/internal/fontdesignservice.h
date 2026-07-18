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

#include "modularity/ioc.h"

#include "../ifontdesignconfiguration.h"
#include "../ifontdesignservice.h"
#include "smufldatabase.h"

namespace mu::fontdesign {
class FontDesignService : public IFontDesignService, public muse::Injectable
{
    muse::Inject<IFontDesignConfiguration> configuration = { this };

public:
    FontDesignService() = default;

    muse::Ret openProject(const muse::io::path_t& fontPath) override;
    muse::Ret newProject(const NewFontParams& params) override;
    muse::Ret saveProject(std::vector<std::string>& warnings) override;
    void closeProject() override;

    FontDesignProjectPtr currentProject() const override;
    bool hasCurrentProject() const override;
    muse::async::Notification currentProjectChanged() const override;

    const SmuflDatabase& smuflDatabase() const override;

    void setActiveEditSurface(IFontDesignEditSurface* surface) override;
    IFontDesignEditSurface* activeEditSurface() const override;

private:
    muse::io::path_t findMetadataFor(const muse::io::path_t& fontPath) const;

    mutable SmuflDatabase m_smuflDb;
    FontDesignProjectPtr m_project;
    muse::async::Notification m_currentProjectChanged;
    IFontDesignEditSurface* m_editSurface = nullptr;
};
}
