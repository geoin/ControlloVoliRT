#-------------------------------------------------
#
# Project created by QtCreator 2013-11-03T16:04:43
#
#-------------------------------------------------

QT       -= core gui

TARGET = CVUtil
TEMPLATE = lib
CONFIG += staticlib

win32 {
        INCLUDEPATH += C:/ControlloVoliRT_Tools/include C:/ControlloVoliRT/include
}
macx {
        INCLUDEPATH += ../include
        INCLUDEPATH += ../../SwTools/include
        INCLUDEPATH += /opt/local/include
}

CONFIG(debug, debug|release) {
        TARGET = $$join(TARGET,,,d)
}

SOURCES += \
    cvspatialite.cpp

HEADERS += \
    cvspatialite.h

DESTDIR = ../lib

incl.path = ../include/CVUtil
incl.files = ../CVUtil/cvspatialite.h
INSTALLS += incl


