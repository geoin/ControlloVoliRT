#-------------------------------------------------
#
# Project created by QtCreator 2013-12-17T14:27:42
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++11

TARGET = ControlloVoli
TEMPLATE = app

CV_INCL = D:\projects\RT\tools\ControlloVoliRT\include
CV_TOOLS_INCL = D:\projects\RT\tools\ControlloVoliRT_Tools\include

INCLUDEPATH += $${CV_INCL} $${CV_TOOLS_INCL}

SOURCES += main.cpp \
    gui/cvmainwindow.cpp \
    gui/cvmenubar.cpp \
    gui/cvtoolbar.cpp \
    gui/cvtreewidget.cpp \
    gui/cvappcontainer.cpp \
    gui/cvtreenode.cpp \
    gui/cvtreenodedelegate.cpp \
    gui/cvnodedetails.cpp \
    gui/status/cvnodeinfo.cpp \
    gui/helper/cvdetailsfactory.cpp \
    gui/cvprojectoverviewdetail.cpp \
    gui/cvstatusbar.cpp \
    gui/details/plandetail.cpp \
    gui/details/missionlistdetails.cpp \
    gui/details/cvcameradetail.cpp \
    gui/cvbasetabwidget.cpp \
    gui/details/cvmissiondetail.cpp \
    gui/dialogs/cvmissiondialog.cpp \
    gui/dialogs/cvprojectdialog.cpp \
    gui/helper/cvactionslinker.cpp \
    core/cvprojectmanager.cpp \
    core/cvproject.cpp

HEADERS  += \
    gui/cvmainwindow.h \
    gui/cvmenubar.h \
    gui/cvtoolbar.h \
    gui/cvtreewidget.h \
    gui/cvappcontainer.h \
    gui/cvtreenode.h \
    gui/cvtreenodedelegate.h \
    gui/cvnodedetails.h \
    gui/status/cvnodeinfo.h \
    gui/helper/cvdetailsfactory.h \
    gui/cvprojectoverviewdetail.h \
    gui/cvstatusbar.h \
    gui/details/plandetail.h \
    gui/details/missionlistdetails.h \
    gui/details/cvcameradetail.h \
    gui/cvbasetabwidget.h \
    gui/details/cvmissiondetail.h \
    gui/dialogs/cvmissiondialog.h \
    gui/dialogs/cvprojectdialog.h \
    gui/helper/cvactionslinker.h \
    core/cvprojectmanager.h \
    core/cvproject.h

RESOURCES += \
    data/res.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../ControlloVoliRT/lib/ -lCVUtil
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ControlloVoliRT/lib/ -lCVUtild
else:unix: LIBS += -L$$PWD/../../ControlloVoliRT/lib/ -lCVUtil

INCLUDEPATH += $$PWD/../../ControlloVoliRT/include
DEPENDPATH += $$PWD/../../ControlloVoliRT/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../ControlloVoliRT/lib/libCVUtil.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../ControlloVoliRT/lib/libCVUtild.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../ControlloVoliRT/lib/CVUtil.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../ControlloVoliRT/lib/CVUtild.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../ControlloVoliRT/lib/libCVUtil.a


unix|win32: LIBS += -L$$PWD/lib/ -lgeos -lspatialite -lsqlite3

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/lib/spatialite.lib $$PWD/lib/sqlite3.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/lib/libgeos.a $$PWD/lib/libspatialite.a $$PWD/lib/libsqlite3.a
