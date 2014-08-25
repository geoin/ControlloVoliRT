TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

win32 {
    QMAKE_CXXFLAGS -= -Zc:wchar_t-
    LIBS += -L"../../ControlloVoliRT_Tools/lib" -L"../lib"
    INCLUDEPATH += ../../ControlloVoliRT_Tools/include ../include
    CONFIG(debug, debug|release) {
        LIBS += -lPocoFoundationd -lPocoUtild -lphoto_utild -ldem_interpolated -lsqlite3_i -lspatialite4 -lCVutild -lgdald_i -llibtiff_i
        TARGET = $$join(TARGET,,,d)
    }
    else {
         LIBS += -lPocoFoundation -lPocoUtil -lphoto_util -ldem_interpolate -lsqlite3_i -lspatialite4 -lCVutil -lgdal_i -llibtiff_i
    }
}

unix {
    INCLUDEPATH += ../include
    CONFIG(debug, debug|release) {
        LIBS += -lPocoFoundation -lPocoUtil -lPocoXML -lphoto_util -ldem_interpolate -lCVUtil -lsqlite3 -lspatialite -lgdal -ltiff
        TARGET = $$join(TARGET,,,d)
    }
    else {
        LIBS += -lPocoFoundation -lPocoUtil -lPocoXML -lphoto_util -ldem_interpolate -lCVUtil -lsqlite3 -lspatialite  -lgdal -ltiff
    }
}

DEFINES += DLL_EXPORTS DEMINTERPOLATE_LIBRARY NOMINMAX

SOURCES += \
    check_lidar_raw.cpp \
    lidar_raw_exec.cpp \
    ../common/lidar.cpp \
    ../common/geo_util.cpp

HEADERS += \
    check_lidar_raw.h \
    lidar_raw_exec.h

DESTDIR = ../bin
