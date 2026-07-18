# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

declare_module(fontdesign)

set(MODULE_QRC
    ${CMAKE_CURRENT_LIST_DIR}/fontdesign.qrc
    )

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/fontdesignmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fontdesignmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/ifontdesignservice.h
    ${CMAKE_CURRENT_LIST_DIR}/ifontdesigneditsurface.h
    ${CMAKE_CURRENT_LIST_DIR}/ifontdesignconfiguration.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/fontdesigntypes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontdesigntypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/smufldatabase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/smufldatabase.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontdesignservice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontdesignservice.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontdesignconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontdesignconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontdesignactioncontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontdesignactioncontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontdesignuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontdesignuiactions.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/project/glyphoutline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/project/glyphoutline.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/project/outlinegeometry.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/project/outlinegeometry.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/project/curvefitter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/project/curvefitter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/project/glyphitem.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/project/fontmetadata.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/project/fontdesignproject.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/project/fontdesignproject.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/project/undostack.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/project/undostack.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/project/projectcommands.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/project/projectcommands.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/puaallocator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/puaallocator.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/io/fontfacereader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/io/fontfacereader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/io/metadatareader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/io/metadatareader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/io/metadatawriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/io/metadatawriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/io/cffwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/io/cffwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/io/sfntwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/io/sfntwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/io/fontexporter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/io/fontexporter.h

    ${CMAKE_CURRENT_LIST_DIR}/view/projectspagemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/projectspagemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/fontdesignpagemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/fontdesignpagemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/newfontmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/newfontmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/installedfontsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/installedfontsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/addglyphsourcemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/addglyphsourcemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/fontlintmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/fontlintmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/canvas/outlinepreview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/canvas/outlinepreview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/glyphbrowsermodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/glyphbrowsermodel.h

    ${CMAKE_CURRENT_LIST_DIR}/view/canvas/glyphcanvas.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/canvas/glyphcanvas.h
    ${CMAKE_CURRENT_LIST_DIR}/view/canvas/glyphcellview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/canvas/glyphcellview.h

    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/abstractfontdesignmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/abstractfontdesignmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/fontinfomodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/fontinfomodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/engravingdefaultsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/engravingdefaultsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/glyphpropertiesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/glyphpropertiesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/anchorsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/anchorsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/alternatesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/alternatesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/ligaturesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/ligaturesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/optionalglyphsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/optionalglyphsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/setsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/inspectors/setsmodel.h
    )

set(MODULE_LINK
    muse_draw
    muse_ui
    muse_uicomponents
    muse_actions
    muse_dockwindow
    Qt::Quick
    )

# FreeType: the draw module adds the vendored target (draw/cmake/SetupFreeType.cmake);
# fall back to the system library for MUSE_COMPILE_USE_SYSTEM_FREETYPE configurations
if (TARGET freetype)
    list(APPEND MODULE_LINK freetype)
else()
    find_package(Freetype REQUIRED)
    list(APPEND MODULE_LINK ${FREETYPE_LIBRARIES})
    set(MODULE_INCLUDE_PRIVATE ${FREETYPE_INCLUDE_DIRS})
endif()

setup_module()

if (MUE_BUILD_FONTDESIGN_TESTS)
    add_subdirectory(tests)
endif()

