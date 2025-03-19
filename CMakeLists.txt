# Generated from zero_elabviewer.pro.

cmake_minimum_required(VERSION 3.16)
project(zero_elabviewer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}")

set(ADAM_BELINGER Adam Berlinger <berlingeradam@gmail.com>)

set(PROJECT_HOMEPAGE_URL "https://github.com/adamberlinger/zero_elabviewer/")
set(GENERIC_NAME "Zero eLab Viewer")
set(LICENCE "GPL-3.0-or-later")
set(LONG_DESCRIPTION
        "Zero eLab Viewer is a PC program for displaying "
        "and controlling external data acquisition system through "
        "standard communication interface such as USB CDC class.")

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
set(MAIN_PROJECT_APPID "cz.cvut.fel.embedded.sdi.data-plotter")
set(MAIN_PROJECT_ORGANIZATION "FEE CTU")
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

find_package(PkgConfig REQUIRED)
pkg_search_module(FFTW REQUIRED fftw3 IMPORTED_TARGET)
include_directories(PkgConfig::FFTW)
link_libraries(PkgConfig::FFTW)

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

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt6_add_resources(rcc_files ${QRC_FILES})
    qt_add_executable(zero_elabviewer MANUAL_FINALIZATION ${PROJECT_SOURCES} ${rcc_files})
else()
    qt5_add_resources(rcc_files ${QRC_FILES})
    add_executable(zero_elabviewer ${PROJECT_SOURCES} ${rcc_files})
endif()

if(QT_VERSION_MAJOR EQUAL 6)
qt_add_qml_module(zero_elabviewer
    URI zero_elabviewer
    VERSION ${PROJECT_VERSION}
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build
)
endif()

target_include_directories(zero_elabviewer PUBLIC
    src
    src/DataUtils
    src/FunctionWidgets
    src/HelperWidgets
)

target_link_libraries(zero_elabviewer PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::SerialPort
    fftw3
)

if((QT_VERSION_MAJOR GREATER 4))
    target_link_libraries(zero_elabviewer PUBLIC
        Qt::PrintSupport
        Qt::Widgets
    )
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DataPlotter)
endif()

install(TARGETS zero_elabviewer
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

# =============================================================================
# Packages
# =============================================================================

if ("${DEV_MODE}")
    # The condition prevents execution of this section during regular user installation.
    # It created files useless to normal users and requires additional tools (git, xz).
    message(STATUS "Packaging tools enabled.")

    string(REPLACE "_" "-" PACKAGE_NAME "${MAIN_PROJECT_NAME_LOWER}")
    #set(PACKAGE_NAME "${MAIN_PROJECT_NAME_LOWER}")
    set(PACKAGE_VERSION "${PROJECT_VERSION}")
    set(PACKAGE_RELEASE "1")
    set(PACKAGE_SOURCE_ARCHIVE_FILE "${PACKAGE_NAME}_${PACKAGE_VERSION}.orig.tar.xz")
    set(PACKAGE_SOURCE_ARCHIVE_PATH "${PACKAGE_OUTPUT_PATH}/${PACKAGE_SOURCE_ARCHIVE_FILE}")
    set(PACKAGE_TOPLEVEL_DIR "${PACKAGE_NAME}-${PACKAGE_VERSION}")
    set(PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
    set(PACKAGE_LONG_DESCRIPTION "${LONG_DESCRIPTION}")
    set(PACKAGE_MAINTAINER "${ADAM_BELINGER}")
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
