# -------------------------------------------------
# Project originally created by QtCreator 2011-11-19T08:29:49
# -------------------------------------------------

# These define the version number in Menu->About
DEFINES += IDEVERSION=1
DEFINES += MINVERSION=0
DEFINES += FIXVERSION=0
VERSION = IDEVERSION"."MINVERSION"."FIXVERSION

QT += core
QT += gui

greaterThan(QT_MAJOR_VERSION, 4): {
    QT -= gui
    QT += widgets
    QT += printsupport
    DEFINES += QT5
}

TARGET = SimpleIDE
TEMPLATE = app
DEFINES += QEXTSERIALPORT_LIB
DEFINES += SPINSIDE

CONFIG  += exceptions

# no more quazip
INCLUDEPATH += $$[QT_INSTALL_PREFIX]/src/3rdparty/zlib

# Turn off SPIN features for EDU release
# DEFINES += SPIN_AUTOCOMPLETE
DEFINES += SPIN

# Experimental AutoLib feature
DEFINES += ENABLE_AUTOLIB

# IDEDEBUG define sends qDebug() output to a Debug TAB.
# This is most useful with windows as console output is not available there.
# Don't use IDEDEBUG in release! It BREAKS Translations!
# DEFINES += IDEDEBUG

# EVENT_DRIVEN QEXTSERIALPORT is no longer used.
# GDBENABLE is not ready, and will only be used for development testing
# DEFINES += GDBENABLE
# ENABLE_AUTO_ENTER turns on AUTO-INDENT
DEFINES += ENABLE_AUTO_ENTER

SOURCES += mainspin.cpp \
    PortConnectionMonitor.cpp \
    PropellerID.cpp \
    editor.cpp \
    ctags.cpp \
    mainspinwindow.cpp \
    cbuildtree.cpp \
    treemodel.cpp \
    treeitem.cpp \
    projecttree.cpp \
    terminal.cpp \
    termprefs.cpp \
    properties.cpp \
    newproject.cpp \
    PortListener.cpp \
    highlighter.cpp \
    hardware.cpp \
    help.cpp \
    console.cpp \
    asideconfig.cpp \
    asideboard.cpp \
    projectoptions.cpp \
    replacedialog.cpp \
    aboutdialog.cpp \
    loader.cpp \
    qextserialport.cpp \
    qextserialenumerator.cpp \
    buildc.cpp \
    buildspin.cpp \
    build.cpp \
    spinhighlighter.cpp \
    spinparser.cpp \
    gdb.cpp \
    highlightc.cpp \
    hintdialog.cpp \
    blinker.cpp \
    buildstatus.cpp \
    highlightbuild.cpp \
    directory.cpp \
    zip.cpp \
    zipper.cpp \
    StatusDialog.cpp \
    workspacedialog.cpp \
    rescuedialog.cpp
HEADERS += mainspinwindow.h \
    PortConnectionMonitor.h \
    PropellerID.h \
    editor.h \
    ctags.h \
    highlighter.h \
    treemodel.h \
    treeitem.h \
    PortListener.h \
    terminal.h \
    termprefs.h \
    properties.h \
    newproject.h \
    console.h \
    hardware.h \
    help.h \
    asideboard.h \
    asideconfig.h \
    cbuildtree.h \
    projectoptions.h \
    Sleeper.h \
    replacedialog.h \
    aboutdialog.h \
    loader.h \
    projecttree.h \
    qextserialport.h \
    qextserialenumerator.h \
    buildc.h \
    buildspin.h \
    build.h \
    spinhighlighter.h \
    spinparser.h \
    gdb.h \
    highlightc.h \
    propertycolor.h \
    qportcombobox.h \
    hintdialog.h \
    blinker.h \
    buildstatus.h \
    highlightbuild.h \
    directory.h \
    zconf.h \
    zipper.h \
    zipreader.h \
    zipwriter.h \
    zlib.h \
    StatusDialog.h \
    workspacedialog.h \
    rescuedialog.h \
    qtversion.h
FORMS += hardware.ui \
    project.ui \
    TermPrefs.ui \
    hintdialog.ui
RESOURCES += resources.qrc

# linux quazip doesn't need version, but windows does
unix { 
    SOURCES += qextserialport_unix.cpp
    LIBS += -lz
}

# dont use EVENT_DRIVEN for linux to be consistent with MAC.
unix:!macx:SOURCES += qextserialenumerator_unix.cpp
macx { 
    ICON = $${PWD}/SimpleIDE.icns
    
    # dont use EVENT_DRIVEN for mac. must open terminal before load because mac would reset boards otherwise.
    SOURCES += qextserialenumerator_osx.cpp
    LIBS += -framework \
        IOKit \
        -framework \
        CoreFoundation
    LIBS += -lz
}
win32 { 
    RC_FILE = myapp.rc
    
    # don't use EVENT_DRIVEN for windows. causes crashes.
    SOURCES += qextserialport_win.cpp
    SOURCES += qextserialenumerator_win.cpp
    DEFINES += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
    LIBS += -lsetupapi
    LIBS += -L$$PWD -lzlib1
}

OTHER_FILES += \
    images/progress-redorange.gif \
    images/update.png \
    images/helpfunction.css
