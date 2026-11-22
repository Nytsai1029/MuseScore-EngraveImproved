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

#include <map>
#include <set>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

#include "engraving/style/style.h"

using namespace mu::engraving;

//! NOTE The pages of the Engraving Options dialog are QML descriptions; these tests read their sources as text
//! (the directories come from the CMakeLists of the tests)
namespace {
const QString OPTIONS_DIR = QString::fromUtf8(ENGRAVING_OPTIONS_QML_DIR);
const QString DIALOG_FILE = QString::fromUtf8(ENGRAVING_OPTIONS_DIALOG_FILE);
const QString QRC_FILE = QString::fromUtf8(ENGRAVING_OPTIONS_QRC_FILE);

QString readText(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        ADD_FAILURE() << "cannot read " << path.toStdString();
        return QString();
    }

    return QString::fromUtf8(file.readAll());
}

QStringList pageFiles()
{
    return QDir(OPTIONS_DIR + "/pages").entryList({ "*.qml" }, QDir::Files, QDir::Name);
}

QStringList allMatches(const QString& text, const QRegularExpression& re, int group = 1)
{
    QStringList result;
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while (it.hasNext()) {
        result << it.next().captured(group);
    }

    return result;
}

//! The quoted names in an `extraSids: [ "a", "b" ]`
QStringList extraSidsOf(const QString& text)
{
    static const QRegularExpression ARRAY(R"(extraSids:\s*\[([^\]]*)\])");
    static const QRegularExpression QUOTED("\"([^\"]*)\"");

    QStringList result;
    for (const QString& array : allMatches(text, ARRAY)) {
        result << allMatches(array, QUOTED);
    }

    return result;
}

QStringList sidsOptionsClaim(const QString& text)
{
    static const QRegularExpression SID(R"re(\bsid:\s*"([^"]+)")re");
    return allMatches(text, SID) + extraSidsOf(text);
}

//! The paths in the qrc, relative to the directory of the qrc
QStringList qrcFiles()
{
    static const QRegularExpression FILE_TAG(R"(<file[^>]*>([^<]+)</file>)");
    return allMatches(readText(QRC_FILE), FILE_TAG);
}

QString qrcDir()
{
    return QFileInfo(QRC_FILE).absolutePath();
}

//! The sources of the dialog which the application packages, as paths relative to the directory of the qrc
QStringList sourcesOfTheDialog()
{
    QStringList result;
    const QDir root(qrcDir());

    QDirIterator it(OPTIONS_DIR, { "*.qml", "*.js", "*.mscx" }, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        result << root.relativeFilePath(it.next());
    }

    result << root.relativeFilePath(DIALOG_FILE);
    result.sort();
    return result;
}
}

class Notation_EngravingOptionsPagesTests : public ::testing::Test
{
};

/**
 * @TEST_CASE everyOptionNamesAStyleValue
 * @details Every `sid`, every name in `extraSids` and every `page.value("x")` of a page is a style value
 */
TEST_F(Notation_EngravingOptionsPagesTests, everyOptionNamesAStyleValue)
{
    const QStringList files = pageFiles();
    ASSERT_FALSE(files.isEmpty());

    static const QRegularExpression PAGE_VALUE(R"re(page\.value\("([^"]+)"\))re");

    size_t names = 0;
    for (const QString& file : files) {
        const QString text = readText(OPTIONS_DIR + "/pages/" + file);

        QStringList used = sidsOptionsClaim(text);
        used << allMatches(text, PAGE_VALUE);

        for (const QString& name : used) {
            ++names;
            EXPECT_NE(MStyle::styleIdx(muse::String::fromQString(name)), Sid::NOSTYLE)
                << file.toStdString() << ": " << name.toStdString() << " is not a style value";
        }
    }

    EXPECT_GT(names, 500u);
}

/**
 * @TEST_CASE noStyleValueIsOnTwoPages
 * @details A style value is the `sid` or in the `extraSids` of one option at most
 */
TEST_F(Notation_EngravingOptionsPagesTests, noStyleValueIsOnTwoPages)
{
    std::map<QString, QString> places;

    for (const QString& file : pageFiles()) {
        const QString text = readText(OPTIONS_DIR + "/pages/" + file);

        std::map<QString, int> inThisPage;
        for (const QString& name : sidsOptionsClaim(text)) {
            ++inThisPage[name];
        }

        for (const auto& [name, count] : inThisPage) {
            EXPECT_EQ(count, 1) << name.toStdString() << " is claimed " << count << " times in " << file.toStdString();

            auto found = places.find(name);
            if (found != places.end()) {
                ADD_FAILURE() << name.toStdString() << " is on " << found->second.toStdString() << " and on "
                              << file.toStdString();
            } else {
                places.emplace(name, file);
            }
        }
    }

    EXPECT_FALSE(places.empty());
}

/**
 * @TEST_CASE everySnippetExists
 * @details Every `snippet: "x"` of a page is a file of the snippets
 */
TEST_F(Notation_EngravingOptionsPagesTests, everySnippetExists)
{
    static const QRegularExpression SNIPPET(R"re(\bsnippet:\s*"([^"]+)")re");

    size_t snippets = 0;
    for (const QString& file : pageFiles()) {
        for (const QString& snippet : allMatches(readText(OPTIONS_DIR + "/pages/" + file), SNIPPET)) {
            ++snippets;
            EXPECT_TRUE(QFileInfo::exists(OPTIONS_DIR + "/snippets/" + snippet + ".mscx"))
                << file.toStdString() << ": no snippet " << snippet.toStdString();
        }
    }

    EXPECT_GT(snippets, 0u);
}

/**
 * @TEST_CASE everyFileIsPackaged
 * @details What the dialog is made of is in the qrc and what the qrc has of it is there, and every page is in
 *          the list of the pages
 */
TEST_F(Notation_EngravingOptionsPagesTests, everyFileIsPackaged)
{
    const QStringList packaged = qrcFiles();
    const std::set<QString> inQrc(packaged.begin(), packaged.end());

    for (const QString& source : sourcesOfTheDialog()) {
        EXPECT_TRUE(inQrc.count(source)) << source.toStdString() << " is not in the qrc";
    }

    const QStringList sources = sourcesOfTheDialog();
    const std::set<QString> sourceSet(sources.begin(), sources.end());
    for (const QString& path : packaged) {
        if (!path.contains("EngravingOptions")) {
            continue;
        }

        EXPECT_TRUE(QFileInfo::exists(qrcDir() + "/" + path)) << path.toStdString() << " of the qrc does not exist";
        EXPECT_TRUE(sourceSet.count(path)) << path.toStdString() << " is in the qrc, but is not a source of the dialog";
    }

    const QString list = readText(OPTIONS_DIR + "/EoPages.qml");
    for (const QString& file : pageFiles()) {
        const QString type = QFileInfo(file).completeBaseName();
        EXPECT_TRUE(list.contains(QRegularExpression("\\b" + type + R"(\s*\{)")))
            << type.toStdString() << " is not in EoPages.qml";
    }
}

/**
 * @TEST_CASE pageCodesAreUnique
 * @details The code of a page is its own, and no code of the style dialog stands for two pages
 */
TEST_F(Notation_EngravingOptionsPagesTests, pageCodesAreUnique)
{
    static const QRegularExpression CODE(R"re(^\s*code:\s*"([^"]+)")re", QRegularExpression::MultilineOption);
    static const QRegularExpression LEGACY(R"(legacyCodes:\s*\[([^\]]*)\])");
    static const QRegularExpression QUOTED("\"([^\"]*)\"");

    std::map<QString, QString> codes;
    std::map<QString, QString> legacyCodes;

    for (const QString& file : pageFiles()) {
        const QString text = readText(OPTIONS_DIR + "/pages/" + file);

        const QStringList own = allMatches(text, CODE);
        ASSERT_EQ(own.size(), 1) << file.toStdString() << " has no code, or more than one";
        EXPECT_FALSE(codes.count(own.front())) << own.front().toStdString() << " is the code of two pages";
        codes.emplace(own.front(), file);

        for (const QString& list : allMatches(text, LEGACY)) {
            for (const QString& legacy : allMatches(list, QUOTED)) {
                auto found = legacyCodes.find(legacy);
                EXPECT_TRUE(found == legacyCodes.end())
                    << legacy.toStdString() << " is claimed by " << file.toStdString() << " and by "
                    << (found != legacyCodes.end() ? found->second.toStdString() : std::string());
                legacyCodes.emplace(legacy, file);
            }
        }
    }

    //! NOTE A code which opens one page must not be the own code of another
    for (const auto& [legacy, file] : legacyCodes) {
        auto found = codes.find(legacy);
        if (found != codes.end()) {
            EXPECT_EQ(found->second, file) << legacy.toStdString() << " is a legacy code of " << file.toStdString()
                                           << " and the code of " << found->second.toStdString();
        }
    }
}
