/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "scorespagemodel.h"

#include <QString>

#include "actions/actiontypes.h"
#include "async/async.h"

using namespace mu::project;
using namespace muse::actions;

ScoresPageModel::ScoresPageModel(QObject* parent)
    : QObject(parent)
{
}

void ScoresPageModel::createNewScore()
{
    dispatcher()->dispatch("file-new");
}

void ScoresPageModel::openOther()
{
    dispatcher()->dispatch("file-open");
}

void ScoresPageModel::openScore(const QString& scorePath, const QString& displayNameOverride)
{
    dispatcher()->dispatch("file-open", ActionData::make_arg2<QUrl, QString>(QUrl::fromLocalFile(scorePath), displayNameOverride));
}

void ScoresPageModel::openScoreManager()
{
    interactive()->openUrl(museScoreComService()->scoreManagerUrl());
}

void ScoresPageModel::openScoreInPage(const QString& scorePath, const QString& displayNameOverride, const QString& pageUri)
{
    static const muse::Uri NOTATION_PAGE_URI("musescore://notation");

    m_pendingPageUri.clear();
    m_pendingScorePath.clear();

    //! NOTE A score is always opened on the notation page; the other page is opened after it
    if (muse::Uri(pageUri.toStdString()) != NOTATION_PAGE_URI) {
        m_pendingPageUri = pageUri;
        m_pendingScorePath = scorePath;

        if (!m_isListeningToPages) {
            m_isListeningToPages = true;

            interactive()->currentUri().ch.onReceive(this, [this](const muse::Uri& uri) {
                if (uri == NOTATION_PAGE_URI && !m_pendingPageUri.isEmpty()) {
                    openPendingPage();
                }
            });
        }
    }

    openScore(scorePath, displayNameOverride);
}

void ScoresPageModel::openPendingPage()
{
    const QString pageUri = m_pendingPageUri;
    const QString scorePath = m_pendingScorePath;

    m_pendingPageUri.clear();
    m_pendingScorePath.clear();

    //! NOTE Another score may have been opened in the meantime (or the opening of this one was cancelled)
    auto project = globalContext()->currentProject();
    if (!project || project->path().toQString() != scorePath) {
        return;
    }

    muse::async::Async::call(this, [this, pageUri]() {
        interactive()->open(pageUri.toStdString());
    });
}

QVariantMap ScoresPageModel::scoreInfo(const QString& scorePath) const
{
    QVariantMap info;

    if (scorePath.isEmpty()) {
        return info;
    }

    muse::RetVal<ProjectMeta> meta = mscMetaReader()->readMeta(scorePath);
    if (!meta.ret) {
        return info;
    }

    info["composer"] = meta.val.composer;
    info["partsCount"] = static_cast<int>(meta.val.partsCount);

    return info;
}

int ScoresPageModel::tabIndex() const
{
    return configuration()->homeScoresPageTabIndex();
}

void ScoresPageModel::setTabIndex(int index)
{
    if (index == tabIndex()) {
        return;
    }

    configuration()->setHomeScoresPageTabIndex(index);
    emit tabIndexChanged();
}

ScoresPageModel::ViewType ScoresPageModel::viewType() const
{
    return static_cast<ViewType>(configuration()->homeScoresPageViewType());
}

void ScoresPageModel::setViewType(ViewType type)
{
    if (viewType() == type) {
        return;
    }

    configuration()->setHomeScoresPageViewType(IProjectConfiguration::HomeScoresPageViewType(type));
    emit viewTypeChanged();
}
