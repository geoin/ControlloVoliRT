#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG += dll
QT       -= core gui qt
TARGET = dem_interpolate
TEMPLATE = lib


INCLUDEPATH += $$_PRO_FILE_PWD_/../include

win32 {
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include
    QMAKE_CFLAGS+= /ZI
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundation.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundationd.lib
    LIBS += -L"$$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/lib"
    LIBS += -llaslib
    DEFINES += DLL_EXPORTS
}

unix{
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include
    LIBS += -L"$$_PRO_FILE_PWD_/../../ControlloVoliRT/lib"
    QMAKE_CXXFLAGS += -fPIC
}

macx {
        LIBS += -L"/opt/local/lib"
        LIBS += -llas
        INCLUDEPATH += "/opt/local/include"
}



CONFIG(debug, debug|release) {
        win32: LIBS += -lPocoFoundation64d
        macx:  LIBS += -lPocoFoundationd
        TARGET = $$join(TARGET,,,d)
}
else {
    win32: LIBS += -lPocoFoundation64
    macx: LIBS += -lPocoFoundation
}

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


DESTDIR = $$_PRO_FILE_PWD_/../lib

incl.path = $$_PRO_FILE_PWD_/../include/dem_interpolate
incl.files = $$_PRO_FILE_PWD_/../dem_interpolate/dsm.h
INSTALLS += incl

