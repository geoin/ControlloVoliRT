QT       -= core gui
TARGET = CVUtil
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$_PRO_FILE_PWD_/../include

win32 {
    OSGEO4_DIR = "D:/OSGeo4W64"
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../ControlloVoliRT_Tools/include
    INCLUDEPATH += $$OSGEO4_DIR/include
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
incl.files += $$_PRO_FILE_PWD_/../CVUtil/cvspatialite.h
incl.files += $$_PRO_FILE_PWD_/../CVUtil/ogrgeomptr.h
INSTALLS += incl


