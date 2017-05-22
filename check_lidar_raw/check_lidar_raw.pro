TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundation.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoUtil.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoXML.lib

win32 {
    QMAKE_CXXFLAGS -= -Zc:wchar_t-
    LIBS += -L"$$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/lib" -L"$$_PRO_FILE_PWD_/../lib"
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include ../include
    CONFIG(debug, debug|release) {
        LIBS += -lPocoFoundation64d -lPocoUtil64d -lphoto_utild -ldem_interpolated -lsqlite3_i -lspatialite_i -lCVutild -lgdal_i -llibtiff_i
        TARGET = $$join(TARGET,,,d)
    }
    else {
         LIBS += -lPocoFoundation64 -lPocoUtil64 -lPocoXML64 -lphoto_util -ldem_interpolate -lsqlite3_i -lspatialite_i -lCVutil -lgdal_i -llibtiff_i
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
    $$_PRO_FILE_PWD_/../common/lidar.cpp \
    $$_PRO_FILE_PWD_/../common/geo_util.cpp \
    $$_PRO_FILE_PWD_/../common/doc_book_util.cpp

HEADERS += \
    check_lidar_raw.h \
    lidar_raw_exec.h

DESTDIR = $$_PRO_FILE_PWD_/../bin
