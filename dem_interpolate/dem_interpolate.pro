#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG += dll
QT       -= core gui qt
TARGET = dem_interpolate
TEMPLATE = lib

win32 {
    QMAKE_CFLAGS+= /ZI
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundation.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundationd.lib
    LIBS += -L"$$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/lib"
    DEFINES += DLL_EXPORTS
}
unix{
    LIBS += -L"../../ControlloVoliRT/lib"
    QMAKE_CXXFLAGS += -fPIC
}
macx {
        LIBS += -L"/Users/andrea/SwTools/lib"
}

CONFIG(debug, debug|release) {
        LIBS += -lPocoFoundation64d -llaslibd
        TARGET = $$join(TARGET,,,d)
}
else {
    LIBS += -lPocoFoundation64 -llaslib
}


INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include \
    $$_PRO_FILE_PWD_/../include/dem_interpolate\
    $$_PRO_FILE_PWD_/../include

DEFINES += DEMINTERPOLATE_LIBRARY ANSI_DECLARATORS NOMINMAX

SOURCES += \
    grid.cpp \
    triangle.cpp \
    $$_PRO_FILE_PWD_/../common/parser.cpp \
    $$_PRO_FILE_PWD_/../common/geom.cpp

HEADERS +=\
    $$_PRO_FILE_PWD_/../include/dem_interpolate/pslg.h \
    $$_PRO_FILE_PWD_/../include/dem_interpolate/dsm.h \
    $$_PRO_FILE_PWD_/../include/common/geom.h \
    $$_PRO_FILE_PWD_/../include/dem_interpolate/triangle.h


#DESTDIR = ../lib

#incl.path = ../include/dem_interpolate
#incl.files = ../dem_interpolate/geom.h ../dem_interpolate/dsm.h
#INSTALLS += incl

DESTDIR = $$_PRO_FILE_PWD_/../bin
