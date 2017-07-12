CONFIG -= qt
CONFIG += console
QT       -= core gui
TARGET = check_ta

OSGEO4_DIR = "D:/OSGeo4W64"

win32 {
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundation.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoUtil.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoXML.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundationd.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoUtild.lib
    QMAKE_LFLAGS+=/NODEFAULTLIB:PocoXMLd.lib
    QMAKE_CXXFLAGS -= -Zc:wchar_t-

    LIBS += -L"$$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/lib" -L"$$_PRO_FILE_PWD_/../lib"
    LIBS += -L"$$OSGEO4_DIR/lib"
    INCLUDEPATH += $$OSGEO4_DIR/include

    CONFIG(debug, debug|release) {
            LIBS += -lPocoFoundation64d -lPocoUtil64d -lPocoXML64d -lCVUtild -lphoto_utild -ldem_interpolated -lsqlite3_i -lspatialite_i -lgdal_i
            TARGET = $$join(TARGET,,,d)
    } else {
            LIBS += -lPocoFoundation64 -lPocoUtil64 -lPocoXML64 -lCVUtil -lphoto_util -ldem_interpolate -lsqlite3_i -lspatialite_i -lgdal_i
    }
}

unix {
    LIBS += -L$$_PRO_FILE_PWD_/../lib
    CONFIG(debug, debug|release) {
            LIBS += -lPocoFoundation -lPocoUtil -lPocoXML -lCVUtil -lphoto_util -lsqlite3 -lspatialite -lgdal
            TARGET = $$join(TARGET,,,d)
    } else {
            LIBS += -lPocoFoundation -lPocoUtil -lPocoXML -lCVUtil -lphoto_util -lspatialite -lsqlite3 -lgdal
    }
}

INCLUDEPATH += ../../ControlloVoliRT_Tools/include $$_PRO_FILE_PWD_/../include

DEFINES += DLL_EXPORTS DEMINTERPOLATE_LIBRARY NOMINMAX

SOURCES += \
    check_ta.cpp \
    ta_exec.cpp \
    $$_PRO_FILE_PWD_/../common/geo_util.cpp \
    $$_PRO_FILE_PWD_/../common/doc_book_util.cpp


HEADERS +=\
    check_ta.h

DESTDIR = $$_PRO_FILE_PWD_/../bin
