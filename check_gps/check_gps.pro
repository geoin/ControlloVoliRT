#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

#QT       -= core gui
#TARGET = check_gps

QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundation.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoUtil.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoXML.lib

win32 {
        QMAKE_CXXFLAGS -= -Zc:wchar_t-
        LIBS += -L"$$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/lib" -L"$$_PRO_FILE_PWD_/../lib"

        CONFIG(debug, debug|release) {
                LIBS += -lPocoFoundation64d -lPocoUtil64d lPocoXML64d -lPocoZip64d -lphoto_utild -lZipLibd -lrtklibd -lspatialite4 -lsqlite3_i -lCVUtild -lgdald_i
                TARGET = $$join(TARGET,,,d)
        }
        else {
                LIBS += -lPocoFoundation64 -lPocoUtil64 -lPocoXML64 -lZipLib -lphoto_util -lrtklib -lspatialite_i -lsqlite3_i -lCVUtil -lgdal_i -lproj_i -lgeos_c
        }
}

unix: {
    LIBS += -L"../lib"
    CONFIG(debug, debug|release) {
            LIBS += -lPocoUtil -lPocoZip -lphoto_util -lziplib -lrtklib -lCVUtil -lspatialite -lsqlite3 -lgdal
            TARGET = $$join(TARGET,,,d)
    }
    else {
            LIBS += -lPocoFoundation -lPocoXML -lPocoUtil -lPocoZip -lrtklib -lphoto_util -lziplib -lCVUtil -lspatialite -lsqlite3 -lgdal
    }
}

INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include ../include


DEFINES += DLL_EXPORTS DEMINTERPOLATE_LIBRARY NOMINMAX

SOURCES += \
    check_gps.cpp \
    gps_exec.cpp \
    rinex_post.cpp

HEADERS +=\
    check_gps.h \
    gps.h

DESTDIR = $$_PRO_FILE_PWD_/../bin
