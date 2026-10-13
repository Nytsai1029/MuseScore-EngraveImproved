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

#include "opensaveprojectscenario.h"

#include "engraving/infrastructure/mscio.h"

using namespace muse;
using namespace mu::project;

RetVal<SaveLocation> OpenSaveProjectScenario::askSaveLocation(INotationProjectPtr project, SaveMode mode) const
{
    RetVal<muse::io::path_t> path = askLocalPath(project, mode);
    if (!path.ret) {
        return path.ret;
    }

    return RetVal<SaveLocation>::make_ok(SaveLocation(path.val));
}

RetVal<muse::io::path_t> OpenSaveProjectScenario::askLocalPath(INotationProjectPtr project, SaveMode saveMode) const
{
    std::string dialogTitle = muse::trc("project/save", "Save score");
    std::string filenameAddition;

    if (saveMode == SaveMode::SaveCopy) {
        //: used to form a filename suggestion, like "originalFile - copy"
        filenameAddition = " - " + muse::trc("project/save", "copy", "a copy of a file");
    } else if (saveMode == SaveMode::SaveSelection) {
        //: used to form a filename suggestion, like "originalFile - selection"
        filenameAddition = " - " + muse::trc("project/save", "selection");
    }

    muse::io::path_t defaultPath = configuration()->defaultSavingFilePath(project, filenameAddition);

    std::vector<std::string> filter {
        muse::trc("project", "MuseScore file") + " (*.msdz)",
        muse::trc("project", "Uncompressed MuseScore folder (experimental)")
#ifdef Q_OS_MAC
        + " (*)"
#else
        + " (*.)"
#endif
    };

    muse::io::path_t selectedPath = interactive()->selectSavingFileSync(dialogTitle, defaultPath, filter);

    if (selectedPath.empty()) {
        return make_ret(Ret::Code::Cancel);
    }

    if (io::suffix(selectedPath) == engraving::MSCZ) {
        muse::io::path_t msdzPath = io::dirpath(selectedPath)
                                    .appendingComponent(io::completeBasename(selectedPath))
                                    .appendingSuffix(engraving::MSDZ);

        // The native save dialog only asked about overwriting the .mscz name the user typed.
        // Since the save is redirected to the corresponding .msdz file, confirm here before
        // silently overwriting an existing .msdz with the same name.
        if (msdzPath != selectedPath && fileSystem()->exists(msdzPath)) {
            IInteractive::ButtonDatas buttons = {
                interactive()->buttonData(IInteractive::Button::Cancel),
                IInteractive::ButtonData(IInteractive::Button::Ok, muse::trc("project/save", "Replace"), true)
            };

            IInteractive::Result result = interactive()->warningSync(
                muse::trc("project/save", "A file with this name already exists. Do you want to replace it?"),
                muse::qtrc("project/save",
                           "MuseScore files are saved with the .msdz extension. "
                           "Replacing it will overwrite its current contents.\n\n"
                           "The score will be saved as:\n%1")
                .arg(msdzPath.toQString())
                .toStdString(),
                buttons,
                int(IInteractive::Button::Cancel));

            if (result.standardButton() != IInteractive::Button::Ok) {
                return make_ret(Ret::Code::Cancel);
            }
        }

        selectedPath = msdzPath;
    }

    if (!engraving::isMuseScoreFile(io::suffix(selectedPath))) {
        // Then it must be that the user is trying to save a mscx file.
        // At the selected path, a folder will be created,
        // and inside the folder, a mscx file will be created.
        // We should return the path to the mscx file.
        selectedPath = selectedPath.appendingComponent(io::filename(selectedPath)).appendingSuffix(engraving::MSCX);
    }

    configuration()->setLastSavedProjectsPath(io::dirpath(selectedPath));

    return RetVal<muse::io::path_t>::make_ok(selectedPath);
}
