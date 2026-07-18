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
#include "noteheadsettingsmodel.h"

#include <algorithm>
#include <limits>

#include "engraving/types/types.h"
#include "engraving/dom/note.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/segment.h"

#include "types/commontypes.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

NoteheadSettingsModel::NoteheadSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(muse::qtrc("inspector", "Head"));
    setModelType(InspectorModelType::TYPE_NOTEHEAD);

    createProperties();
}

void NoteheadSettingsModel::createProperties()
{
    m_isHeadHidden = buildPropertyItem(mu::engraving::Pid::VISIBLE, [this](const mu::engraving::Pid pid, const QVariant& isHeadHidden) {
        onPropertyValueChanged(pid, !isHeadHidden.toBool());
    });

    m_isHeadSmall = buildPropertyItem(mu::engraving::Pid::SMALL);
    m_hasHeadParentheses = buildPropertyItem(mu::engraving::Pid::HAS_PARENTHESES);
    m_headDirection = buildPropertyItem(mu::engraving::Pid::MIRROR_HEAD);
    m_headGroup = buildPropertyItem(mu::engraving::Pid::HEAD_GROUP);
    m_headType = buildPropertyItem(mu::engraving::Pid::HEAD_TYPE);
    m_headSystem = buildPropertyItem(mu::engraving::Pid::HEAD_SCHEME);
    m_dotPosition = buildPropertyItem(mu::engraving::Pid::DOT_POSITION);
    m_offset = buildPointFPropertyItem(mu::engraving::Pid::OFFSET);
    m_ledgerLineLengthOffsetLeft = buildPropertyItem(mu::engraving::Pid::LEDGER_LINE_LENGTH_OFFSET_LEFT);
    m_ledgerLineLengthOffsetRight = buildPropertyItem(mu::engraving::Pid::LEDGER_LINE_LENGTH_OFFSET_RIGHT);

    // Distance to the previous note. Computed (not stored): edits are applied through the
    // segment's leading space, and reset restores the natural spacing.
    m_prevNoteDistance = buildPropertyItem(mu::engraving::Pid::PREV_NOTE_DISTANCE,
                                           [this](const mu::engraving::Pid, const QVariant& newValue) {
        applyPrevNoteDistance(newValue);
    }, nullptr, [this](const mu::engraving::Pid) {
        if (m_elementList.empty()) {
            return;
        }
        beginCommand(muse::TranslatableString("undoableAction", "Reset %1").arg(propertyUserName(Pid::PREV_NOTE_DISTANCE)));
        for (EngravingItem* item : m_elementList) {
            if (!item || !item->isNote()) {
                continue;
            }
            Chord* chord = toNote(item)->chord();
            Segment* seg = chord ? chord->segment() : nullptr;
            if (seg) {
                seg->undoResetProperty(Pid::LEADING_SPACE);
            }
        }
        updateNotation();
        endCommand();
    });
}

void NoteheadSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::NOTEHEAD);
}

void NoteheadSettingsModel::loadProperties()
{
    static PropertyIdSet propertyIdSet {
        Pid::VISIBLE,
        Pid::SMALL,
        Pid::HAS_PARENTHESES,
        Pid::MIRROR_HEAD,
        Pid::HEAD_GROUP,
        Pid::HEAD_TYPE,
        Pid::HEAD_SCHEME,
        Pid::DOT_POSITION,
        Pid::OFFSET,
        Pid::LEDGER_LINE_LENGTH_OFFSET_LEFT,
        Pid::LEDGER_LINE_LENGTH_OFFSET_RIGHT,
        Pid::PREV_NOTE_DISTANCE,
    };

    loadProperties(propertyIdSet);
    updateIsTrillCueNote();
    updateIsLedgerLineSelection();
}

void NoteheadSettingsModel::resetProperties()
{
    m_isHeadHidden->resetToDefault();
    m_isHeadSmall->resetToDefault();
    m_hasHeadParentheses->resetToDefault();
    m_headDirection->resetToDefault();
    m_headGroup->resetToDefault();
    m_headType->resetToDefault();
    m_headSystem->resetToDefault();
    m_dotPosition->resetToDefault();
    m_offset->resetToDefault();
    m_ledgerLineLengthOffsetLeft->resetToDefault();
    m_ledgerLineLengthOffsetRight->resetToDefault();
    m_prevNoteDistance->resetToDefault();
}

void NoteheadSettingsModel::onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet, const mu::engraving::StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
}

void NoteheadSettingsModel::loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::VISIBLE)) {
        loadPropertyItem(m_isHeadHidden, [](const QVariant& isVisible) -> QVariant {
            return !isVisible.toBool();
        });
    }

    if (muse::contains(propertyIdSet, Pid::SMALL)) {
        loadPropertyItem(m_isHeadSmall);
    }

    if (muse::contains(propertyIdSet, Pid::HAS_PARENTHESES)) {
        loadPropertyItem(m_hasHeadParentheses);
    }

    if (muse::contains(propertyIdSet, Pid::MIRROR_HEAD)) {
        loadPropertyItem(m_headDirection);
    }

    if (muse::contains(propertyIdSet, Pid::HEAD_GROUP)) {
        loadPropertyItem(m_headGroup);
    }

    if (muse::contains(propertyIdSet, Pid::HEAD_TYPE)) {
        loadPropertyItem(m_headType);
    }

    if (muse::contains(propertyIdSet, Pid::HEAD_SCHEME)) {
        loadPropertyItem(m_headSystem);
    }

    if (muse::contains(propertyIdSet, Pid::DOT_POSITION)) {
        loadPropertyItem(m_dotPosition);
    }

    if (muse::contains(propertyIdSet, Pid::OFFSET)) {
        loadPropertyItem(m_offset);
    }

    if (muse::contains(propertyIdSet, Pid::LEDGER_LINE_LENGTH_OFFSET_LEFT)) {
        loadPropertyItem(m_ledgerLineLengthOffsetLeft);
    }

    if (muse::contains(propertyIdSet, Pid::LEDGER_LINE_LENGTH_OFFSET_RIGHT)) {
        loadPropertyItem(m_ledgerLineLengthOffsetRight);
    }

    // The distance also changes when the note is moved (leading space / offset) or when
    // auto-place is toggled, so reload it on any of those.
    if (muse::contains(propertyIdSet, Pid::PREV_NOTE_DISTANCE)
        || muse::contains(propertyIdSet, Pid::LEADING_SPACE)
        || muse::contains(propertyIdSet, Pid::OFFSET)
        || muse::contains(propertyIdSet, Pid::AUTOPLACE)) {
        loadPrevNoteDistance();
    }
}

void NoteheadSettingsModel::loadPrevNoteDistance()
{
    loadPropertyItem(m_prevNoteDistance, formatDoubleFunc);

    // loadPropertyItem enables the field when a previous note exists; additionally, only
    // allow editing when auto-place is off for every selected note.
    if (m_prevNoteDistance->isEnabled()) {
        bool autoplaceOff = !m_elementList.empty();
        for (EngravingItem* item : m_elementList) {
            if (!item || item->getProperty(Pid::AUTOPLACE).toBool()) {
                autoplaceOff = false;
                break;
            }
        }
        m_prevNoteDistance->setIsEnabled(autoplaceOff);
    }

    // Lower limit shown next to the field: the closest the note can be pulled to the
    // previous one (the collision minimum). Use the most restrictive across the selection.
    qreal minDistance = -std::numeric_limits<qreal>::max();
    for (EngravingItem* item : m_elementList) {
        if (item && item->isNote()) {
            minDistance = std::max(minDistance, toNote(item)->minPrevNoteDistance().val());
        }
    }
    if (minDistance == -std::numeric_limits<qreal>::max()) {
        minDistance = 0.0;
    }
    if (!qFuzzyCompare(m_prevNoteDistanceMin, minDistance)) {
        m_prevNoteDistanceMin = minDistance;
        emit prevNoteDistanceMinChanged();
    }
}

void NoteheadSettingsModel::applyPrevNoteDistance(const QVariant& newValue)
{
    if (m_elementList.empty()) {
        return;
    }

    const double requestedSp = newValue.toDouble();

    beginCommand(muse::TranslatableString("undoableAction", "Edit %1").arg(propertyUserName(Pid::PREV_NOTE_DISTANCE)));
    for (EngravingItem* item : m_elementList) {
        if (!item || !item->isNote()) {
            continue;
        }
        Note* note = toNote(item);
        Chord* chord = note->chord();
        Segment* seg = chord ? chord->segment() : nullptr;
        if (!seg || !note->prevChordOnStaff()) {
            continue;
        }
        // Never closer than the note's own collision minimum.
        const double targetSp = std::max(requestedSp, note->minPrevNoteDistance().val());
        const double curSp = note->prevNoteDistance().val();
        const Spatium newLeadingSpace = seg->extraLeadingSpace() + Spatium(targetSp - curSp);
        seg->undoChangeProperty(Pid::LEADING_SPACE, newLeadingSpace);
    }
    updateNotation();
    endCommand();
}

PropertyItem* NoteheadSettingsModel::isHeadHidden() const
{
    return m_isHeadHidden;
}

PropertyItem* NoteheadSettingsModel::isHeadSmall() const
{
    return m_isHeadSmall;
}

PropertyItem* NoteheadSettingsModel::hasHeadParentheses() const
{
    return m_hasHeadParentheses;
}

PropertyItem* NoteheadSettingsModel::headDirection() const
{
    return m_headDirection;
}

PropertyItem* NoteheadSettingsModel::headGroup() const
{
    return m_headGroup;
}

PropertyItem* NoteheadSettingsModel::headType() const
{
    return m_headType;
}

PropertyItem* NoteheadSettingsModel::headSystem() const
{
    return m_headSystem;
}

PropertyItem* NoteheadSettingsModel::dotPosition() const
{
    return m_dotPosition;
}

PropertyItem* NoteheadSettingsModel::offset() const
{
    return m_offset;
}

PropertyItem* NoteheadSettingsModel::ledgerLineLengthOffsetLeft() const
{
    return m_ledgerLineLengthOffsetLeft;
}

PropertyItem* NoteheadSettingsModel::ledgerLineLengthOffsetRight() const
{
    return m_ledgerLineLengthOffsetRight;
}

PropertyItem* NoteheadSettingsModel::prevNoteDistance() const
{
    return m_prevNoteDistance;
}

qreal NoteheadSettingsModel::prevNoteDistanceMin() const
{
    return m_prevNoteDistanceMin;
}

bool NoteheadSettingsModel::isTrillCueNote() const
{
    return m_isTrillCueNote;
}

bool NoteheadSettingsModel::isLedgerLineSelection() const
{
    return m_isLedgerLineSelection;
}

QVariantList NoteheadSettingsModel::possibleHeadSystemTypes() const
{
    QMap<mu::engraving::NoteHeadScheme, QString> types {
        { mu::engraving::NoteHeadScheme::HEAD_AUTO,                    muse::qtrc("inspector", "Auto") },
        { mu::engraving::NoteHeadScheme::HEAD_NORMAL,                  muse::qtrc("inspector", "Normal") },
        { mu::engraving::NoteHeadScheme::HEAD_PITCHNAME,               muse::qtrc("inspector", "Pitch names") },
        { mu::engraving::NoteHeadScheme::HEAD_PITCHNAME_GERMAN,        muse::qtrc("inspector", "German pitch names") },
        { mu::engraving::NoteHeadScheme::HEAD_SOLFEGE,                 muse::qtrc("inspector", "Solfège movable do") },
        { mu::engraving::NoteHeadScheme::HEAD_SOLFEGE_FIXED,           muse::qtrc("inspector", "Solfège fixed do") },
        { mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_4,            muse::qtrc("inspector", "4-shape (Walker)") },
        { mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN,      muse::qtrc("inspector", "7-shape (Aikin)") },
        { mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK,       muse::qtrc("inspector", "7-shape (Funk)") },
        { mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER,     muse::qtrc("inspector", "7-shape (Walker)") },
    };

    QVariantList result;

    for (mu::engraving::NoteHeadScheme type : types.keys()) {
        QVariantMap obj;

        obj["text"] = types[type];
        obj["value"] = static_cast<int>(type);

        result << obj;
    }

    return result;
}

void NoteheadSettingsModel::updateIsTrillCueNote()
{
    bool isTrillCueNote = true;
    for (EngravingItem* item : m_elementList) {
        if (item->isNote() && !toNote(item)->isTrillCueNote()) {
            isTrillCueNote = false;
            break;
        }
    }
    setIsTrillCueNote(isTrillCueNote);
}

void NoteheadSettingsModel::updateIsLedgerLineSelection()
{
    bool isLedgerLineSelection = !m_elementList.empty();
    if (isLedgerLineSelection) {
        for (EngravingItem* item : m_elementList) {
            if (!item || !item->isLedgerLine()) {
                isLedgerLineSelection = false;
                break;
            }
        }
    }

    if (m_isLedgerLineSelection == isLedgerLineSelection) {
        return;
    }

    m_isLedgerLineSelection = isLedgerLineSelection;
    emit isLedgerLineSelectionChanged(m_isLedgerLineSelection);
}

void NoteheadSettingsModel::setIsTrillCueNote(bool v)
{
    if (v == m_isTrillCueNote) {
        return;
    }

    m_isTrillCueNote = v;
    emit isTrillCueNoteChanged(m_isTrillCueNote);
}
