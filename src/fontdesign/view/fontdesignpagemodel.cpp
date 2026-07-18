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
#include "fontdesignpagemodel.h"

#include <QDir>
#include <QFile>

#include "dockwindow/idockwindow.h"
#include "io/fileinfo.h"
#include "translation.h"

#include "../internal/io/fontexporter.h"
#include "../internal/io/metadatawriter.h"

using namespace mu::fontdesign;
using namespace muse;

FontDesignPageModel::FontDesignPageModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void FontDesignPageModel::init()
{
    fontDesignService()->currentProjectChanged().onNotify(this, [this]() {
        attachToProject();
        emit hasProjectChanged();
        emit stackStateChanged();
    });

    attachToProject();

    dockWindowProvider()->windowChanged().onNotify(this, [this]() {
        listenDocksOpenStatus();
        emit panelsOpenChanged();
    });

    listenDocksOpenStatus();
}

static const QString GLYPHS_PANEL_NAME("fontDesignGlyphBrowserPanel");
static const QString INSPECTOR_PANEL_NAME("fontDesignInspectorPanel");

void FontDesignPageModel::listenDocksOpenStatus()
{
    muse::dock::IDockWindow* window = dockWindowProvider()->window();
    if (!window) {
        return;
    }

    window->docksOpenStatusChanged().onReceive(this, [this](const QStringList&) {
        emit panelsOpenChanged();
    }, muse::async::Asyncable::AsyncMode::AsyncSetRepeat);
}

bool FontDesignPageModel::isDockOpen(const QString& dockName) const
{
    const muse::dock::IDockWindow* window = dockWindowProvider()->window();
    return window && window->isDockOpen(dockName);
}

void FontDesignPageModel::toggleDock(const QString& dockName)
{
    if (muse::dock::IDockWindow* window = dockWindowProvider()->window()) {
        window->toggleDock(dockName);
        emit panelsOpenChanged();
    }
}

bool FontDesignPageModel::glyphsPanelOpen() const
{
    return isDockOpen(GLYPHS_PANEL_NAME);
}

bool FontDesignPageModel::inspectorPanelOpen() const
{
    return isDockOpen(INSPECTOR_PANEL_NAME);
}

void FontDesignPageModel::toggleGlyphsPanel()
{
    toggleDock(GLYPHS_PANEL_NAME);
}

void FontDesignPageModel::toggleInspectorPanel()
{
    toggleDock(INSPECTOR_PANEL_NAME);
}

void FontDesignPageModel::attachToProject()
{
    FontDesignProjectPtr project = fontDesignService()->currentProject();
    if (project.get() == m_attachedProject) {
        return;
    }

    m_attachedProject = project.get();

    if (project) {
        project->undoStack().stackChanged().onNotify(this, [this]() {
            emit stackStateChanged();
        });
    }
}

void FontDesignPageModel::undo()
{
    if (FontDesignProjectPtr project = fontDesignService()->currentProject()) {
        project->undoStack().undo();
    }
}

void FontDesignPageModel::redo()
{
    if (FontDesignProjectPtr project = fontDesignService()->currentProject()) {
        project->undoStack().redo();
    }
}

void FontDesignPageModel::save()
{
    if (!fontDesignService()->hasCurrentProject()) {
        return;
    }

    std::vector<std::string> warnings;
    Ret ret = fontDesignService()->saveProject(warnings);
    if (!ret) {
        interactive()->error(trc("fontdesign", "Unable to save font"), ret.text());
        return;
    }

    if (!warnings.empty()) {
        std::string detail;
        for (const std::string& w : warnings) {
            if (!detail.empty()) {
                detail += "\n";
            }
            detail += "• ";
            detail += w;
        }
        interactive()->warning(trc("fontdesign", "Font saved with warnings"), detail);
    }
}

void FontDesignPageModel::installToMuseScore()
{
    FontDesignProjectPtr project = fontDesignService()->currentProject();
    if (!project) {
        return;
    }

    std::string fontName = project->metadata().fontName;
    if (fontName.empty()) {
        fontName = io::FileInfo(project->fontPath()).baseName().toStdString();
    }

    // 扫描约定：目录名 = 字体名；名称含 "Text" 会被跳过
    if (fontName.find("Text") != std::string::npos) {
        interactive()->error(trc("fontdesign", "Cannot install font"),
                             trc("fontdesign", "Font name must not contain “Text” (reserved for text companion fonts)."));
        return;
    }

    for (char c : fontName) {
        if (c == '/' || c == '\\' || c == ':' || c == '\0') {
            interactive()->error(trc("fontdesign", "Cannot install font"),
                                 trc("fontdesign", "Font name contains invalid path characters."));
            return;
        }
    }

    if (!notationConfiguration()) {
        interactive()->error(trc("fontdesign", "Cannot install font"),
                             trc("fontdesign", "Notation configuration is unavailable."));
        return;
    }

    io::path_t userFonts = notationConfiguration()->userMusicFontsPath();
    if (userFonts.empty()) {
        interactive()->error(trc("fontdesign", "Cannot install font"),
                             trc("fontdesign", "User music fonts path is not configured."));
        return;
    }

    const io::path_t fontDirPath = userFonts + "/" + fontName;
    const QString fontDir = fontDirPath.toQString();
    if (!QDir().mkpath(fontDir)) {
        interactive()->error(trc("fontdesign", "Cannot install font"),
                             trc("fontdesign", "Failed to create font directory.") + "\n" + fontDirPath.toStdString());
        return;
    }

    // 覆盖确认
    const io::path_t otfPath = fontDirPath + "/" + fontName + ".otf";
    const io::path_t metaPath = fontDirPath + "/" + fontName + ".json";
    if (QFile::exists(otfPath.toQString())) {
        IInteractive::Result result = interactive()->questionSync(
            trc("fontdesign", "Overwrite installed font?"),
            trc("fontdesign", "A font with this name is already installed. Replace it?"),
            { IInteractive::Button::Yes, IInteractive::Button::No },
            IInteractive::Button::Yes);
        if (result.standardButton() != IInteractive::Button::Yes) {
            return;
        }
    }

    FontExporter::Report report;
    Ret fontRet = FontExporter::exportFont(*project, otfPath, &report);
    if (!fontRet) {
        interactive()->error(trc("fontdesign", "Unable to install font"), fontRet.text());
        return;
    }

    Ret metaRet = MetadataWriter::write(*project, metaPath);
    if (!metaRet) {
        interactive()->error(trc("fontdesign", "Font written but metadata failed"), metaRet.text());
        return;
    }

    // 即时重扫描（弱依赖）
    if (fontsScanner()) {
        fontsScanner()->rescanFonts();
        interactive()->info(trc("fontdesign", "Font installed"),
                            trc("fontdesign", "The font is available under Format → Style → Score → Musical symbol font.")
                            + "\n" + fontDirPath.toStdString());
    } else {
        interactive()->warning(trc("fontdesign", "Font installed"),
                               trc("fontdesign", "Files were written, but the font scanner is unavailable. Restart MuseScore to use the font.")
                               + "\n" + fontDirPath.toStdString());
    }

    if (!report.warnings.empty()) {
        std::string detail = report.message;
        for (const std::string& w : report.warnings) {
            detail += "\n• ";
            detail += w;
        }
        interactive()->warning(trc("fontdesign", "Installed with validation warnings"), detail);
    }
}

void FontDesignPageModel::exportFontAs()
{
    FontDesignProjectPtr project = fontDesignService()->currentProject();
    if (!project) {
        return;
    }

    std::vector<std::string> filter = { trc("fontdesign", "OpenType font") + " (*.otf)" };
    io::path_t defaultDir = io::FileInfo(project->fontPath()).dirPath();
    std::string defaultName = project->metadata().fontName.empty()
                              ? io::FileInfo(project->fontPath()).baseName().toStdString()
                              : project->metadata().fontName;

    io::path_t path = interactive()->selectSavingFileSync(
        trc("fontdesign", "Export font"),
        defaultDir + "/" + defaultName + ".otf",
        filter);
    if (path.empty()) {
        return;
    }

    FontExporter::Report report;
    Ret ret = FontExporter::exportFont(*project, path, &report);
    if (!ret) {
        interactive()->error(trc("fontdesign", "Unable to export font"), ret.text());
        return;
    }

    // 同步写出 metadata 到同目录（与字体同名）
    io::FileInfo fi(path);
    io::path_t metaPath = fi.dirPath() + "/" + fi.baseName() + ".json";
    Ret metaRet = MetadataWriter::write(*project, metaPath);
    if (!metaRet) {
        interactive()->error(trc("fontdesign", "Font exported but metadata failed"), metaRet.text());
        return;
    }

    if (!report.warnings.empty()) {
        std::string detail = report.message;
        for (const std::string& w : report.warnings) {
            detail += "\n• ";
            detail += w;
        }
        interactive()->warning(trc("fontdesign", "Font exported with warnings"), detail);
    }
}

void FontDesignPageModel::goToProjectsSection()
{
    interactive()->open("musescore://home?section=fontdesign");
}

bool FontDesignPageModel::confirmDiscardOrSave()
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
        save();
        return !fontDesignService()->currentProject()->isDirty();
    }

    return true;
}

void FontDesignPageModel::openFont()
{
    std::vector<std::string> filter = { trc("fontdesign", "SMuFL fonts") + " (*.otf *.ttf)" };
    io::path_t defaultDir = io::FileInfo(configuration()->lastOpenedFontPath()).dirPath();

    io::path_t path = interactive()->selectOpeningFileSync(trc("fontdesign", "Open font"), defaultDir, filter);
    if (path.empty()) {
        return;
    }

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
    }
}

void FontDesignPageModel::newFont()
{
    if (!confirmDiscardOrSave()) {
        return;
    }

    interactive()->open("musescore://fontdesign/newfont");
}

void FontDesignPageModel::openAddGlyphDialog()
{
    interactive()->open("musescore://fontdesign/addglyph");
}

void FontDesignPageModel::openLintDialog()
{
    interactive()->open("musescore://fontdesign/lint");
}

bool FontDesignPageModel::hasProject() const
{
    return fontDesignService()->hasCurrentProject();
}

QString FontDesignPageModel::projectTitle() const
{
    FontDesignProjectPtr project = fontDesignService()->currentProject();
    return project ? QString::fromStdString(project->title()) : QString();
}

bool FontDesignPageModel::canUndo() const
{
    FontDesignProjectPtr project = fontDesignService()->currentProject();
    return project && project->undoStack().canUndo();
}

bool FontDesignPageModel::canRedo() const
{
    FontDesignProjectPtr project = fontDesignService()->currentProject();
    return project && project->undoStack().canRedo();
}

bool FontDesignPageModel::isDirty() const
{
    FontDesignProjectPtr project = fontDesignService()->currentProject();
    return project && project->isDirty();
}
