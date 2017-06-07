TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundation.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoUtil.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoXML.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundationd.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoUtild.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoXMLd.lib

win32 {
    QMAKE_CXXFLAGS -= -Zc:wchar_t-
    LIBS += -L"$$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/lib" -L"$$_PRO_FILE_PWD_/../lib"
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include $$_PRO_FILE_PWD_/../include
    CONFIG(debug, debug|release) {
        LIBS += -lPocoFoundation64d -lPocoUtil64d -lPocoXML64d -lphoto_utild -ldem_interpolated -lsqlite3_i -lspatialite_i -lCVutild -lgdal_i -llibtiff_i -lproj_i
        TARGET = $$join(TARGET,,,d)
    }
    else {
         LIBS += -lPocoFoundation64 -lPocoUtil64 -lPocoXML64 -lphoto_util -ldem_interpolate -lsqlite3_i -lspatialite_i -lCVutil -lgdal_i -llibtiff_i -lproj_i
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
    $$_PRO_FILE_PWD_/../common/doc_book_util.cpp \
    $$_PRO_FILE_PWD_/../common/geo_util.cpp \
    $$_PRO_FILE_PWD_/../common/sampler.cpp \
    $$_PRO_FILE_PWD_/../common/lidar.cpp

HEADERS += \
    check_lidar_final.h \
    lidar_final_exec.h \
    $$_PRO_FILE_PWD_/../include/cv/lidar.h

DESTDIR = $$_PRO_FILE_PWD_/../bin
