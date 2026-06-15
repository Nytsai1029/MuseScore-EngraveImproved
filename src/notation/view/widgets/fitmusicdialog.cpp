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

#include "fitmusicdialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>

#include "translation.h"

using namespace mu::notation;

static constexpr int DEFAULT_SYSTEM_COUNT = 2;

FitMusicDialog::FitMusicDialog(QWidget* parent)
    : QDialog(parent), muse::Injectable(muse::iocCtxForQWidget(this))
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(muse::qtrc("notation/fitmusic", "Fit music"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QFormLayout* form = new QFormLayout();

    m_relativeModeCheck = new QCheckBox(muse::qtrc("notation/fitmusic", "Relative to current number of systems"), this);
    form->addRow(m_relativeModeCheck);

    m_systemsLabel = new QLabel(muse::qtrc("notation/fitmusic", "Number of systems:"), this);
    m_systemsSpin = new QSpinBox(this);
    m_systemsSpin->setMinimum(1);
    m_systemsSpin->setMaximum(999);
    m_systemsSpin->setValue(DEFAULT_SYSTEM_COUNT);
    form->addRow(m_systemsLabel, m_systemsSpin);

    m_smoothingCheck = new QCheckBox(muse::qtrc("notation/fitmusic", "Smooth adjacent systems"), this);
    m_smoothingCheck->setChecked(true);
    form->addRow(m_smoothingCheck);

    mainLayout->addLayout(form);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &FitMusicDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FitMusicDialog::reject);
    connect(m_relativeModeCheck, &QCheckBox::toggled, this, &FitMusicDialog::onRelativeModeToggled);
}

void FitMusicDialog::onRelativeModeToggled(bool relative)
{
    if (relative) {
        m_systemsLabel->setText(muse::qtrc("notation/fitmusic", "Change in number of systems:"));
        m_systemsSpin->setMinimum(-999);
        m_systemsSpin->setValue(0);
    } else {
        m_systemsLabel->setText(muse::qtrc("notation/fitmusic", "Number of systems:"));
        m_systemsSpin->setMinimum(1);
        if (m_systemsSpin->value() < 1) {
            m_systemsSpin->setValue(DEFAULT_SYSTEM_COUNT);
        }
    }
}

void FitMusicDialog::accept()
{
    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    INotationInteractionPtr interaction = notation->interaction();

    FitMusicReflowOptions options;
    options.relativeMode = m_relativeModeCheck->isChecked();
    options.smoothing = m_smoothingCheck->isChecked();
    if (options.relativeMode) {
        options.relativeDelta = m_systemsSpin->value();
    } else {
        options.targetSystemCount = m_systemsSpin->value();
    }

    bool ok = interaction->fitMusicReflow(options);
    if (!ok) {
        QMessageBox::warning(this, muse::qtrc("notation/fitmusic", "Fit music"),
                             muse::qtrc("notation/fitmusic",
                                        "Cannot reflow the selection. Select at least two measures and choose a number of "
                                        "systems between 1 and the number of measures in the selection."));
        return;
    }

    QDialog::accept();
}
