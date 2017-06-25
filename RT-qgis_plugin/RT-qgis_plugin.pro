#base directory of the qgis installation


TEMPLATE = lib
QT += xml gui core

#unix:LIBS += -L/$$QGIS_DIR/lib -lqgis_core -lqgis_gui

win32 {
    OSGEO4W_DIR = "D:/OSGeo4W64"
    QGIS_DIR = $$OSGEO4W_DIR"/apps/qgis-ltr-dev"
    #QGIS_DIR = $$OSGEO4W_DIR"/apps/qgis-rel-dev"


    LIBS += $$QGIS_DIR/lib/qgis_core.lib -L$$OSGEO4W_DIR/lib
    INCLUDEPATH += \
    $$QGIS_DIR/include \
    $$OSGEO4W_DIR/include

    DEFINES += WIN32
    DESTDIR = $$_PRO_FILE_PWD_/../bin
}

unix {
    LIBS += -L../lib
    INCLUDEPATH +=          \
    /usr/include/gdal   \
    /usr/include/qgis

    DESTDIR = ../bin
}

DEFINES += GUI_EXPORT=""
DEFINES += CORE_EXPORT=""

SOURCES = RT-qgis_plugin.cpp
HEADERS = RT-qgis_plugin.h

CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
}


