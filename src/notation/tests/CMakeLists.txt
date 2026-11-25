# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
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

set(MODULE_TEST notation_tests)

set(MODULE_TEST_SRC
    ${PROJECT_SOURCE_DIR}/src/engraving/tests/utils/scorerw.cpp
    ${PROJECT_SOURCE_DIR}/src/engraving/tests/utils/scorerw.h

    ${CMAKE_CURRENT_LIST_DIR}/mocks/globalcontextmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/msczreadermock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/notationmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/notationconfigurationmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/notationinteractionmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/notationselectionmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/notationselectionrangemock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/notationstylemock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/notationundostackmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/controlledviewmock.h

    ${CMAKE_CURRENT_LIST_DIR}/environment.cpp
    ${CMAKE_CURRENT_LIST_DIR}/engravingoptionsmodel_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/engravingoptionsnavigation_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/engravingoptionsstate_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/engravingoptionspages_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/engravingpreview_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/engravingpreview_contactsheets.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ledgerguides_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/linepreview_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/notationruler_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/notationviewinputcontroller_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/previewmusic_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/previewoverlay_tests.cpp
)

set(MODULE_TEST_LINK
    engraving
    notation
)

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

#! NOTE The headers of the views include their neighbours of the module by their short names
set(MODULE_TEST_INCLUDE ${PROJECT_SOURCE_DIR}/src/notation)

set(ENGRAVING_OPTIONS_QML_DIR ${PROJECT_SOURCE_DIR}/src/notation/qml/MuseScore/NotationScene/internal/EngravingOptions)
set(MODULE_TEST_DEF
    ENGRAVING_OPTIONS_QML_DIR="${ENGRAVING_OPTIONS_QML_DIR}"
    ENGRAVING_OPTIONS_SNIPPETS_DIR="${ENGRAVING_OPTIONS_QML_DIR}/snippets"
    ENGRAVING_OPTIONS_DIALOG_FILE="${PROJECT_SOURCE_DIR}/src/notation/qml/MuseScore/NotationScene/EngravingOptionsDialog.qml"
    ENGRAVING_OPTIONS_QRC_FILE="${PROJECT_SOURCE_DIR}/src/notation/notationscene.qrc"
    )

include(SetupGTest)
