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

#ifndef MU_NOTATION_FITMUSICDIALOG_H
#define MU_NOTATION_FITMUSICDIALOG_H

#include <QDialog>

#include "context/iglobalcontext.h"
#include "modularity/ioc.h"

class QCheckBox;
class QLabel;
class QSpinBox;

namespace mu::notation {
//---------------------------------------------------------
//   FitMusicDialog
//   Collects parameters for the "fit music" reflow command and invokes it
//   on the current notation's interaction.
//---------------------------------------------------------

class FitMusicDialog : public QDialog, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<context::IGlobalContext> context = { this };

public:
    explicit FitMusicDialog(QWidget* parent = nullptr);

private slots:
    void accept() override;
    void onRelativeModeToggled(bool relative);

private:
    QLabel* m_systemsLabel = nullptr;
    QSpinBox* m_systemsSpin = nullptr;
    QCheckBox* m_relativeModeCheck = nullptr;
    QCheckBox* m_smoothingCheck = nullptr;
};
}

#endif // MU_NOTATION_FITMUSICDIALOG_H
