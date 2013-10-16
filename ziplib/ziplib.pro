#-------------------------------------------------
#
# Project created by QtCreator 2013-10-10T17:26:24
#
#-------------------------------------------------
CONFIG += sharedlib
QT       -= core gui

TARGET = ziplib
TEMPLATE = lib

win32 {
        LIBS += -L"C:/ControlloVoliRT_Tools/lib"
}
macx {
        LIBS += -L"/Users/andrea/SwTools/lib"
}


CONFIG(debug, debug|release) {
        LIBS += -lPocoFoundationd -lPocoZipd
        TARGET = $$join(TARGET,,,d)
}
else {
        LIBS += -lPocoFoundation -lPocoZip
}


win32 {
        INCLUDEPATH += C:/ControlloVoliRT_Tools/include C:/ControlloVoliRT/include
}
macx {
        INCLUDEPATH += /Users/andrea/SwTools/include /Users/andrea/ControlloVoliRT/include
}

DEFINES +=ZIPLIB_LIBRARY

SOURCES += \
    gzip.c \
    unlzw.c \
    util.c \
    ziplib.cpp

HEADERS +=\
    getopt.h \
    gzip.h \
    lzw.h \
    revision.h \
    tailor.h \
    ziplib.h

DESTDIR = ../lib

incl.path = ../include/ziplib
incl.files = ../ziplib/ziplib.h
INSTALLS += incl
