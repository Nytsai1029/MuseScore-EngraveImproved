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

#include "japanesespacingdialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

#include "inotation.h"
#include "inotationstyle.h"
#include "internal/inotationundostack.h"
#include "notationtypes.h"

#include "types/translatablestring.h"
#include "translation.h"

using namespace mu::notation;

JapaneseSpacingDialog::JapaneseSpacingDialog(QWidget* parent)
    : QDialog(parent), muse::Injectable(muse::iocCtxForQWidget(this))
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(muse::qtrc("notation/japanesespacing", "Apply Japanese engraving spacing"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* intro = new QLabel(muse::qtrc("notation/japanesespacing",
                                          "Choose which groups of settings to apply. The values are a starting point "
                                          "and can be fine-tuned afterwards in Format › Style › Bars."), this);
    intro->setWordWrap(true);
    mainLayout->addWidget(intro);

    m_ratioCheck = new QCheckBox(muse::qtrc("notation/japanesespacing", "Flatter spacing ratio (1.4)"), this);
    m_ratioCheck->setChecked(true);
    mainLayout->addWidget(m_ratioCheck);

    m_densityCheck = new QCheckBox(muse::qtrc("notation/japanesespacing", "Tighter overall density"), this);
    m_densityCheck->setChecked(true);
    mainLayout->addWidget(m_densityCheck);

    m_compactCheck = new QCheckBox(muse::qtrc("notation/japanesespacing", "Compact note and barline distances"), this);
    m_compactCheck->setChecked(true);
    mainLayout->addWidget(m_compactCheck);

    m_adaptiveCheck = new QCheckBox(muse::qtrc("notation/japanesespacing", "Enable density-adaptive spacing ratio"), this);
    m_adaptiveCheck->setChecked(true);
    mainLayout->addWidget(m_adaptiveCheck);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &JapaneseSpacingDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &JapaneseSpacingDialog::reject);
}

void JapaneseSpacingDialog::accept()
{
    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        QDialog::reject();
        return;
    }

    INotationStylePtr style = notation->style();
    INotationUndoStackPtr undoStack = notation->undoStack();
    if (!style || !undoStack) {
        QDialog::reject();
        return;
    }

    const bool applyRatio = m_ratioCheck->isChecked();
    const bool applyDensity = m_densityCheck->isChecked();
    const bool applyCompact = m_compactCheck->isChecked();
    const bool applyAdaptive = m_adaptiveCheck->isChecked();

    if (!(applyRatio || applyDensity || applyCompact || applyAdaptive)) {
        QDialog::accept();
        return;
    }

    // Curated "Japanese engraving" starting values; tune afterwards in the style dialog.
    undoStack->prepareChanges(muse::TranslatableString("undoableAction", "Apply Japanese spacing"));

    if (applyRatio) {
        style->setStyleValue(StyleId::measureSpacing, 1.4);
    }
    if (applyDensity) {
        style->setStyleValue(StyleId::spacingDensity, 1.1);
    }
    if (applyCompact) {
        style->setStyleValue(StyleId::minNoteDistance, mu::engraving::Spatium(0.30));
        style->setStyleValue(StyleId::barNoteDistance, mu::engraving::Spatium(1.10));
        style->setStyleValue(StyleId::noteBarDistance, mu::engraving::Spatium(1.30));
    }
    if (applyAdaptive) {
        style->setStyleValue(StyleId::useAdaptiveSpacingRatio, true);
    }

    undoStack->commitChanges();

    QDialog::accept();
}
