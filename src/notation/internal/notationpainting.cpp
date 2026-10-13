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
#include "notationpainting.h"

#include <algorithm>
#include <iterator>

#include <QScreen>

#include "engraving/dom/page.h"
#include "engraving/dom/score.h"

#include "engravemode.h"
#include "ledgerguides.h"
#include "notation.h"
#include "notationinteraction.h"

#include "log.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse::draw;

//! NOTE The staves of a system take these colours in turn
static const Color LEDGER_GUIDE_COLORS[] = { Color(240, 168, 96), Color(120, 160, 224) };

//! NOTE Guides which would be closer to each other than this on the screen are not drawn (in px):
//! first the ones in the spaces, then, at a still smaller zoom, all of them
static constexpr double LEDGER_GUIDES_MIN_STEP = 3.0;

NotationPainting::NotationPainting(Notation* notation)
    : m_notation(notation)
{
}

mu::engraving::Score* NotationPainting::score() const
{
    return m_notation->score();
}

void NotationPainting::setViewMode(const ViewMode& viewMode)
{
    if (!score()) {
        return;
    }

    if (score()->layoutMode() == viewMode) {
        return;
    }

    score()->setLayoutMode(viewMode);
    score()->doLayout();

    m_viewModeChanged.notify();
    m_notation->notifyAboutNotationChanged();
}

ViewMode NotationPainting::viewMode() const
{
    if (!score()) {
        return ViewMode::PAGE;
    }

    return score()->layoutMode();
}

muse::async::Notification NotationPainting::viewModeChanged() const
{
    return m_viewModeChanged;
}

int NotationPainting::pageCount() const
{
    if (!score()) {
        return 0;
    }

    return static_cast<int>(score()->npages());
}

SizeF NotationPainting::pageSizeInch() const
{
    return scoreRenderer()->pageSizeInch(score());
}

SizeF NotationPainting::pageSizeInch(const Options& opt) const
{
    return scoreRenderer()->pageSizeInch(score(), opt);
}

bool NotationPainting::isPaintPageBorder() const
{
    switch (score()->layoutMode()) {
    case engraving::LayoutMode::LINE:
    case engraving::LayoutMode::HORIZONTAL_FIXED:
    case engraving::LayoutMode::SYSTEM:
        return false;
    case engraving::LayoutMode::FLOAT:
    case engraving::LayoutMode::PAGE: {
        return !score()->printing();
    }
    }
    return false;
}

void NotationPainting::doPaint(Painter* painter, const Options& opt)
{
    TRACEFUNC;
    if (!score()) {
        return;
    }

    Options myopt = opt;
    bool printPageBackground = myopt.printPageBackground;

    //! NOTE The ledger guides are a working aid of the Engrave mode, they are not a part of the score
    const bool showLedgerGuides = !myopt.isPrinting && configuration()->isLedgerGuidesVisible()
                                  && isEngraveModeOpened(interactive());
    const RectF frame = myopt.frameRect;

    myopt.onPaintPageSheet = [this, printPageBackground, showLedgerGuides, frame](Painter* painter, const Page* page,
                                                                                 const RectF& pageRect) {
        paintPageSheet(painter, page, pageRect, printPageBackground);

        if (showLedgerGuides) {
            paintLedgerGuides(painter, page, frame.isValid() ? frame.translated(-page->pos()) : RectF());
        }
    };

    scoreRenderer()->paintScore(painter, score(), myopt);

    if (!myopt.isPrinting) {
        static_cast<NotationInteraction*>(m_notation->interaction().get())->paint(painter);
    }
}

void NotationPainting::paintPageSheet(Painter* painter, const Page* page, const RectF& pageRect, bool printPageBackground) const
{
    TRACEFUNC;
    if (score()->printing()) {
        if (!printPageBackground) {
            return;
        }

        painter->fillRect(pageRect, Color::WHITE);
        return;
    }

    if (configuration()->foregroundUseColor()) {
        painter->fillRect(pageRect, configuration()->foregroundColor());
    } else {
        const QPixmap& wallpaper = configuration()->foregroundWallpaper();
        if (!wallpaper.isNull()) {
            painter->drawTiledPixmap(pageRect, wallpaper);
        } else {
            //! NOTE We can use the color from the configuration,
            //! but in this case I believe it is better to use the "unassigned" color
            painter->fillRect(pageRect, Color::WHITE);
        }
    }

    if (!isPaintPageBorder()) {
        return;
    }

    painter->setBrush(BrushStyle::NoBrush);
    painter->setPen(Pen(configuration()->borderColor(), configuration()->borderWidth()));
    painter->drawRect(pageRect);

    if (!score()->showPageborders()) {
        return;
    }

    RectF pageContentRect = page->ldata()->bbox().adjusted(page->lm(), page->tm(), -page->rm(), -page->bm());

    painter->setBrush(BrushStyle::NoBrush);
    painter->setPen(engravingConfiguration()->scoreGreyColor());
    painter->drawRect(pageContentRect);

    if (!page->isOdd()) {
        painter->drawLine(pageContentRect.right(), 0.0, pageContentRect.right(), pageContentRect.bottom());
    }
}

void NotationPainting::paintLedgerGuides(Painter* painter, const Page* page, const RectF& visibleRect) const
{
    TRACEFUNC;

    const double scaling = painter->worldTransform().m11();
    const double spatium = score()->style().spatium() * scaling;
    if (spatium < LEDGER_GUIDES_MIN_STEP) {
        return;
    }

    const bool isSpacesShown = spatium * 0.5 >= LEDGER_GUIDES_MIN_STEP;

    const std::vector<LedgerGuide> guides = ledgerGuides(page, visibleRect);
    if (guides.empty()) {
        return;
    }

    //! NOTE Not thinner than a pixel, so the guides are seen at a small zoom too
    const double minWidth = 1.0 / scaling;

    painter->save();
    painter->setBrush(BrushStyle::NoBrush);

    for (const LedgerGuide& guide : guides) {
        if (guide.isSpace && !isSpacesShown) {
            continue;
        }

        const double width = std::max(guide.width, minWidth);

        Pen pen(LEDGER_GUIDE_COLORS[guide.colorIndex % std::size(LEDGER_GUIDE_COLORS)], width);
        pen.setCapStyle(PenCapStyle::FlatCap);

        if (guide.isSpace) {
            //! NOTE The pattern is in the widths of the pen
            const double dash = guide.dash / width;
            pen.setDashPattern({ dash, dash });
        }

        painter->setPen(pen);
        painter->drawLine(guide.line);
    }

    painter->restore();
}

void NotationPainting::paintView(Painter* painter, const RectF& frameRect, bool isPrinting)
{
    Options opt;
    opt.isSetViewport = false;
    opt.isMultiPage = true;
    opt.frameRect = frameRect;
    opt.deviceDpi = uiConfiguration()->logicalDpi();
    opt.isPrinting = isPrinting;
    doPaint(painter, opt);
}

void NotationPainting::paintPdf(Painter* painter, const Options& opt)
{
    Q_ASSERT(opt.deviceDpi > 0);
    Options myopt = opt;
    myopt.isSetViewport = true;
    myopt.isMultiPage = false;
    myopt.isPrinting = true;
    doPaint(painter, myopt);
}

void NotationPainting::paintPrint(Painter* painter, const Options& opt)
{
    Q_ASSERT(opt.deviceDpi > 0);
    Options myopt = opt;
    myopt.isSetViewport = true;
    myopt.isMultiPage = false;
    myopt.isPrinting = true;
    doPaint(painter, myopt);
}

void NotationPainting::paintPng(Painter* painter, const Options& opt)
{
    Q_ASSERT(opt.deviceDpi > 0);
    Options myopt = opt;
    myopt.isSetViewport = true;
    myopt.isMultiPage = false;
    myopt.isPrinting = true;
    doPaint(painter, myopt);
}
