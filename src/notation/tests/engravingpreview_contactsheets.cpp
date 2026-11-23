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
#include <QImage>
#include <QPainter>
#include <QRegularExpression>
#include <QString>

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
        if (text.startsWith('"')) {
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
            //! NOTE A choice whose values come from the model (not a literal `choices` list) is not
            //! probeable from the source: as the pane and the effect test do, it is left unprobed
            return std::nullopt;
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

    return EngravingPreviewRenderer::probeOf(style, overrides, id, dragMaximumOf(option));
}

//! One option on a sheet: its picture, marked as the pane marks it
struct Cell {
    QString sid;
    QImage image;
    bool probed = true;
    bool hasEffect = true;
};

Cell cellOf(const QString& sid, const QString& music, const MStyle& style,
            const std::vector<EngravingPreviewRenderer::Override>& overrides,
            const std::optional<PropertyValue>& probe, double systemWidth)
{
    Cell cell;
    cell.sid = sid;

    EngravingPreviewRenderer renderer(nullptr);
    if (!renderer.setMusic(music)) {
        return cell;
    }

    const StyleId id = MStyle::styleIdx(muse::String::fromQString(sid));
    EngravingPreviewRenderer::Effect effect;
    if (probe) {
        effect = renderer.effectOf(style, overrides, id, *probe, systemWidth);
    } else {
        renderer.layout(style, overrides, systemWidth);
    }

    const QImage picture = renderer.picture(PIXELS_PER_STAFF_SPACE);
    cell.probed = probe.has_value();
    cell.hasEffect = !effect.isEmpty();

    //! NOTE As the pane paints it: the washes behind the music, the handle and the control points over it
    cell.image = QImage(picture.size() + QSize(2 * CELL_PADDING, 2 * CELL_PADDING),
                        QImage::Format_ARGB32_Premultiplied);
    cell.image.fill(Qt::white);

    PreviewOverlay overlay;
    overlay.accent = QColor(0x4A, 0x7D, 0xD9);
    const QPointF offset(CELL_PADDING, CELL_PADDING);
    const double growth = 0.15 * PIXELS_PER_STAFF_SPACE;
    for (const muse::RectF& box : effect.changed) {
        const QRectF rect(box.left() * PIXELS_PER_STAFF_SPACE + offset.x(),
                          box.top() * PIXELS_PER_STAFF_SPACE + offset.y(),
                          box.width() * PIXELS_PER_STAFF_SPACE, box.height() * PIXELS_PER_STAFF_SPACE);
        overlay.washes.push_back(rect.adjusted(-growth, -growth, growth, growth));
    }
    if (effect.hasHandle) {
        overlay.handle = PreviewOverlay::Handle {
            offset + QPointF(effect.anchor.x(), effect.anchor.y()) * PIXELS_PER_STAFF_SPACE,
            QPointF(effect.direction.x(), effect.direction.y())
        };
    }

    QPainter painter(&cell.image);
    overlay.paintBelow(&painter);
    painter.drawImage(offset, picture);
    overlay.paintAbove(&painter);

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

    for (const QString& fileName : QDir(pagesDir).entryList({ "*Page.qml" }, QDir::Files, QDir::Name)) {
        const SourceBlock page = readPageSource(pagesDir + fileName);
        if (page.type != "EoPage") {
            continue;
        }

        const QString code = SourceBlock::unquoted(page.property("code").value_or(QString()));
        std::vector<Cell> cells;

        for (const SourceBlock& section : page.children) {
            for (const auto& entry : optionsOf(section)) {
                const SourceBlock& option = *entry.first;
                const SourceBlock* grid = entry.second;
                const QString sid = option.sid();
                const auto [music, systemWidth] = musicInfoOf(option, grid, section, page);
                if (sid.isEmpty() || music.isEmpty()) {
                    continue;
                }

                const StyleId id = MStyle::styleIdx(muse::String::fromQString(sid));
                if (id == StyleId::NOSTYLE) {
                    continue;
                }

                const std::vector<EngravingPreviewRenderer::Override> overrides
                    = musicStyleOf(option, grid, section, page);
                cells.push_back(cellOf(sid, music, style, overrides, probeOf(option, style, overrides, id),
                                     systemWidth));
            }
        }

        writeSheets(dir, code.isEmpty() ? fileName.chopped(4) : code, cells);
    }

    MScore::useRead302InTestMode = useRead302;
}
