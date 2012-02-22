#-------------------------------------------------
#
# Project created by QtCreator 2011-11-19T08:29:49
#
#-------------------------------------------------

#
# add spin compiler
# make builder handle complicated projects
# hook terminal to loader
#

QT       += core gui

TARGET = SimpleIDE
TEMPLATE = app

DEFINES += QEXTSERIALPORT_LIB
DEFINES += IDEVERSION=0
DEFINES += MINVERSION=4
DEFINES += FIXVERSION=0

SOURCES += main.cpp\
    mainwindow.cpp \
    treemodel.cpp \
    treeitem.cpp \
    terminal.cpp \
    qextserialport.cpp \
    properties.cpp \
    PortListener.cpp \
    highlighter.cpp \
    hardware.cpp \
    console.cpp \
    asideconfig.cpp \
    asideboard.cpp \
    cbuildtree.cpp \
    finddialog.cpp \
    projectoptions.cpp

HEADERS  += mainwindow.h \
    highlighter.h \
    treemodel.h \
    treeitem.h \
    qextserialport.h \
    qextserialenumerator.h \
    qextserialport_global.h \
    PortListener.h \
    terminal.h \
    properties.h \
    console.h \
    hardware.h \
    asideboard.h \
    asideconfig.h \
    cbuildtree.h \
    finddialog.h \
    projectoptions.h \
    Sleeper.h

FORMS    += \
    hardware.ui \
    project.ui \
    TermPrefs.ui

RESOURCES += \
    resources.qrc

unix:SOURCES       += posix_qextserialport.cpp
unix:!macx:SOURCES += qextserialenumerator_unix.cpp
macx {
  SOURCES          += qextserialenumerator_osx.cpp
  LIBS             += -framework IOKit -framework CoreFoundation
}
win32 {
  SOURCES          += win_qextserialport.cpp qextserialenumerator_win.cpp
  DEFINES          += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
  LIBS             += -lsetupapi
}
