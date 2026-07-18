/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include <vector>

#include <QAbstractListModel>

#include "modularity/ioc.h"

#include "../ifontdesignservice.h"

namespace mu::fontdesign {
//! SMuFL 校验（lint）：字体名规则 / 核心字形覆盖 / 空字形 / advance /
//! 轮廓环绕方向（镂空正确性）/ optionalGlyphs PUA 冲突
class FontLintModel : public QAbstractListModel, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(QString summary READ summary NOTIFY resultsChanged)
    Q_PROPERTY(int count READ count NOTIFY resultsChanged)

    muse::Inject<IFontDesignService> fontDesignService = { this };

public:
    explicit FontLintModel(QObject* parent = nullptr);

    enum Roles {
        SeverityRole = Qt::UserRole + 1,   // "error" | "warning" | "info"
        MessageRole,
        CodepointRole,                     // 0 = 与具体字形无关
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return static_cast<int>(m_items.size()); }
    QString summary() const { return m_summary; }

    Q_INVOKABLE void run();
    //! 跳到问题字形（浏览器与画布随 currentGlyphChanged 联动）
    Q_INVOKABLE void goToGlyph(int row);

signals:
    void resultsChanged();

private:
    struct Item {
        QString severity;
        QString message;
        char32_t codepoint = 0;
    };

    void addItem(const QString& severity, const QString& message, char32_t codepoint = 0);

    std::vector<Item> m_items;
    QString m_summary;
};
}
