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

CV_INCL = ..\include
CV_TOOLS_INCL = ..\..\ControlloVoliRT_Tools\include

INCLUDEPATH += $${CV_INCL}

win32 {
    INCLUDEPATH += $${CV_TOOLS_INCL}
}

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
    core/cvproject.cpp \
    core/cvjournal.cpp \
    core/cvbaseinputverifier.cpp \
    core/categories/cvstations.cpp \
    core/categories/cvshapelayer.cpp \
    core/categories/cvrinex.cpp \
    core/categories/cvmissionobject.cpp \
    core/categories/cvflyattitude.cpp \
    core/categories/cvfileinput.cpp \
    core/categories/cvcamera.cpp \
    core/sql/querybuilder.cpp \
    gui/details/cvuniondetail.cpp \
    gui/details/cvstationsdetail.cpp \
    gui/details/cvstationdelegate.cpp \
    gui/details/cvrinexdetail.cpp \
    gui/details/cvortodetail.cpp \
    gui/details/cvflydetail.cpp \
    gui/details/cvflyaxis_p.cpp \
    gui/details/cvflyattitudedetail.cpp \
    gui/details/cvdemdetail.cpp \
    gui/details/cvcontourdetail.cpp \
    gui/details/cvbasedetail.cpp \
    gui/details/cvareadetail.cpp

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
    core/cvproject.h \
    core/cvjournal.h \
    core/cvcore_utils.h \
    core/cvbaseinputverifier.h \
    core/categories/cvstations.h \
    core/categories/cvshapelayer.h \
    core/categories/cvrinex.h \
    core/categories/cvmissionobject.h \
    core/categories/cvflyattitude.h \
    core/categories/cvfileinput.h \
    core/categories/cvcontrol.h \
    core/categories/cvcamera.h \
    core/sql/querybuilder.h \
    gui/cvgui_utils.h \
    gui/details/cvuniondetail.h \
    gui/details/cvstationsdetail.h \
    gui/details/cvstationdelegate.h \
    gui/details/cvrinexdetail.h \
    gui/details/cvortodetail.h \
    gui/details/cvflydetail.h \
    gui/details/cvflyaxis_p.h \
    gui/details/cvflyattitudedetail.h \
    gui/details/cvdemdetail.h \
    gui/details/cvcontourdetail.h \
    gui/details/cvbasedetail.h \
    gui/details/cvareadetail.h

RESOURCES += \
    data/data.qrc

OTHER_FILES += \
    data/update.sql \
    data/db.sql

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build/lib/ -lPocoZip -lspatialite -lsqlite3 -lgeos_c -lproj -lfreexl -lz -liconv -lgdal -lPocoFoundation
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build/lib/ -lPocoZipd -lCVUtild -lspatialite -lsqlite3 -lgeos_c -lproj -lfreexl -lz -liconv -lgdal -lPocoFoundationd
else:unix:!macx: LIBS += -L$$PWD/../build/lib/ -lPocoZip -lspatialite -lsqlite3 -lgeos_c -lproj -lfreexl -lz -liconv -lgdal -lPocoFoundation

INCLUDEPATH += $$PWD/../build
DEPENDPATH += $$PWD/../build

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build/lib/ -lCVUtil
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build/lib/ -lCVUtild -lspatialite -lsqlite3 -lgeos_c -lproj -lfreexl -lz -liconv -lgdal -lPocoFoundationd
else:unix:!macx: LIBS += -L$$PWD/../build/lib/ -lCVUtil

INCLUDEPATH += $$PWD/../build
DEPENDPATH += $$PWD/../build

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build/lib/libCVUtil.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build/lib/libCVUtild.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build/lib/CVUtil.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build/lib/CVUtild.lib
else:unix:!macx: PRE_TARGETDEPS += $$PWD/../build/lib/libCVUtil.a
