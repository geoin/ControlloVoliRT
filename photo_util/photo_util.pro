#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG += sharedlib
QT       -= core gui

release {
    TARGET = photo_util
} else {
    TARGET = photo_util
}
TEMPLATE = lib

#LIBS += -L"C:/ControlloVoliRT_Tools/lib" -lPocoFoundationd

INCLUDEPATH = C:/ControlloVoliRT_Tools/include C:/ControlloVoliRT/include

DEFINES +=PHOTOUTIL_LIBRARY

SOURCES += \
    vdp.cpp \
    sun.cpp

HEADERS +=\
    vdp.h \
    sun.h

DESTDIR = ../lib

incl.path = ../include/tools
incl.files = ../photo_util/sun.h ../photo_util/vdp.h
INSTALLS += incl
