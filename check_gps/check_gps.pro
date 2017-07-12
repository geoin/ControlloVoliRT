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

        CONFIG(debug, debug|release) {
                LIBS += -lPocoFoundation64d -lPocoUtil64d -lPocoXML64d -lZipLibd -lphoto_utild -lrtklibd -lCVUtild -lspatialite_i -lsqlite3_i  -lgdal_i -lproj_i -lgeos_c
                TARGET = $$join(TARGET,,,d)
        }
        else {
                LIBS += -lPocoFoundation64 -lPocoUtil64 -lPocoXML64 -lZipLib -lphoto_util -lrtklib lCVUtil -lspatialite_i -lsqlite3_i - -lgdal_i -lproj_i -lgeos_c
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
