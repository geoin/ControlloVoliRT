#base directory of the qgis installation

TEMPLATE = lib
QT += xml gui core

#unix:LIBS += -L/$$QGIS_DIR/lib -lqgis_core -lqgis_gui

win32 {
    OSGEO4_DIR = "D:/OSGeo4W64"
    QGIS_DIR = $$OSGEO4_DIR"/apps/qgis-ltr-dev"

    LIBS += -L$$OSGEO4W_DIR/lib $$QGIS_DIR/lib/qgis_core.lib
    INCLUDEPATH += \
    $$QGIS_DIR/include \
    $$OSGEO4_DIR/include

    DEFINES += WIN32
}

unix {
    LIBS += -L../lib
    INCLUDEPATH +=          \
    /usr/include/gdal   \
    /usr/include/qgis
}

DEFINES += GUI_EXPORT=""
DEFINES += CORE_EXPORT=""

SOURCES = RT-qgis_plugin.cpp
HEADERS = RT-qgis_plugin.h

CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
}

DESTDIR = $$_PRO_FILE_PWD_/../bin

