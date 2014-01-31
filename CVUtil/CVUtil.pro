#-------------------------------------------------
#
# Project created by QtCreator 2013-11-03T16:04:43
#
#-------------------------------------------------

QT       -= core gui

TARGET = CVUtil
TEMPLATE = lib
CONFIG += staticlib

win32 {
        INCLUDEPATH += ../include ../../ControlloVoliRT_Tools/include
}
macx {
        INCLUDEPATH += ../include
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

DESTDIR = ../lib

incl.path = ../include/CVUtil
incl.files += ../CVUtil/cvspatialite.h
incl.files += ../CVUtil/ogrgeomptr.h
INSTALLS += incl



win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../RTClient/ControlloVoli/lib/ -lPocoUtil
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../RTClient/ControlloVoli/lib/ -lPocoUtil
else:unix: LIBS += -L$$PWD/../../RTClient/ControlloVoli/lib/ -lPocoUtil

INCLUDEPATH += $$PWD/../../RTClient/ControlloVoli/include
DEPENDPATH += $$PWD/../../RTClient/ControlloVoli/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../RTClient/ControlloVoli/lib/libPocoUtil.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../RTClient/ControlloVoli/lib/libPocoUtild.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../RTClient/ControlloVoli/lib/PocoUtil.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../RTClient/ControlloVoli/lib/PocoUtild.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../RTClient/ControlloVoli/lib/libPocoUtil.a
