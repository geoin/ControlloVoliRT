#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG += sharedlib
QT       -= core gui
TARGET = dem_interpolate
TEMPLATE = lib

win32 {
        LIBS += -L"C:/ControlloVoliRT_Tools/lib"
        DEFINES += DLL_EXPORTS
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

win32 {
        INCLUDEPATH += C:/ControlloVoliRT_Tools/include C:/ControlloVoliRT/include
}
macx {
        INCLUDEPATH += /Users/andrea/SwTools/include /Users/andrea/ControlloVoliRT/include
}

DEFINES +=DEMINTERPOLATE_LIBRARY

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
