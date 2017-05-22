#-------------------------------------------------
#
# Project created by QtCreator 2013-11-03T16:04:43
#
#-------------------------------------------------

QT       -= core gui

TARGET = CVUtil
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$_PRO_FILE_PWD_/../include

win32 {
        INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include
}
macx {
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

DESTDIR = $$_PRO_FILE_PWD_/../lib

incl.path = $$_PRO_FILE_PWD_/../include/CVUtil
incl.files += $$_PRO_FILE_PWD/_../CVUtil/cvspatialite.h
incl.files += $$_PRO_FILE_PWD_/../CVUtil/ogrgeomptr.h
INSTALLS += incl

LIBS += -lPocoFoundation -lspatialite -lsqlite3

