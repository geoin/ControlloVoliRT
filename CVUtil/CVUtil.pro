#-------------------------------------------------
#
# Project created by QtCreator 2013-11-03T16:04:43
#
#-------------------------------------------------

QT       -= core gui

TARGET = CVUtil
TEMPLATE = lib
CONFIG += staticlib

win32 {
        INCLUDEPATH += ../include ../../ControlloVoliRT_Tools/include
}
macx {
        INCLUDEPATH += ../include
        INCLUDEPATH += ../../SwTools/include
        INCLUDEPATH += /opt/local/include
}

CONFIG(debug, debug|release) {
        TARGET = $$join(TARGET,,,d)
}

SOURCES += \
    cvspatialite.cpp \
    ogrgeomptr.cpp

HEADERS += \
    cvspatialite.h \
    ogrgeomptr.h

DESTDIR = ../lib

incl.path = ../include/CVUtil
incl.files += ../CVUtil/cvspatialite.h
incl.files += ../CVUtil/ogrgeomptr.h
INSTALLS += incl


win32 {
        LIBS += -L"C:/ControlloVoliRT_Tools/lib"
        DEFINES += DLL_EXPORTS
}
macx {
        LIBS += -L"/Users/andrea/SwTools/lib"
}
CONFIG(debug, debug|release) {
        LIBS += -lPocoUtild
        TARGET = $$join(TARGET,,,d)
}
else {
        LIBS += -lPocoUtil
}

INCLUDEPATH += $$PWD/../../RTClient/ControlloVoli/include
DEPENDPATH += $$PWD/../../RTClient/ControlloVoli/include


