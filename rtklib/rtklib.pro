#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG += sharedlib
QT       -= core gui

TARGET = rtklib
TEMPLATE = lib

win32:LIBS += -L"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Lib" -lWs2_32 -lwinmm

CONFIG(debug, debug|release) {
        TARGET = $$join(TARGET,,,d)
}

win32 {
        INCLUDEPATH += C:/ControlloVoliRT_Tools/include C:/ControlloVoliRT/include
}
macx {
        INCLUDEPATH += /Users/andrea/SwTools/include /Users/andrea/ControlloVoliRT/include
}

DEFINES += RTKLIB_LIBRARY

SOURCES += \
    ublox.c \
    streamsvr.c \
    stream.c \
    ss2.c \
    solution.c \
    skytraq.c \
    sbas.c \
    rtksvr.c \
    rtkpos.c \
    rtkcmn.c \
    rtcm.c \
    rinex.c \
    rcvraw.c \
    preceph.c \
    ppp.c \
    postpos.c \
    pntpos.c \
    options.c \
    novatel.c \
    lambda.c \
    javad.c \
    ionex.c \
    gw10.c \
    geoid.c \
    ephemeris.c \
    datum.c \
    crx2rnx.c \
    crescent.c \
    convrnx.c \
    convkml.c

HEADERS +=\
    rtklib.h

DESTDIR = ../lib

incl.path = ../include/rtklib
incl.files = ../rtklib/rtklib.h
INSTALLS += incl
