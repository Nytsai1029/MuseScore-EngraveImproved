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

    //! NOTE A view with its own zoom (see syncViewState) can start with the whole page fitted in,
    //! whatever zoom the score is edited with: the page where the score is edited (see INotationViewState::centerTick)
    Q_PROPERTY(bool startWithWholePage READ startWithWholePage WRITE setStartWithWholePage NOTIFY startWithWholePageChanged)

    //! NOTE The zoom of this view (which is not the zoom of the other views, if syncViewState is false)
    Q_PROPERTY(int zoomPercentage READ zoomPercentage NOTIFY matrixChanged)
    Q_PROPERTY(ZoomKind zoomKind READ zoomKind NOTIFY zoomKindChanged)

public:
    explicit NotationPaintView(QQuickItem* parent = nullptr);

    enum ZoomKind {
        ZoomByPercentage,
        ZoomToPageWidth,
        ZoomToWholePage,
        ZoomToTwoPages
    };
    Q_ENUM(ZoomKind)

    bool syncViewState() const;
    void setSyncViewState(bool sync);

    bool startWithWholePage() const;
    void setStartWithWholePage(bool wholePage);

    int zoomPercentage() const;
    ZoomKind zoomKind() const;

    Q_INVOKABLE void zoomToPageWidth();
    Q_INVOKABLE void zoomToWholePage();
    Q_INVOKABLE void zoomToTwoPages();

signals:
    void syncViewStateChanged();
    void startWithWholePageChanged();
    void zoomKindChanged();

private:
    void onLoadNotation(INotationPtr notation) override;
    void onUnloadNotation(INotationPtr notation) override;

    void initZoomAndPosition() override;

    void updateCenterTick();
    int findCenterTick() const;
    void showPageAt(int tick);

    void onViewSizeChanged() override;
    void onMatrixChanged(const muse::draw::Transform& oldMatrix, const muse::draw::Transform& newMatrix,
                         bool overrideZoomType = true) override;

    bool isMatrixInited() const override;
    void setMatrixInited(bool inited) override;
    ZoomType zoomType() const override;
    void setZoomType(ZoomType type) override;

    bool m_isLocalMatrixUpdate = false;
    bool m_viewStateMatrixConnectionActive = false;
    bool m_syncViewState = true;
    bool m_startWithWholePage = false;
    bool m_isLocalMatrixInited = false;
    ZoomType m_localZoomType = ZoomType::Percentage;
};
}

#endif // MU_NOTATION_NOTATIONPAINTVIEW_H
