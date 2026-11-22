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

#include <gtest/gtest.h>

#include <cmath>
#include <string>
#include <vector>

#include <QDir>
#include <QImage>

#include "draw/painter.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/masterscore.h"
#include "engraving/style/style.h"
#include "engraving/types/types.h"

#include "notation/view/engravingoptions/engravingpreviewrenderer.h"

using namespace mu::notation;
using namespace mu::engraving;

namespace {
const QString SNIPPETS_DIR = QString::fromUtf8(ENGRAVING_OPTIONS_SNIPPETS_DIR);

//! NOTE The style which a score gets when nothing is set, as the preview takes it from the current score
MStyle baseStyle()
{
    std::unique_ptr<MasterScore> score(compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr));
    return score->style();
}

muse::io::path_t snippetPath(const QString& name)
{
    return muse::io::path_t(SNIPPETS_DIR.toStdString() + "/" + name.toStdString() + ".mscx");
}
}

class Notation_EngravingPreviewTests : public ::testing::Test
{
protected:
    //! A renderer which has loaded the snippet, and laid it out
    static std::unique_ptr<EngravingPreviewRenderer> laidOut(const QString& snippet, const MStyle& style,
                                                             const std::vector<EngravingPreviewRenderer::Override>& overrides = {},
                                                             double systemWidthSp = 0.0)
    {
        auto renderer = std::make_unique<EngravingPreviewRenderer>(nullptr);
        const muse::Ret ret = renderer->load(snippetPath(snippet));
        EXPECT_TRUE(ret) << snippet.toStdString() << ": " << ret.toString();
        if (!ret) {
            return nullptr;
        }

        renderer->layout(style, overrides, systemWidthSp);
        return renderer;
    }
};

/**
 * @TEST_CASE everySnippetIsLaidOut
 * @details Each snippet loads, and with the base style it is of a size a preview can show
 */
TEST_F(Notation_EngravingPreviewTests, everySnippetIsLaidOut)
{
    const QStringList files = QDir(SNIPPETS_DIR).entryList({ "*.mscx" }, QDir::Files, QDir::Name);
    ASSERT_FALSE(files.isEmpty());

    const MStyle style = baseStyle();

    for (const QString& file : files) {
        const QString name = file.chopped(5);
        auto renderer = laidOut(name, style);
        ASSERT_TRUE(renderer) << name.toStdString();

        const muse::SizeF size = renderer->sizeInStaffSpaces();
        EXPECT_GE(size.width(), 6.0) << name.toStdString();
        EXPECT_LE(size.width(), 200.0) << name.toStdString();
        EXPECT_GE(size.height(), 4.0) << name.toStdString();
        EXPECT_LE(size.height(), 40.0) << name.toStdString();
    }
}

/**
 * @TEST_CASE overrideChangesTheLayout
 * @details A value which the preview sets is the one the layout uses
 */
TEST_F(Notation_EngravingPreviewTests, overrideChangesTheLayout)
{
    const MStyle style = baseStyle();

    auto plain = laidOut("spacing", style);
    auto wide = laidOut("spacing", style, { { Sid::minMeasureWidth, Spatium(40) } });
    ASSERT_TRUE(plain);
    ASSERT_TRUE(wide);

    EXPECT_GT(wide->sizeInStaffSpaces().width(), plain->sizeInStaffSpaces().width());
    EXPECT_GE(wide->sizeInStaffSpaces().width(), 80.0);
}

/**
 * @TEST_CASE sizeDoesNotDependOnTheStaffSize
 * @details The size in staff spaces is the same whatever the size of the staff space is
 */
TEST_F(Notation_EngravingPreviewTests, sizeDoesNotDependOnTheStaffSize)
{
    const MStyle style = baseStyle();
    MStyle doubled = style;
    doubled.set(Sid::spatium, style.spatium() * 2.0);

    auto normal = laidOut("spacing", style);
    auto large = laidOut("spacing", doubled);
    ASSERT_TRUE(normal);
    ASSERT_TRUE(large);

    const muse::SizeF a = normal->sizeInStaffSpaces();
    const muse::SizeF b = large->sizeInStaffSpaces();
    EXPECT_NEAR(a.width(), b.width(), a.width() * 0.02);
    EXPECT_NEAR(a.height(), b.height(), a.height() * 0.02);
}

/**
 * @TEST_CASE givenSystemWidthIsNotExceeded
 * @details With a width given, the systems are not wider
 */
TEST_F(Notation_EngravingPreviewTests, givenSystemWidthIsNotExceeded)
{
    auto renderer = laidOut("spacing", baseStyle(), {}, 60.0);
    ASSERT_TRUE(renderer);

    EXPECT_LE(renderer->sizeInStaffSpaces().width(), 60.5);
    EXPECT_GT(renderer->sizeInStaffSpaces().width(), 0.0);
}

/**
 * @TEST_CASE paintsInk
 * @details Painting into an image of the size which the preview takes puts ink in it, and the image is as big as
 *          the snippet is
 */
TEST_F(Notation_EngravingPreviewTests, paintsInk)
{
    auto renderer = laidOut("spacing", baseStyle());
    ASSERT_TRUE(renderer);

    constexpr double PIXELS_PER_STAFF_SPACE = 7.0;
    const muse::SizeF size = renderer->sizeInStaffSpaces() * PIXELS_PER_STAFF_SPACE;
    const QSize imageSize(static_cast<int>(std::ceil(size.width())), static_cast<int>(std::ceil(size.height())));
    ASSERT_GT(imageSize.width(), 0);
    ASSERT_GT(imageSize.height(), 0);

    QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    {
        muse::draw::Painter painter(&image, "engravingpreview_tests");
        renderer->paint(painter, PIXELS_PER_STAFF_SPACE);
    }

    EXPECT_EQ(image.width(), imageSize.width());
    EXPECT_EQ(image.height(), imageSize.height());

    size_t inkPixels = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(image.pixel(x, y)) == 255) {
                ++inkPixels;
            }
        }
    }

    EXPECT_GT(inkPixels, 100u);
}
