#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG -= qt
QT       -= core gui
CONFIG += console
TARGET = check_ortho

QMAKE_LFLAGS+=/NODEFAULTLIB:PocoFoundation.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoUtil.lib
QMAKE_LFLAGS+=/NODEFAULTLIB:PocoXML.lib

win32 {
        QMAKE_CXXFLAGS -= -Zc:wchar_t-
        LIBS += -L"$$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/lib" -L"$$_PRO_FILE_PWD_/../lib" -L"C:/OSGeo4W/lib/"
        INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include $$_PRO_FILE_PWD_/../include $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include/tifflib
        CONFIG(debug, debug|release) {
                LIBS += -lPocoFoundation64d -lPocoUtild -lphoto_utild -ldem_interpolated -lsqlite3_i -lspatialite_i -lCVutild -lgdald_i -llibtiff_i
                TARGET = $$join(TARGET,,,d)
        }
        else {
                LIBS += -lPocoFoundation64 -lPocoUtil -lphoto_util -ldem_interpolate -lsqlite3_i -lspatialite4 -lCVutil -lgdal_i -llibtiff_i
        }
}

unix {
        INCLUDEPATH +=  $$_PRO_FILE_PWD_/../include
        LIBS += -L$$_PRO_FILE_PWD_/../lib
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
    check_ortho.cpp \
    tiff_util.cpp \
    ortho_exec.cpp \
    $$_PRO_FILE_PWD_/../common/parser.cpp \
    $$_PRO_FILE_PWD_/../common/geo_util.cpp \
    $$_PRO_FILE_PWD_/../common/doc_book_util.cpp

HEADERS +=\
    check_ortho.h \
    tiff_util.h

DESTDIR = $$_PRO_FILE_PWD_/../../bin
