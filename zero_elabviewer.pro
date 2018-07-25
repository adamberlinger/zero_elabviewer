QT += core gui serialport qml
CONFIG += serialport
CONFIG += c++11
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport
TEMPLATE = app
TARGET = zero_elabviewer
DEPENDPATH += src
INCLUDEPATH += src src/DataUtils src/FunctionWidgets src/HelperWidgets
LIBS+=-lfftw3
RESOURCES = resources/resources.qrc

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
binfile.path=/usr/bin

datafiles.files=data/*
datafiles.path=/usr/share/zero_elabviewer

INSTALLS += binfile datafiles
