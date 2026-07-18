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
#include "fontdesignmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/iuiengine.h"
#include "ui/iuiactionsregister.h"

#include "internal/fontdesignservice.h"
#include "internal/fontdesignconfiguration.h"
#include "internal/fontdesignactioncontroller.h"
#include "internal/fontdesignuiactions.h"

#include "view/projectspagemodel.h"
#include "view/fontdesignpagemodel.h"
#include "view/newfontmodel.h"
#include "view/installedfontsmodel.h"
#include "view/addglyphsourcemodel.h"
#include "view/fontlintmodel.h"
#include "view/canvas/outlinepreview.h"
#include "view/glyphbrowsermodel.h"
#include "view/canvas/glyphcanvas.h"
#include "view/canvas/glyphcellview.h"
#include "view/inspectors/fontinfomodel.h"
#include "view/inspectors/engravingdefaultsmodel.h"
#include "view/inspectors/glyphpropertiesmodel.h"
#include "view/inspectors/anchorsmodel.h"
#include "view/inspectors/alternatesmodel.h"
#include "view/inspectors/ligaturesmodel.h"
#include "view/inspectors/optionalglyphsmodel.h"
#include "view/inspectors/setsmodel.h"

using namespace mu::fontdesign;
using namespace muse;
using namespace muse::modularity;

static void fontdesign_init_qrc()
{
    Q_INIT_RESOURCE(fontdesign);
}

std::string FontDesignModule::moduleName() const
{
    return "fontdesign";
}

void FontDesignModule::registerExports()
{
    m_service = std::make_shared<FontDesignService>();
    m_configuration = std::make_shared<FontDesignConfiguration>();
    m_actionController = std::make_shared<FontDesignActionController>();
    m_uiActions = std::make_shared<FontDesignUiActions>(m_actionController);

    ioc()->registerExport<IFontDesignService>(moduleName(), m_service);
    ioc()->registerExport<IFontDesignConfiguration>(moduleName(), m_configuration);
}

void FontDesignModule::resolveImports()
{
    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(m_uiActions);
    }

    auto ir = ioc()->resolve<muse::ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://fontdesign/newfont"), "MuseScore/FontDesign/NewFontDialog.qml");
        ir->registerQmlUri(Uri("musescore://fontdesign/addglyph"), "MuseScore/FontDesign/AddGlyphFromFontDialog.qml");
        ir->registerQmlUri(Uri("musescore://fontdesign/lint"), "MuseScore/FontDesign/FontLintDialog.qml");
    }
}

void FontDesignModule::registerResources()
{
    fontdesign_init_qrc();
}

void FontDesignModule::registerUiTypes()
{
    qmlRegisterType<ProjectsPageModel>("MuseScore.FontDesign", 1, 0, "ProjectsPageModel");
    qmlRegisterType<FontDesignPageModel>("MuseScore.FontDesign", 1, 0, "FontDesignPageModel");
    qmlRegisterType<GlyphBrowserModel>("MuseScore.FontDesign", 1, 0, "GlyphBrowserModel");
    qmlRegisterType<NewFontModel>("MuseScore.FontDesign", 1, 0, "NewFontModel");
    qmlRegisterType<InstalledFontsModel>("MuseScore.FontDesign", 1, 0, "InstalledFontsModel");
    qmlRegisterType<AddGlyphSourceModel>("MuseScore.FontDesign", 1, 0, "AddGlyphSourceModel");
    qmlRegisterType<FontLintModel>("MuseScore.FontDesign", 1, 0, "FontLintModel");
    qmlRegisterType<OutlinePreview>("MuseScore.FontDesign", 1, 0, "OutlinePreview");
    qmlRegisterType<GlyphCanvas>("MuseScore.FontDesign", 1, 0, "GlyphCanvas");
    qmlRegisterType<GlyphCellView>("MuseScore.FontDesign", 1, 0, "GlyphCellView");
    qmlRegisterType<FontInfoModel>("MuseScore.FontDesign", 1, 0, "FontInfoModel");
    qmlRegisterType<EngravingDefaultsModel>("MuseScore.FontDesign", 1, 0, "EngravingDefaultsModel");
    qmlRegisterType<GlyphPropertiesModel>("MuseScore.FontDesign", 1, 0, "GlyphPropertiesModel");
    qmlRegisterType<AnchorsModel>("MuseScore.FontDesign", 1, 0, "AnchorsModel");
    qmlRegisterType<AlternatesModel>("MuseScore.FontDesign", 1, 0, "AlternatesModel");
    qmlRegisterType<LigaturesModel>("MuseScore.FontDesign", 1, 0, "LigaturesModel");
    qmlRegisterType<OptionalGlyphsModel>("MuseScore.FontDesign", 1, 0, "OptionalGlyphsModel");
    qmlRegisterType<SetsModel>("MuseScore.FontDesign", 1, 0, "SetsModel");

    ioc()->resolve<muse::ui::IUiEngine>(moduleName())->addSourceImportPath(fontdesign_QML_IMPORT);
}

void FontDesignModule::onInit(const IApplication::RunMode& mode)
{
    if (IApplication::RunMode::GuiApp != mode) {
        return;
    }

    m_configuration->init();
    m_actionController->init();
}

void FontDesignModule::onDeinit()
{
    ioc()->unregisterIfRegistered<IFontDesignService>(moduleName(), m_service);
    ioc()->unregisterIfRegistered<IFontDesignConfiguration>(moduleName(), m_configuration);

    m_service.reset();
    m_configuration.reset();
}
