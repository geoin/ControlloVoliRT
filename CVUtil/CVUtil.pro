#-------------------------------------------------
#
# Project created by QtCreator 2013-11-03T16:04:43
#
#-------------------------------------------------

QT       -= core gui

TARGET = CVUtil
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += ../include

win32 {
         ../../ControlloVoliRT_Tools/include
}
macx {
        INCLUDEPATH += ../../SwTools/include
        INCLUDEPATH += /opt/local/include
}

CONFIG(debug, debug|release) {
        TARGET = $$join(TARGET,,,d)
}

SOURCES += \
    cvspatialite.cpp \
    ogrgeomptr.cpp

HEADERS += \
    cvspatialite.h \
    ogrgeomptr.h

DESTDIR = ../lib

incl.path = ../include/CVUtil
incl.files += ../CVUtil/cvspatialite.h
incl.files += ../CVUtil/ogrgeomptr.h
INSTALLS += incl

LIBS += -lPocoFoundation

