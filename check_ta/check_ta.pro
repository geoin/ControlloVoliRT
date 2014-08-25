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
            LIBS += -lPocoFoundationd -lPocoUtild -lCVUtild -lphoto_utild -lsqlite3_i -lspatialite4 -lgdald_i
            TARGET = $$join(TARGET,,,d)
    } else {
            LIBS += -lPocoFoundation -lPocoUtil -lCVUtil -lphoto_utild -lspatialite4 -lsqlite3_i -lgdal_i
    }
}

unix {
    LIBS += -L$$_PRO_FILE_PWD_/../lib
    CONFIG(debug, debug|release) {
            LIBS += -lPocoFoundation -lPocoUtil -lPocoXML -lCVUtil -lphoto_util -lsqlite3 -lspatialite -lgdal
            TARGET = $$join(TARGET,,,d)
    } else {
            LIBS += -lPocoFoundation -lPocoUtil -lPocoXML -lCVUtil -lphoto_util -lspatialite -lsqlite3 -lgdal
    }
}

INCLUDEPATH += ../../ControlloVoliRT_Tools/include ../include

DEFINES += DLL_EXPORTS DEMINTERPOLATE_LIBRARY NOMINMAX

SOURCES += \
    check_ta.cpp \
    ta_exec.cpp \
    ../common/geo_util.cpp \
    ../common/doc_book_util.cpp


HEADERS +=\
    check_ta.h

DESTDIR = ../bin
