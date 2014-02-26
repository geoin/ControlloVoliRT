#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG -= qt
CONFIG += console
QT       -= core gui
TARGET = check_lidar
#TEMPLATE = app

win32 {
    QMAKE_CXXFLAGS -= -Zc:wchar_t-
    LIBS += -L"../../ControlloVoliRT_Tools/lib" -L"../lib"
    INCLUDEPATH += ../../ControlloVoliRT_Tools/include ../include
    LIBS += -lspatialite_i
    CONFIG(debug, debug|release) {
            LIBS += -lPocoUtild -lphoto_utild -lsqlite3_i
            TARGET = $$join(TARGET,,,d)
    }
    else {
            LIBS += -lPocoUtil -lphoto_utild
    }
}

macx {
    LIBS += -L"/Users/andrea/SwTools/lib" -L"/Users/andrea/ControlloVoliRT/lib"
    INCLUDEPATH += /Users/andrea/SwTools/include /Users/andrea/ControlloVoliRT/include
    LIBS += -lspatialite_i
    CONFIG(debug, debug|release) {
            LIBS += -lPocoUtild -lphoto_utild -lsqlite3_i
            TARGET = $$join(TARGET,,,d)
    }
    else {
            LIBS += -lPocoUtil -lphoto_utild
    }
}

unix {
    INCLUDEPATH += ../../ControlloVoliRT_Tools/include ../include
    CONFIG(debug, debug|release) {
        LIBS += -lPocoUtild -lphoto_utild -CVUtild -lsqlite3 -lspatialite
        TARGET = $$join(TARGET,,,d)
    }
    else {
        LIBS += -lPocoUtil -lphoto_util -CVUtil -lsqlite3 -lspatialite
    }
}

DEFINES += DLL_EXPORTS DEMINTERPOLATE_LIBRARY NOMINMAX

SOURCES += \
    check_lidar.cpp \
    lidar_exec.cpp

HEADERS +=\
    check_lidar.h

DESTDIR = ../bin
