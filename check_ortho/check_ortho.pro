#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG -= qt
CONFIG += console
QT       -= core gui
TARGET = check_ortho
#TEMPLATE = app

win32 {
        QMAKE_CXXFLAGS -= -Zc:wchar_t-
        LIBS += -L"C:/ControlloVoliRT_Tools/lib" -L"C:/ControlloVoliRT/lib" -L"C:/OSGeo4W/lib/"
}
macx {
        LIBS += -L"/Users/andrea/SwTools/lib" -L"/Users/andrea/ControlloVoliRT/lib"
}

#LIBS += -lspatialite_i
CONFIG(debug, debug|release) {
        LIBS += -lPocoFoundationd -lPocoUtild -lphoto_utild -ldem_interpolated -lsqlite3_i -lspatialite4 -lCVutild -lgdald_i -llibtiff_i
        TARGET = $$join(TARGET,,,d)
}
else {
        LIBS += -lPocoFoundation -lPocoUtil -lphoto_util -ldem_interpolate -lsqlite3_i -lspatialite4 -lCVutil -lgdal_i -llibtiff_i
}

win32 {
        INCLUDEPATH += C:/ControlloVoliRT_Tools/include C:/ControlloVoliRT/include
}
macx {
        INCLUDEPATH += /Users/andrea/SwTools/include /Users/andrea/ControlloVoliRT/include
}

DEFINES += DLL_EXPORTS DEMINTERPOLATE_LIBRARY

SOURCES += \
    check_ortho.cpp \
    tiff_util.cpp \
    ortho_exec.cpp

HEADERS +=\
    check_ortho.h \
    tiff_util.h

DESTDIR = C:\OSGeo4W\apps\qgis\plugins
