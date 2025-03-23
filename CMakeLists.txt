cmake_minimum_required(VERSION 3.10)
cmake_policy(VERSION 3.10)

project(zero_elabviewer_community VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

set(ADAM_BELINGER "Adam Berlinger <berlingeradam@gmail.com>")
set(JAKUB_VANEK "Jakub Vaněk <linuxtardis@gmail.com>")

set(PROJECT_HOMEPAGE_URL "https://github.com/sdi-hackers/zero_elabviewer_community/")
set(GENERIC_NAME "Zero eLab Viewer Community")
set(LICENCE "GPL-3.0-or-later")
set(PROJECT_DESCRIPTION "Zero eLab Viewer Community for Cheap SDI Instruments")
set(LONG_DESCRIPTION
        "Zero eLab Viewer Community is a PC program for displaying "
        "and controlling external data acquisition system through "
        "standard communication interface such as USB CDC class."
        "This program is a community fork of the official "
        "Zero eLab Viewer code base.")

include(cmake/CopyrightTools.cmake)

copyright(
        "Copyright (c) 2016-2018 ${ADAM_BELINGER}")

include(cmake/GPL-3.0-or-later.cmake)

# =============================================================================
# Configurable options
# =============================================================================

set(DEV_MODE false CACHE BOOL "Enable developer options in this CMake, like packaging.\
    They should be ignored, when user just wants to build this project.")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/target"
        CACHE STRING "Absolute path to place executables to.")
set(PACKAGE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/pkg"
        CACHE STRING "Absolute path to place generated package files.")

# =============================================================================
# Generated variables
# =============================================================================

# I don't want to relly on the assumption, that this file is invoked as root
# project. Therefore I propagate the information to all subprojects
# MAIN_PROJECT_*. Lowercase and uppercase are used for executable names and
# C defines, respectively.
set(MAIN_PROJECT_NAME "${PROJECT_NAME}")
set(MAIN_PROJECT_VERSION "${PROJECT_VERSION}")
set(MAIN_PROJECT_APPID "com.github.sdi-hackers.zero-elabviewer-community")
set(MAIN_PROJECT_ORGANIZATION "SDI Hackers")
set(MAIN_PROJECT_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
string(TOLOWER "${PROJECT_NAME}" MAIN_PROJECT_NAME_LOWER)
string(TOUPPER "${PROJECT_NAME}" MAIN_PROJECT_NAME_UPPER)
set(MAIN_EXECUTEBLE_NAME "${PROJECT_NAME}")

# =============================================================================
# CMake config and tools
# =============================================================================

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Qml SerialPort)
find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS PrintSupport Widgets)
set(QtLib "Qt${QT_VERSION_MAJOR}")

find_package(PkgConfig REQUIRED)
pkg_search_module(FFTW REQUIRED fftw3 IMPORTED_TARGET)
include_directories(PkgConfig::FFTW)

# =============================================================================
# Sources
# =============================================================================

set(PROJECT_SOURCES
    src/AboutWindow.cpp
    src/AboutWindow.h
    src/Application.cpp
    src/Application.h
    src/DataUtils/DataAveraging.h
    src/DataUtils/DataConverter.cpp
    src/DataUtils/DataConverter.h
    src/DataUtils/DataSet.cpp
    src/DataUtils/DataSet.h
    src/DataUtils/LiveJSScript.cpp
    src/DataUtils/LiveJSScript.h
    src/DataUtils/Protocol.cpp
    src/DataUtils/Protocol.h
    src/DataUtils/SignalMeasurement.cpp
    src/DataUtils/SignalMeasurement.h
    src/DataUtils/SpectralAnalysis.cpp
    src/DataUtils/SpectralAnalysis.h
    src/FunctionWidgets/GeneratorWidget.cpp
    src/FunctionWidgets/GeneratorWidget.h
    src/FunctionWidgets/PulseCounterWidget.cpp
    src/FunctionWidgets/PulseCounterWidget.h
    src/FunctionWidgets/PWMInputWidget.cpp
    src/FunctionWidgets/PWMInputWidget.h
    src/FunctionWidgets/PWMWidget.cpp
    src/FunctionWidgets/PWMWidget.h
    src/FunctionWidgets/ScopeWidget.cpp
    src/FunctionWidgets/ScopeWidget.h
    src/FunctionWidgets/VoltmeterWidget.cpp
    src/FunctionWidgets/VoltmeterWidget.h
    src/HelperWidgets/ChannelControl.cpp
    src/HelperWidgets/ChannelControl.h
    src/HelperWidgets/ExtendedPlot.cpp
    src/HelperWidgets/ExtendedPlot.h
    src/HelperWidgets/FrequencyControl.cpp
    src/HelperWidgets/FrequencyControl.h
    src/HelperWidgets/RangeControl.cpp
    src/HelperWidgets/RangeControl.h
    src/HelperWidgets/RecordWidget.cpp
    src/HelperWidgets/RecordWidget.h
    src/HelperWidgets/ResponseMeasurement.cpp
    src/HelperWidgets/ResponseMeasurement.h
    src/HelperWidgets/RunningIcon.cpp
    src/HelperWidgets/RunningIcon.h
    src/HelperWidgets/SliderControl.cpp
    src/HelperWidgets/SliderControl.h
    src/HelperWidgets/SpectralAnalysisWidget.cpp
    src/HelperWidgets/SpectralAnalysisWidget.h
    src/HelperWidgets/StatusWidget.cpp
    src/HelperWidgets/StatusWidget.h
    src/HelpWindow.cpp
    src/HelpWindow.h
    src/main.cpp
    src/qcustomplot.cpp
    src/qcustomplot.h
)

set(QRC_FILES
    resources/resources.qrc
)

# =============================================================================
# Build targets
# =============================================================================

# MACOS
set(ICON_NAME zero-elabviewer-community)
set(ICON_PATH ${CMAKE_SOURCE_DIR}/data/icons/baked/${ICON_NAME}.icns)
# END MACOS

if(WIN32)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/data/resources.rc)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt6_add_resources(rcc_files ${QRC_FILES})
    qt_add_executable(zero_elabviewer_community MANUAL_FINALIZATION ${ICON_PATH} ${PROJECT_SOURCES} ${rcc_files})
else()
    qt5_add_resources(rcc_files ${QRC_FILES})
    add_executable(zero_elabviewer_community ${ICON_PATH} ${PROJECT_SOURCES} ${rcc_files})
endif()

if(QT_VERSION_MAJOR EQUAL 6)
qt_add_qml_module(zero_elabviewer_community
    URI zero_elabviewer_community
    VERSION ${PROJECT_VERSION}
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build
)
endif()

target_include_directories(zero_elabviewer_community PUBLIC
    src
    src/DataUtils
    src/FunctionWidgets
    src/HelperWidgets
)

target_link_libraries(zero_elabviewer_community PUBLIC
    ${QtLib}::Core
    ${QtLib}::Gui
    ${QtLib}::Qml
    ${QtLib}::SerialPort
    PkgConfig::FFTW
)

if((QT_VERSION_MAJOR GREATER 4))
    target_link_libraries(zero_elabviewer_community PUBLIC
        ${QtLib}::PrintSupport
        ${QtLib}::Widgets
    )
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(zero_elabviewer_community)
endif()

string(REPLACE "_" "-" PACKAGE_NAME "${MAIN_PROJECT_NAME_LOWER}")

# MACOS
set_property(SOURCE ${ICON_PATH}
        PROPERTY MACOSX_PACKAGE_LOCATION Resources)
set_target_properties(zero_elabviewer_community PROPERTIES
        MACOSX_BUNDLE true
        MACOSX_BUNDLE_GUI_IDENTIFIER cz.cvut.fel.embedded.${MAIN_PROJECT_NAME}
        MACOSX_BUNDLE_BUNDLE_NAME ${MAIN_PROJECT_NAME}
        MACOSX_BUNDLE_BUNDLE_VERSION "${MAIN_PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${MAIN_PROJECT_VERSION}"
        MACOSX_BUNDLE_ICONFILE ${ICON_NAME}
        )
# END MACOS

# =============================================================================
# Installation
# =============================================================================

if (NOT "${WASM}")
    install(TARGETS zero_elabviewer_community
           RUNTIME DESTINATION bin
           BUNDLE DESTINATION ${EXECUTABLE_OUTPUT_PATH})

    function(install_icon dimensions extension)
        install(FILES "data/icons/baked/${dimensions}.${extension}"
               DESTINATION "share/icons/hicolor/${dimensions}/apps"
               RENAME "${PACKAGE_NAME}.${extension}")
    endfunction()
    install_icon("scalable" "svg")
    install_icon("16x16" "png")
    install_icon("24x24" "png")
    install_icon("32x32" "png")
    install_icon("48x48" "png")
    install_icon("64x64" "png")
    install_icon("128x128" "png")
    install_icon("256x256" "png")
    install_icon("512x512" "png")

    configure_file("data/${MAIN_PROJECT_APPID}.metainfo.xml.in"
            "${EXECUTABLE_OUTPUT_PATH}/${MAIN_PROJECT_APPID}.metainfo.xml")
    install(FILES "${EXECUTABLE_OUTPUT_PATH}/${MAIN_PROJECT_APPID}.metainfo.xml"
            DESTINATION share/metainfo)

    configure_file(data/desktop.in
            "${EXECUTABLE_OUTPUT_PATH}/${PACKAGE_NAME}.desktop")
    install(FILES "${EXECUTABLE_OUTPUT_PATH}/${PACKAGE_NAME}.desktop"
            DESTINATION share/applications)
endif ()

# =============================================================================
# Packages
# =============================================================================

if ("${DEV_MODE}")
    # The condition prevents execution of this section during regular user installation.
    # It created files useless to normal users and requires additional tools (git, xz).
    message(STATUS "Packaging tools enabled.")

    #set(PACKAGE_NAME "${MAIN_PROJECT_NAME_LOWER}")
    set(PACKAGE_VERSION "${PROJECT_VERSION}")
    set(PACKAGE_RELEASE "2")
    set(PACKAGE_SOURCE_ARCHIVE_FILE "${PACKAGE_NAME}_${PACKAGE_VERSION}.orig.tar.xz")
    set(PACKAGE_SOURCE_ARCHIVE_PATH "${PACKAGE_OUTPUT_PATH}/${PACKAGE_SOURCE_ARCHIVE_FILE}")
    set(PACKAGE_TOPLEVEL_DIR "${PACKAGE_NAME}-${PACKAGE_VERSION}")
    set(PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
    set(PACKAGE_LONG_DESCRIPTION "${LONG_DESCRIPTION}")
    set(PACKAGE_MAINTAINER "${JAKUB_VANEK}")
    set(PACKAGE_URL "${PROJECT_HOMEPAGE_URL}")
    set(PACKAGE_GIT "${PROJECT_HOMEPAGE_URL}")
    set(PACKAGE_LICENCE "${LICENCE}")

    include(cmake/PackageTools.cmake)

    # Inject up-to-date information into package config files.
    package_config_file(appimage appimage.yml extras/packaging/appimage/appimage.yml.in)
    package_config_file(archlinux PKGBUILD extras/packaging/arch/PKGBUILD.in)
    package_config_file(rpm ${PACKAGE_NAME}.spec extras/packaging/rpm/spec.in)
    # Debian uses whole directory which has to be saved to archive and shipped.
    package_debian_quilt(deb
            ${PACKAGE_NAME}_${PACKAGE_VERSION}-${PACKAGE_RELEASE}.dsc
            extras/packaging/deb/dsc.in
            extras/packaging/deb/debian
            ${PACKAGE_NAME}_${PACKAGE_VERSION}-${PACKAGE_RELEASE}.debian.tar.xz)
    # Creates bunch of files in ${CMAKE_BINARY_DIR}/target/pkg that you can just pass to
    # Open Build Service and it will build all packaging.
    # TODO: Currently changelog is not handled automatically.
    add_custom_target(open_build_service_bundle
            DEPENDS ${PACKAGE_SOURCE_ARCHIVE_FILE} appimage archlinux deb rpm
            WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/pkg)
endif ()
