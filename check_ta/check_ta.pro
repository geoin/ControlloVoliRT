#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG -= qt
CONFIG += console
QT       -= core gui
TARGET = check_ta
#TEMPLATE = app

win32 {
    QMAKE_CXXFLAGS -= -Zc:wchar_t-
    LIBS += -L"../../ControlloVoliRT_Tools/lib" -L"../lib"

    CONFIG(debug, debug|release) {
            LIBS += -lPocoFoundationd -lPocoUtild -lphoto_utild -lsqlite3_i -lspatialite4
            TARGET = $$join(TARGET,,,d)
    } else {
            LIBS += -lPocoFoundation -lPocoUtil -lphoto_utild -lspatialite4
    }
}

unix {
    CONFIG(debug, debug|release) {
            LIBS += -lPocoFoundationd -lPocoUtild -lPocoXMLd -lCVUtild -lphoto_utild -lsqlite3 -lspatialite-lgdal
            TARGET = $$join(TARGET,,,d)
    } else {
            LIBS += -lPocoFoundation -lPocoUtil -lPocoXML -lCVUtil -lphoto_util -lspatialite -lsqlite3 -lgdal
    }
}

INCLUDEPATH += ../../ControlloVoliRT_Tools/include ../include

DEFINES += DLL_EXPORTS DEMINTERPOLATE_LIBRARY

SOURCES += \
    check_ta.cpp \
    ta_exec.cpp \
    ../common/geo_util.cpp \
    ../common/doc_book_util.cpp


HEADERS +=\
    check_ta.h

DESTDIR = ../bin
