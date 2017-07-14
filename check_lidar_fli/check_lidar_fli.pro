CONFIG -= qt
CONFIG += console
QT       -= core gui
TARGET = check_lidar

INCLUDEPATH += $$_PRO_FILE_PWD_/../include
LIBS += -L"$$_PRO_FILE_PWD_/../lib"

win32 {
    OSGEO4_DIR = "D:/OSGeo4W64"
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundation.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoUtil.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoXML.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundationd.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoUtild.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoXMLd.lib
    QMAKE_CXXFLAGS -= -Zc:wchar_t-

    LIBS += -L"$$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/lib"
    LIBS += -L"$$OSGEO4_DIR/lib"
    INCLUDEPATH += ../../ControlloVoliRT_Tools/include
    INCLUDEPATH += $$OSGEO4_DIR/include
    DEFINES += NOMINMAX
}

macx {
    LIBS += -L"/opt/local/lib"
    INCLUDEPATH += /opt/local/include
}

unix {
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include
}

CONFIG(debug, debug|release) {
    win32:LIBS += -lPocoUtil64d -lPocoFoundation64d -lPocoXML64d -lphoto_utild -ldem_interpolated -lCVUtild -lspatialite_i -lsqlite3_i -lgdal_i -llibtiff_i
#            unix: LIBS += -lPocoUtil -lPocoFoundation -lPocoXML -lphoto_util -ldem_interpolate -lCVUtil -lspatialite -lsqlite3 -lgdal
    macx: LIBS += -lPocoUtild -lPocoFoundationd -lPocoXMLd -lphoto_utild -ldem_interpolated -lCVUtild -lspatialite -lsqlite3 -lgdal
    TARGET = $$join(TARGET,,,d)
}
else {
    win32: LIBS += -lPocoUtil64 -lPocoFoundation64 -lPocoXML64 -lphoto_util -ldem_interpolate -lCVUtil -lspatialite_i -lsqlite3_i -lgdal_i  -llibtiff_i
#            unix: LIBS += -lPocoUtil -lPocoFoundation -lPocoXML -lphoto_util -ldem_interpolate -lCVUtil -lspatialite -lsqlite3 -lgdal
    macx: LIBS += -lPocoUtil -lPocoFoundation -lPocoXML -lphoto_util -ldem_interpolate -lCVUtil -lspatialite -lsqlite3 -lgdal
}

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
