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
#include "engravingoptionsmodel.h"

#include <QHash>
#include <QQmlEngine>

#include "engraving/dom/score.h"
#include "engraving/dom/masterscore.h"
#include "engraving/iengravingfont.h"
#include "engraving/style/style.h"
#include "engraving/style/textstyle.h"
#include "engraving/types/symnames.h"
#include "engraving/types/typesconv.h"
#include "types/translatablestring.h"

#include "notation/inotation.h"
#include "notation/inotationelements.h"
#include "notation/inotationstyle.h"
#include "notation/internal/inotationundostack.h"

#include "../styledialog/styleuivalue.h"
#include "../styledialog/textstylepagecodes.h"
#include "linkedstylevalues.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;

//! NOTE The dialog asks for the values by their names all the time (counting the changed values of all the pages),
//! and the style finds a name by going through all of them
static StyleId styleIdByName(const QString& name)
{
    static const QHash<QString, StyleId> IDS_BY_NAMES = []() {
        QHash<QString, StyleId> result;
        for (int i = static_cast<int>(StyleId::NOSTYLE) + 1; i < static_cast<int>(StyleId::STYLES); ++i) {
            const StyleId id = static_cast<StyleId>(i);
            result.insert(QString::fromLatin1(MStyle::valueName(id)), id);
        }
        return result;
    }();

    return IDS_BY_NAMES.value(name, StyleId::NOSTYLE);
}

//! NOTE The ids of the names which are style values; the other names are reported
static std::vector<StyleId> styleIdsByNames(const QStringList& names)
{
    std::vector<StyleId> ids;
    ids.reserve(names.size());

    for (const QString& name : names) {
        const StyleId id = styleIdByName(name);
        if (id == StyleId::NOSTYLE) {
            LOGE() << "no style value with the name: " << name;
            continue;
        }

        ids.push_back(id);
    }

    return ids;
}

EngravingOptionsModel::EngravingOptionsModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

EngravingOptionsModel::~EngravingOptionsModel()
{
    if (isLoaded()) {
        endSession(false);
    }
}

bool EngravingOptionsModel::isLoaded() const
{
    return m_notation != nullptr;
}

bool EngravingOptionsModel::canApplyToAllParts() const
{
    return isLoaded() && style()->canApplyToAllParts();
}

int EngravingOptionsModel::revision() const
{
    return m_revision;
}

QString EngravingOptionsModel::userStylesPath() const
{
    return configuration()->userStylesPath().toQString();
}

bool EngravingOptionsModel::loadsFontDefaults() const
{
    return m_loadsFontDefaults;
}

void EngravingOptionsModel::setLoadsFontDefaults(bool loads)
{
    if (m_loadsFontDefaults == loads) {
        return;
    }

    m_loadsFontDefaults = loads;
    emit loadsFontDefaultsChanged();
}

void EngravingOptionsModel::load()
{
    IF_ASSERT_FAILED(!isLoaded()) {
        return;
    }

    m_notation = globalContext()->currentNotation();
    if (!m_notation) {
        return;
    }

    m_notation->undoStack()->prepareChanges(muse::TranslatableString("undoableAction", "Edit style"));

    style()->styleChanged().onNotify(this, [this]() {
        onStyleChanged();
    });

    emit isLoadedChanged();
}

void EngravingOptionsModel::accept()
{
    endSession(true);
}

void EngravingOptionsModel::reject()
{
    endSession(false);
}

void EngravingOptionsModel::endSession(bool accepted)
{
    if (!isLoaded()) {
        return;
    }

    //! NOTE The notification of the end must not come back to the values of a session which is over
    const INotationPtr notation = m_notation;
    notation->style()->styleChanged().resetOnNotify(this);
    m_notation = nullptr;

    if (accepted) {
        notation->undoStack()->commitChanges();
    } else {
        notation->undoStack()->rollbackChanges();
    }

    notation->style()->styleChanged().notify();

    emit isLoadedChanged();
}

bool EngravingOptionsModel::contains(const QString& name) const
{
    return styleIdByName(name) != StyleId::NOSTYLE;
}

bool EngravingOptionsModel::isInStaffSpaces(const QString& name) const
{
    const StyleId id = styleIdByName(name);
    return id != StyleId::NOSTYLE && MStyle::valueType(id) == P_TYPE::SPATIUM;
}

QString EngravingOptionsModel::valueKind(const QString& name) const
{
    const StyleId id = styleIdByName(name);
    if (id == StyleId::NOSTYLE || !isLoaded()) {
        return QString();
    }

    if (MStyle::valueType(id) == P_TYPE::SPATIUM) {
        return QStringLiteral("spatium");
    }

    //! NOTE The interface has an alignment as a text (see styleValueToUi), but it is not edited as one
    if (MStyle::valueType(id) == P_TYPE::ALIGN) {
        return QStringLiteral("align");
    }

    //! NOTE By what the interface gets, so that an enumeration, which it gets as a number, is a number
    switch (styleValueToUi(id, style()->defaultStyleValue(id)).typeId()) {
    case QMetaType::Bool:
        return QStringLiteral("bool");
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
        return QStringLiteral("int");
    case QMetaType::Double:
    case QMetaType::Float:
        return QStringLiteral("real");
    case QMetaType::QString:
        return QStringLiteral("string");
    case QMetaType::QPointF:
        return QStringLiteral("point");
    case QMetaType::QSizeF:
        return QStringLiteral("size");
    case QMetaType::QColor:
        return QStringLiteral("color");
    default:
        break;
    }

    return QStringLiteral("other");
}

StyleItem* EngravingOptionsModel::item(const QString& name)
{
    if (!isLoaded()) {
        return nullptr;
    }

    const StyleId id = styleIdByName(name);
    if (id == StyleId::NOSTYLE) {
        LOGE() << "no style value with the name: " << name;
        return nullptr;
    }

    auto found = m_items.find(id);
    if (found != m_items.end()) {
        return found->second;
    }

    StyleItem* item = new StyleItem(this, styleValueToUi(id, style()->styleValue(id)),
                                    styleValueToUi(id, style()->defaultStyleValue(id)));

    //! NOTE The item is handed to QML by a method, which must not make QML its owner
    QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);

    connect(item, &StyleItem::valueModified, this, [this, id](const QVariant& newValue) {
        if (isLoaded()) {
            setValue(id, styleValueFromUi(id, newValue));
        }
    });

    m_items.emplace(id, item);
    return item;
}

void EngravingOptionsModel::setValue(StyleId id, const PropertyValue& value)
{
    if (id == StyleId::musicalSymbolFont) {
        setMusicFont(value);
        return;
    }

    //! NOTE The value first: what comes with it is set after it, so that the chord list is read again with the
    //! preset which the new description file belongs to
    style()->setStyleValue(id, value);
    for (const auto& [linkedId, linkedValue] : linkedStyleValues(id, value)) {
        style()->setStyleValue(linkedId, linkedValue);
    }
}

//! NOTE The font of the musical symbols takes other values with it, as in the style dialog:
//! the font of the dynamics, unless it is chosen apart, and what the font recommends for the engraving
void EngravingOptionsModel::setMusicFont(const PropertyValue& fontName)
{
    const INotationStylePtr notationStyle = style();

    if (!notationStyle->styleValue(StyleId::dynamicsOverrideFont).toBool()) {
        notationStyle->setStyleValue(StyleId::dynamicsFont, fontName);
    }

    if (m_loadsFontDefaults) {
        const IEngravingFontPtr font = engravingFonts()->fontByName(fontName.value<String>().toStdString());
        if (font) {
            for (const auto& [defaultId, defaultValue] : font->engravingDefaults()) {
                notationStyle->setStyleValue(defaultId, defaultValue);
            }
        }
    }

    notationStyle->setStyleValue(StyleId::musicalSymbolFont, fontName);
}

QVariantList EngravingOptionsModel::choices(const QString& list) const
{
    QVariantList result;

    const auto add = [&result](const QString& title, const QVariant& value) {
        result << QVariantMap { { "title", title }, { "value", value } };
    };

    if (list == "musicFonts" || list == "musicTextFonts") {
        for (const IEngravingFontPtr& font : engravingFonts()->fonts()) {
            const QString name = QString::fromStdString(font->name());

            if (list == "musicFonts") {
                add(name, name);
            } else {
                //! NOTE The value of the text font is the name of its family
                add(name + " Text", QString::fromStdString(font->family()) + " Text");
            }
        }
    } else if (list == "textStyles") {
        //! NOTE Every text style by its name, as the style dialog lists them for the text of measure numbers
        for (TextStyleType type : allTextStyles()) {
            add(TConv::userName(type).qTranslated(), static_cast<int>(type));
        }
    } else if (list == "dividerSymbols") {
        for (SymId id : { SymId::systemDivider, SymId::systemDividerLong, SymId::systemDividerExtraLong }) {
            add(SymNames::translatedUserNameForSymId(id).toQString(),
                QString(SymNames::nameForSymId(id).toQLatin1String()));
        }
    } else {
        LOGE() << "no such list of choices: " << list;
    }

    return result;
}

QStringList EngravingOptionsModel::names() const
{
    QStringList result;
    for (int i = static_cast<int>(StyleId::NOSTYLE) + 1; i < static_cast<int>(StyleId::STYLES); ++i) {
        result << QString::fromLatin1(MStyle::valueName(static_cast<StyleId>(i)));
    }

    return result;
}

int EngravingOptionsModel::modifiedCount(const QStringList& names) const
{
    if (!isLoaded()) {
        return 0;
    }

    const INotationStylePtr notationStyle = style();
    int count = 0;

    for (StyleId id : styleIdsByNames(names)) {
        if (notationStyle->styleValue(id) != notationStyle->defaultStyleValue(id)) {
            ++count;
        }
    }

    return count;
}

QVariantList EngravingOptionsModel::textStyles() const
{
    QVariantList result;

    const std::vector<TextStyleType>& types = editableTextStyles();
    const QStringList& codes = textStyleSubPageCodes();
    IF_ASSERT_FAILED(codes.size() == static_cast<qsizetype>(types.size())) {
        return result;
    }

    for (size_t i = 0; i < types.size(); ++i) {
        const TextStyleType type = types.at(i);
        const int userIndex = static_cast<int>(type) - static_cast<int>(TextStyleType::USER1);
        const bool isUser = type >= TextStyleType::USER1 && type <= TextStyleType::USER12;
        const QString nameSid = isUser ? QString("user%1Name").arg(userIndex + 1) : QString();

        //! NOTE A user style is called by the name the score gives it, if any (as Score::getTextStyleUserName)
        QString name = isLoaded() && isUser ? style()->styleValue(styleIdByName(nameSid)).value<String>().toQString()
                       : QString();
        if (name.isEmpty()) {
            name = TConv::userName(type).qTranslated();
        }

        result << QVariantMap {
            { "type", static_cast<int>(type) },
            { "code", codes.at(static_cast<qsizetype>(i)) },
            { "name", name },
            { "nameSid", nameSid }
        };
    }

    return result;
}

QVariantList EngravingOptionsModel::textStyleProperties(int textStyleType) const
{
    static const std::map<TextStylePropertyType, QString> PROPERTIES {
        { TextStylePropertyType::FontFace, "fontFace" },
        { TextStylePropertyType::FontSize, "fontSize" },
        { TextStylePropertyType::LineSpacing, "lineSpacing" },
        { TextStylePropertyType::LetterSpacing, "letterSpacing" },
        { TextStylePropertyType::SizeSpatiumDependent, "sizeSpatiumDependent" },
        { TextStylePropertyType::FontStyle, "fontStyle" },
        { TextStylePropertyType::Color, "color" },
        { TextStylePropertyType::TextAlign, "textAlign" },
        { TextStylePropertyType::Offset, "offset" },
        { TextStylePropertyType::FrameType, "frameType" },
        { TextStylePropertyType::FramePadding, "framePadding" },
        { TextStylePropertyType::FrameWidth, "frameWidth" },
        { TextStylePropertyType::FrameRound, "frameRound" },
        { TextStylePropertyType::FrameBorderColor, "frameBorderColor" },
        { TextStylePropertyType::FrameFillColor, "frameFillColor" },
        { TextStylePropertyType::MusicalSymbolsScale, "musicalSymbolsScale" },
        { TextStylePropertyType::Position, "position" }
    };

    QVariantList result;
    const engraving::TextStyle* textStyle = engraving::textStyle(static_cast<TextStyleType>(textStyleType));
    if (!textStyle) {
        return result;
    }

    for (const engraving::TextStyleProperty& property : *textStyle) {
        const auto name = PROPERTIES.find(property.type);
        //! NOTE Most styles give their musical symbols a scale which nothing reads (dummyMusicalSymbolsScale)
        if (name == PROPERTIES.end() || property.sid == Sid::dummyMusicalSymbolsScale) {
            continue;
        }

        result << QVariantMap {
            { "property", name->second },
            { "sid", QString::fromLatin1(MStyle::valueName(property.sid)) }
        };
    }

    return result;
}

QVariantList EngravingOptionsModel::metaTags() const
{
    QVariantList result;

    const INotationPtr notation = m_notation ? m_notation : globalContext()->currentNotation();
    if (!notation || !notation->elements()) {
        return result;
    }

    const engraving::Score* score = notation->elements()->msScore();
    if (!score) {
        return result;
    }

    const auto addTags = [&result](const std::map<muse::String, muse::String>& tags) {
        for (const auto& tag : tags) {
            result.append(QVariantMap {
                { "tag", tag.first.toQString() },
                { "value", tag.second.empty() ? QStringLiteral("-") : tag.second.toQString() }
            });
        }
    };

    if (!score->isMaster()) {
        addTags(score->masterScore()->metaTags());
    }
    addTags(score->metaTags());

    return result;
}

void EngravingOptionsModel::resetValues(const QStringList& names)
{
    if (!isLoaded()) {
        return;
    }

    //! NOTE A value which is reset is chosen again as its default is: what comes with it is set too (the file of
    //! the default preset of the chord symbols)
    const std::vector<StyleId> ids = styleIdsByNames(names);
    style()->resetStyleValues(ids);
    for (const StyleId id : ids) {
        for (const auto& [linkedId, linkedValue] : linkedStyleValues(id, style()->defaultStyleValue(id))) {
            style()->setStyleValue(linkedId, linkedValue);
        }
    }
}

void EngravingOptionsModel::resetAllValues()
{
    if (!isLoaded()) {
        return;
    }

    static const StyleIdSet KEPT_VALUES {
        StyleId::pageWidth,
        StyleId::pageHeight,
        StyleId::pagePrintableWidth,
        StyleId::pageEvenTopMargin,
        StyleId::pageEvenBottomMargin,
        StyleId::pageEvenLeftMargin,
        StyleId::pageOddTopMargin,
        StyleId::pageOddBottomMargin,
        StyleId::pageOddLeftMargin,
        StyleId::pageTwosided,
        StyleId::spatium,
        StyleId::concertPitch,
        StyleId::createMultiMeasureRests
    };

    style()->resetAllStyleValues(KEPT_VALUES);
}

void EngravingOptionsModel::applyToAllParts()
{
    if (isLoaded()) {
        style()->applyToAllParts();
    }
}

INotationStylePtr EngravingOptionsModel::style() const
{
    return m_notation ? m_notation->style() : nullptr;
}

void EngravingOptionsModel::onStyleChanged()
{
    const INotationStylePtr notationStyle = style();
    for (const auto& [id, item] : m_items) {
        item->setValue(styleValueToUi(id, notationStyle->styleValue(id)));
    }

    ++m_revision;
    emit revisionChanged();
}
