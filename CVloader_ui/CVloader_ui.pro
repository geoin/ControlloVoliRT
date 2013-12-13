#base directory of the qgis installation
QGIS_DIR = "C:/OSGeo4W/apps/qgis"
COMP_DIR = "C:/ControlloVoliRT_Tools"
OSGEO4W_DIR = "C:/OSGeo4W"

HEADERS = droparea.h \
          cvloader.h \
          dropwindow.h
SOURCES = droparea.cpp \
          cvloader.cpp \
          dropwindow.cpp \
          CVloader_ui.cpp

win32 {
        QMAKE_CXXFLAGS -= -Zc:wchar_t-
        LIBS += -L"C:/ControlloVoliRT_Tools/lib" -L"C:/ControlloVoliRT/lib"
}
macx {
        LIBS += -L"/Users/andrea/SwTools/lib" -L"/Users/andrea/ControlloVoliRT/lib"
}

CONFIG(debug, debug|release) {
        LIBS += -lspatialite4 -lsqlite3_i -lCVutild -lgdald_i
        TARGET = $$join(TARGET,,,d)
}
else {
        LIBS += -lspatialite4 -lsqlite3_i -lCVutil -lgdal_i
}

win32 {
        INCLUDEPATH += C:/ControlloVoliRT_Tools/include C:/ControlloVoliRT/include
}
macx {
        INCLUDEPATH += /Users/andrea/SwTools/include /Users/andrea/ControlloVoliRT/include
}

DESTDIR = $$QGIS_DIR/plugins
