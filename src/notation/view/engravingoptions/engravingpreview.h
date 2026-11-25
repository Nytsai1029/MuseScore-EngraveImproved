/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <QColor>
#include <QImage>
#include <QMarginsF>
#include <QQuickPaintedItem>
#include <QVariant>
#include <QVariantMap>

#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "context/iglobalcontext.h"
#include "notation/inotationstyle.h"

#include "engravingpreviewrenderer.h"
#include "previewhandle.h"
#include "previewoverlay.h"

namespace mu::notation {
//! NOTE A picture of music (written as text, see PreviewMusic), laid out by the engraving engine with the style of
//! the current score. It follows the style: when an option of the Engraving Options dialog is changed, the picture
//! shows the result. With `overrides` the picture shows what given values of some options would do (the choices
//! of an option).
//! With `activeSid` the picture shows what that option changes: a wash behind the items it moves or reshapes and
//! a handle which is dragged to change the value
class EngravingPreview : public QQuickPaintedItem, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString music READ music WRITE setMusic NOTIFY musicChanged)

    //! NOTE Style values by their names, as the dialog has them, which the picture sets whatever the score has
    Q_PROPERTY(QVariantMap overrides READ overrides WRITE setOverrides NOTIFY overridesChanged)

    //! NOTE The width of the systems in staff spaces, for music which shows the end of a system.
    //! Not positive: each system is as wide as its music needs
    Q_PROPERTY(double systemWidth READ systemWidth WRITE setSystemWidth NOTIFY systemWidthChanged)
    //! NOTE Whether the picture is the pages of the music, whole and side by side, laid out as the style has the
    //! pages (size, margins, header and footer, vertical spacing): for the options of the page itself
    Q_PROPERTY(bool showsPage READ showsPage WRITE setShowsPage NOTIFY showsPageChanged)

    //! NOTE 1.0 is the staff space of BASE_STAFF_SPACE pixels
    Q_PROPERTY(double zoom READ zoom WRITE setZoom NOTIFY zoomChanged)

    //! NOTE The size of the picture at the zoom 1: staff spaces × BASE_STAFF_SPACE. Independent of the
    //! zoom, so that a fit can be computed before the picture is made
    Q_PROPERTY(QSizeF naturalSize READ naturalSize NOTIFY naturalSizeChanged)
    //! NOTE The size of the first page at the zoom 1, if the picture shows the pages (a pane fits one page);
    //! else empty
    Q_PROPERTY(QSizeF naturalPageSize READ naturalPageSize NOTIFY naturalSizeChanged)

    //! NOTE The size of the picture at this zoom. The picture is always made at the size it is shown
    Q_PROPERTY(QSizeF contentSize READ contentSize NOTIFY contentSizeChanged)

    Q_PROPERTY(bool isValid READ isValid NOTIFY isValidChanged)

    //! NOTE What is wrong with the music, if it is not valid: the page author must see it at once
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

    //! NOTE The name of the option which is being edited; empty: a plain picture
    Q_PROPERTY(QString activeSid READ activeSid WRITE setActiveSid NOTIFY activeSidChanged)

    //! NOTE The value the option is shown with, in the form the interface has it (of a choice: the next
    //! choice). Not valid: an automatic probe is used
    Q_PROPERTY(QVariant probeValue READ probeValue WRITE setProbeValue NOTIFY probeValueChanged)

    //! NOTE Whether the option may be changed by dragging its handle (only a number may; the washes of a
    //! choice are shown regardless)
    Q_PROPERTY(bool isDraggable READ isDraggable WRITE setIsDraggable NOTIFY isDraggableChanged)

    Q_PROPERTY(bool showsHandles READ showsHandles WRITE setShowsHandles NOTIFY showsHandlesChanged)
    Q_PROPERTY(bool showsControlPoints READ showsControlPoints WRITE setShowsControlPoints
               NOTIFY showsControlPointsChanged)
    //! NOTE While true, shows the probed ("after") picture aligned to the same staff position for comparison
    Q_PROPERTY(bool showingProbed READ showingProbed WRITE setShowingProbed NOTIFY showingProbedChanged)
    Q_PROPERTY(QColor accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)

    //! NOTE Whether the music shows the option `activeSid`, and whether there is a handle to change it with
    Q_PROPERTY(bool hasEffect READ hasEffect NOTIFY effectChanged)
    Q_PROPERTY(bool hasHandle READ hasHandle NOTIFY effectChanged)

    //! NOTE Where the change is, in the item's pixels: the pane scrolls its picture there when it is wider
    //! than what is shown
    Q_PROPERTY(QRectF changeBox READ changeBox NOTIFY effectChanged)

    //! NOTE While the handle is dragged: the value under the pointer, as the interface has it. The picture is
    //! laid out with it at once, but the style is not written: that is for the one who takes `valueDropped`
    Q_PROPERTY(bool isDragging READ isDragging NOTIFY isDraggingChanged)
    Q_PROPERTY(QVariant dragValue READ dragValue NOTIFY dragValueChanged)
    Q_PROPERTY(double dragMinimum READ dragMinimum WRITE setDragMinimum NOTIFY dragRangeChanged)
    Q_PROPERTY(double dragMaximum READ dragMaximum WRITE setDragMaximum NOTIFY dragRangeChanged)
    Q_PROPERTY(double dragStep READ dragStep WRITE setDragStep NOTIFY dragRangeChanged)

    muse::Inject<context::IGlobalContext> globalContext = { this };

public:
    explicit EngravingPreview(QQuickItem* parent = nullptr);
    ~EngravingPreview() override;

    QString music() const;
    void setMusic(const QString& music);

    QVariantMap overrides() const;
    void setOverrides(const QVariantMap& overrides);

    double systemWidth() const;
    void setSystemWidth(double widthSp);

    bool showsPage() const;
    void setShowsPage(bool showsPage);

    double zoom() const;
    void setZoom(double zoom);

    QSizeF naturalSize() const;
    QSizeF naturalPageSize() const;
    QSizeF contentSize() const;
    bool isValid() const;
    QString error() const;

    QString activeSid() const;
    void setActiveSid(const QString& sid);

    QVariant probeValue() const;
    void setProbeValue(const QVariant& value);

    bool isDraggable() const;
    void setIsDraggable(bool draggable);

    bool showsHandles() const;
    void setShowsHandles(bool shows);

    bool showsControlPoints() const;
    void setShowsControlPoints(bool shows);

    bool showingProbed() const;
    void setShowingProbed(bool showing);

    QColor accentColor() const;
    void setAccentColor(const QColor& color);

    bool hasEffect() const;
    bool hasHandle() const;
    QRectF changeBox() const;

    bool isDragging() const;
    QVariant dragValue() const;

    double dragMinimum() const;
    void setDragMinimum(double minimum);
    double dragMaximum() const;
    void setDragMaximum(double maximum);
    double dragStep() const;
    void setDragStep(double step);

    void paint(QPainter* painter) override;

signals:
    void musicChanged();
    void overridesChanged();
    void systemWidthChanged();
    void showsPageChanged();
    void zoomChanged();
    void naturalSizeChanged();
    void contentSizeChanged();
    void isValidChanged();
    void errorChanged();
    void activeSidChanged();
    void probeValueChanged();
    void isDraggableChanged();
    void showsHandlesChanged();
    void showsControlPointsChanged();
    void showingProbedChanged();
    void accentColorChanged();
    void effectChanged();
    void isDraggingChanged();
    void dragValueChanged();
    void dragRangeChanged();

    //! The handle was let go inside of the item, with this value
    void valueDropped(const QVariant& value);

protected:
    void componentComplete() override;
    void itemChange(ItemChange change, const ItemChangeData& value) override;

    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    friend class EngravingPreviewQueue;

    void listenStyle();

    //! NOTE Something the picture is made of changed: it is laid out again (and painted again)
    void invalidateLayout();
    //! NOTE Only how the picture is shown changed: it is painted again of the same layout
    void invalidatePaint();
    //! NOTE Has the queued turn of the event loop do what is due
    void enqueue();

    void render();

    void layoutMusic(const engraving::MStyle& style);
    std::vector<EngravingPreviewRenderer::Override> currentOverrides() const;
    void updateEffect(const engraving::MStyle& style);
    void paintPicture();

    double pixelsPerStaffSpace() const;
    QRectF pictureRect() const;
    QPointF toItem(const muse::PointF& inStaffSpaces) const;

    bool isHandleShown() const;
    void placeHandle();
    PreviewOverlay overlay();

    void finishDrag();
    void setDragValue(const QVariant& value);
    QVariant valueOf(StyleId id, double number) const;
    double currentNumberOf(StyleId id) const;

    QString m_music;
    QVariantMap m_overrides;
    double m_systemWidth = 0.0;
    bool m_showsPage = false;
    double m_zoom = 1.0;

    QString m_activeSid;
    StyleId m_activeStyleId = StyleId::NOSTYLE;
    QVariant m_probeValue;
    bool m_isDraggable = true;
    bool m_showsHandles = true;
    bool m_showsControlPoints = false;
    bool m_showingProbed = false;
    QColor m_accentColor = QColor(0x4A, 0x7D, 0xD9);

    double m_dragMinimum = -1e9;
    double m_dragMaximum = 1e9;
    double m_dragStep = 0.01;

    //! NOTE The style which the picture follows: the one of the current score
    INotationStylePtr m_style;

    std::unique_ptr<EngravingPreviewRenderer> m_renderer;
    bool m_isMusicLoaded = false;
    QString m_error;

    EngravingPreviewRenderer::Effect m_effect;
    std::optional<PropertyValue> m_activeProbe;
    muse::RectF m_changeBox;    // the union of the changed boxes, in staff spaces
    std::vector<std::vector<muse::PointF> > m_curves;

    PreviewHandle m_handle;
    QVariant m_dragValue;
    double m_pressedNumber = 0.0;   // the value when the drag began, in engine units
    double m_draggedNumber = 0.0;   // the value under the pointer, in engine units

    QImage m_image;
    QImage m_probedImage;
    PreviewOverlay::Ghost m_probedGhost;
    //! NOTE Room outside `m_image` needed so a flipped or extended probed ghost/preview is not clipped
    QMarginsF m_extraBounds;
    QSizeF m_naturalSize;
    QSizeF m_naturalPageSize;
    std::vector<muse::RectF> m_pages;   // where the pages are in the picture, in staff spaces, if it shows them
    QSizeF m_contentSize;
    bool m_needsLayout = true;
    bool m_needsPaint = true;
};
}
