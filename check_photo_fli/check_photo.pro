#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG -= qt
CONFIG += console
QT       -= core gui
TARGET = check_photo
#TEMPLATE = app

win32 {
        QMAKE_CXXFLAGS -= -Zc:wchar_t-
        LIBS += -L"C:/ControlloVoliRT_Tools/lib" -L"C:/ControlloVoliRT/lib"
}
macx {
        LIBS += -L"/Users/andrea/SwTools/lib" -L"/Users/andrea/ControlloVoliRT/lib"
}

LIBS += -lspatialite_i
CONFIG(debug, debug|release) {
        LIBS += -lPocoUtild -lphoto_utild -ldem_interpolated -lspatialite4 -lsqlite3_i -lCVutild -lgdald_i
        TARGET = $$join(TARGET,,,d)
}
else {
        LIBS += -lPocoUtil -lphoto_utild -ldem_interpolate -lspatialite4 -lsqlite3_i -lCVutil -lgdal_i
}

win32 {
        INCLUDEPATH += C:/ControlloVoliRT_Tools/include C:/ControlloVoliRT/include
}
macx {
        INCLUDEPATH += /Users/andrea/SwTools/include /Users/andrea/ControlloVoliRT/include
}

DEFINES += DLL_EXPORTS DEMINTERPOLATE_LIBRARY

SOURCES += \
    check_photo.cpp \
    photo_exec.cpp

HEADERS +=\
    check_photo.h

DESTDIR = ../bin
