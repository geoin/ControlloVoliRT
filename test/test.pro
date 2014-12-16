TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = $$_PRO_FILE_PWD_/bin

SOURCES += main.cpp \
    dem_interpolate.cpp \
    cvutil.cpp

INCLUDEPATH += $$_PRO_FILE_PWD_/../include

LIBS += -L$$_PRO_FILE_PWD_/libs -L$$_PRO_FILE_PWD_/../lib
LIBS += -lgtest -lgtest_main -ldem_interpolate

unix {
    LIBS += -lpthread -lCVUtil -lPocoFoundation -lgdal -lspatialite -lsqlite3
}
