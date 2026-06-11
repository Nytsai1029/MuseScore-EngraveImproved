/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#include "symbolsettingsmodel.h"

#include "engraving/dom/symbol.h"
#include "engraving/types/symnames.h"

#include "translation.h"

using namespace mu::inspector;

namespace {
QList<mu::engraving::EngravingItem*> keyboardHandBracketSymbols(const QList<mu::engraving::EngravingItem*>& elements)
{
    QList<mu::engraving::EngravingItem*> result;

    for (mu::engraving::EngravingItem* element : elements) {
        if (element && element->isSymbol() && mu::engraving::toSymbol(element)->isKeyboardHandBracketSymbol()) {
            result << element;
        }
    }

    return result;
}

QList<mu::engraving::EngravingItem*> timeSigBracketSymbols(const QList<mu::engraving::EngravingItem*>& elements)
{
    QList<mu::engraving::EngravingItem*> result;

    for (mu::engraving::EngravingItem* element : elements) {
        if (element && element->isSymbol() && mu::engraving::toSymbol(element)->isTimeSigBracketSymbol()) {
            result << element;
        }
    }

    return result;
}
}

SymbolSettingsModel::SymbolSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_SYMBOL);
    setTitle(muse::qtrc("inspector", "Symbol"));
    setIcon(muse::ui::IconCode::Code::TRIANGLE_SYMBOL);
    createProperties();
}

void SymbolSettingsModel::createProperties()
{
    m_sym = buildPropertyItem(mu::engraving::Pid::SYMBOL);
    m_scoreFont = buildPropertyItem(mu::engraving::Pid::SCORE_FONT);
    m_symbolSize = buildPropertyItem(mu::engraving::Pid::SYMBOLS_SIZE,
                                     [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue.toDouble() / 100);
    },
                                     [this](const mu::engraving::Sid sid, const QVariant& newValue) {
        updateStyleValue(sid, newValue.toDouble() / 100);
        emit requestReloadPropertyItems();
    });
    m_symAngle = buildPropertyItem(mu::engraving::Pid::SYMBOL_ANGLE);
    m_keyboardHandShortSide = buildPropertyItem(mu::engraving::Pid::SYMBOL_SHORT_SIDE_LENGTH);
    m_keyboardHandLongSide = buildPropertyItem(mu::engraving::Pid::SYMBOL_LONG_SIDE_LENGTH);
    m_keyboardHandLineWidth = buildPropertyItem(mu::engraving::Pid::LINE_WIDTH);
    m_timeSigBracketHeight = buildPropertyItem(mu::engraving::Pid::SYMBOL_LONG_SIDE_LENGTH);
    m_timeSigBracketTopHook = buildPropertyItem(mu::engraving::Pid::SYMBOL_TOP_HOOK_LENGTH);
    m_timeSigBracketBottomHook = buildPropertyItem(mu::engraving::Pid::SYMBOL_BOTTOM_HOOK_LENGTH);
    m_timeSigBracketLineWidth = buildPropertyItem(mu::engraving::Pid::LINE_WIDTH);
    m_keyboardHandShortSide->setIsVisible(false);
    m_keyboardHandLongSide->setIsVisible(false);
    m_keyboardHandLineWidth->setIsVisible(false);
    m_timeSigBracketHeight->setIsVisible(false);
    m_timeSigBracketTopHook->setIsVisible(false);
    m_timeSigBracketBottomHook->setIsVisible(false);
    m_timeSigBracketLineWidth->setIsVisible(false);
}

void SymbolSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::SYMBOL);
}

void SymbolSettingsModel::loadProperties()
{
    loadPropertyItem(m_sym);
    loadPropertyItem(m_scoreFont);
    loadPropertyItem(m_symbolSize, [](const QVariant& elementPropertyValue) -> QVariant {
        return muse::DataFormatter::roundDouble(elementPropertyValue.toDouble()) * 100;
    });
    loadPropertyItem(m_symAngle);

    const QList<mu::engraving::EngravingItem*> keyboardHandElements = keyboardHandBracketSymbols(m_elementList);
    loadPropertyItem(m_keyboardHandShortSide, keyboardHandElements);
    loadPropertyItem(m_keyboardHandLongSide, keyboardHandElements);
    loadPropertyItem(m_keyboardHandLineWidth, keyboardHandElements);

    const bool showKeyboardHandProperties = !keyboardHandElements.empty();
    m_keyboardHandShortSide->setIsVisible(showKeyboardHandProperties);
    m_keyboardHandLongSide->setIsVisible(showKeyboardHandProperties);
    m_keyboardHandLineWidth->setIsVisible(showKeyboardHandProperties);

    const QList<mu::engraving::EngravingItem*> timeSigBracketElements = timeSigBracketSymbols(m_elementList);
    loadPropertyItem(m_timeSigBracketHeight, timeSigBracketElements);
    loadPropertyItem(m_timeSigBracketTopHook, timeSigBracketElements);
    loadPropertyItem(m_timeSigBracketBottomHook, timeSigBracketElements);
    loadPropertyItem(m_timeSigBracketLineWidth, timeSigBracketElements);

    const bool showTimeSigBracketProperties = !timeSigBracketElements.empty();
    m_timeSigBracketHeight->setIsVisible(showTimeSigBracketProperties);
    m_timeSigBracketTopHook->setIsVisible(showTimeSigBracketProperties);
    m_timeSigBracketBottomHook->setIsVisible(showTimeSigBracketProperties);
    m_timeSigBracketLineWidth->setIsVisible(showTimeSigBracketProperties);
}

void SymbolSettingsModel::resetProperties()
{
    m_sym->resetToDefault();
    m_scoreFont->resetToDefault();
    m_symbolSize->resetToDefault();
    m_symAngle->resetToDefault();
    if (m_keyboardHandShortSide->isVisible()) {
        m_keyboardHandShortSide->resetToDefault();
    }
    if (m_keyboardHandLongSide->isVisible()) {
        m_keyboardHandLongSide->resetToDefault();
    }
    if (m_keyboardHandLineWidth->isVisible()) {
        m_keyboardHandLineWidth->resetToDefault();
    }
    if (m_timeSigBracketHeight->isVisible()) {
        m_timeSigBracketHeight->resetToDefault();
    }
    if (m_timeSigBracketTopHook->isVisible()) {
        m_timeSigBracketTopHook->resetToDefault();
    }
    if (m_timeSigBracketBottomHook->isVisible()) {
        m_timeSigBracketBottomHook->resetToDefault();
    }
    if (m_timeSigBracketLineWidth->isVisible()) {
        m_timeSigBracketLineWidth->resetToDefault();
    }
}

PropertyItem* SymbolSettingsModel::sym() const
{
    return m_sym;
}

PropertyItem* SymbolSettingsModel::scoreFont() const
{
    return m_scoreFont;
}

PropertyItem* SymbolSettingsModel::symbolSize() const
{
    return m_symbolSize;
}

PropertyItem* SymbolSettingsModel::symAngle() const
{
    return m_symAngle;
}

PropertyItem* SymbolSettingsModel::keyboardHandShortSide() const
{
    return m_keyboardHandShortSide;
}

PropertyItem* SymbolSettingsModel::keyboardHandLongSide() const
{
    return m_keyboardHandLongSide;
}

PropertyItem* SymbolSettingsModel::keyboardHandLineWidth() const
{
    return m_keyboardHandLineWidth;
}

PropertyItem* SymbolSettingsModel::timeSigBracketHeight() const
{
    return m_timeSigBracketHeight;
}

PropertyItem* SymbolSettingsModel::timeSigBracketTopHook() const
{
    return m_timeSigBracketTopHook;
}

PropertyItem* SymbolSettingsModel::timeSigBracketBottomHook() const
{
    return m_timeSigBracketBottomHook;
}

PropertyItem* SymbolSettingsModel::timeSigBracketLineWidth() const
{
    return m_timeSigBracketLineWidth;
}

QVariantList SymbolSettingsModel::symFonts()
{
    if (m_symFonts.empty()) {
        for (const engraving::IEngravingFontPtr& f : engravingFonts()->fonts()) {
            QVariantMap style;
            style["text"] = QString::fromStdString(f->name());
            style["value"] = QString::fromStdString(f->name());
            m_symFonts << style;
        }
    }
    return m_symFonts;
}
