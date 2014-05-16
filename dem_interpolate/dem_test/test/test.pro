#-------------------------------------------------
#
# Project created by QtCreator 2014-03-14T10:52:52
#
#-------------------------------------------------

QT       -= gui core

TARGET = test
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    projector.cpp \
    dem_test.cpp

INCLUDEPATH += /home/geoin/projects/ControlloVoliRT/include /home/geoin/projects/dep/proj-4.8.0/src

 LIBS += -L"/home/geoin/projects/ControlloVoliRT/lib"

CONFIG(debug, debug|release) {
        LIBS += -lPocoUtil -lPocoFoundation -ldem_interpolate -lproj
        TARGET = $$join(TARGET,,,d)
}
else {
        LIBS += -lPocoUtil -lPocoFoundation -ldem_interpolate -lproj
}
