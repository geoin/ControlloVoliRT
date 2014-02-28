#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------

QT       -= core gui
TARGET = photo_util
TEMPLATE = lib

DEFINES += DLL_EXPORTS

SOURCES += \
    vdp.cpp \
    sun.cpp

HEADERS +=\
    vdp.h \
    sun.h

INCLUDEPATH += $$_PRO_FILE_PWD_/../include

win32 {
        INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include
        CONFIG += wd4996
}
macx {
        INCLUDEPATH += /Users/andrea/SwTools/include /Users/andrea/ControlloVoliRT/include
}

CONFIG(debug, debug|release) {
        TARGET = $$join(TARGET,,,d)
}

DESTDIR = $$_PRO_FILE_PWD_/../lib

incl.path = ../include/photo_util
incl.files = ../photo_util/sun.h ../photo_util/vdp.h
INSTALLS += incl
