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

#include <optional>
#include <vector>

#include <QFile>
#include <QMap>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

//! NOTE The pages of the Engraving Options dialog are QML descriptions: a page holds sections, a section holds
//! options. The tests read them as text, as a tree of the blocks `EoSomething { … }` with the lines of each block
//! which are its own
namespace mu::notation::tests {
struct SourceBlock {
    QString type;
    QStringList ownLines;
    std::vector<SourceBlock> children;

    //! What follows `name:` in a line of the block itself. A `[ … ]` array or `({ … })` object may span
    //! several lines; they are joined into the one expression
    std::optional<QString> property(const QString& name) const
    {
        const QRegularExpression line("^\\s*" + name + ":\\s*(.+?)\\s*$");
        for (int i = 0; i < ownLines.size(); ++i) {
            const QRegularExpressionMatch match = line.match(ownLines.at(i));
            if (!match.hasMatch()) {
                continue;
            }

            QString expression = match.captured(1);
            for (int j = i + 1; j < ownLines.size() && !balanced(expression); ++j) {
                expression += ' ' + ownLines.at(j).trimmed();
            }

            return expression;
        }

        return std::nullopt;
    }

    //! The style values of `musicStyle: ({ name: value, … })` of the block, as text, by name
    std::vector<std::pair<QString, QString> > musicStyle() const
    {
        static const QRegularExpression PAIR("(\\w+)\\s*:\\s*(true|false|-?[0-9.]+|\"(?:[^\"\\\\]|\\\\.)*\")");

        std::vector<std::pair<QString, QString> > result;
        const std::optional<QString> expression = property("musicStyle");
        if (!expression) {
            return result;
        }

        QRegularExpressionMatchIterator it = PAIR.globalMatch(*expression);
        while (it.hasNext()) {
            const QRegularExpressionMatch match = it.next();
            result.emplace_back(match.captured(1), match.captured(2));
        }

        return result;
    }

    QString sid() const
    {
        const std::optional<QString> expression = property("sid");
        return expression ? unquoted(*expression) : QString();
    }

    //! The `value:` of each `{ value: …, title: … }` of a `choices: [ … ]` list of the block, as text
    QStringList choiceValues() const
    {
        static const QRegularExpression OPENING("^\\s*choices:\\s*\\[");
        static const QRegularExpression ENTRY("\\{\\s*value:\\s*([^,}]+)");

        QStringList result;
        bool inside = false;
        for (const QString& line : ownLines) {
            if (!inside) {
                inside = OPENING.match(line).hasMatch();
                continue;
            }
            if (line.contains(']')) {
                break;
            }

            const QRegularExpressionMatch entry = ENTRY.match(line);
            if (entry.hasMatch()) {
                result << entry.captured(1).trimmed();
            }
        }

        return result;
    }

    //! Every opening bracket of the expression is closed outside of a string literal
    static bool balanced(const QString& expression)
    {
        static const QRegularExpression STRINGS("\"(?:[^\"\\\\]|\\\\.)*\"");

        QString bare = expression;
        bare.remove(STRINGS);
        return bare.count('[') == bare.count(']') && bare.count('(') == bare.count(')');
    }

    //! The text of an expression which is nothing but a string literal
    static QString unquoted(const QString& expression)
    {
        static const QRegularExpression LITERAL("^\"((?:[^\"\\\\]|\\\\.)*)\"$");
        const QRegularExpressionMatch match = LITERAL.match(expression);
        return match.hasMatch() ? unescaped(match.captured(1)) : QString();
    }

    static QString unescaped(const QString& text)
    {
        QString result;
        for (int i = 0; i < text.size(); ++i) {
            if (text.at(i) == '\\' && i + 1 < text.size()) {
                ++i;
            }
            result += text.at(i);
        }

        return result;
    }

    //! Every string literal which can be the value of the expression: the expression itself, or the branches
    //! of a conditional
    static QStringList literalsOf(const QString& expression)
    {
        static const QRegularExpression BRANCH("(?:^|[?:])\\s*\"((?:[^\"\\\\]|\\\\.)*)\"");

        QStringList result;
        QRegularExpressionMatchIterator it = BRANCH.globalMatch(expression);
        while (it.hasNext()) {
            result << unescaped(it.next().captured(1));
        }

        return result;
    }
};

//! NOTE A page, its sections and their options, from the text of the file
inline SourceBlock readPageSource(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    static const QRegularExpression OPENING("^\\s*(Eo\\w+)\\s*\\{\\s*$");
    static const QRegularExpression STRINGS("\"(?:[^\"\\\\]|\\\\.)*\"");

    SourceBlock root;
    std::vector<SourceBlock*> open { &root };
    std::vector<int> innerDepth { 0 };      // braces of the lines of a block which are not blocks

    const QStringList lines = QString::fromUtf8(file.readAll()).split('\n');
    for (const QString& line : lines) {
        const QRegularExpressionMatch opening = OPENING.match(line);
        if (opening.hasMatch()) {
            open.back()->children.push_back({ opening.captured(1), {}, {} });
            open.push_back(&open.back()->children.back());
            innerDepth.push_back(0);
            continue;
        }

        QString bare = line;
        bare.remove(STRINGS);
        const int depthChange = bare.count('{') - bare.count('}');

        const bool closesBlock = innerDepth.back() == 0 && depthChange < 0 && open.size() > 1;
        if (closesBlock) {
            open.pop_back();
            innerDepth.pop_back();
            continue;
        }

        open.back()->ownLines << line;
        innerDepth.back() += depthChange;
    }

    return root.children.empty() ? SourceBlock() : root.children.front();
}

//! The `id` → shown `value` (the first choice) of the EoContext blocks among the children
inline QMap<QString, QString> contextsOf(const SourceBlock& parent)
{
    QMap<QString, QString> contexts;
    for (const SourceBlock& child : parent.children) {
        if (child.type != "EoContext") {
            continue;
        }

        const std::optional<QString> id = child.property("id");
        const std::optional<QString> value = child.property("value");
        if (id && value) {
            const QString plain = SourceBlock::unquoted(*value);
            contexts.insert(*id, plain.isEmpty() ? *value : plain);
        }
    }

    return contexts;
}

//! The music an expression gives with the contexts' first values: a plain literal, or the branch of a
//! `name.value === "x" ? "a" : "b"` which the value of the context `name` picks
inline QString musicResolved(const QString& expression, const QMap<QString, QString>& contexts)
{
    const QString literal = SourceBlock::unquoted(expression);
    if (!literal.isEmpty()) {
        return literal;
    }

    static const QRegularExpression TERNARY(
        "^\\s*(\\w+)\\.value\\s*===\\s*\"([^\"]*)\"\\s*\\?\\s*"
        "(\"(?:[^\"\\\\]|\\\\.)*\")\\s*:\\s*(\"(?:[^\"\\\\]|\\\\.)*\")\\s*$");
    const QRegularExpressionMatch match = TERNARY.match(expression);
    if (!match.hasMatch()) {
        return QString();
    }

    return SourceBlock::unquoted(contexts.value(match.captured(1)) == match.captured(2) ? match.captured(3)
                                 : match.captured(4));
}

//! The music of an option and the width its systems are laid out with: both of the block which gives the
//! music (its own, else its grid's, else its section's, else its page's), resolved with the first value of
//! the contexts which the expression can name — the same resolution as `EoMusic.musicInfoOf` on the QML side
inline std::pair<QString, double> musicInfoOf(const SourceBlock& option, const SourceBlock* grid,
                                              const SourceBlock& section, const SourceBlock& page)
{
    const QMap<QString, QString> contexts = [&section, &page] {
        QMap<QString, QString> merged = contextsOf(page);
        merged.insert(contextsOf(section));
        return merged;
    }();

    for (const SourceBlock* block : { &option, grid, &section, &page }) {
        if (!block) {
            continue;
        }
        const std::optional<QString> expression = block->property("music");
        if (expression) {
            const QString music = musicResolved(*expression, contexts);
            if (!music.isEmpty()) {
                const double systemWidth = block->property("systemWidth").value_or(QStringLiteral("0")).toDouble();
                return { music, systemWidth };
            }
        }
    }

    return { QString(), 0.0 };
}

//! The options of a section as the dialog lists them: the cells of a grid in the place of the grid —
//! { option, the grid it is a cell of, else nullptr }
inline std::vector<std::pair<const SourceBlock*, const SourceBlock*> > optionsOf(const SourceBlock& section)
{
    std::vector<std::pair<const SourceBlock*, const SourceBlock*> > result;
    for (const SourceBlock& option : section.children) {
        if (option.type == "EoGrid") {
            for (const SourceBlock& cell : option.children) {
                if (cell.type != "EoGap") {
                    result.emplace_back(&cell, &option);
                }
            }
        } else {
            result.emplace_back(&option, nullptr);
        }
    }

    return result;
}

//! The `max` of a number option, as the renderer's probe uses it to pick the direction with room
inline double dragMaximumOf(const SourceBlock& option)
{
    const std::optional<QString> max = option.property("max");
    if (!max) {
        return 1e9;
    }

    const bool percent = option.property("percent").value_or(QString()) == "true";
    const double unitScale = option.property("unitScale").value_or(QStringLiteral("1")).toDouble();
    return max->toDouble() * (percent ? 0.01 : 1.0 / unitScale);
}
}
