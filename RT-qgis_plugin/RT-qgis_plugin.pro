#base directory of the qgis installation


TEMPLATE = lib
QT += xml gui core

#unix:LIBS += -L/$$QGIS_DIR/lib -lqgis_core -lqgis_gui

win32 {
    QGIS_DIR = "C:/OSGeo4W64/apps/qgis-rel-dev"
    OSGEO4W_DIR = "C:/OSGeo4W64"

    LIBS += $$QGIS_DIR/lib/qgis_core.lib -L$$OSGEO4W_DIR/lib
    INCLUDEPATH += \
    $$QGIS_DIR/include \
    $$OSGEO4W_DIR/include

    DEFINES += WIN32
    DESTDIR = ../bin64
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


