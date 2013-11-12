#base directory of the qgis installation
QGIS_DIR = "C:/OSGeo4W/apps/qgis"
COMP_DIR = "C:/ControlloVoliRT_Tools"
OSGEO4W_DIR = "C:/OSGeo4W"

TEMPLATE = lib
QT += xml
#unix:LIBS += -L/$$QGIS_DIR/lib -lqgis_core -lqgis_gui
LIBS += $$QGIS_DIR/lib/qgis_core.lib -L$$COMP_DIR/lib

INCLUDEPATH += \
$$QGIS_DIR/include \
$$COMP_DIR/include \
$$OSGEO4W_DIR/include

SOURCES = RT-qgis_plugin.cpp
HEADERS = RT-qgis_plugin.h
DESTDIR = $$QGIS_DIR/plugins
DEFINES += GUI_EXPORT= CORE_EXPORT= WIN32
