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
#include "tremolosettingsmodel.h"

#include <QList>

#include "translation.h"

#include "engraving/dom/tremolotwochord.h"

using namespace mu::inspector;
using namespace mu::engraving;

TremoloSettingsModel::TremoloSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_TREMOLO);
    setTitle(muse::qtrc("inspector", "Tremolos"));
    setIcon(muse::ui::IconCode::Code::TREMOLO_TWO_NOTES);
    createProperties();
}

void TremoloSettingsModel::createProperties()
{
    m_style = buildPropertyItem(mu::engraving::Pid::TREMOLO_STYLE);
    m_direction = buildPropertyItem(mu::engraving::Pid::STEM_DIRECTION);
    m_strokeStartOffsetX = buildPropertyItem(mu::engraving::Pid::TREMOLO_START_X_OFFSET);
    m_strokeStartOffsetY = buildPropertyItem(mu::engraving::Pid::TREMOLO_START_Y_OFFSET);
    m_strokeEndOffsetX = buildPropertyItem(mu::engraving::Pid::TREMOLO_END_X_OFFSET);
    m_strokeEndOffsetY = buildPropertyItem(mu::engraving::Pid::TREMOLO_END_Y_OFFSET);
}

void TremoloSettingsModel::requestElements()
{
    // all two-note tremolos: the stroke offsets apply to every one of them,
    // the style setting additionally requires customStyleApplicable()
    m_elementList = m_repository->findElementsByType(ElementType::TREMOLO_TWOCHORD);

    updateIsStyleAvailable();
}

void TremoloSettingsModel::updateIsStyleAvailable()
{
    bool styleAvailable = false;
    for (EngravingItem* it : m_elementList) {
        if (item_cast<TremoloTwoChord*>(it)->customStyleApplicable()) {
            styleAvailable = true;
            break;
        }
    }

    if (styleAvailable != m_isStyleAvailable) {
        m_isStyleAvailable = styleAvailable;
        emit isStyleAvailableChanged();
    }
}

void TremoloSettingsModel::loadProperties(const PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::TREMOLO_STYLE)) {
        loadPropertyItem(m_style);
    }
    if (muse::contains(propertyIdSet, Pid::STEM_DIRECTION)) {
        loadPropertyItem(m_direction);
    }
    if (muse::contains(propertyIdSet, Pid::TREMOLO_START_X_OFFSET)) {
        loadPropertyItem(m_strokeStartOffsetX);
    }
    if (muse::contains(propertyIdSet, Pid::TREMOLO_START_Y_OFFSET)) {
        loadPropertyItem(m_strokeStartOffsetY);
    }
    if (muse::contains(propertyIdSet, Pid::TREMOLO_END_X_OFFSET)) {
        loadPropertyItem(m_strokeEndOffsetX);
    }
    if (muse::contains(propertyIdSet, Pid::TREMOLO_END_Y_OFFSET)) {
        loadPropertyItem(m_strokeEndOffsetY);
    }
}

void TremoloSettingsModel::loadProperties()
{
    loadProperties(PropertyIdSet { Pid::TREMOLO_STYLE, Pid::STEM_DIRECTION,
                                   Pid::TREMOLO_START_X_OFFSET, Pid::TREMOLO_START_Y_OFFSET,
                                   Pid::TREMOLO_END_X_OFFSET, Pid::TREMOLO_END_Y_OFFSET });
}

void TremoloSettingsModel::resetProperties()
{
    m_style->resetToDefault();
    m_direction->resetToDefault();
    m_strokeStartOffsetX->resetToDefault();
    m_strokeStartOffsetY->resetToDefault();
    m_strokeEndOffsetX->resetToDefault();
    m_strokeEndOffsetY->resetToDefault();
}

void TremoloSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
}

PropertyItem* TremoloSettingsModel::style() const
{
    return m_style;
}

PropertyItem* TremoloSettingsModel::direction() const
{
    return m_direction;
}

PropertyItem* TremoloSettingsModel::strokeStartOffsetX() const
{
    return m_strokeStartOffsetX;
}

PropertyItem* TremoloSettingsModel::strokeStartOffsetY() const
{
    return m_strokeStartOffsetY;
}

PropertyItem* TremoloSettingsModel::strokeEndOffsetX() const
{
    return m_strokeEndOffsetX;
}

PropertyItem* TremoloSettingsModel::strokeEndOffsetY() const
{
    return m_strokeEndOffsetY;
}

bool TremoloSettingsModel::isStyleAvailable() const
{
    return m_isStyleAvailable;
}
