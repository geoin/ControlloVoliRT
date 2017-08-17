TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

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
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include
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
    win32:LIBS += -lPocoFoundation64d -lPocoUtil64d -lPocoXML64d -lZipLibd -lphoto_utild -lrtklibd -lCVUtild -lspatialite_i -lsqlite3_i  -lgdal_i -lproj_i -lgeos_c
#            unix: LIBS += -lPocoUtil -lPocoFoundation -lPocoXML -lphoto_util -ldem_interpolate -lCVUtil -lspatialite -lsqlite3 -lgdal
    macx: LIBS += -lPocoUtild -lPocoFoundationd -lPocoXMLd -lphoto_utild -ldem_interpolated -lCVUtild -lspatialite -lsqlite3 -lgdal
    TARGET = $$join(TARGET,,,d)
}
else {
    win32: LIBS += -lPocoFoundation64 -lPocoUtil64 -lPocoXML64 -lZipLib -lphoto_util -lrtklib -lCVutil -lsqlite3_i -lspatialite_i -lgdal_i
#            unix: LIBS += -lPocoUtil -lPocoFoundation -lPocoXML -lphoto_util -ldem_interpolate -lCVUtil -lspatialite -lsqlite3 -lgdal
    macx: LIBS += -lPocoUtil -lPocoFoundation -lPocoXML -lphoto_util -ldem_interpolate -lCVUtil -lspatialite -lsqlite3 -lgdal
}

SOURCES += \
    check_gps.cpp \
    gps_exec.cpp \
    rinex_post.cpp

HEADERS +=\
    check_gps.h \
    gps.h

DESTDIR = $$_PRO_FILE_PWD_/../bin
