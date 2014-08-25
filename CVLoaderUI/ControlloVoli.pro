#-------------------------------------------------
#
# Project created by QtCreator 2013-12-17T14:27:42
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CVloader
TEMPLATE = app

CV_INCL = ../include
CV_TOOLS_INCL = ../../ControlloVoliRT_Tools/include
OUT_DIR = $$_PRO_FILE_PWD_/../bin

DEFINES += NOMINMAX

INCLUDEPATH += $${CV_INCL}

win32 {
    INCLUDEPATH += $$_PRO_FILE_PWD_/$${CV_TOOLS_INCL}
    LIBS += -L$$_PRO_FILE_PWD_"/../lib" -L$$_PRO_FILE_PWD_"/../../ControlloVoliRT_Tools/lib"
    CONFIG(debug, debug|release) {
         LIBS += -lCVUtild -lPocoZipd -lspatialite -lPocoFoundationd -lsqlite3_i
         TARGET = $$join(TARGET,,,d)
    } else {
         LIBS += -lCVUtil -lPocoZip -lspatialite -lPocoFoundation -lsqlite3_i
    }
}

unix {
    LIBS += -L$$_PRO_FILE_PWD_/../lib
    LIBS += -lCVUtil -lPocoZip -lspatialite -lPocoFoundation -lsqlite3
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
    gui/details/cvareadetail.cpp \
    core/categories/cvcontrol.cpp \
    gui/details/cvlidarplandetail.cpp \
    gui/details/cvsensordetail.cpp \
    core/categories/cvsensor.cpp \
    gui/dialogs/cvreferenceviewer.cpp \
    gui/details/cvlidarflydetail.cpp \
    gui/details/cvcontrolpointsdetail.cpp \
    gui/details/cvcloudsampledetail.cpp \
    core/categories/cvfolderinput.cpp \
    gui/details/cvlidarrawdetail.cpp \
    gui/details/cvrawstripfolderdetail.cpp

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
    gui/details/cvareadetail.h \
    gui/details/cvlidarplandetail.h \
    gui/details/cvsensordetail.h \
    core/categories/cvsensor.h \
    gui/dialogs/cvreferenceviewer.h \
    gui/details/cvlidarflydetail.h \
    gui/details/cvcontrolpointsdetail.h \
    gui/details/cvcloudsampledetail.h \
    core/categories/cvfolderinput.h \
    gui/details/cvlidarrawdetail.h \
    gui/details/cvrawstripfolderdetail.h

RESOURCES += \
    data/data.qrc

OTHER_FILES += \
    data/update.sql \
    data/db.sql

DESTDIR = $${OUT_DIR}
