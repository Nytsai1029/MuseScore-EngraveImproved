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
#include "projectspagemodel.h"

#include <QFile>
#include <QVariantMap>

#include "io/fileinfo.h"
#include "translation.h"

#include "../internal/io/metadatawriter.h"

using namespace mu::fontdesign;
using namespace muse;

ProjectsPageModel::ProjectsPageModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void ProjectsPageModel::openFontDesignPage()
{
    interactive()->open("musescore://fontdesign");
}

bool ProjectsPageModel::confirmDiscardOrSave()
{
    FontDesignProjectPtr project = fontDesignService()->currentProject();
    if (!project || !project->isDirty()) {
        return true;
    }

    IInteractive::Result result = interactive()->questionSync(
        trc("fontdesign", "Save changes?"),
        trc("fontdesign", "The current font has unsaved metadata changes."),
        { IInteractive::Button::Save, IInteractive::Button::DontSave, IInteractive::Button::Cancel },
        IInteractive::Button::Save);

    if (result.standardButton() == IInteractive::Button::Cancel) {
        return false;
    }

    if (result.standardButton() == IInteractive::Button::Save) {
        io::path_t path = project->metadataPath();
        if (path.empty()) {
            io::FileInfo fontInfo(project->fontPath());
            path = fontInfo.dirPath() + "/" + fontInfo.baseName() + ".json";
            project->setMetadataPath(path);
        }

        Ret ret = MetadataWriter::write(*project, path);
        if (!ret) {
            interactive()->error(trc("fontdesign", "Unable to save metadata"), ret.text());
            return false;
        }

        project->undoStack().markClean();
    }

    return true;
}

void ProjectsPageModel::openFont()
{
    std::vector<std::string> filter = { trc("fontdesign", "SMuFL fonts") + " (*.otf *.ttf)" };
    io::path_t defaultDir = io::FileInfo(configuration()->lastOpenedFontPath()).dirPath();

    io::path_t path = interactive()->selectOpeningFileSync(trc("fontdesign", "Open font"), defaultDir, filter);
    if (path.empty()) {
        return;
    }

    openFontPath(path);
}

void ProjectsPageModel::openFontFile(const QString& path)
{
    if (!path.isEmpty()) {
        openFontPath(io::path_t(path));
    }
}

void ProjectsPageModel::openFontPath(const io::path_t& path)
{
    if (fontDesignService()->hasCurrentProject()) {
        constexpr int replaceBtn = static_cast<int>(IInteractive::Button::CustomButton) + 1;
        constexpr int newWindowBtn = static_cast<int>(IInteractive::Button::CustomButton) + 2;

        IInteractive::Result res = interactive()->questionSync(
            trc("fontdesign", "Open font"),
            trc("fontdesign", "A font is already open. Where do you want to open the new one?"),
            { IInteractive::ButtonData(replaceBtn, trc("fontdesign", "This window")),
              IInteractive::ButtonData(newWindowBtn, trc("fontdesign", "New window"), true),
              interactive()->buttonData(IInteractive::Button::Cancel) });

        if (res.button() == newWindowBtn) {
            multiInstancesProvider()->openNewAppInstance({ "--fontdesign", path.toQString() });
            return;
        }
        if (res.button() != replaceBtn) {
            return;
        }
        if (!confirmDiscardOrSave()) {
            return;
        }
    }

    Ret ret = fontDesignService()->openProject(path);
    if (!ret) {
        interactive()->error(trc("fontdesign", "Unable to open font"), ret.text());
        return;
    }

    openFontDesignPage();
}

QVariantList ProjectsPageModel::recentFonts() const
{
    QVariantList result;
    for (const io::path_t& path : configuration()->recentFontPaths()) {
        const QString pathStr = path.toQString();
        if (!QFile::exists(pathStr)) {
            configuration()->removeRecentFontPath(path);
            continue;
        }
        QVariantMap item;
        item["name"] = io::FileInfo(path).baseName().toQString();
        item["path"] = pathStr;
        result << item;
    }
    return result;
}

void ProjectsPageModel::newFont()
{
    if (!confirmDiscardOrSave()) {
        return;
    }

    //! 对话框内完成创建（NewFontModel::create → service->newProject）；
    //! 以项目指针是否更换判断是否创建成功（不依赖对话框 ret 约定）
    FontDesignProjectPtr before = fontDesignService()->currentProject();
    interactive()->open("musescore://fontdesign/newfont");

    FontDesignProjectPtr after = fontDesignService()->currentProject();
    if (after && after != before) {
        openFontDesignPage();
    }
}
