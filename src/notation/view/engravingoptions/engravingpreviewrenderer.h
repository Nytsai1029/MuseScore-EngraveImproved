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

#include <QImage>
#include <QString>

#include "draw/painter.h"
#include "draw/types/geometry.h"
#include "modularity/ioc.h"
#include "types/ret.h"

#include "engraving/rendering/iscorerenderer.h"
#include "engraving/style/style.h"
#include "engraving/types/types.h"

#include "notation/notationtypes.h"

namespace mu::engraving {
class MasterScore;
}

namespace mu::notation {
//! NOTE Music laid out by the engraving engine with a given style, for the previews of the Engraving Options
//! dialog: what the user sees there is what the options do to a score, not a drawing of it.
//! The music is written as text (see PreviewMusic). It is laid out as a line of its natural width (or of a given
//! width, when it shows what happens at the end of a system), with nothing of a page around it — or, for the
//! options of the page itself (margins, vertical spacing, header and footer), on the first page of the score as
//! the style has it (see setShowsPage)
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

    //! NOTE One laid out item of the first page
    struct Item {
        engraving::ElementType type = engraving::ElementType::INVALID;
        int track = -1;     // -1 where the item has none
        int tick = -1;
        int system = -1;    // index of the system the item stands on; -1 if outside any system
        muse::RectF box;    // in staff spaces, from the top left corner of the picture
        std::vector<muse::PointF> curve;    // of a slur or tie: start, bezier 1, bezier 2, end; else empty
    };

    //! NOTE What a change of one style value does to the picture
    struct Effect {
        //! the boxes, in the layout as it is, of the items which the change moves or reshapes
        std::vector<muse::RectF> changed;
        //! the boxes, in the same frame, where the changed or added items land in the probed layout
        std::vector<muse::RectF> afterChanged;
        //! how far the probed picture's origin shifts relative to the layout as it is, in staff spaces
        muse::PointF probeFrameShift;
        bool hasHandle = false;
        muse::PointF anchor;                // where the handle sits: on the edge which moves most
        muse::PointF direction;             // the unit vector along which that edge moves when the value grows
        double staffSpacesPerUnit = 0.0;    // how far it moves for a change of the value by 1

        bool isEmpty() const { return changed.empty(); }
    };

    //! NOTE Takes the music as text, and keeps the reason (with the place in the text) if it is not valid
    muse::Ret setMusic(const QString& notation);
    QString error() const;
    bool isLoaded() const;

    //! NOTE Whether the music is laid out on pages as the style has them (their size and margins, the header and
    //! the footer, the vertical spacing), and the picture is every page of it, whole, side by side as the layout puts
    //! them; else the music alone, on the first page
    void setShowsPage(bool showsPage);
    bool showsPage() const;

    //! NOTE Lays the music out with the style: the given one without what belongs to a page
    //! (see makePreviewStyle) unless the picture shows the page, then the overrides. The width of the systems is
    //! given in staff spaces; not positive: each system is as wide as its music needs (a page has its own width).
    //! `keptId` is the style value the preview is showing: makePreviewStyle leaves it as the score has it
    void layout(const engraving::MStyle& style, const std::vector<Override>& overrides, double systemWidthSp = 0.0,
                StyleId keptId = StyleId::NOSTYLE);

    //! NOTE The size of the laid out music, which is the first page cut down to what is printed on it (the whole
    //! pages if it shows the pages). In staff spaces, so it does not depend on the staff size of the style
    muse::SizeF sizeInStaffSpaces() const;

    //! NOTE Where the pages are in the picture, in staff spaces, if it shows the pages; else nothing
    std::vector<muse::RectF> pagesInStaffSpaces() const;

    //! NOTE The items of the first page (of every page if it shows the pages), in a stable order (type, track,
    //! tick, position), so that two layouts of the same music can be compared item by item
    std::vector<Item> items() const;

    //! NOTE What it does when the value `id` is `probe` (the style and the overrides have the other values).
    //! The music stays laid out as it was
    Effect effectOf(const engraving::MStyle& style, const std::vector<Override>& overrides, StyleId id,
                    const PropertyValue& probe, double systemWidthSp = 0.0);

    //! NOTE The items of two layouts of the same music, marked: which of them the change really moves or
    //! reshapes (the pairs), and which exist in one layout only (the boxes; afterBoxes keeps only those added
    //! in the probed layout). `pairs` keeps `before` first
    struct MarkedChanges {
        std::vector<std::pair<Item, Item> > pairs;
        std::vector<muse::RectF> boxes;
        std::vector<muse::RectF> afterBoxes;
    };

    //! NOTE Which items are marked as changed between two layouts of the same music: an item which is only
    //! carried along is not (the same size, moved horizontally only, by as much as the nearest item to its left
    //! on the same staff; the first item of such a run stays marked). The staff lines are marked only if nothing
    //! else is. Public for the tests
    static MarkedChanges markedChanges(const std::vector<Item>& before, const std::vector<Item>& after);

    //! NOTE The value of `id` as the music is laid out with it: the style's, unless an override of the same id
    //! wins
    static PropertyValue valueOf(const engraving::MStyle& style, const std::vector<Override>& overrides, StyleId id);

    //! NOTE What a value is changed to, to see what it does: a visible step in the direction which has room
    //! (below `maximum`) — of a scale, in both dimensions —, or the other value of a switch. Nothing for a value
    //! which a handle cannot take
    static std::optional<PropertyValue> probeOf(const engraving::MStyle& style, const std::vector<Override>& overrides,
                                                StyleId id, double maximum = 1e9);

    //! NOTE The score, for what the items do not tell
    const engraving::MasterScore* score() const;

    //! NOTE Paints the laid out music from the top left corner of the painter. A staff space is
    //! `pixelsPerStaffSpace` units of the painter, on a device whose logical DPI is `deviceLogicalDpi`:
    //! it has to be the DPI the painter draws on, so that the glyphs and the text are the right size.
    //! The painter's transform is changed while painting and restored afterwards
    void paint(muse::draw::Painter& painter, double pixelsPerStaffSpace, int deviceLogicalDpi) const;

    //! NOTE The laid out music as an image: `pixelsPerStaffSpace` logical pixels to a staff space, for a
    //! device with the given pixel ratio (the image has it set). Empty if nothing is laid out
    QImage picture(double pixelsPerStaffSpace, double devicePixelRatio = 1.0) const;

    //! NOTE The style of a preview: nothing of a page (size, margins, header, footer), no instrument names,
    //! no measure numbers, the systems where the music puts them. A preview which shows one of these keeps the
    //! score's own value for `keptId` (or sets it again with an override)
    static void makePreviewStyle(engraving::MStyle& style, double systemWidthSp, StyleId keptId = StyleId::NOSTYLE);

private:
    //! NOTE The part of the first page which the picture shows: what is printed, plus a margin around it,
    //! in page coordinates. It is made once, at the end of layout()
    muse::RectF pictureRect() const { return m_pictureRect; }

    std::unique_ptr<engraving::MasterScore> m_score;
    bool m_isLaidOut = false;   // the music has been laid out at least once since it was read
    bool m_showsPage = false;
    muse::RectF m_pictureRect;
    QString m_error;
};
}
