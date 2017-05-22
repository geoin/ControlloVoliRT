#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG -= qt
CONFIG += console
QT       -= core gui
TARGET = check_photo

QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundation.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoUtil.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoXML.lib

win32 {
        QMAKE_CXXFLAGS -= -Zc:wchar_t-
        LIBS += -L"$$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/lib" -L"$$_PRO_FILE_PWD_/../lib"
        DEFINES += NOMINMAX
        INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include $$_PRO_FILE_PWD_/../include

        CONFIG(debug, debug|release) {
                LIBS += -lPocoUtil64d -lPocoFoundation64d -lPocoXML64d -lphoto_utild -ldem_interpolated -lCVUtild -lspatialite_i -lsqlite3_i -lgdal_i
                TARGET = $$join(TARGET,,,d)
        }
        else {
                LIBS += -lPocoUtil64 -lPocoFoundation64 -lPocoXML64 -lphoto_util -ldem_interpolate -lCVUtil -lspatialite_i -lsqlite3_i -lgdal_i
        }
}
macx {
        LIBS += -L"/Users/andrea/SwTools/lib" -L"/Users/andrea/ControlloVoliRT/lib"
        INCLUDEPATH += /Users/andrea/SwTools/include /Users/andrea/ControlloVoliRT/include
}
unix {
        LIBS += -L"../lib"
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
    $$_PRO_FILE_PWD_/../common/geo_util.cpp \
    $$_PRO_FILE_PWD_/../common/doc_book_util.cpp

HEADERS +=\
    check_photo.h

DESTDIR = $$_PRO_FILE_PWD_/../bin
