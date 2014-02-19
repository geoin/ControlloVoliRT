#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG += sharedlib
QT       -= core gui qt
TARGET = dem_interpolate
#TEMPLATE = lib

win32 {
        LIBS += -L"../../ControlloVoliRT_Tools/lib"
        DEFINES += DLL_EXPORTS
}
unix{
QMAKE_CXXFLAGS += -fPIC
    LIBS += -L"/usr/lib"
}
macx {
        LIBS += -L"/Users/andrea/SwTools/lib"
}

CONFIG(debug, debug|release) {
        LIBS += -lPocoFoundationd -lPocoZipd -llaslib
        #TARGET = $$join(TARGET,,,d)
}
CONFIG(release, debug|release) {
    LIBS += -lPocoFoundation -lPocoZip -llaslib
}


INCLUDEPATH += ../../ControlloVoliRT_Tools/include \
    ../include/dem_interpolate

DEFINES += DEMINTERPOLATE_LIBRARY ANSI_DECLARATORS

SOURCES += \
    geom.cpp \
    grid.cpp \
    triangle.c \
    ../common/parser.cpp

HEADERS +=\
    ../include/dem_interpolate/pslg.h \
    ../include/dem_interpolate/dsm.h \
    ../include/dem_interpolate/geom.h \
    triangle.h


#DESTDIR = ../lib

#incl.path = ../include/dem_interpolate
#incl.files = ../dem_interpolate/geom.h ../dem_interpolate/dsm.h
#INSTALLS += incl

DESTDIR = ../lib
