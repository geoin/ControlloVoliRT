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

SOURCES += main.cpp \
    gui/cvmainwindow.cpp \
    gui/cvmenubar.cpp \
    gui/cvtoolbar.cpp \
    gui/cvtreewidget.cpp \
    gui/cvappcontainer.cpp \
    gui/cvtreenode.cpp \
    gui/cvtreenodedelegate.cpp \
    gui/cvnodedetails.cpp

HEADERS  += \
    gui/cvmainwindow.h \
    gui/cvmenubar.h \
    gui/cvtoolbar.h \
    gui/cvtreewidget.h \
    gui/cvappcontainer.h \
    gui/cvtreenode.h \
    gui/cvtreenodedelegate.h \
    gui/cvnodedetails.h
