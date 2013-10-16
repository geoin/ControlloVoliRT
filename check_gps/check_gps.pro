#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
#CONFIG += sharedlib
QT       -= core gui
TARGET = check_gps
TEMPLATE = app

win32 {
        LIBS += -L"C:/ControlloVoliRT_Tools/lib" -L"C:/ControlloVoliRT/lib"
}
macx {
        LIBS += -L"/Users/andrea/SwTools/lib" -L"/Users/andrea/ControlloVoliRT/lib"
}

CONFIG(debug, debug|release) {
        LIBS += -lPocoFoundationd -lPocoUtild -lrtklibd -lziplibd
        TARGET = $$join(TARGET,,,d)
}
else {
        LIBS += -lPocoFoundation -lPocoUtil
}

win32 {
        INCLUDEPATH += C:/ControlloVoliRT_Tools/include C:/ControlloVoliRT/include
}
macx {
        INCLUDEPATH += /Users/andrea/SwTools/include /Users/andrea/ControlloVoliRT/include
}

DEFINES +=DEMINTERPOLATE_LIBRARY

SOURCES += \
    check_gps.cpp \
    gps_exec.cpp \
    rinex_post.cpp

HEADERS +=\
    check_gps.h \
    gps.h

DESTDIR = ../bin
