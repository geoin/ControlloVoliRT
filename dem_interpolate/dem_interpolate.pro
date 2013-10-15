#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG += sharedlib
QT       -= core gui

TARGET = dem_interpolate
TEMPLATE = lib

# win32:LIBS += -L"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Lib" -lWs2_32 -lwinmm
#LIBS += -L"C:/ControlloVoliRT_Tools/lib" -lPocoFoundationd -lPocoZipd

INCLUDEPATH = C:/ControlloVoliRT_Tools/include C:/ControlloVoliRT/include

DEFINES +=DEMINTERPOLATE_LIBRARY

SOURCES += \
    geom.cpp \
    grid.cpp \
    triangle.c \

HEADERS +=\
    pslg.h \
    dsm.h \
    geom.h \
    triangle.h \
    txtfile.h

DESTDIR = ../lib

incl.path = ../include/tools
incl.files = ../dem_interpolate/geom.h ../dem_interpolate/dsm.h
INSTALLS += incl
