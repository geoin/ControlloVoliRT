#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG += sharedlib
QT       -= core gui

TARGET = rtklib
TEMPLATE = lib

#win32:LIBS += -lWs2_32 -lwinmm

INCLUDEPATH += $$_PRO_FILE_PWD_/../include
    CONFIG(debug, debug|release) {
        TARGET = $$join(TARGET,,,d)
    }

win32 {
    DEFINES += DLL_EXPORTS RTKLIB_LIBRARY
    DEFINES -= UNICODE
    QMAKE_CXXFLAGS -= -Zc:wchar_t-
    QMAKE_CFLAGS -= -Zc:wchar_t-
    LIBS += -lWs2_32 -lwinmm
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include

}
macx {
    INCLUDEPATH += /opt/local/include
}

unix {
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include
}


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

DESTDIR = $$_PRO_FILE_PWD_/../lib

incl.path = $$_PRO_FILE_PWD_/../include/rtklib
incl.files = $$_PRO_FILE_PWD_/../rtklib/rtklib.h
INSTALLS += incl
