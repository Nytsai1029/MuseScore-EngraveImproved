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

#include <algorithm>
#include <cmath>
#include <optional>
#include <QImage>
#include <tuple>

#include "io/buffer.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/chordlist.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/page.h"
#include "engraving/dom/slurtie.h"
#include "engraving/iengravingfontsprovider.h"
#include "engraving/infrastructure/localfileinfoprovider.h"
#include "engraving/infrastructure/mscreader.h"
#include "engraving/rw/mscloader.h"
#include "engraving/types/types.h"

#include "previewmusic.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace mu::engraving::rendering;

//! NOTE A page on which any snippet has room, so that no system or page break comes from the page.
//! A system fills the width only if a width is given
static constexpr double PAGE_SIZE_INCH = 80.0;

//! NOTE Room around what is printed, in staff spaces: half of a line on the edge and the antialiasing of a
//! glyph would be cut otherwise
static constexpr double PICTURE_MARGIN = 0.5;

EngravingPreviewRenderer::EngravingPreviewRenderer(const muse::modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx)
{
}

//! NOTE Here, where the score is a complete type
EngravingPreviewRenderer::~EngravingPreviewRenderer() = default;

//! NOTE The reader of a score file wants a file (or a container of files): the one file in an XML container,
//! which a buffer holds
static muse::ByteArray inXmlContainer(const muse::ByteArray& mscx)
{
    QByteArray text = QByteArray::fromRawData(mscx.constChar(), static_cast<qsizetype>(mscx.size()));
    text.replace("]]>", "]]]]><![CDATA[>");

    return muse::ByteArray::fromQByteArray("<files><file name=\"score.mscx\"><![CDATA[" + text + "]]></file></files>");
}

muse::Ret EngravingPreviewRenderer::setMusic(const QString& notation)
{
    m_score.reset();
    m_error.clear();

    const muse::RetVal<muse::ByteArray> mscx = PreviewMusic::toMscx(notation);
    if (!mscx.ret) {
        m_error = QString::fromStdString(mscx.ret.text());
        return mscx.ret;
    }

    muse::ByteArray container = inXmlContainer(mscx.val);
    muse::io::Buffer buffer(&container);
    buffer.open(muse::io::IODevice::ReadOnly);

    MscReader::Params params;
    params.device = &buffer;
    params.mode = MscIoMode::XmlFile;

    MscReader reader(params);
    muse::Ret ret = reader.open();
    if (ret) {
        std::unique_ptr<MasterScore> score(compat::ScoreAccess::createMasterScoreWithBaseStyle(iocContext()));

        //! NOTE The loader asks the score for the name of its file
        score->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(muse::io::path_t("preview.mscx")));

        SettingsCompat audioSettings;
        ret = MscLoader().loadMscz(score.get(), reader, audioSettings, false);
        if (ret) {
            m_score = std::move(score);
        }
    }

    if (!ret) {
        m_error = QString::fromStdString(ret.toString());
        LOGE() << "failed to load the music of a preview: " << ret.toString();
    }

    return ret;
}

QString EngravingPreviewRenderer::error() const
{
    return m_error;
}

bool EngravingPreviewRenderer::isLoaded() const
{
    return m_score != nullptr;
}

void EngravingPreviewRenderer::setShowsPage(bool showsPage)
{
    m_showsPage = showsPage;
}

bool EngravingPreviewRenderer::showsPage() const
{
    return m_showsPage;
}

const MasterScore* EngravingPreviewRenderer::score() const
{
    return m_score.get();
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
    if (!m_showsPage) {
        makePreviewStyle(previewStyle, systemWidthSp);
    }

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
    m_score->createPaddingTable();

    m_score->setLayoutAll();
    m_score->doLayout();

    if (m_score->pages().empty()) {
        m_pictureMargin = 0;
        m_pictureRect = muse::RectF();
    } else if (m_showsPage) {
        m_pictureMargin = 0;
        m_pictureRect = muse::RectF();
        for (const Page* page : m_score->pages()) {
            m_pictureRect.unite(page->ldata()->bbox().translated(page->pos()));
        }
    } else {
        m_pictureMargin = static_cast<int>(std::lrint(PICTURE_MARGIN * previewStyle.spatium()));
        m_pictureRect = m_score->pages().front()->tbbox().adjusted(-m_pictureMargin, -m_pictureMargin,
                                                                 m_pictureMargin, m_pictureMargin);
    }
}

muse::SizeF EngravingPreviewRenderer::sizeInStaffSpaces() const
{
    if (!isLoaded() || m_score->pages().empty()) {
        return muse::SizeF();
    }

    return pictureRect().size() / m_score->style().spatium();
}

std::vector<muse::RectF> EngravingPreviewRenderer::pagesInStaffSpaces() const
{
    std::vector<muse::RectF> result;
    if (!isLoaded() || !m_showsPage) {
        return result;
    }

    const double spatium = m_score->style().spatium();
    for (const Page* page : m_score->pages()) {
        const muse::RectF rect = page->ldata()->bbox().translated(page->pos() - pictureRect().topLeft());
        result.emplace_back(rect.x() / spatium, rect.y() / spatium, rect.width() / spatium, rect.height() / spatium);
    }

    return result;
}

void EngravingPreviewRenderer::paint(muse::draw::Painter& painter, double pixelsPerStaffSpace,
                                     int deviceLogicalDpi) const
{
    if (!isLoaded() || m_score->pages().empty()) {
        return;
    }

    IScoreRenderer::PaintOptions options;
    options.isSetViewport = false;
    options.isPrinting = true;
    options.printPageBackground = false;
    options.fromPage = 0;
    options.deviceDpi = deviceLogicalDpi;
    if (m_showsPage) {
        //! NOTE Every page, whole, where the layout put it (beside the one before)
        options.toPage = -1;
        options.isMultiPage = true;
    } else {
        options.toPage = 0;
        options.trimMarginPixelSize = m_pictureMargin;
    }

    const double pixelsPerSpatium = pixelsPerStaffSpace / m_score->style().spatium();
    painter.save();
    painter.scale(pixelsPerSpatium, pixelsPerSpatium);
    if (m_showsPage) {
        painter.translate(-pictureRect().topLeft());
    }
    scoreRenderer()->paintScore(&painter, m_score.get(), options);
    painter.restore();
}

QImage EngravingPreviewRenderer::picture(double pixelsPerStaffSpace, double devicePixelRatio) const
{
    const muse::SizeF size = sizeInStaffSpaces() * pixelsPerStaffSpace * devicePixelRatio;
    if (size.width() <= 0.0 || size.height() <= 0.0) {
        return QImage();
    }

    QImage image(QSize(static_cast<int>(std::ceil(size.width())), static_cast<int>(std::ceil(size.height()))),
                 QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(devicePixelRatio);
    image.fill(Qt::transparent);

    {
        muse::draw::Painter painter(&image, "engravingpreviewrenderer");
        paint(painter, pixelsPerStaffSpace, image.logicalDpiX());
    }

    return image;
}

//---------------------------------------------------------
//   What the layout is made of
//---------------------------------------------------------

namespace {
struct ItemCollector {
    muse::PointF origin;
    std::vector<EngravingPreviewRenderer::Item>* items = nullptr;
};

//! NOTE Pixels of a staff space of the pictures which are compared
constexpr double COMPARED_PIXELS_PER_STAFF_SPACE = 8.0;

//! The part of two pictures, in staff spaces, in which they differ; nothing if they are the same
std::optional<muse::RectF> differingPart(const QImage& a, const QImage& b)
{
    if (a.size() != b.size()) {
        return muse::RectF(0.0, 0.0, a.width() / COMPARED_PIXELS_PER_STAFF_SPACE,
                           a.height() / COMPARED_PIXELS_PER_STAFF_SPACE);
    }

    QRect box;
    for (int y = 0; y < a.height(); ++y) {
        for (int x = 0; x < a.width(); ++x) {
            const QRgb p = a.pixel(x, y);
            const QRgb q = b.pixel(x, y);
            if (std::abs(qAlpha(p) - qAlpha(q)) > 16 || std::abs(qRed(p) - qRed(q)) > 16) {
                box = box.isNull() ? QRect(x, y, 1, 1) : box.united(QRect(x, y, 1, 1));
            }
        }
    }

    if (box.isNull()) {
        return std::nullopt;
    }

    return muse::RectF(box.x() / COMPARED_PIXELS_PER_STAFF_SPACE, box.y() / COMPARED_PIXELS_PER_STAFF_SPACE,
                       box.width() / COMPARED_PIXELS_PER_STAFF_SPACE, box.height() / COMPARED_PIXELS_PER_STAFF_SPACE);
}

muse::RectF inStaffSpaces(const muse::RectF& rect, const muse::PointF& origin, double spatium)
{
    return muse::RectF((rect.topLeft() - origin) / spatium, rect.size() / spatium);
}

int tickOf(const EngravingItem* item)
{
    const Fraction tick = item->tick();
    return tick.isValid() && !tick.negative() ? tick.ticks() : -1;
}

//! The four points of a slur or tie, in the same places as the box
std::vector<muse::PointF> curveOf(const EngravingItem* item, const muse::PointF& origin, double spatium)
{
    std::vector<muse::PointF> curve;
    if (!item->isSlurTieSegment()) {
        return curve;
    }

    const SlurTieSegment* segment = toSlurTieSegment(item);
    for (Grip grip : { Grip::START, Grip::BEZIER1, Grip::BEZIER2, Grip::END }) {
        curve.push_back((item->pagePos() + segment->ups(grip).pos() - origin) / spatium);
    }

    return curve;
}

void collectItem(void* data, EngravingItem* item)
{
    //! NOTE The page is not music, an item without a box (an empty key signature) has nothing to compare, and a
    //! mark which is not printed (a layout break, a spacer) is not in the picture
    if (item->isPage() || item->ldata()->bbox().isEmpty() || item->isLayoutBreak() || item->isSpacer()) {
        return;
    }

    const ItemCollector* collector = static_cast<ItemCollector*>(data);
    const double spatium = item->spatium();
    const muse::PointF origin = collector->origin;

    EngravingPreviewRenderer::Item result;
    result.type = item->type();
    result.track = item->track() == muse::nidx ? -1 : static_cast<int>(item->track());
    result.tick = tickOf(item);
    result.box = inStaffSpaces(item->ldata()->bbox().translated(item->pagePos()), origin, spatium);
    result.curve = curveOf(item, origin, spatium);

    collector->items->push_back(std::move(result));
}

bool comesBefore(const EngravingPreviewRenderer::Item& a, const EngravingPreviewRenderer::Item& b)
{
    return std::make_tuple(a.type, a.track, a.tick, a.box.top(), a.box.left())
           < std::make_tuple(b.type, b.track, b.tick, b.box.top(), b.box.left());
}

//! Whether two items, which have the same type, track and tick, differ by more than a hair
constexpr double SAME_WITHIN = 0.005;

bool differ(double a, double b)
{
    return std::abs(a - b) > SAME_WITHIN;
}

bool differ(const EngravingPreviewRenderer::Item& a, const EngravingPreviewRenderer::Item& b)
{
    if (differ(a.box.left(), b.box.left()) || differ(a.box.right(), b.box.right())
        || differ(a.box.top(), b.box.top()) || differ(a.box.bottom(), b.box.bottom())) {
        return true;
    }

    if (a.curve.size() != b.curve.size()) {
        return true;
    }

    for (size_t i = 0; i < a.curve.size(); ++i) {
        if (differ(a.curve[i].x(), b.curve[i].x()) || differ(a.curve[i].y(), b.curve[i].y())) {
            return true;
        }
    }

    return false;
}

bool kindComesBefore(const EngravingPreviewRenderer::Item& a, const EngravingPreviewRenderer::Item& b)
{
    return std::tie(a.type, a.track, a.tick) < std::tie(b.type, b.track, b.tick);
}

//! One edge of a box, and how far it moved
struct EdgeShift {
    double shift = 0.0;
    muse::PointF anchor;        // the middle of the edge in the layout as it is
    muse::PointF axis;          // (1, 0) or (0, 1)
};

std::optional<EdgeShift> largestEdgeShift(const EngravingPreviewRenderer::Item& was,
                                          const EngravingPreviewRenderer::Item& is)
{
    const muse::RectF& a = was.box;
    const muse::RectF& b = is.box;

    const EdgeShift edges[] = {
        { b.left() - a.left(), { a.left(), a.center().y() }, { 1.0, 0.0 } },
        { b.right() - a.right(), { a.right(), a.center().y() }, { 1.0, 0.0 } },
        { b.top() - a.top(), { a.center().x(), a.top() }, { 0.0, 1.0 } },
        { b.bottom() - a.bottom(), { a.center().x(), a.bottom() }, { 0.0, 1.0 } },
    };

    const EdgeShift* largest = std::max_element(std::begin(edges), std::end(edges),
                                                [](const EdgeShift& x, const EdgeShift& y) {
        return std::abs(x.shift) < std::abs(y.shift);
    });

    return differ(largest->shift, 0.0) ? std::optional<EdgeShift>(*largest) : std::nullopt;
}

//! Records what is different between the two layouts of an item, and keeps the edge which moved most
void noteChange(const EngravingPreviewRenderer::Item& was, const EngravingPreviewRenderer::Item& is,
                EngravingPreviewRenderer::Effect& effect, std::optional<EdgeShift>& largest)
{
    if (!differ(was, is)) {
        return;
    }

    effect.changed.push_back(was.box);

    const std::optional<EdgeShift> shift = largestEdgeShift(was, is);
    if (shift && (!largest || std::abs(shift->shift) > std::abs(largest->shift))) {
        largest = shift;
    }
}

//! NOTE How much the probe changes the value, in the units the value has; nothing when the change is not one a
//! handle can drag
std::optional<double> differenceOf(const PropertyValue& current, const PropertyValue& probe)
{
    const auto number = [](const PropertyValue& value) -> std::optional<double> {
        switch (value.type()) {
        case P_TYPE::SPATIUM: return value.value<Spatium>().val();
        case P_TYPE::REAL: return value.toReal();
        case P_TYPE::INT: return static_cast<double>(value.toInt());
        default: return std::nullopt;
        }
    };

    if (current.type() == P_TYPE::POINT && probe.type() == P_TYPE::POINT) {
        const muse::PointF a = current.value<muse::PointF>();
        const muse::PointF b = probe.value<muse::PointF>();
        const bool xDiffers = differ(a.x(), b.x());
        const bool yDiffers = differ(a.y(), b.y());
        if (xDiffers == yDiffers) {
            return std::nullopt;
        }

        return xDiffers ? b.x() - a.x() : b.y() - a.y();
    }

    const std::optional<double> a = number(current);
    const std::optional<double> b = number(probe);
    return a && b ? std::optional<double>(*b - *a) : std::nullopt;
}
}

std::vector<EngravingPreviewRenderer::Item> EngravingPreviewRenderer::items() const
{
    std::vector<Item> result;
    if (!isLoaded() || m_score->pages().empty()) {
        return result;
    }

    //! NOTE The items of every page shown, from the top left corner of the picture
    for (Page* page : m_score->pages()) {
        ItemCollector collector { pictureRect().topLeft() - page->pos(), &result };
        page->scanElements(&collector, collectItem, false);
        if (!m_showsPage) {
            break;
        }
    }

    std::stable_sort(result.begin(), result.end(), comesBefore);
    return result;
}

//! NOTE Whether an item is only carried along by the change: same size, moved horizontally only (the top and
//! the bottom did not move), and — for a curve — the same shape, so only moved by `dx`
static bool isCarriedAlong(const EngravingPreviewRenderer::Item& was, const EngravingPreviewRenderer::Item& is,
                           double dx)
{
    if (differ(was.box.width(), is.box.width()) || differ(was.box.height(), is.box.height())
        || differ(was.box.top(), is.box.top()) || differ(was.box.bottom(), is.box.bottom())) {
        return false;
    }

    if (was.curve.size() != is.curve.size()) {
        return false;
    }

    for (size_t i = 0; i < was.curve.size(); ++i) {
        if (differ(is.curve[i].x() - was.curve[i].x(), dx) || differ(is.curve[i].y(), was.curve[i].y())) {
            return false;
        }
    }

    return true;
}

//! NOTE The staff an item belongs to, from its track; items without a track form a group of their own
static int staffGroupOf(int track)
{
    return track < 0 ? -1 : track / static_cast<int>(VOICES);
}

EngravingPreviewRenderer::MarkedChanges EngravingPreviewRenderer::markedChanges(const std::vector<Item>& before,
                                                                              const std::vector<Item>& after)
{
    MarkedChanges result;
    if (before.empty() && after.empty()) {
        return result;
    }

    //! NOTE Items are paired in order within those of the same type, track and tick. An item which has no
    //! partner exists in one layout only, and counts as changed
    std::vector<std::pair<Item, Item> > pairs;
    //! NOTE An item without a partner in the other layout
    constexpr size_t NO_PAIR = static_cast<size_t>(-1);

    std::vector<size_t> pairOf(before.size(), NO_PAIR);   // of a before item into `pairs`
    std::vector<Item> unmatched;

    size_t i = 0;
    size_t j = 0;
    while (i < before.size() || j < after.size()) {
        if (j >= after.size() || (i < before.size() && kindComesBefore(before[i], after[j]))) {
            unmatched.push_back(before[i++]);
        } else if (i >= before.size() || kindComesBefore(after[j], before[i])) {
            unmatched.push_back(after[j++]);
        } else {
            pairOf[i] = pairs.size();
            pairs.push_back({ before[i], after[j] });
            ++i;
            ++j;
        }
    }

    //! NOTE The order in which the items follow each other on their staff, in the layout as it is:
    //! the predecessor of an item is the item nearest to its left
    std::vector<int> predecessorOf(before.size(), -1);
    {
        std::vector<std::vector<size_t> > groups;
        for (size_t b = 0; b < before.size(); ++b) {
            const size_t at = static_cast<size_t>(staffGroupOf(before[b].track) + 1);
            if (groups.size() <= at) {
                groups.resize(at + 1);
            }
            groups[at].push_back(b);
        }
        for (std::vector<size_t>& group : groups) {
            std::stable_sort(group.begin(), group.end(), [&before](size_t a, size_t b) {
                return before[a].box.left() < before[b].box.left();
            });
            for (size_t k = 1; k < group.size(); ++k) {
                predecessorOf[group[k]] = static_cast<int>(group[k - 1]);
            }
        }
    }

    //! NOTE An item which differs but is only carried along — the change moved what is to its left and it
    //! followed by as much — is not marked; the first item of such a run stays marked
    std::vector<bool> carried(pairs.size(), false);
    for (size_t b = 0; b < before.size(); ++b) {
        const size_t pairIndex = pairOf[b];
        if (pairIndex == NO_PAIR) {
            continue;
        }

        const Item& was = pairs[pairIndex].first;
        const Item& is = pairs[pairIndex].second;
        const double dx = is.box.left() - was.box.left();
        if (!differ(was, is) || !isCarriedAlong(was, is, dx)) {
            continue;
        }

        const int predecessor = predecessorOf[b];
        if (predecessor < 0 || pairOf[static_cast<size_t>(predecessor)] == NO_PAIR) {
            continue;
        }

        const std::pair<Item, Item>& neighbour = pairs[pairOf[static_cast<size_t>(predecessor)]];
        const double neighbourDx = neighbour.second.box.left() - neighbour.first.box.left();
        if (!differ(neighbourDx, dx)) {
            carried[pairIndex] = true;
        }
    }

    for (size_t p = 0; p < pairs.size(); ++p) {
        if (differ(pairs[p].first, pairs[p].second) && !carried[p]) {
            result.pairs.push_back(pairs[p]);
        }
    }

    //! NOTE The staff lines span the whole system, so a change which touches the music marks them too.
    //! They count only when nothing else is marked (an option of the staff lines themselves)
    const bool onlyStaffLinesMarked = std::all_of(unmatched.begin(), unmatched.end(), [](const Item& item) {
        return item.type == engraving::ElementType::STAFF_LINES;
    }) && std::all_of(result.pairs.begin(), result.pairs.end(), [](const std::pair<Item, Item>& pair) {
        return pair.first.type == engraving::ElementType::STAFF_LINES;
    });
    if (!onlyStaffLinesMarked) {
        result.pairs.erase(std::remove_if(result.pairs.begin(), result.pairs.end(),
                                         [](const std::pair<Item, Item>& pair) {
            return pair.first.type == engraving::ElementType::STAFF_LINES;
        }), result.pairs.end());
    }

    for (const Item& item : unmatched) {
        if (onlyStaffLinesMarked || item.type != engraving::ElementType::STAFF_LINES) {
            result.boxes.push_back(item.box);
        }
    }

    return result;
}

PropertyValue EngravingPreviewRenderer::valueOf(const MStyle& style, const std::vector<Override>& overrides,
                                              StyleId id)
{
    PropertyValue value = style.styleV(id);
    for (const Override& override : overrides) {
        if (override.id == id) {
            value = override.value;
        }
    }

    return value;
}

EngravingPreviewRenderer::Effect EngravingPreviewRenderer::effectOf(const MStyle& style,
                                                                    const std::vector<Override>& overrides, StyleId id,
                                                                    const PropertyValue& probe, double systemWidthSp)
{
    layout(style, overrides, systemWidthSp);
    const std::vector<Item> before = items();
    const muse::PointF originBefore = pictureRect().topLeft();
    const QImage pictureBefore = picture(COMPARED_PIXELS_PER_STAFF_SPACE);

    std::vector<Override> probed = overrides;
    probed.push_back({ id, probe });
    layout(style, probed, systemWidthSp);
    std::vector<Item> after = items();
    const muse::PointF originAfter = pictureRect().topLeft();
    const QImage pictureAfter = picture(COMPARED_PIXELS_PER_STAFF_SPACE);

    //! NOTE The picture is the one of the real values
    layout(style, overrides, systemWidthSp);

    //! NOTE The top or left edge of the picture may move with the probe: the origins of the two item
    //! lists differ then, and everything would seem to have moved. The items of the probed layout are
    //! brought into the frame of the layout as it is before they are compared
    const muse::PointF frameShift = (originAfter - originBefore) / m_score->style().spatium();
    for (Item& item : after) {
        item.box.translate(frameShift);
        for (muse::PointF& point : item.curve) {
            point += frameShift;
        }
    }

    const PropertyValue current = valueOf(style, overrides, id);

    Effect effect;
    std::optional<EdgeShift> largest;

    const MarkedChanges marked = markedChanges(before, after);
    for (const muse::RectF& box : marked.boxes) {
        effect.changed.push_back(box);
    }
    for (const std::pair<Item, Item>& pair : marked.pairs) {
        noteChange(pair.first, pair.second, effect, largest);
    }

    //! NOTE A value which only the painting uses (a line of dashes, a thickness) moves no box: it is the picture
    //! which differs
    if (effect.changed.empty()) {
        const std::optional<muse::RectF> differing = differingPart(pictureBefore, pictureAfter);
        if (differing) {
            effect.changed.push_back(*differing);
        }
    }

    const std::optional<double> valueChange = differenceOf(current, probe);
    if (largest && valueChange && differ(*valueChange, 0.0)) {
        effect.hasHandle = true;
        effect.anchor = largest->anchor;
        effect.staffSpacesPerUnit = std::abs(largest->shift / *valueChange);

        const double towards = (largest->shift > 0.0) == (*valueChange > 0.0) ? 1.0 : -1.0;
        effect.direction = largest->axis * towards;
    }

    return effect;
}

//! NOTE How much of a value is changed to see what it does, in the units of the value
static constexpr double PROBE_AMOUNT = 0.25;

std::optional<PropertyValue> EngravingPreviewRenderer::probeOf(const MStyle& style,
                                                               const std::vector<Override>& overrides,
                                                               StyleId id, double maximum)
{
    const PropertyValue current = valueOf(style, overrides, id);

    const auto step = [maximum](double value, double amount) {
        return value + amount <= maximum ? value + amount : value - amount;
    };

    switch (MStyle::valueType(id)) {
    case P_TYPE::BOOL:
        return PropertyValue(!current.toBool());
    case P_TYPE::INT:
        return PropertyValue(static_cast<int>(step(current.toInt(), 1.0)));
    case P_TYPE::REAL:
        return PropertyValue(step(current.toReal(), PROBE_AMOUNT));
    case P_TYPE::SPATIUM:
        return PropertyValue(Spatium(step(current.value<Spatium>().val(), PROBE_AMOUNT)));
    default:
        return std::nullopt;
    }
}
