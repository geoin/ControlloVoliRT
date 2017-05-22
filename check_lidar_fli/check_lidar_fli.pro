#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG -= qt
CONFIG += console
QT       -= core gui
TARGET = check_lidar

QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundation.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoUtil.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoXML.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundationd.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoUtild.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoXMLd.lib

win32 {
    QMAKE_CXXFLAGS -= -Zc:wchar_t-
    LIBS += -L"$$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/lib" -L"$$_PRO_FILE_PWD_/../lib"
    INCLUDEPATH += ../../ControlloVoliRT_Tools/include ../include
    CONFIG(debug, debug|release) {
            LIBS += -lPocoFoundation64d -lPocoUtil64d -lPocoXML64d -lphoto_utild -lCVUtild -ldem_interpolated -lsqlite3_i -lgdal_i -lgeos_c -lproj_i -lspatialite_i
            TARGET = $$join(TARGET,,,d)
    }
    else {
            LIBS += -lPocoFoundation64 -lPocoUtil64 -lPocoXML64 -lphoto_util -lCVUtil -lsqlite3_i -lgdal_i -ldem_interpolate -lgeos_c -lproj_i -lspatialite_i
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
        LIBS += -lPocoUtild -lPocoFoundationd -lPocoXMLd -lphoto_util -lCVUtil -lsqlite3 -lspatialite -lgdal -ldem_interpolate
        TARGET = $$join(TARGET,,,d)
    }
    else {
        LIBS += -lPocoUtil -lPocoFoundation -lPocoXML -lphoto_util -lCVUtil -lsqlite3 -lspatialite -lgdal -ldem_interpolate
    }
}

DEFINES += DLL_EXPORTS DEMINTERPOLATE_LIBRARY NOMINMAX

SOURCES += \
    $$_PRO_FILE_PWD_/check_lidar.cpp \
    $$_PRO_FILE_PWD_/lidar_exec.cpp \
    $$_PRO_FILE_PWD_/../common/geo_util.cpp \
    $$_PRO_FILE_PWD_/../common/doc_book_util.cpp\
    $$_PRO_FILE_PWD_/../common/sampler.cpp \
    $$_PRO_FILE_PWD_/../common/lidar.cpp

HEADERS +=\
    check_lidar.h

DESTDIR = $$_PRO_FILE_PWD_/../bin
