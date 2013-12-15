#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG -= qt
CONFIG += console
QT       -= core gui
TARGET = check_gps
#TEMPLATE = app

win32 {
        QMAKE_CXXFLAGS -= -Zc:wchar_t-
        LIBS += -L"C:/ControlloVoliRT_Tools/lib" -L"C:/ControlloVoliRT/lib"
}
macx {
        LIBS += -L"/Users/andrea/SwTools/lib" -L"/Users/andrea/ControlloVoliRT/lib"
}

CONFIG(debug, debug|release) {
        LIBS += -lPocoUtild -l PocoZipd -lphoto_utild -l ZipLibd -lrtklibd -lspatialite4 -lsqlite3_i -lCVUtild -lgdald_i
        TARGET = $$join(TARGET,,,d)
}
else {
        LIBS += -lPocoUtil -lPocoZip -lrtklib -lphoto_util -lziplib -lspatialite4 -lsqlite3_i -lCVUtil -lgdal_i
}

win32 {
        INCLUDEPATH += C:/ControlloVoliRT_Tools/include C:/ControlloVoliRT/include
}
macx {
        INCLUDEPATH += /Users/andrea/SwTools/include /Users/andrea/ControlloVoliRT/include
}

DEFINES += DLL_EXPORTS DEMINTERPOLATE_LIBRARY

SOURCES += \
    check_gps.cpp \
    gps_exec.cpp \
    rinex_post.cpp

HEADERS +=\
    check_gps.h \
    gps.h

DESTDIR = C:/OSGeo4W/apps/qgis/plugins
