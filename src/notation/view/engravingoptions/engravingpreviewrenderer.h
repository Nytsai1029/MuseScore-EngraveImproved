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
#include <vector>

#include "draw/painter.h"
#include "draw/types/geometry.h"
#include "io/path.h"
#include "modularity/ioc.h"
#include "types/ret.h"

#include "engraving/rendering/iscorerenderer.h"
#include "engraving/style/style.h"

#include "notation/notationtypes.h"

namespace mu::engraving {
class MasterScore;
}

namespace mu::notation {
//! NOTE A snippet of music laid out by the engraving engine with a given style, for the previews of the
//! Engraving Options dialog: what the user sees there is what the options do to a score, not a drawing of it.
//! The snippet is a small score file. It is laid out as a line of its natural width (or of a given width,
//! when it shows what happens at the end of a system), with nothing of a page around it
class EngravingPreviewRenderer : public muse::Injectable
{
    muse::Inject<engraving::rendering::IScoreRenderer> scoreRenderer = { this };

public:
    explicit EngravingPreviewRenderer(const muse::modularity::ContextPtr& iocCtx);
    ~EngravingPreviewRenderer();

    //! NOTE A style value which the preview sets, whatever the style of the score has
    struct Override {
        StyleId id = StyleId::NOSTYLE;
        PropertyValue value;
    };

    muse::Ret load(const muse::io::path_t& snippetPath);
    bool isLoaded() const;

    //! NOTE Lays the snippet out with the style: the given one without what belongs to a page
    //! (see makePreviewStyle), then the overrides. The width of the systems is given
    //! in staff spaces; not positive: each system is as wide as its music needs
    void layout(const engraving::MStyle& style, const std::vector<Override>& overrides, double systemWidthSp = 0.0);

    //! NOTE The size of the laid out snippet, which is the first page cut down to what is printed on it.
    //! In staff spaces, so it does not depend on the staff size of the style
    muse::SizeF sizeInStaffSpaces() const;

    //! NOTE Paints the laid out snippet from the top left corner of the painter,
    //! with the given number of device pixels to a staff space
    void paint(muse::draw::Painter& painter, double pixelsPerStaffSpace) const;

    //! NOTE The style of a preview: nothing of a page (size, margins, header, footer), no instrument names,
    //! no measure numbers, the systems where the music puts them. A preview which shows one of these
    //! sets it again with an override
    static void makePreviewStyle(engraving::MStyle& style, double systemWidthSp);

private:
    engraving::rendering::IScoreRenderer::PaintOptions paintOptions(double pixelsPerStaffSpace) const;

    std::unique_ptr<engraving::MasterScore> m_score;
};
}
