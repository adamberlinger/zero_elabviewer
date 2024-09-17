include(env.pro)
QT += core gui serialport qml
CONFIG += serialport
CONFIG += c++11
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport
TEMPLATE = app
TARGET = zero_elabviewer
DEPENDPATH += src
INCLUDEPATH += src src/DataUtils src/FunctionWidgets src/HelperWidgets
LIBS += -lfftw3
RESOURCES = resources/resources.qrc
RC_ICONS = resources/icons/zero_elabviewer.ico

# Input
SOURCES += $$files(src/*.cpp)
SOURCES += $$files(src/DataUtils/*.cpp)
SOURCES += $$files(src/FunctionWidgets/*.cpp)
SOURCES += $$files(src/HelperWidgets/*.cpp)
HEADERS += $$files(src/*.h)
HEADERS += $$files(src/DataUtils/*.h)
HEADERS += $$files(src/FunctionWidgets/*.h)
HEADERS += $$files(src/HelperWidgets/*.h)

DESTDIR = build
OBJECTS_DIR = build
MOC_DIR = build
RCC_DIR = build

binfile.files+=build/zero_elabviewer
binfile.path=$$INSTALL_ROOT/usr/bin

datafiles.files=data/*
datafiles.path=$$INSTALL_ROOT/usr/share/zero_elabviewer

desktopentry.files = extras/launcher/zero_elabviewer.desktop
desktopentry.path  = $$INSTALL_ROOT/usr/share/applications

desktopicon_svg.files = resources/icons/zero_elabviewer.svg
desktopicon_svg.path  = $$INSTALL_ROOT/usr/share/icons/hicolor/scalable/apps

desktopicon_png16.files = resources/icons/16x16/zero_elabviewer.png
desktopicon_png16.path  = $$INSTALL_ROOT/usr/share/icons/hicolor/16x16/apps

desktopicon_png24.files = resources/icons/24x24/zero_elabviewer.png
desktopicon_png24.path  = $$INSTALL_ROOT/usr/share/icons/hicolor/24x24/apps

desktopicon_png32.files = resources/icons/32x32/zero_elabviewer.png
desktopicon_png32.path  = $$INSTALL_ROOT/usr/share/icons/hicolor/32x32/apps

desktopicon_png48.files = resources/icons/48x48/zero_elabviewer.png
desktopicon_png48.path  = $$INSTALL_ROOT/usr/share/icons/hicolor/48x48/apps

desktopicon_png64.files = resources/icons/64x64/zero_elabviewer.png
desktopicon_png64.path  = $$INSTALL_ROOT/usr/share/icons/hicolor/64x64/apps

desktopicon_png128.files = resources/icons/128x128/zero_elabviewer.png
desktopicon_png128.path  = $$INSTALL_ROOT/usr/share/icons/hicolor/128x128/apps

desktopicon_png256.files = resources/icons/256x256/zero_elabviewer.png
desktopicon_png256.path  = $$INSTALL_ROOT/usr/share/icons/hicolor/256x256/apps

udevrule.files = extras/udev/71-zero-elabviewer.rules
udevrule.path  = $$INSTALL_ROOT/lib/udev/rules.d

INSTALLS += binfile datafiles desktopentry udevrule
INSTALLS += desktopicon_svg desktopicon_png16 desktopicon_png24 desktopicon_png32
INSTALLS += desktopicon_png48 desktopicon_png64 desktopicon_png128 desktopicon_png256
