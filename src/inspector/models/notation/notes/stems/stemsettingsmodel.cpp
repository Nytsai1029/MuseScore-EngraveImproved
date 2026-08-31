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
#include "stemsettingsmodel.h"

#include <algorithm>
#include <set>

#include "engraving/dom/beam.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/stem.h"

#include "translation.h"
#include "log.h"

using namespace mu::inspector;
using namespace mu::engraving;

StemSettingsModel::StemSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_STEM);
    setTitle(muse::qtrc("inspector", "Stem"));

    createProperties();
}

void StemSettingsModel::createProperties()
{
    m_thickness = buildPropertyItem(Pid::LINE_WIDTH);
    m_length = buildPropertyItem(Pid::USER_LEN);

    // Actual stem length. Computed (not stored): an unbeamed stem is adjusted through its own
    // user length, a beamed one by moving the beam it has to reach. Reset restores both.
    m_stemLength = buildPropertyItem(Pid::STEM_LENGTH, [this](const Pid, const QVariant& newValue) {
        applyStemLength(newValue);
    }, nullptr, [this](const Pid) {
        if (m_elementList.empty()) {
            return;
        }
        beginCommand(muse::TranslatableString("undoableAction", "Reset %1").arg(propertyUserName(Pid::STEM_LENGTH)));
        for (EngravingItem* item : m_elementList) {
            if (!item || !item->isStem()) {
                continue;
            }
            Stem* stem = toStem(item);
            Chord* chord = stem->chord();
            if (Beam* beam = chord ? chord->beam() : nullptr) {
                beam->undoResetProperty(Pid::USER_MODIFIED);
            }
            stem->undoResetProperty(Pid::USER_LEN);
        }
        updateNotation();
        endCommand();
    });

    m_stemDirection = buildPropertyItem(Pid::STEM_DIRECTION, [this](const Pid, const QVariant& newValue) {
        onStemDirectionChanged(static_cast<mu::engraving::DirectionV>(newValue.toInt()));
    });

    m_offset = buildPointFPropertyItem(Pid::OFFSET);
}

void StemSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(ElementType::STEM);
}

void StemSettingsModel::loadProperties()
{
    static const PropertyIdSet propertyIdSet {
        Pid::VISIBLE,
        Pid::LINE_WIDTH,
        Pid::USER_LEN,
        Pid::STEM_DIRECTION,
        Pid::OFFSET,
        Pid::STEM_LENGTH,
    };

    loadProperties(propertyIdSet);
    emit useStraightNoteFlagsChanged();
}

void StemSettingsModel::resetProperties()
{
    m_thickness->resetToDefault();
    m_length->resetToDefault();
    m_stemLength->resetToDefault();
    m_stemDirection->resetToDefault();
    m_offset->resetToDefault();
}

PropertyItem* StemSettingsModel::thickness() const
{
    return m_thickness;
}

PropertyItem* StemSettingsModel::length() const
{
    return m_length;
}

PropertyItem* StemSettingsModel::stemLength() const
{
    return m_stemLength;
}

PropertyItem* StemSettingsModel::offset() const
{
    return m_offset;
}

void StemSettingsModel::applyStemLength(const QVariant& newValue)
{
    if (m_elementList.empty()) {
        return;
    }

    const double targetSp = std::max(newValue.toDouble(), 0.0);

    beginCommand(muse::TranslatableString("undoableAction", "Edit %1").arg(propertyUserName(Pid::STEM_LENGTH)));

    std::set<Beam*> movedBeams;
    for (EngravingItem* item : m_elementList) {
        if (!item || !item->isStem()) {
            continue;
        }
        Stem* stem = toStem(item);
        Chord* chord = stem->chord();
        if (!chord) {
            continue;
        }
        // Current length as shown (measured from the notehead nearest the tip); the delta to
        // the target is the same whether measured from that note or the stem base.
        const double currentSp = stem->getProperty(Pid::STEM_LENGTH).value<Spatium>().val();

        if (Beam* beam = chord->beam()) {
            // A beamed stem has to reach its beam, so lengthening it means moving the beam.
            // Translate the beam, keeping its slant, until this stem is the requested length.
            // Only once per beam, so selecting several stems of one group can't compound.
            if (!movedBeams.insert(beam).second) {
                continue;
            }
            const double deltaSp = chord->up() ? currentSp - targetSp : targetSp - currentSp;
            const muse::PairF pos = beam->beamPos();
            beam->undoChangeProperty(Pid::USER_MODIFIED, true);
            beam->undoChangeProperty(Pid::BEAM_POS,
                                     PropertyValue::fromValue(muse::PairF(pos.first + deltaSp, pos.second + deltaSp)));
        } else {
            // Unbeamed: the stem's own user length carries the difference from the current length.
            stem->undoChangeProperty(Pid::USER_LEN, stem->userLength() + Spatium(targetSp - currentSp));
        }
    }

    updateNotation();
    endCommand();
}

PropertyItem* StemSettingsModel::stemDirection() const
{
    return m_stemDirection;
}

bool StemSettingsModel::useStraightNoteFlags() const
{
    return styleValue(Sid::useStraightNoteFlags).toBool();
}

void StemSettingsModel::setUseStraightNoteFlags(bool use)
{
    if (updateStyleValue(Sid::useStraightNoteFlags, use)) {
        emit useStraightNoteFlagsChanged();
    }
}

void StemSettingsModel::onStemDirectionChanged(DirectionV newDirection)
{
    beginCommand(muse::TranslatableString("undoableAction", "Change stem direction"));

    for (EngravingItem* element : m_elementList) {
        Stem* stem = toStem(element);
        IF_ASSERT_FAILED(stem) {
            continue;
        }

        EngravingItem* root = stem;
        if (Beam* beam = stem->chord()->beam()) {
            root = beam;
        }

        root->undoChangeProperty(Pid::STEM_DIRECTION, newDirection);
    }

    endCommand();
    updateNotation();
}

void StemSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet& changedStyleIdSet)
{
    loadProperties(changedPropertyIdSet);

    if (muse::contains(changedStyleIdSet, Sid::useStraightNoteFlags)) {
        emit useStraightNoteFlagsChanged();
    }
}

void StemSettingsModel::loadProperties(const PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::LINE_WIDTH)) {
        loadPropertyItem(m_thickness, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::USER_LEN)) {
        loadPropertyItem(m_length, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::STEM_DIRECTION)) {
        loadPropertyItem(m_stemDirection);
    }

    if (muse::contains(propertyIdSet, Pid::OFFSET)) {
        loadPropertyItem(m_offset);
    }

    // The drawn length also changes when the stem's user length is edited or when the beam it
    // reaches is repositioned, so reload it on any of those.
    if (muse::contains(propertyIdSet, Pid::STEM_LENGTH)
        || muse::contains(propertyIdSet, Pid::USER_LEN)
        || muse::contains(propertyIdSet, Pid::BEAM_POS)
        || muse::contains(propertyIdSet, Pid::USER_MODIFIED)) {
        loadPropertyItem(m_stemLength, formatDoubleFunc);
    }
}
