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
#ifndef MU_NOTATION_NOTATIONPAINTVIEW_H
#define MU_NOTATION_NOTATIONPAINTVIEW_H

#include "abstractnotationpaintview.h"

namespace mu::notation {
class NotationPaintView : public AbstractNotationPaintView
{
    Q_OBJECT

    Q_PROPERTY(bool syncViewState READ syncViewState WRITE setSyncViewState NOTIFY syncViewStateChanged)

public:
    explicit NotationPaintView(QQuickItem* parent = nullptr);

    bool syncViewState() const;
    void setSyncViewState(bool sync);

signals:
    void syncViewStateChanged();

private:
    void onLoadNotation(INotationPtr notation) override;
    void onUnloadNotation(INotationPtr notation) override;

    void initZoomAndPosition() override;

    void onMatrixChanged(const muse::draw::Transform& oldMatrix, const muse::draw::Transform& newMatrix,
                         bool overrideZoomType = true) override;

    bool isMatrixInited() const override;
    void setMatrixInited(bool inited) override;
    ZoomType zoomType() const override;
    void setZoomType(ZoomType type) override;

    bool m_isLocalMatrixUpdate = false;
    bool m_viewStateMatrixConnectionActive = false;
    bool m_syncViewState = true;
    bool m_isLocalMatrixInited = false;
    ZoomType m_localZoomType = ZoomType::Percentage;
};
}

#endif // MU_NOTATION_NOTATIONPAINTVIEW_H
