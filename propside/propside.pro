# -------------------------------------------------
# Project created by QtCreator 2011-11-19T08:29:49
# -------------------------------------------------
# add spin compiler
# make builder handle complicated projects
# hook terminal to loader
QT += core \
    gui
TARGET = SimpleIDE
TEMPLATE = app
DEFINES += QEXTSERIALPORT_LIB

# EVENT_DRIVEN QEXTSERIALPORT causes zombie procs on windows works on Linux/Mac
# DEFINES += EVENT_DRIVEN
# GDBENABLE is not ready, and will only be used for development testing
# DEFINES += GDBENABLE
# These define the version number in Menu->About
DEFINES += IDEVERSION=0
DEFINES += MINVERSION=6
DEFINES += FIXVERSION=2
SOURCES += main.cpp \
    editor.cpp \
    ctags.cpp \
    mainwindow.cpp \
    treemodel.cpp \
    treeitem.cpp \
    terminal.cpp \
    qextserialport.cpp \
    properties.cpp \
    newproject.cpp \
    PortListener.cpp \
    highlighter.cpp \
    hardware.cpp \
    console.cpp \
    asideconfig.cpp \
    asideboard.cpp \
    cbuildtree.cpp \
    projectoptions.cpp \
    replacedialog.cpp \
    aboutdialog.cpp \
    gdb.cpp \
    loader.cpp \
    projecttree.cpp
HEADERS += mainwindow.h \
    editor.h \
    ctags.h \
    highlighter.h \
    treemodel.h \
    treeitem.h \
    qextserialport.h \
    qextserialenumerator.h \
    qextserialport_global.h \
    PortListener.h \
    terminal.h \
    properties.h \
    newproject.h \
    console.h \
    hardware.h \
    asideboard.h \
    asideconfig.h \
    cbuildtree.h \
    projectoptions.h \
    Sleeper.h \
    replacedialog.h \
    aboutdialog.h \
    gdb.h \
    loader.h \
    projecttree.h
FORMS += hardware.ui \
    project.ui \
    TermPrefs.ui
RESOURCES += resources.qrc
unix { 
    DEFINES += EVENT_DRIVEN
    SOURCES += posix_qextserialport.cpp
}
unix:!macx:SOURCES += qextserialenumerator_unix.cpp
macx { 
    SOURCES += qextserialenumerator_osx.cpp
    LIBS += -framework \
        IOKit \
        -framework \
        CoreFoundation
}
win32 { 
    # DEFINES += LOADER_TERMINAL
    RC_FILE = myapp.rc
    SOURCES += win_qextserialport.cpp \
        qextserialenumerator_win.cpp
    DEFINES += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
    LIBS += -lsetupapi
}
