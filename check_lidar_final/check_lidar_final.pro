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
    QMAKE_CXXFLAGS += -std=c++11
    INCLUDEPATH += ../include
    CONFIG(debug, debug|release) {
        LIBS += -lPocoFoundation -lPocoUtil -lPocoXML -lphoto_util -ldem_interpolate -lCVUtil -lsqlite3 -lspatialite -lgdal -ltiff -lproj
        TARGET = $$join(TARGET,,,d)
    }
    else {
        LIBS += -lPocoFoundation -lPocoUtil -lPocoXML -lphoto_util -ldem_interpolate -lCVUtil -lsqlite3 -lspatialite  -lgdal -ltiff -lproj
    }
}

DEFINES += DLL_EXPORTS DEMINTERPOLATE_LIBRARY NOMINMAX

SOURCES += \
    check_lidar_final.cpp \
    lidar_final_exec.cpp \
    ../common/doc_book_util.cpp \
    ../common/geo_util.cpp \
    ../common/lidar.cpp

HEADERS += \
    check_lidar_final.h \
    lidar_final_exec.h \
    ../include/cv/lidar.h

DESTDIR = ../bin
