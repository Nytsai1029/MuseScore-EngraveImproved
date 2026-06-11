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
#ifndef MU_INSPECTOR_SYMBOLSETTINGSMODEL_H
#define MU_INSPECTOR_SYMBOLSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"
#include "engraving/iengravingfontsprovider.h"

namespace mu::inspector {
class SymbolSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * sym READ sym CONSTANT)
    Q_PROPERTY(PropertyItem * scoreFont READ scoreFont CONSTANT)
    Q_PROPERTY(PropertyItem * symbolSize READ symbolSize CONSTANT)
    Q_PROPERTY(PropertyItem * symAngle READ symAngle CONSTANT)
    Q_PROPERTY(PropertyItem * keyboardHandShortSide READ keyboardHandShortSide CONSTANT)
    Q_PROPERTY(PropertyItem * keyboardHandLongSide READ keyboardHandLongSide CONSTANT)
    Q_PROPERTY(PropertyItem * keyboardHandLineWidth READ keyboardHandLineWidth CONSTANT)
    Q_PROPERTY(PropertyItem * timeSigBracketHeight READ timeSigBracketHeight CONSTANT)
    Q_PROPERTY(PropertyItem * timeSigBracketTopHook READ timeSigBracketTopHook CONSTANT)
    Q_PROPERTY(PropertyItem * timeSigBracketBottomHook READ timeSigBracketBottomHook CONSTANT)
    Q_PROPERTY(PropertyItem * timeSigBracketLineWidth READ timeSigBracketLineWidth CONSTANT)

    Q_PROPERTY(QVariantList symFonts READ symFonts NOTIFY symFontsChanged)

    INJECT(engraving::IEngravingFontsProvider, engravingFonts)

public:
    explicit SymbolSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* sym() const;
    PropertyItem* scoreFont() const;
    PropertyItem* symbolSize() const;
    PropertyItem* symAngle() const;
    PropertyItem* keyboardHandShortSide() const;
    PropertyItem* keyboardHandLongSide() const;
    PropertyItem* keyboardHandLineWidth() const;
    PropertyItem* timeSigBracketHeight() const;
    PropertyItem* timeSigBracketTopHook() const;
    PropertyItem* timeSigBracketBottomHook() const;
    PropertyItem* timeSigBracketLineWidth() const;

    QVariantList symFonts();

signals:
    void symFontsChanged();

private:
    PropertyItem* m_sym = nullptr;
    PropertyItem* m_scoreFont = nullptr;
    PropertyItem* m_symbolSize = nullptr;
    PropertyItem* m_symAngle = nullptr;
    PropertyItem* m_keyboardHandShortSide = nullptr;
    PropertyItem* m_keyboardHandLongSide = nullptr;
    PropertyItem* m_keyboardHandLineWidth = nullptr;
    PropertyItem* m_timeSigBracketHeight = nullptr;
    PropertyItem* m_timeSigBracketTopHook = nullptr;
    PropertyItem* m_timeSigBracketBottomHook = nullptr;
    PropertyItem* m_timeSigBracketLineWidth = nullptr;

    QVariantList m_symFonts;
};
}

#endif // MU_INSPECTOR_SYMBOLSETTINGSMODEL_H
