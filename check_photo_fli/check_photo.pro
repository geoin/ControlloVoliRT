#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG -= qt
CONFIG += console
QT       -= core gui
TARGET = check_photo
#TEMPLATE = app

win32 {
        QMAKE_CXXFLAGS -= -Zc:wchar_t-
        LIBS += -L"../../ControlloVoliRT_Tools/lib" -L"../lib"
        DEFINES += NOMINMAX
        INCLUDEPATH += ../../ControlloVoliRT_Tools/include ../include

        CONFIG(debug, debug|release) {
                LIBS += -lPocoUtild -lPocoFoundationd -lPocoXMLd -lphoto_util -ldem_interpolate -lCVUtil -lspatialite -lsqlite3_i -lgdal_i
                TARGET = $$join(TARGET,,,d)
        }
        else {
                LIBS += -lPocoUtil -lPocoFoundation -lPocoXML -lphoto_util -ldem_interpolate -lCVUtil -lspatialite -lsqlite3_i -lgdal_i
        }
}
macx {
        LIBS += -L"/Users/andrea/SwTools/lib" -L"/Users/andrea/ControlloVoliRT/lib"
        INCLUDEPATH += /Users/andrea/SwTools/include /Users/andrea/ControlloVoliRT/include
}
unix {
        INCLUDEPATH += ../../ControlloVoliRT_Tools/include ../include
        CONFIG(debug, debug|release) {
                LIBS += -lPocoUtil -lPocoFoundation -lPocoXML -lphoto_util -ldem_interpolate -lCVUtil -lspatialite -lsqlite3 -lgdal
                TARGET = $$join(TARGET,,,d)
        }
        else {
                LIBS += -lPocoUtil -lPocoFoundation -lPocoXML -lphoto_util -ldem_interpolate -lCVUtil -lspatialite -lsqlite3 -lgdal
        }
}

DEFINES += DLL_EXPORTS DEMINTERPOLATE_LIBRARY

SOURCES += \
    check_photo.cpp \
    photo_exec.cpp \
    photo_report.cpp \
    ../common/geo_util.cpp \
    ../common/doc_book_util.cpp

HEADERS +=\
    check_photo.h

DESTDIR = ../bin
