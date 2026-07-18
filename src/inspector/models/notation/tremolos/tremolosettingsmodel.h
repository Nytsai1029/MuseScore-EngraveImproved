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
#ifndef MU_INSPECTOR_TREMOLOSETTINGSMODEL_H
#define MU_INSPECTOR_TREMOLOSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class TremoloSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * style READ style CONSTANT)
    Q_PROPERTY(PropertyItem * direction READ direction CONSTANT)
    Q_PROPERTY(PropertyItem * strokeStartOffsetX READ strokeStartOffsetX CONSTANT)
    Q_PROPERTY(PropertyItem * strokeStartOffsetY READ strokeStartOffsetY CONSTANT)
    Q_PROPERTY(PropertyItem * strokeEndOffsetX READ strokeEndOffsetX CONSTANT)
    Q_PROPERTY(PropertyItem * strokeEndOffsetY READ strokeEndOffsetY CONSTANT)
    Q_PROPERTY(bool isStyleAvailable READ isStyleAvailable NOTIFY isStyleAvailableChanged)

public:
    explicit TremoloSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    PropertyItem* style() const;
    PropertyItem* direction() const;
    PropertyItem* strokeStartOffsetX() const;
    PropertyItem* strokeStartOffsetY() const;
    PropertyItem* strokeEndOffsetX() const;
    PropertyItem* strokeEndOffsetY() const;
    bool isStyleAvailable() const;

signals:
    void isStyleAvailableChanged();

private:
    void loadProperties(const mu::engraving::PropertyIdSet& allowedPropertyIdSet);
    void updateIsStyleAvailable();

    PropertyItem* m_style = nullptr;
    PropertyItem* m_direction = nullptr;
    PropertyItem* m_strokeStartOffsetX = nullptr;
    PropertyItem* m_strokeStartOffsetY = nullptr;
    PropertyItem* m_strokeEndOffsetX = nullptr;
    PropertyItem* m_strokeEndOffsetY = nullptr;
    bool m_isStyleAvailable = false;
};
}

#endif // MU_INSPECTOR_TREMOLOSETTINGSMODEL_H
