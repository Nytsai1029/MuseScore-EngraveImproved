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
//! NOTE A review sheet of every preview of the Engraving Options dialog: for every page with music and every
//! option with a sid, the picture the pane shows (washes and handle included), one sheet per page so that all
//! of them can be looked over at once. Not a test of the suite: it is run on demand:
//!   EO_CONTACT_SHEET_DIR=/abs/dir notation_tests --gtest_also_run_disabled_tests --gtest_filter='*ContactSheets'
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

#include <QDir>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mscore.h"
#include "engraving/style/style.h"
#include "engraving/types/types.h"

#include "notation/view/engravingoptions/engravingpreviewrenderer.h"
#include "notation/view/engravingoptions/previewoverlay.h"

#include "view/styledialog/styleuivalue.h"

#include "engravingoptionspagesource.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace mu::notation::tests;

namespace {
//! NOTE The size of the pictures on a sheet, and the room they are given
constexpr double PIXELS_PER_STAFF_SPACE = 14.0;
//! a whole page is smaller, as the pane fits it to its width
constexpr double PIXELS_PER_STAFF_SPACE_OF_A_PAGE = 4.0;
constexpr int CELL_PADDING = 10;
constexpr int CAPTION_HEIGHT = 18;
constexpr int CAPTION_GAP = 4;
constexpr int SHEET_WIDTH = 1500;
constexpr int SHEET_HEIGHT = 1900;

//! NOTE The style which a score gets when nothing is set, as the preview takes it from the current score
MStyle baseStyle()
{
    std::unique_ptr<MasterScore> score(compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr));
    return score->style();
}

//! The style values which the music of an option sets: those of the page, the section and the option,
//! the nearer one wins
std::vector<EngravingPreviewRenderer::Override> musicStyleOf(const SourceBlock& option, const SourceBlock* grid,
                                                             const SourceBlock& section, const SourceBlock& page)
{
    std::map<QString, QString> merged;
    for (const SourceBlock* block : { &page, &section, grid, &option }) {
        if (!block) {
            continue;
        }
        for (const auto& [name, value] : block->musicStyle()) {
            merged[name] = value;
        }
    }

    std::vector<EngravingPreviewRenderer::Override> overrides;
    for (const auto& [name, text] : merged) {
        const StyleId id = MStyle::styleIdx(muse::String::fromQString(name));
        if (id == StyleId::NOSTYLE) {
            continue;
        }

        const QVariant plain = text == "true" || text == "false" ? QVariant(text == "true")
                               : text.startsWith('"') ? QVariant(SourceBlock::unquoted(text))
                               : text.contains('.') ? QVariant(text.toDouble()) : QVariant(text.toInt());
        overrides.push_back({ id, styleValueFromUi(id, plain) });
    }

    return overrides;
}

//! The values the choices of an option take, in their order (of `choices: [{ value: x, … }, …]`)
QVariantList choiceValuesOf(const SourceBlock& option)
{
    QVariantList values;
    for (const QString& text : option.choiceValues()) {
        if (text == "true" || text == "false") {
            values << (text == "true");
        } else if (text.startsWith('"')) {
            values << SourceBlock::unquoted(text);
        } else if (text.contains('.')) {
            values << text.toDouble();
        } else {
            values << text.toInt();
        }
    }

    return values;
}

//! What the option is shown with: the automatic probe, or — of a choice — the choice after the current one,
//! as the pane does
std::optional<PropertyValue> probeOf(const SourceBlock& option, const MStyle& style,
                                     const std::vector<EngravingPreviewRenderer::Override>& overrides, StyleId id)
{
    if (option.type == "EoChoice") {
        const QVariantList values = choiceValuesOf(option);
        if (values.size() < 2) {
            return EngravingPreviewRenderer::probeOf(style, overrides, id, dragMaximumOf(option));
        }

        const QVariant current = styleValueToUi(id, EngravingPreviewRenderer::valueOf(style, overrides, id));
        int index = 0;
        for (int i = 0; i < values.size(); ++i) {
            if (values.at(i) == current) {
                index = i;
                break;
            }
        }
        return styleValueFromUi(id, values.at((index + 1) % values.size()));
    }

    if (option.type == "EoText") {
        //! NOTE As EoMusic.probeValueOf: the text with a sample added
        const QVariant current = styleValueToUi(id, EngravingPreviewRenderer::valueOf(style, overrides, id));
        return styleValueFromUi(id, current.toString() + "Text");
    }

    return EngravingPreviewRenderer::probeOf(style, overrides, id, dragMaximumOf(option));
}

//! One option on a sheet: its picture, marked as the pane marks it
struct Cell {
    QString sid;
    QImage image;
    bool probed = true;
    bool hasEffect = true;
};

Cell cellOf(const QString& sid, const MusicInfo& music, const MStyle& style,
            const std::vector<EngravingPreviewRenderer::Override>& overrides,
            const std::optional<PropertyValue>& probe)
{
    Cell cell;
    cell.sid = sid;

    EngravingPreviewRenderer renderer(nullptr);
    if (!renderer.setMusic(music.music)) {
        return cell;
    }
    renderer.setShowsPage(music.showsPage);

    const StyleId id = MStyle::styleIdx(muse::String::fromQString(sid));
    EngravingPreviewRenderer::Effect effect;
    if (probe) {
        effect = renderer.effectOf(style, overrides, id, *probe, music.systemWidth);
    } else {
        renderer.layout(style, overrides, music.systemWidth, id);
    }

    const double pixels = music.showsPage ? PIXELS_PER_STAFF_SPACE_OF_A_PAGE : PIXELS_PER_STAFF_SPACE;
    const QImage picture = renderer.picture(pixels);
    cell.probed = probe.has_value();
    cell.hasEffect = !effect.isEmpty();

    const QColor accent(0x4A, 0x7D, 0xD9);
    PreviewOverlay::Ghost ghost;
    if (probe && !effect.isEmpty() && !effect.afterChanged.empty()) {
        std::vector<EngravingPreviewRenderer::Override> probed = overrides;
        probed.push_back({ id, *probe });
        renderer.layout(style, probed, music.systemWidth, id);
        const QImage probedPicture = renderer.picture(pixels);

        std::vector<QRectF> afterRegions;
        afterRegions.reserve(effect.afterChanged.size());
        for (const muse::RectF& box : effect.afterChanged) {
            afterRegions.emplace_back(box.left() * pixels, box.top() * pixels,
                                      box.width() * pixels, box.height() * pixels);
        }
        const QPointF shift(effect.probeFrameShift.x() * pixels, effect.probeFrameShift.y() * pixels);
        ghost = PreviewOverlay::probedGhost(picture, probedPicture, shift, afterRegions, pixels, accent);
    }

    //! NOTE As the pane paints it: the washes and probed ghost behind the music, the handle over it
    QRectF bounds(QPointF(0.0, 0.0), picture.size());
    if (!ghost.isNull()) {
        bounds = bounds.united(QRectF(ghost.offset, ghost.image.deviceIndependentSize()));
    }
    const int padLeft = CELL_PADDING + static_cast<int>(std::ceil(std::max(0.0, -bounds.left())));
    const int padTop = CELL_PADDING + static_cast<int>(std::ceil(std::max(0.0, -bounds.top())));
    const int padRight
        = CELL_PADDING + static_cast<int>(std::ceil(std::max(0.0, bounds.right() - picture.width())));
    const int padBottom
        = CELL_PADDING + static_cast<int>(std::ceil(std::max(0.0, bounds.bottom() - picture.height())));

    cell.image = QImage(picture.width() + padLeft + padRight, picture.height() + padTop + padBottom,
                        QImage::Format_ARGB32_Premultiplied);
    cell.image.fill(Qt::white);

    PreviewOverlay overlay;
    overlay.accent = accent;
    const QPointF offset(padLeft, padTop);
    for (const muse::RectF& box : effect.changed) {
        const QRectF rect(box.left() * pixels + offset.x(), box.top() * pixels + offset.y(),
                          box.width() * pixels, box.height() * pixels);
        overlay.washes.push_back(PreviewOverlay::washRectFor(rect, pixels));
    }
    if (effect.hasHandle) {
        overlay.handle = PreviewOverlay::Handle {
            offset + QPointF(effect.anchor.x(), effect.anchor.y()) * pixels,
            QPointF(effect.direction.x(), effect.direction.y())
        };
    }

    QPainter painter(&cell.image);
    overlay.paintBelow(&painter);
    if (!ghost.isNull()) {
        painter.drawImage(offset + ghost.offset, ghost.image);
    }
    painter.drawImage(offset, picture);
    overlay.paintAbove(&painter);
    if (music.showsPage) {
        painter.setPen(QColor(0, 0, 0, 56));
        painter.drawRect(QRectF(offset, picture.size()).adjusted(-0.5, -0.5, 0.5, 0.5));
    }

    return cell;
}

void writeSheets(const QDir& dir, const QString& pageCode, const std::vector<Cell>& cells)
{
    if (cells.empty()) {
        return;
    }

    int widest = 0;
    for (const Cell& cell : cells) {
        widest = std::max(widest, cell.image.width());
    }
    const int columnWidth = std::max(widest, 200);

    int page = 0;
    int x = 0;
    int y = 0;
    int rowHeight = 0;
    QImage sheet;
    QPainter painter;

    const auto beginSheet = [&]() {
        sheet = QImage(SHEET_WIDTH, SHEET_HEIGHT, QImage::Format_ARGB32_Premultiplied);
        sheet.fill(QColor(0xF0, 0xF0, 0xF0));
        painter.begin(&sheet);
        x = 0;
        y = 0;
        rowHeight = 0;
    };

    const auto endSheet = [&]() {
        painter.end();
        const QImage trimmed = sheet.copy(0, 0, SHEET_WIDTH, std::min(y + rowHeight + CELL_PADDING, SHEET_HEIGHT));
        trimmed.save(dir.filePath(QString("%1-%2.png").arg(pageCode).arg(++page, 2, 10, QLatin1Char('0'))));
    };

    beginSheet();
    for (const Cell& cell : cells) {
        const int cellHeight = cell.image.height() + CAPTION_GAP + CAPTION_HEIGHT + CELL_PADDING;
        if (x + columnWidth > SHEET_WIDTH) {
            x = 0;
            y += rowHeight;
            rowHeight = 0;
        }
        if (y + cellHeight > SHEET_HEIGHT - CELL_PADDING) {
            endSheet();
            beginSheet();
        }

        painter.drawImage(x + (columnWidth - cell.image.width()) / 2, y + CELL_PADDING, cell.image);

        painter.setPen(cell.probed && !cell.hasEffect ? QColor(0xD9, 0x3A, 0x3A) : QColor(0x33, 0x33, 0x33));
        painter.drawText(QRect(x, y + CELL_PADDING + cell.image.height() + CAPTION_GAP, columnWidth, CAPTION_HEIGHT),
                         Qt::AlignHCenter,
                         cell.probed && !cell.hasEffect ? cell.sid + " — NO EFFECT" : cell.sid);

        x += columnWidth;
        rowHeight = std::max(rowHeight, cellHeight);
    }
    endSheet();
}
}

/**
 * @TEST_CASE DISABLED_ContactSheets
 * @details Not a check: writes the preview pictures of every option of every page with music into
 *          EO_CONTACT_SHEET_DIR, a sheet of at most 1500×1900 per page at a time
 */
TEST(Notation_EngravingPreviewContactSheets, DISABLED_ContactSheets)
{
    const QString target = qEnvironmentVariable("EO_CONTACT_SHEET_DIR");
    if (target.isEmpty()) {
        GTEST_SKIP() << "EO_CONTACT_SHEET_DIR is not set";
    }

    const bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    QDir dir(target);
    ASSERT_TRUE(dir.mkpath("."));

    const MStyle style = baseStyle();
    const QString pagesDir = QString::fromUtf8(ENGRAVING_OPTIONS_QML_DIR) + "/pages/";

    const QString onlyPage = qEnvironmentVariable("EO_CONTACT_SHEET_PAGE");

    for (const QString& fileName : QDir(pagesDir).entryList({ "*Page.qml" }, QDir::Files, QDir::Name)) {
        const SourceBlock page = readPageSource(pagesDir + fileName);
        if (page.type != "EoPage") {
            continue;
        }

        const QString code = SourceBlock::unquoted(page.property("code").value_or(QString()));
        if (!onlyPage.isEmpty() && code != onlyPage) {
            continue;
        }
        std::vector<Cell> cells;

        for (const SourceBlock& section : page.children) {
            for (const auto& entry : optionsOf(section)) {
                const SourceBlock& option = *entry.first;
                const SourceBlock* grid = entry.second;
                const QString sid = option.sid();
                const MusicInfo music = musicInfoOf(option, grid, section, page);
                if (sid.isEmpty() || music.music.isEmpty()) {
                    continue;
                }

                const StyleId id = MStyle::styleIdx(muse::String::fromQString(sid));
                if (id == StyleId::NOSTYLE) {
                    continue;
                }

                const std::vector<EngravingPreviewRenderer::Override> overrides
                    = musicStyleOf(option, grid, section, page);
                cells.push_back(cellOf(sid, music, style, overrides, probeOf(option, style, overrides, id)));
            }
        }

        writeSheets(dir, code.isEmpty() ? fileName.chopped(4) : code, cells);
    }

    MScore::useRead302InTestMode = useRead302;
}

namespace {
QString formatUiValue(const QVariant& value)
{
    if (value.typeId() == QMetaType::QPointF) {
        const QPointF p = value.toPointF();
        return QString("%1,%2").arg(p.x(), 0, 'f', 2).arg(p.y(), 0, 'f', 2);
    }
    if (value.typeId() == QMetaType::QSizeF) {
        const QSizeF s = value.toSizeF();
        return QString("%1x%2").arg(s.width(), 0, 'f', 2).arg(s.height(), 0, 'f', 2);
    }
    return value.toString().replace('\t', ' ').replace('\n', ' ');
}

struct AuditMetrics {
    double widthSp = 0.0;
    double heightSp = 0.0;
    bool hasEffect = false;
    bool hasHandle = false;
    int diffPixels = 0;
    int inkPixels = 0;
    double maxShiftSp = 0.0;
    int washCount = 0;
    double washAreaSp2 = 0.0;
    double washFraction = 0.0;
    int unmatchedBefore = 0;
    int unmatchedAfter = 0;
    int pairsChanged = 0;
    bool pixelFallback = false;
    bool sizeChanged = false;
};

AuditMetrics measureOption(const MusicInfo& music, const MStyle& style,
                           const std::vector<EngravingPreviewRenderer::Override>& overrides,
                           StyleId id, const PropertyValue& probe)
{
    AuditMetrics m;
    EngravingPreviewRenderer renderer(nullptr);
    if (!renderer.setMusic(music.music)) {
        return m;
    }
    renderer.setShowsPage(music.showsPage);

    const double pixels = music.showsPage ? PIXELS_PER_STAFF_SPACE_OF_A_PAGE : PIXELS_PER_STAFF_SPACE;

    renderer.layout(style, overrides, music.systemWidth, id);
    const std::vector<EngravingPreviewRenderer::Item> before = renderer.items();
    const muse::SizeF sizeBefore = renderer.sizeInStaffSpaces();
    m.widthSp = sizeBefore.width();
    m.heightSp = sizeBefore.height();
    const QImage picBefore = renderer.picture(pixels);

    //! NOTE Measure exact effect as the pane computes it
    const EngravingPreviewRenderer::Effect effect
        = renderer.effectOf(style, overrides, id, probe, music.systemWidth);
    m.hasEffect = !effect.isEmpty();
    m.hasHandle = effect.hasHandle;
    m.washCount = static_cast<int>(effect.changed.size());

    //! NOTE Re-layout with probe to compare items and aligned images
    std::vector<EngravingPreviewRenderer::Override> probed = overrides;
    probed.push_back({ id, probe });
    renderer.layout(style, probed, music.systemWidth, id);
    std::vector<EngravingPreviewRenderer::Item> after = renderer.items();
    const QImage picAfter = renderer.picture(pixels);
    m.sizeChanged = (picBefore.size() != picAfter.size());

    //! NOTE Align after-items into before-frame using the first unchanged anchor (clef/staff) or top-left
    muse::PointF frameShift;
    if (!before.empty() && !after.empty()) {
        //! NOTE Find frame shift via first matching CLEF or STAFF_LINES if present; fallback (0,0)
        for (const auto& bItem : before) {
            if (bItem.type != ElementType::STAFF_LINES && bItem.type != ElementType::CLEF) {
                continue;
            }
            for (const auto& aItem : after) {
                if (aItem.type == bItem.type && aItem.track == bItem.track && aItem.tick == bItem.tick) {
                    frameShift = bItem.box.topLeft() - aItem.box.topLeft();
                    break;
                }
            }
            if (frameShift.x() != 0.0 || frameShift.y() != 0.0) {
                break;
            }
        }
    }

    //! NOTE Count unmatched items and max shift from markedChanges on aligned items
    std::vector<EngravingPreviewRenderer::Item> alignedAfter = after;
    for (auto& item : alignedAfter) {
        item.box.translate(frameShift);
        for (auto& pt : item.curve) {
            pt += frameShift;
        }
    }

    const auto kindLess = [](const EngravingPreviewRenderer::Item& a, const EngravingPreviewRenderer::Item& b) {
        return std::tie(a.type, a.track, a.tick) < std::tie(b.type, b.track, b.tick);
    };
    size_t i = 0;
    size_t j = 0;
    while (i < before.size() || j < alignedAfter.size()) {
        if (j >= alignedAfter.size() || (i < before.size() && kindLess(before[i], alignedAfter[j]))) {
            if (before[i].type != ElementType::STAFF_LINES) {
                ++m.unmatchedBefore;
            }
            ++i;
        } else if (i >= before.size() || kindLess(alignedAfter[j], before[i])) {
            if (alignedAfter[j].type != ElementType::STAFF_LINES) {
                ++m.unmatchedAfter;
            }
            ++j;
        } else {
            ++i;
            ++j;
        }
    }

    const EngravingPreviewRenderer::MarkedChanges marked
        = EngravingPreviewRenderer::markedChanges(before, alignedAfter);
    m.pairsChanged = static_cast<int>(marked.pairs.size());
    m.pixelFallback = m.hasEffect && marked.pairs.empty() && marked.boxes.empty();
    for (const auto& [was, is] : marked.pairs) {
        m.maxShiftSp = std::max(m.maxShiftSp, std::abs(is.box.left() - was.box.left()));
        m.maxShiftSp = std::max(m.maxShiftSp, std::abs(is.box.right() - was.box.right()));
        m.maxShiftSp = std::max(m.maxShiftSp, std::abs(is.box.top() - was.box.top()));
        m.maxShiftSp = std::max(m.maxShiftSp, std::abs(is.box.bottom() - was.box.bottom()));
        for (size_t k = 0; k < std::min(was.curve.size(), is.curve.size()); ++k) {
            const double dx = is.curve[k].x() - was.curve[k].x();
            const double dy = is.curve[k].y() - was.curve[k].y();
            m.maxShiftSp = std::max(m.maxShiftSp, std::hypot(dx, dy));
        }
    }

    //! NOTE Pixel diff aligned by frameShift (falls back to (0,0) when dimensions match and shift is 0)
    const int shiftX = static_cast<int>(std::lround(frameShift.x() * pixels));
    const int shiftY = static_cast<int>(std::lround(frameShift.y() * pixels));
    const int minX = std::min(0, shiftX);
    const int minY = std::min(0, shiftY);
    const int maxX = std::max(picBefore.width(), shiftX + picAfter.width());
    const int maxY = std::max(picBefore.height(), shiftY + picAfter.height());

    for (int y = minY; y < maxY; ++y) {
        const bool inB = y >= 0 && y < picBefore.height();
        const int ay = y - shiftY;
        const bool inA = ay >= 0 && ay < picAfter.height();
        const QRgb* rowB = inB ? reinterpret_cast<const QRgb*>(picBefore.constScanLine(y)) : nullptr;
        const QRgb* rowA = inA ? reinterpret_cast<const QRgb*>(picAfter.constScanLine(ay)) : nullptr;
        for (int x = minX; x < maxX; ++x) {
            const QRgb pB = (rowB && x >= 0 && x < picBefore.width()) ? rowB[x] : 0;
            const int ax = x - shiftX;
            const QRgb pA = (rowA && ax >= 0 && ax < picAfter.width()) ? rowA[ax] : 0;
            if (qAlpha(pB) > 16) {
                ++m.inkPixels;
            }
            if (std::abs(qAlpha(pB) - qAlpha(pA)) > 16 || std::abs(qRed(pB) - qRed(pA)) > 16) {
                ++m.diffPixels;
            }
        }
    }

    //! NOTE Compute exact union wash area and fraction on a 4 px/sp mask
    const int maskW = std::max(1, static_cast<int>(std::ceil(m.widthSp * 4.0)));
    const int maskH = std::max(1, static_cast<int>(std::ceil(m.heightSp * 4.0)));
    std::vector<uint8_t> mask(static_cast<size_t>(maskW * maskH), 0);
    for (const muse::RectF& box : effect.changed) {
        m.washAreaSp2 += box.width() * box.height();
        const int x0 = std::clamp(static_cast<int>(std::floor(box.left() * 4.0)), 0, maskW);
        const int x1 = std::clamp(static_cast<int>(std::ceil(box.right() * 4.0)), 0, maskW);
        const int y0 = std::clamp(static_cast<int>(std::floor(box.top() * 4.0)), 0, maskH);
        const int y1 = std::clamp(static_cast<int>(std::ceil(box.bottom() * 4.0)), 0, maskH);
        for (int y = y0; y < y1; ++y) {
            for (int x = x0; x < x1; ++x) {
                mask[static_cast<size_t>(y * maskW + x)] = 1;
            }
        }
    }
    int covered = 0;
    for (uint8_t v : mask) {
        covered += v;
    }
    m.washFraction = static_cast<double>(covered) / static_cast<double>(maskW * maskH);

    return m;
}
}

/**
 * @TEST_CASE DISABLED_ClarityAudit
 * @details Writes quantitative clarity metrics for every probed option across all pages to EO_CLARITY_REPORT
 */
TEST(Notation_EngravingPreviewContactSheets, DISABLED_ClarityAudit)
{
    const QString reportPath = qEnvironmentVariable("EO_CLARITY_REPORT");
    if (reportPath.isEmpty()) {
        GTEST_SKIP() << "EO_CLARITY_REPORT is not set";
    }

    const bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    QFile outFile(reportPath);
    ASSERT_TRUE(outFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
    QTextStream out(&outFile);
    out << "page\tsid\toptType\tvalType\tcurrent\tprobe\tshowsPage\twSp\thSp\thasEffect\thasHandle\t"
        << "diffPx\tinkPx\tdiffPct\tmaxShiftSp\twashCount\twashAreaSp2\twashFrac\t"
        << "unmatchedBefore\tunmatchedAfter\tpairsChanged\tpixelFallback\tsizeChanged\n";

    const MStyle style = baseStyle();
    const QString pagesDir = QString::fromUtf8(ENGRAVING_OPTIONS_QML_DIR) + "/pages/";
    const QString onlyPage = qEnvironmentVariable("EO_CONTACT_SHEET_PAGE");

    for (const QString& fileName : QDir(pagesDir).entryList({ "*Page.qml" }, QDir::Files, QDir::Name)) {
        const SourceBlock page = readPageSource(pagesDir + fileName);
        if (page.type != "EoPage") {
            continue;
        }

        const QString code = SourceBlock::unquoted(page.property("code").value_or(QString()));
        if (!onlyPage.isEmpty() && code != onlyPage) {
            continue;
        }

        for (const SourceBlock& section : page.children) {
            for (const auto& entry : optionsOf(section)) {
                const SourceBlock& option = *entry.first;
                const SourceBlock* grid = entry.second;
                const QString sid = option.sid();
                const MusicInfo music = musicInfoOf(option, grid, section, page);
                if (sid.isEmpty() || music.music.isEmpty()) {
                    continue;
                }

                const StyleId id = MStyle::styleIdx(muse::String::fromQString(sid));
                if (id == StyleId::NOSTYLE) {
                    continue;
                }

                const std::vector<EngravingPreviewRenderer::Override> overrides
                    = musicStyleOf(option, grid, section, page);
                const std::optional<PropertyValue> probe = probeOf(option, style, overrides, id);
                if (!probe) {
                    continue;
                }

                const AuditMetrics m = measureOption(music, style, overrides, id, *probe);
                const PropertyValue currentVal = EngravingPreviewRenderer::valueOf(style, overrides, id);
                const double diffPct = m.inkPixels > 0 ? (100.0 * m.diffPixels / m.inkPixels) : 0.0;

                out << code << '\t' << sid << '\t' << option.type << '\t'
                    << static_cast<int>(MStyle::valueType(id)) << '\t'
                    << formatUiValue(styleValueToUi(id, currentVal)) << '\t'
                    << formatUiValue(styleValueToUi(id, *probe)) << '\t'
                    << (music.showsPage ? 1 : 0) << '\t'
                    << QString::number(m.widthSp, 'f', 1) << '\t'
                    << QString::number(m.heightSp, 'f', 1) << '\t'
                    << (m.hasEffect ? 1 : 0) << '\t'
                    << (m.hasHandle ? 1 : 0) << '\t'
                    << m.diffPixels << '\t'
                    << m.inkPixels << '\t'
                    << QString::number(diffPct, 'f', 2) << '\t'
                    << QString::number(m.maxShiftSp, 'f', 3) << '\t'
                    << m.washCount << '\t'
                    << QString::number(m.washAreaSp2, 'f', 2) << '\t'
                    << QString::number(m.washFraction, 'f', 3) << '\t'
                    << m.unmatchedBefore << '\t'
                    << m.unmatchedAfter << '\t'
                    << m.pairsChanged << '\t'
                    << (m.pixelFallback ? 1 : 0) << '\t'
                    << (m.sizeChanged ? 1 : 0) << '\n';
            }
        }
    }

    MScore::useRead302InTestMode = useRead302;
}
