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
#include "engravingpreview.h"

#include <cmath>

#include <QList>
#include <QPainter>
#include <QPointer>
#include <QQuickWindow>
#include <QTimer>

#include "draw/painter.h"
#include "engraving/dom/score.h"
#include "engraving/style/style.h"

#include "notation/inotation.h"
#include "notation/inotationelements.h"
#include "notation/inotationstyle.h"

#include "../styledialog/styleuivalue.h"
#include "engravingpreviewrenderer.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;

//! NOTE The staff space of a picture at the zoom 1.0, in pixels: a staff is as tall as two lines of text
static constexpr double BASE_STAFF_SPACE = 7.0;

static QString s_snippetsDir = QStringLiteral(":/qml/MuseScore/NotationScene/internal/EngravingOptions/snippets");

namespace mu::notation {
//! NOTE A dialog page has many pictures, and a change of the style makes all of them out of date at once.
//! They are made again one by one, each in a turn of the event loop of its own, so that the interface
//! stays alive while they are
class EngravingPreviewQueue : public QObject
{
public:
    static EngravingPreviewQueue* instance()
    {
        static EngravingPreviewQueue queue;
        return &queue;
    }

    void add(EngravingPreview* preview)
    {
        if (!m_previews.contains(preview)) {
            m_previews.append(preview);
        }

        if (!m_timer.isActive()) {
            m_timer.start();
        }
    }

private:
    EngravingPreviewQueue()
    {
        m_timer.setInterval(0);
        m_timer.setSingleShot(true);

        connect(&m_timer, &QTimer::timeout, this, [this]() {
            while (!m_previews.isEmpty()) {
                const QPointer<EngravingPreview> preview = m_previews.takeFirst();
                if (preview) {
                    preview->render();
                    break;
                }
            }

            if (!m_previews.isEmpty()) {
                m_timer.start();
            }
        });
    }

    QList<QPointer<EngravingPreview> > m_previews;
    QTimer m_timer;
};
}

EngravingPreview::EngravingPreview(QQuickItem* parent)
    : QQuickPaintedItem(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

EngravingPreview::~EngravingPreview() = default;

QString EngravingPreview::snippetPath(const QString& snippet)
{
    return s_snippetsDir + "/" + snippet + ".mscx";
}

void EngravingPreview::setSnippetsDir(const QString& dir)
{
    s_snippetsDir = dir;
}

QString EngravingPreview::snippet() const
{
    return m_snippet;
}

void EngravingPreview::setSnippet(const QString& snippet)
{
    if (m_snippet == snippet) {
        return;
    }

    m_snippet = snippet;
    m_isSnippetLoaded = false;
    emit snippetChanged();

    invalidate();
}

QVariantMap EngravingPreview::overrides() const
{
    return m_overrides;
}

void EngravingPreview::setOverrides(const QVariantMap& overrides)
{
    if (m_overrides == overrides) {
        return;
    }

    m_overrides = overrides;
    emit overridesChanged();

    invalidate();
}

double EngravingPreview::systemWidth() const
{
    return m_systemWidth;
}

void EngravingPreview::setSystemWidth(double widthSp)
{
    if (qFuzzyCompare(m_systemWidth, widthSp)) {
        return;
    }

    m_systemWidth = widthSp;
    emit systemWidthChanged();

    invalidate();
}

double EngravingPreview::zoom() const
{
    return m_zoom;
}

void EngravingPreview::setZoom(double zoom)
{
    if (zoom <= 0.0 || qFuzzyCompare(m_zoom, zoom)) {
        return;
    }

    m_zoom = zoom;
    emit zoomChanged();

    invalidate();
}

QSizeF EngravingPreview::contentSize() const
{
    return m_contentSize;
}

bool EngravingPreview::isValid() const
{
    return !m_image.isNull();
}

void EngravingPreview::componentComplete()
{
    QQuickPaintedItem::componentComplete();

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        listenStyle();
        invalidate();
    });

    listenStyle();
    invalidate();
}

void EngravingPreview::listenStyle()
{
    const INotationPtr notation = globalContext()->currentNotation();
    const INotationStylePtr style = notation ? notation->style() : nullptr;
    if (style == m_style) {
        return;
    }

    //! NOTE The style of a score which is no longer the current one is not followed any more
    if (m_style) {
        m_style->styleChanged().resetOnNotify(this);
    }

    m_style = style;

    if (m_style) {
        m_style->styleChanged().onNotify(this, [this]() {
            invalidate();
        });
    }
}

void EngravingPreview::itemChange(ItemChange change, const ItemChangeData& value)
{
    QQuickPaintedItem::itemChange(change, value);

    //! NOTE A picture which is not shown is not made; it is made when it is shown, if it is out of date
    const bool isShown = (change == ItemVisibleHasChanged && value.boolValue)
                         || (change == ItemSceneChange && value.window);
    if (isShown && m_isDirty) {
        invalidate();
    }
}

void EngravingPreview::invalidate()
{
    m_isDirty = true;

    if (isComponentComplete() && isVisible() && window()) {
        EngravingPreviewQueue::instance()->add(this);
    }
}

void EngravingPreview::render()
{
    if (!m_isDirty) {
        return;
    }

    m_isDirty = false;

    const bool wasValid = isValid();
    const QSizeF oldContentSize = m_contentSize;

    m_image = QImage();
    m_contentSize = QSizeF();

    const auto finish = [this, wasValid, oldContentSize]() {
        setImplicitSize(m_contentSize.width(), m_contentSize.height());

        if (m_contentSize != oldContentSize) {
            emit contentSizeChanged();
        }

        if (isValid() != wasValid) {
            emit isValidChanged();
        }

        update();
    };

    const INotationPtr notation = globalContext()->currentNotation();
    if (!notation || m_snippet.isEmpty()) {
        finish();
        return;
    }

    if (!m_renderer) {
        m_renderer = std::make_unique<EngravingPreviewRenderer>(iocContext());
    }

    if (!m_isSnippetLoaded) {
        if (!m_renderer->load(snippetPath(m_snippet))) {
            finish();
            return;
        }

        m_isSnippetLoaded = true;
    }

    std::vector<EngravingPreviewRenderer::Override> overrides;
    for (auto it = m_overrides.constBegin(); it != m_overrides.constEnd(); ++it) {
        const StyleId id = MStyle::styleIdx(String::fromQString(it.key()));
        if (id == StyleId::NOSTYLE) {
            LOGE() << "no style value with the name: " << it.key() << ", snippet: " << m_snippet;
            continue;
        }

        overrides.push_back({ id, styleValueFromUi(id, it.value()) });
    }

    m_renderer->layout(notation->elements()->msScore()->style(), overrides, m_systemWidth);

    const double staffSpace = BASE_STAFF_SPACE * m_zoom;
    const muse::SizeF size = m_renderer->sizeInStaffSpaces() * staffSpace;
    if (size.width() <= 0.0 || size.height() <= 0.0) {
        finish();
        return;
    }

    m_contentSize = QSizeF(std::ceil(size.width()), std::ceil(size.height()));

    const qreal pixelRatio = window() ? window()->effectiveDevicePixelRatio() : 1.0;

    QImage image((m_contentSize * pixelRatio).toSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    {
        //! NOTE The engine paints in the pixels of the image, hence the staff space in them
        muse::draw::Painter painter(&image, "engravingpreview");
        m_renderer->paint(painter, staffSpace * pixelRatio);
    }

    image.setDevicePixelRatio(pixelRatio);
    m_image = image;

    finish();
}

void EngravingPreview::paint(QPainter* painter)
{
    if (m_image.isNull() || width() <= 0.0 || height() <= 0.0) {
        return;
    }

    //! NOTE In the middle of the item, scaled down if the item is smaller than the picture, never scaled up
    const double scale = std::min({ 1.0, width() / m_contentSize.width(), height() / m_contentSize.height() });
    const QSizeF size = m_contentSize * scale;
    const QRectF target((width() - size.width()) / 2.0, (height() - size.height()) / 2.0, size.width(), size.height());

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->drawImage(target, m_image);
}
