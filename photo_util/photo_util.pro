#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG += sharedlib
QT       -= core gui
TARGET = photo_util
TEMPLATE = lib

win32 {
        INCLUDEPATH += C:/ControlloVoliRT_Tools/include C:/ControlloVoliRT/include
}
macx {
        INCLUDEPATH += /Users/andrea/SwTools/include /Users/andrea/ControlloVoliRT/include
}

CONFIG(debug, debug|release) {
        TARGET = $$join(TARGET,,,d)
}

DEFINES +=PHOTOUTIL_LIBRARY

SOURCES += \
    vdp.cpp \
    sun.cpp

HEADERS +=\
    vdp.h \
    sun.h

DESTDIR = ../lib

incl.path = ../include/photo_util
incl.files = ../photo_util/sun.h ../photo_util/vdp.h
INSTALLS += incl
