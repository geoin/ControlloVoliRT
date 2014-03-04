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
    LIBS += -lspatialite4
    CONFIG(debug, debug|release) {
            LIBS += -lPocoUtild -lphoto_utild -lCVUtild -lsqlite3_i -lgdald_i -ldem_interpolated
            TARGET = $$join(TARGET,,,d)
    }
    else {
            LIBS += -lPocoUtil -lphoto_util -lCVUtil -lsqlite3_i -lgdal_i -ldem_interpolate
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
        LIBS += -lPocoUtil -lphoto_util -lCVUtil -lsqlite3 -lspatialite -lgdal -ldem_interpolate
        TARGET = $$join(TARGET,,,d)
    }
    else {
        LIBS += -lPocoUtil -lphoto_util -lCVUtil -lsqlite3 -lspatialite -lgdal -ldem_interpolate
    }
}

DEFINES += DLL_EXPORTS DEMINTERPOLATE_LIBRARY NOMINMAX

SOURCES += \
    check_lidar.cpp \
    lidar_exec.cpp \
    ../common/geo_util.cpp \
    ../common/doc_book_util.cpp

HEADERS +=\
    check_lidar.h

DESTDIR = ../bin
