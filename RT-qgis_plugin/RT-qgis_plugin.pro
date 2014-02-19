#base directory of the qgis installation


TEMPLATE = lib
QT += xml gui core

#unix:LIBS += -L/$$QGIS_DIR/lib -lqgis_core -lqgis_gui

win32 {
    QGIS_DIR = "C:/OSGeo4W/apps/qgis"
    COMP_DIR = "C:/ControlloVoliRT_Tools"
    OSGEO4W_DIR = "C:/OSGeo4W"

    LIBS += $$QGIS_DIR/lib/qgis_core.lib -L$$COMP_DIR/lib
    INCLUDEPATH += \
    $$QGIS_DIR/include \
    $$COMP_DIR/include \
    $$OSGEO4W_DIR/include

    DEFINES += WIN32
}
unix {
INCLUDEPATH +=          \
    /usr/include/gdal   \
    /usr/include/qgis

    QGIS_DIR = /usr/lib/qgis
}

DEFINES += GUI_EXPORT=""
DEFINES += CORE_EXPORT=""

SOURCES = RT-qgis_plugin.cpp
HEADERS = RT-qgis_plugin.h

DESTDIR = ../lib
