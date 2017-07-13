#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG += dll
QT       -= core gui
TARGET = ziplib
TEMPLATE = lib


win32 {
    OSGEO4_DIR = "D:/OSGeo4W64"
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundation.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoZip.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundationd.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoZipd.lib
    QMAKE_CFLAGS+= /ZI

    LIBS += -L"$$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/lib"

    DEFINES += DLL_EXPORTS
    DEFINES +=ZIPLIB_LIBRARY
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include
}

INCLUDEPATH += $$_PRO_FILE_PWD_/../include

macx {
        INCLUDEPATH += /opt/local/include
        LIBS += -L"/opt/local/lib"
}

CONFIG(debug, debug|release) {
        win32: LIBS += -lPocoFoundation64d -lPocoZip64d
        macx:  LIBS += -lPocoFoundationd -lPocoZipd
        TARGET = $$join(TARGET,,,d)
}
else {
        win32: LIBS += -lPocoFoundation64 -lPocoZip64
        macx: LIBS += -lPocoFoundation -lPocoZip
}


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

DESTDIR = $$_PRO_FILE_PWD_/../lib

incl.path = $$_PRO_FILE_PWD_/../include/ziplib
incl.files = $$_PRO_FILE_PWD_/../ziplib/ziplib.h
INSTALLS += incl
