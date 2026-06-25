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

#ifndef MU_NOTATION_JAPANESESPACINGDIALOG_H
#define MU_NOTATION_JAPANESESPACINGDIALOG_H

#include <QDialog>

#include "context/iglobalcontext.h"
#include "modularity/ioc.h"

class QCheckBox;

namespace mu::notation {
//---------------------------------------------------------
//   JapaneseSpacingDialog
//   Applies a curated "Japanese engraving" (浄書) spacing preset. The user
//   picks which groups of settings to apply; only the checked groups are
//   written to the current score's style, as a single undoable change.
//---------------------------------------------------------

class JapaneseSpacingDialog : public QDialog, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<context::IGlobalContext> context = { this };

public:
    explicit JapaneseSpacingDialog(QWidget* parent = nullptr);

private slots:
    void accept() override;

private:
    QCheckBox* m_ratioCheck = nullptr;
    QCheckBox* m_densityCheck = nullptr;
    QCheckBox* m_compactCheck = nullptr;
    QCheckBox* m_adaptiveCheck = nullptr;
};
}

#endif // MU_NOTATION_JAPANESESPACINGDIALOG_H
