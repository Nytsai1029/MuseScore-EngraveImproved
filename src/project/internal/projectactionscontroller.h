/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_PROJECT_PROJECTACTIONSCONTROLLER_H
#define MU_PROJECT_PROJECTACTIONSCONTROLLER_H

#include "iprojectfilescontroller.h"

#include <QObject>

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "context/iglobalcontext.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "multiinstances/iprojectprovider.h"
#include "playback/iplaybackcontroller.h"
#include "print/iprintprovider.h"
#include "inotationreadersregister.h"
#include "iopensaveprojectscenario.h"
#include "io/ifilesystem.h"
#include "extensions/iextensionsprovider.h"

#include "async/asyncable.h"

#include "iprojectconfiguration.h"
#include "iprojectcreator.h"
#include "irecentfilescontroller.h"
#include "iprojectautosaver.h"

namespace mu::project {
class ProjectActionsController : public IProjectFilesController, public muse::mi::IProjectProvider, public muse::Injectable,
    public muse::actions::Actionable, public muse::async::Asyncable
{
    muse::Inject<IProjectConfiguration> configuration = { this };
    muse::Inject<INotationReadersRegister> readers = { this };
    muse::Inject<IProjectCreator> projectCreator = { this };
    muse::Inject<IRecentFilesController> recentFilesController = { this };
    muse::Inject<IProjectAutoSaver> projectAutoSaver = { this };
    muse::Inject<IOpenSaveProjectScenario> openSaveProjectScenario = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<muse::mi::IMultiInstancesProvider> multiInstancesProvider = { this };
    muse::Inject<playback::IPlaybackController> playbackController = { this };
    muse::Inject<print::IPrintProvider> printProvider = { this };
    muse::Inject<muse::io::IFileSystem> fileSystem = { this };
    muse::Inject<muse::extensions::IExtensionsProvider> extensionsProvider = { this };

public:

    ProjectActionsController(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    void init();

    bool canReceiveAction(const muse::actions::ActionCode& code) const override;

    bool isUrlSupported(const QUrl& url) const override;
    bool isFileSupported(const muse::io::path_t& path) const override;
    muse::Ret openProject(const ProjectFile& file) override;
    bool closeOpenedProject(bool goToHome = true) override;
    bool saveProject(const muse::io::path_t& path = muse::io::path_t()) override;
    bool saveProjectLocally(
        const muse::io::path_t& path = muse::io::path_t(), SaveMode saveMode = SaveMode::Save, bool createBackup = true) override;

    // mi::IProjectProvider
    bool isProjectOpened(const muse::io::path_t& scorePath) const override;
    bool isAnyProjectOpened() const override;

private:
    void setupConnections();

    project::INotationProjectPtr currentNotationProject() const;
    notation::IMasterNotationPtr currentMasterNotation() const;
    notation::INotationPtr currentNotation() const;
    notation::INotationInteractionPtr currentInteraction() const;
    notation::INotationSelectionPtr currentNotationSelection() const;

    void newProject();

    void openProject(const muse::actions::ActionData& args);
    muse::Ret openProject(const muse::io::path_t& path, const QString& displayNameOverride = QString());

    bool shouldRetryLoadAfterError(const muse::Ret& ret, const muse::io::path_t& filepath);
    bool askIfUserAgreesToOpenProjectWithIncompatibleVersion(const std::string& errorText);
    void warnFileTooNew(const muse::io::path_t& filepath);
    bool askIfUserAgreesToOpenCorruptedProject(const muse::String& projectName, const std::string& errorText);
    void warnProjectCriticallyCorrupted(const muse::String& projectName, const std::string& errorText);
    void warnProjectCannotBeOpened(const muse::Ret& ret, const muse::io::path_t& filepath);

    muse::IInteractive::Button askAboutSavingScore(INotationProjectPtr project);

    muse::Ret canSaveProject() const;
    bool saveProject(SaveMode saveMode, bool force = false);
    void saveProjectAt(const muse::actions::ActionData& args);
    bool saveProjectAt(const SaveLocation& saveLocation, SaveMode saveMode = SaveMode::Save, bool force = false);
    bool warnBeforeSavingLegacyMsczAsMsdz(const muse::io::path_t& sourcePath, const muse::io::path_t& targetPath) const;

    bool askIfUserAgreesToSaveProjectWithErrors(const muse::Ret& ret);
    void warnScoreWithoutPartsCannotBeSaved();
    bool askIfUserAgreesToSaveCorruptedScoreLocally(const std::string& errorText, bool canRevert);

    void warnScoreCouldnotBeSaved(const muse::Ret& ret);
    void warnScoreCouldnotBeSaved(const std::string& errorText);
    int warnScoreHasBecomeCorruptedAfterSave(const muse::Ret& ret);

    void revertCorruptedScoreToLastSaved();

    RecentFile makeRecentFile(INotationProjectPtr project);

    void clearRecentScores();

    void continueLastSession();

    void openProjectProperties();

    muse::async::Promise<muse::io::path_t> selectScoreOpeningFile() const;
    muse::io::path_t selectScoreSavingFile(const muse::io::path_t& defaultFilePath, const QString& saveTitle);

    muse::RetVal<INotationProjectPtr> loadProject(const muse::io::path_t& filePath);
    muse::Ret loadWithFallback(const std::shared_ptr<INotationProject>& project, const muse::io::path_t& loadPath,
                               const std::string& format);
    muse::Ret doOpenProject(const muse::io::path_t& filePath);

    muse::Ret doFinishOpenProject();
    muse::Ret openPageIfNeed(muse::Uri pageUri);

    void exportScore();
    void printScore();

    bool hasSelection() const;

    bool m_isProjectSaving = false;
    bool m_isProjectClosing = false;
    bool m_isProjectProcessing = false;
};
}

#endif // MU_PROJECT_PROJECTACTIONSCONTROLLER_H
