#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG += dll
QT       -= core gui
TARGET = ziplib
TEMPLATE = lib

OSGEO4_DIR = "D:/OSGeo4W64"

win32 {
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundation.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoZip.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundationd.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoZipd.lib
    QMAKE_CFLAGS+= /ZI

    LIBS += -L"$$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/lib"
    DEFINES += DLL_EXPORTS
}
macx {
        LIBS += -L"/Users/andrea/SwTools/lib"
}


CONFIG(debug, debug|release) {
        LIBS += -lPocoFoundation64d -lPocoZip64d
        TARGET = $$join(TARGET,,,d)
}
else {
        LIBS += -lPocoFoundation64 -lPocoZip64
}


INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include $$_PRO_FILE_PWD_/../include


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

DESTDIR = $$_PRO_FILE_PWD_/../lib

incl.path = $$_PRO_FILE_PWD_/../include/ziplib
incl.files = $$_PRO_FILE_PWD_/../ziplib/ziplib.h
INSTALLS += incl
