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
#include "testcaselauncher.h"

#include <QCoreApplication>

#include "log.h"

using namespace muse::autobot;

namespace mu::app {
//! NOTE Several signals of the script can ask to quit; the first one gives the exit code
static void quitOnce(int code)
{
    static bool hasQuit = false;
    if (hasQuit) {
        return;
    }

    hasQuit = true;
    qApp->exit(code);
}

void launchTestCase(const std::shared_ptr<IAutobot>& autobot, const CmdOptions::Autobot& task)
{
    IF_ASSERT_FAILED(autobot) {
        quitOnce(1);
        return;
    }

    muse::async::Channel<StepInfo, muse::Ret> stepCh = autobot->stepStatusChanged();
    stepCh.onReceive(nullptr, [](const StepInfo& step, const muse::Ret& ret) {
        if (!ret) {
            LOGE() << "failed step: " << step.name << ", ret: " << ret.toString();
            quitOnce(ret.code());
        } else {
            LOGI() << "success step: " << step.name << ", ret: " << ret.toString();
        }
    });

    muse::async::Channel<muse::io::path_t, IAutobot::Status> statusCh = autobot->statusChanged();
    statusCh.onReceive(nullptr, [](const muse::io::path_t& path, IAutobot::Status status) {
        switch (status) {
        case IAutobot::Status::Finished:
            LOGI() << "success finished, path: " << path;
            quitOnce(0);
            break;
        case IAutobot::Status::Aborted:
        case IAutobot::Status::Error:
            LOGE() << "not finished (" << IAutobot::statusToString(status) << "), path: " << path;
            quitOnce(1);
            break;
        case IAutobot::Status::Undefined:
        case IAutobot::Status::Running:
        case IAutobot::Status::Paused:
            break;
        }
    });

    IAutobot::Options opt;
    opt.context = task.testCaseContextNameOrFile;
    opt.contextVal = task.testCaseContextValue.toStdString();
    opt.func = task.testCaseFunc.toStdString();
    opt.funcArgs = task.testCaseFuncArgs.toStdString();

    autobot->execScript(task.testCaseNameOrFile, opt);
}
}
