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

#include <map>

#include <QObject>
#include <QStringList>

#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "context/iglobalcontext.h"
#include "engraving/iengravingfontsprovider.h"
#include "notation/inotation.h"
#include "notation/inotationconfiguration.h"
#include "notation/notationtypes.h"

#include "../styledialog/styleitem.h"

namespace mu::notation {
//! NOTE What the Engraving Options dialog needs:
//! - any style value by its name (the tag which the value has in a style file), so that an option of the dialog
//!   is one line of QML and needs nothing here;
//! - the session of the dialog: a change is applied to the score at once, all the changes are one step of the
//!   undo history when the dialog is accepted, and they are taken back when it is rejected
class EngravingOptionsModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY isLoadedChanged)
    Q_PROPERTY(bool canApplyToAllParts READ canApplyToAllParts NOTIFY isLoadedChanged)

    //! NOTE Goes up with every change of the style. What depends on many values at once
    //! (the number of the changed values of a page, a filter) is bound to it
    Q_PROPERTY(int revision READ revision NOTIFY revisionChanged)

    //! NOTE Whether a change of the font of the musical symbols also sets the values which suit the font
    //! (the thicknesses of its lines and the like, which the font recommends)
    Q_PROPERTY(bool loadsFontDefaults READ loadsFontDefaults WRITE setLoadsFontDefaults NOTIFY loadsFontDefaultsChanged)

    //! NOTE Where a file of the user's own (a chord symbol style file) is looked for first
    Q_PROPERTY(QString userStylesPath READ userStylesPath CONSTANT)

    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<engraving::IEngravingFontsProvider> engravingFonts = { this };
    muse::Inject<INotationConfiguration> configuration = { this };

public:
    explicit EngravingOptionsModel(QObject* parent = nullptr);
    ~EngravingOptionsModel() override;

    bool isLoaded() const;
    bool canApplyToAllParts() const;
    int revision() const;

    bool loadsFontDefaults() const;
    void setLoadsFontDefaults(bool loads);

    QString userStylesPath() const;

    //! NOTE Begins the session with the current score; nothing else works before
    Q_INVOKABLE void load();

    //! NOTE The end of the session. A session which is not ended when the model is destroyed is rejected
    Q_INVOKABLE void accept();
    Q_INVOKABLE void reject();

    Q_INVOKABLE bool contains(const QString& name) const;

    //! NOTE Whether the value is a distance in staff spaces, which is shown with that unit
    Q_INVOKABLE bool isInStaffSpaces(const QString& name) const;

    //! NOTE What kind of value it is, for a control which is chosen by the value and not by a description:
    //! "spatium" (a distance in staff spaces), "real", "int", "bool", "string", "point", "size" (a width and a
    //! height), "color", or "other" (a value which only a control made for it can edit); empty if there is no such
    //! value
    Q_INVOKABLE QString valueKind(const QString& name) const;

    //! NOTE The value with this name; null (and an error in the log) if there is no such value
    Q_INVOKABLE mu::notation::StyleItem* item(const QString& name);

    //! NOTE The choices of an option which the program knows and a page cannot write down, as a list of
    //! { title, value }: "musicFonts" (the fonts of musical symbols), "musicTextFonts" (their text fonts),
    //! "dividerSymbols" (the symbols which divide the systems of a page)
    Q_INVOKABLE QVariantList choices(const QString& list) const;

    //! NOTE The names of all the style values, in the order of the style
    Q_INVOKABLE QStringList names() const;

    //! NOTE The metadata tags of the score with their values ("-" when empty), as a list of
    //! { tag, value }: for a part the tags of the master score first — the variables a header
    //! or footer can use
    Q_INVOKABLE QVariantList metaTags() const;

    //! NOTE How many of these values differ from their defaults
    Q_INVOKABLE int modifiedCount(const QStringList& names) const;

    Q_INVOKABLE void resetValues(const QStringList& names);

    //! NOTE All the values but the ones of the page, the staff size and the ones which change the notes
    //! (concert pitch, multimeasure rests)
    Q_INVOKABLE void resetAllValues();

    Q_INVOKABLE void applyToAllParts();

signals:
    void isLoadedChanged();
    void revisionChanged();
    void loadsFontDefaultsChanged();

private:
    INotationStylePtr style() const;

    void onStyleChanged();
    void setValue(StyleId id, const PropertyValue& value);
    void setMusicFont(const PropertyValue& fontName);
    void endSession(bool accepted);

    INotationPtr m_notation;
    std::map<StyleId, StyleItem*> m_items;
    int m_revision = 0;
    bool m_loadsFontDefaults = true;
};
}
