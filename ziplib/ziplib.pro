#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG += sharedlib
QT       -= core gui

TARGET = ziplib
TEMPLATE = lib

# win32:LIBS += -L"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Lib" -lWs2_32 -lwinmm
LIBS += -L"C:\ControlloVoliRT_Tools\lib" -lPocoFoundationd -lPocoZipd

INCLUDEPATH = C:/ControlloVoliRT_Tools/include C:/ControlloVoliRT/include

DEFINES +=ZIPLIB_LIBRARY

SOURCES += \
    gzip.c \
    unlzw.c \
    util.c \
    ziplib.cpp

HEADERS +=\
    getopt.h \
    gzip.h \
    lzw.h \
    revision.h \
    tailor.h \
    ziplib.h

DESTDIR = ../lib

incl.path = ../include/ziplib
incl.files = ../ziplib/ziplib.h
INSTALLS += incl
