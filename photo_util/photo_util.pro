#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------

QT       -= core gui
TARGET = photo_util
TEMPLATE = lib


SOURCES += \
    vdp.cpp \
    sun.cpp \
    $$_PRO_FILE_PWD_/../common/geom.cpp

HEADERS +=\
    vdp.h \
    sun.h \
    $$_PRO_FILE_PWD_/../common/geom.h

INCLUDEPATH += $$_PRO_FILE_PWD_/../include

win32 {
        DEFINES += DLL_EXPORTS
        INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include
        CONFIG += wd4996
}
macx {
        INCLUDEPATH += /opt/local/include
}

CONFIG(debug, debug|release) {
        TARGET = $$join(TARGET,,,d)
}

DESTDIR = $$_PRO_FILE_PWD_/../lib

incl.path = $$_PRO_FILE_PWD_/../include/photo_util
incl.files = $$_PRO_FILE_PWD_/../photo_util/sun.h $$_PRO_FILE_PWD_/../photo_util/vdp.h
INSTALLS += incl
