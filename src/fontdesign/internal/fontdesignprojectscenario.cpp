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
#include "fontdesignprojectscenario.h"

#include "translation.h"

using namespace mu::fontdesign;
using namespace muse;

static std::string pathList(const std::vector<io::path_t>& paths)
{
    std::string list;
    for (const io::path_t& path : paths) {
        list += "\n";
        list += path.toStdString();
    }
    return list;
}

bool FontDesignProjectScenario::saveCurrentProject()
{
    if (!fontDesignService()->hasCurrentProject()) {
        return false;
    }

    SaveTargets targets;
    Ret ret = fontDesignService()->saveTargets(targets);
    if (!ret) {
        interactive()->error(trc("fontdesign", "Unable to save font"), ret.text());
        return false;
    }

    //! 文件名跟随字体名：新建到已有同名字体的文件夹、或改名撞上别的字体时，
    //! 目标位置上是别人的文件——覆盖前必须问
    if (!targets.foreignFiles.empty()) {
        IInteractive::Result result = interactive()->questionSync(
            trc("fontdesign", "Replace existing files?"),
            trc("fontdesign", "Saving this font will replace files that do not belong to it:")
            + pathList(targets.foreignFiles),
            { IInteractive::Button::Yes, IInteractive::Button::No },
            IInteractive::Button::No);
        if (result.standardButton() != IInteractive::Button::Yes) {
            return false;
        }
    }

    SaveReport report;
    ret = fontDesignService()->saveProject(report);
    if (!ret) {
        interactive()->error(trc("fontdesign", "Unable to save font"), ret.text());
        return false;
    }

    if (!report.backups.empty()) {
        interactive()->info(trc("fontdesign", "Original font kept"),
                            trc("fontdesign", "This font was not created in Font design. "
                                              "Saving rebuilds it from its outlines, advance widths and metadata only, "
                                              "so the original files were kept as:")
                            + pathList(report.backups));
    }

    if (!report.warnings.empty()) {
        std::string detail;
        for (const std::string& w : report.warnings) {
            if (!detail.empty()) {
                detail += "\n";
            }
            detail += "• ";
            detail += w;
        }
        interactive()->warning(trc("fontdesign", "Font saved with warnings"), detail);
    }

    return true;
}

bool FontDesignProjectScenario::confirmDiscardOrSave()
{
    FontDesignProjectPtr project = fontDesignService()->currentProject();
    if (!project || !project->isDirty()) {
        return true;
    }

    IInteractive::Result result = interactive()->questionSync(
        trc("fontdesign", "Save changes?"),
        qtrc("fontdesign", "The font “%1” has unsaved changes.")
        .arg(QString::fromStdString(project->title())).toStdString(),
        { IInteractive::Button::Save, IInteractive::Button::DontSave, IInteractive::Button::Cancel },
        IInteractive::Button::Save);

    if (result.standardButton() == IInteractive::Button::Cancel) {
        return false;
    }

    if (result.standardButton() == IInteractive::Button::Save) {
        return saveCurrentProject();
    }

    return true;
}
