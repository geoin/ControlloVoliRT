#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG += staticlib
QT       -= core gui
TARGET = dem_interpolate
TEMPLATE = lib

win32 {
        LIBS += -L"../../ControlloVoliRT_Tools/lib"
        DEFINES += DLL_EXPORTS
}
linux{
    LIBS += -L"/usr/local/lib"
}
macx {
        LIBS += -L"/Users/andrea/SwTools/lib"
}

CONFIG(debug, debug|release) {
        LIBS += -lPocoFoundationd -lPocoZipd -llaslibd
        TARGET = $$join(TARGET,,,d)
}
else {
        LIBS += -lPocoFoundation -lPocoZip -llaslib
}


INCLUDEPATH += ../../ControlloVoliRT_Tools/include ../include

macx {
        INCLUDEPATH += /Users/andrea/SwTools/include /Users/andrea/ControlloVoliRT/include
}

DEFINES += DEMINTERPOLATE_LIBRARY ANSI_DECLARATORS

SOURCES += \
    geom.cpp \
    grid.cpp \
    triangle.c \
    ../common/parser.cpp

HEADERS +=\
    pslg.h \
    dsm.h \
    geom.h \
    triangle.h


DESTDIR = ../lib

incl.path = ../include/dem_interpolate
incl.files = ../dem_interpolate/geom.h ../dem_interpolate/dsm.h
INSTALLS += incl
