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

QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundation.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoUtil.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoXML.lib

win32 {
    QMAKE_CXXFLAGS -= -Zc:wchar_t-
    LIBS += -L"$$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/lib" -L"$$_PRO_FILE_PWD_/../lib"

    CONFIG(debug, debug|release) {
            LIBS += -lPocoFoundation64d -lPocoUtil64d lPocoXML64d -lCVUtild -lphoto_utild -ldem_interpolated -lsqlite3_i -lspatialite_i -lgdal_i
            TARGET = $$join(TARGET,,,d)
    } else {
            LIBS += -lPocoFoundation64 -lPocoUtil64 -lPocoXML64 -lCVUtil -lphoto_util -lspatialite_i -lsqlite3_i -lgdal_i -ldem_interpolate
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
    $$_PRO_FILE_PWD_/../common/geo_util.cpp \
    $$_PRO_FILE_PWD_/../common/doc_book_util.cpp


HEADERS +=\
    check_ta.h

DESTDIR = $$_PRO_FILE_PWD_/../bin
