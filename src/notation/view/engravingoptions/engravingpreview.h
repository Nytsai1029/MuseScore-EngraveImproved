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

#include <QImage>
#include <QQuickPaintedItem>
#include <QVariantMap>

#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "context/iglobalcontext.h"
#include "notation/inotationstyle.h"

namespace mu::notation {
class EngravingPreviewRenderer;

//! NOTE A picture of a snippet of music, laid out by the engraving engine with the style of the current score.
//! It follows the style: when an option of the Engraving Options dialog is changed, the picture shows the result.
//! With `overrides` the picture shows what given values of some options would do (the choices of an option)
class EngravingPreview : public QQuickPaintedItem, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    //! NOTE The name of the snippet (a score file among the resources of the dialog), without the suffix
    Q_PROPERTY(QString snippet READ snippet WRITE setSnippet NOTIFY snippetChanged)

    //! NOTE Style values by their names, as the dialog has them, which the picture sets whatever the score has
    Q_PROPERTY(QVariantMap overrides READ overrides WRITE setOverrides NOTIFY overridesChanged)

    //! NOTE The width of the systems in staff spaces, for a snippet which shows the end of a system.
    //! Not positive: each system is as wide as its music needs
    Q_PROPERTY(double systemWidth READ systemWidth WRITE setSystemWidth NOTIFY systemWidthChanged)

    //! NOTE 1.0 is the staff space of BASE_STAFF_SPACE pixels
    Q_PROPERTY(double zoom READ zoom WRITE setZoom NOTIFY zoomChanged)

    //! NOTE The size of the picture at this zoom. An item which is smaller shows the picture scaled down
    Q_PROPERTY(QSizeF contentSize READ contentSize NOTIFY contentSizeChanged)

    Q_PROPERTY(bool isValid READ isValid NOTIFY isValidChanged)

    muse::Inject<context::IGlobalContext> globalContext = { this };

public:
    explicit EngravingPreview(QQuickItem* parent = nullptr);
    ~EngravingPreview() override;

    QString snippet() const;
    void setSnippet(const QString& snippet);

    QVariantMap overrides() const;
    void setOverrides(const QVariantMap& overrides);

    double systemWidth() const;
    void setSystemWidth(double widthSp);

    double zoom() const;
    void setZoom(double zoom);

    QSizeF contentSize() const;
    bool isValid() const;

    void paint(QPainter* painter) override;

    //! NOTE Where the snippets are; the tests give the folder of the sources
    static QString snippetPath(const QString& snippet);
    static void setSnippetsDir(const QString& dir);

signals:
    void snippetChanged();
    void overridesChanged();
    void systemWidthChanged();
    void zoomChanged();
    void contentSizeChanged();
    void isValidChanged();

protected:
    void componentComplete() override;
    void itemChange(ItemChange change, const ItemChangeData& value) override;

private:
    friend class EngravingPreviewQueue;

    void listenStyle();
    void invalidate();
    void render();

    QString m_snippet;
    QVariantMap m_overrides;
    double m_systemWidth = 0.0;
    double m_zoom = 1.0;

    //! NOTE The style which the picture follows: the one of the current score
    INotationStylePtr m_style;

    std::unique_ptr<EngravingPreviewRenderer> m_renderer;
    bool m_isSnippetLoaded = false;

    QImage m_image;
    QSizeF m_contentSize;
    bool m_isDirty = true;
};
}
