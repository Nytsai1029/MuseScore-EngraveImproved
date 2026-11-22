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
#include "engravingpreviewrenderer.h"

#include <cmath>

#include "engraving/compat/mscxcompat.h"
#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/chordlist.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/page.h"
#include "engraving/iengravingfontsprovider.h"
#include "engraving/types/types.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace mu::engraving::rendering;

//! NOTE A page on which any snippet has room, so that no system or page break comes from the page.
//! A system fills the width only if a width is given
static constexpr double PAGE_SIZE_INCH = 80.0;

EngravingPreviewRenderer::EngravingPreviewRenderer(const muse::modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx)
{
}

//! NOTE Here, where the score is a complete type
EngravingPreviewRenderer::~EngravingPreviewRenderer() = default;

muse::Ret EngravingPreviewRenderer::load(const muse::io::path_t& snippetPath)
{
    m_score.reset();

    std::unique_ptr<MasterScore> score(compat::ScoreAccess::createMasterScoreWithBaseStyle(iocContext()));

    const muse::Ret ret = compat::loadMsczOrMscx(score.get(), snippetPath);
    if (!ret) {
        LOGE() << "failed to load the snippet: " << snippetPath << ", " << ret.toString();
        return ret;
    }

    m_score = std::move(score);
    return muse::make_ok();
}

bool EngravingPreviewRenderer::isLoaded() const
{
    return m_score != nullptr;
}

void EngravingPreviewRenderer::makePreviewStyle(MStyle& style, double systemWidthSp)
{
    const double systemWidthInch = systemWidthSp > 0.0 ? systemWidthSp * style.spatium() / DPI : PAGE_SIZE_INCH;

    style.set(Sid::pageWidth, systemWidthInch);
    style.set(Sid::pagePrintableWidth, systemWidthInch);
    style.set(Sid::pageHeight, PAGE_SIZE_INCH);
    style.set(Sid::pageTwosided, false);

    for (Sid margin : { Sid::pageEvenLeftMargin, Sid::pageOddLeftMargin, Sid::pageEvenTopMargin,
                        Sid::pageOddTopMargin, Sid::pageEvenBottomMargin, Sid::pageOddBottomMargin }) {
        style.set(margin, 0.0);
    }

    style.set(Sid::showHeader, false);
    style.set(Sid::showFooter, false);
    style.set(Sid::showMeasureNumber, false);

    style.set(Sid::enableIndentationOnFirstSystem, false);
    style.set(Sid::firstSystemInstNameVisibility, static_cast<int>(InstrumentLabelVisibility::HIDE));
    style.set(Sid::subsSystemInstNameVisibility, static_cast<int>(InstrumentLabelVisibility::HIDE));

    //! NOTE The systems are neither spread over the height of the page, nor is the last one made as wide as the page
    style.set(Sid::enableVerticalSpread, false);
    style.set(Sid::lastSystemFillLimit, 1.0);
}

void EngravingPreviewRenderer::layout(const MStyle& style, const std::vector<Override>& overrides, double systemWidthSp)
{
    IF_ASSERT_FAILED(isLoaded()) {
        return;
    }

    MStyle previewStyle = style;
    makePreviewStyle(previewStyle, systemWidthSp);

    for (const Override& override : overrides) {
        previewStyle.set(override.id, override.value);
    }

    //! NOTE What a change of the style takes with it, as when a style is applied to a score (see ChangeStyle)
    const MStyle& oldStyle = m_score->style();
    const double oldSpatium = oldStyle.spatium();

    if (oldStyle.styleSt(Sid::musicalSymbolFont) != previewStyle.styleSt(Sid::musicalSymbolFont)) {
        const std::string fontName = previewStyle.styleSt(Sid::musicalSymbolFont).toStdString();
        m_score->setEngravingFont(m_score->engravingFonts()->fontByName(fontName));
    }

    m_score->setStyle(previewStyle);
    m_score->chordList()->unload();
    m_score->checkChordList();

    if (oldSpatium != previewStyle.spatium()) {
        m_score->spatiumChanged(oldSpatium, previewStyle.spatium());
    }

    m_score->styleChanged();

    m_score->setLayoutAll();
    m_score->doLayout();
}

IScoreRenderer::PaintOptions EngravingPreviewRenderer::paintOptions(double pixelsPerStaffSpace) const
{
    IScoreRenderer::PaintOptions options;
    options.isPrinting = true;
    options.printPageBackground = false;
    options.fromPage = 0;
    options.toPage = 0;
    options.trimMarginPixelSize = 0;
    options.deviceDpi = std::max(1, static_cast<int>(std::lrint(DPI * pixelsPerStaffSpace / m_score->style().spatium())));

    return options;
}

muse::SizeF EngravingPreviewRenderer::sizeInStaffSpaces() const
{
    if (!isLoaded() || m_score->pages().empty()) {
        return muse::SizeF();
    }

    return m_score->pages().front()->tbbox().size() / m_score->style().spatium();
}

void EngravingPreviewRenderer::paint(muse::draw::Painter& painter, double pixelsPerStaffSpace) const
{
    if (!isLoaded() || m_score->pages().empty()) {
        return;
    }

    scoreRenderer()->paintScore(&painter, m_score.get(), paintOptions(pixelsPerStaffSpace));
}
