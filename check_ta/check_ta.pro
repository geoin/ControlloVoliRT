#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG -= qt
CONFIG += console
QT       -= core gui
TARGET = check_ta
#TEMPLATE = app

win32 {
        QMAKE_CXXFLAGS -= -Zc:wchar_t-
        LIBS += -L"C:/ControlloVoliRT_Tools/lib" -L"C:/ControlloVoliRT/lib"
}
macx {
        LIBS += -L"/Users/andrea/SwTools/lib" -L"/Users/andrea/ControlloVoliRT/lib"
}

CONFIG(debug, debug|release) {
        LIBS += -lPocoFoundationd -lPocoUtild -lphoto_utild -lsqlite3_i -lspatialite4
        TARGET = $$join(TARGET,,,d)
}
else {
        LIBS += -lPocoFoundation -lPocoUtil -lphoto_utild -lspatialite4
}

win32 {
        INCLUDEPATH += C:/ControlloVoliRT_Tools/include C:/ControlloVoliRT/include
}
macx {
        INCLUDEPATH += /Users/andrea/SwTools/include /Users/andrea/ControlloVoliRT/include
}

DEFINES += DLL_EXPORTS DEMINTERPOLATE_LIBRARY

SOURCES += \
    check_ta.cpp \
    ta_exec.cpp

HEADERS +=\
    check_ta.h

DESTDIR = C:/OSGeo4W/apps/qgis/plugins
