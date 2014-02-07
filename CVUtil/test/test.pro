#-------------------------------------------------
#
# Project created by QtCreator 2013-11-07T13:00:32
#
#-------------------------------------------------

QT       -= core
QT       -= gui

TARGET = test
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app

macx {
    LIBS += -L"../../../SwTools/lib"
    LIBS += -L"../../lib"
    LIBS += -L"/opt/local/lib"
}

LIBS += -lspatialite -lsqlite3 -lgdal

CONFIG(debug, debug|release) {
        LIBS += -lPocoFoundationd  -lPocoUtild -lCVUtild
        TARGET = $$join(TARGET,,,d)
}
else {
        LIBS += -lPocoUtil -lPocoFoundation -lCVUtil
}

INCLUDEPATH += ../../

win32 {
        INCLUDEPATH += C:/ControlloVoliRT_Tools/include
}
macx {
        INCLUDEPATH += ../../../SwTools/include
        INCLUDEPATH += ../../include
        INCLUDEPATH += /opt/local/include
}




SOURCES += main.cpp
