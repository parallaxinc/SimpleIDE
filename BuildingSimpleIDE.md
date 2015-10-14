# Introduction #

SimpleIDE is based on the Qt (freeware license) platform.

# Details #

The [QtCreator](http://qt.nokia.com/products/developer-tools) can be used for building SimpleIDE. It can also be built with qmake and make.

SimpleIDE is compiled with commonly available GNU tools on Mac (XCode command line option), Linux (GCC development), and Windows (MinGW). SimpleIDE will compile with the Qt 4.7.0 or 4.8.0 libraries (5.0 has not been tested).

I'll add more detailed step-by-step information here when possible.

Do not download the entire QtSDK at the top of the download page.
It is a waste of time and Windows version with VS2008/2010 will not build this code base.

Recommend one or more of the following QtCreator downloads:

  * [Windows-QtCreator](http://get.qt.nokia.com/qtcreator/qt-creator-win-opensource-2.5.0.exe)
  * [Linux-i686-QtCreator](http://get.qt.nokia.com/qtcreator/qt-creator-linux-x86-opensource-2.5.0.bin)
  * [Linux-x86\_64-QtCreator](http://get.qt.nokia.com/qtcreator/qt-creator-linux-x86_64-opensource-2.5.0.bin)
  * [MacOSX-QtCreator](http://get.qt.nokia.com/qtcreator/qt-creator-mac-opensource-2.5.0.dmg)

Download one of these library packages:

  * [Windows-MinGW](http://releases.qt-project.org/qt4/source/qt-win-opensource-4.8.1-mingw.exe)
  * [Linux](http://releases.qt-project.org/qt4/source/qt-everywhere-opensource-src-4.8.1.tar.gz)
  * [MacOSX](http://releases.qt-project.org/qt4/source/qt-mac-opensource-4.8.1.dmg)

The above items can be found here: [Qt-Downloads](http://qt-project.org/downloads)

The SimpleIDE project is accessible using mercurial. You can clone the source from the propside source page. If you are a contributor, make sure to sign in before pulling a new clone.

The project file to open with QtCreator is propside/propside.pro

Once you have something installed, you will need to select the Qt Version from the Project Properties. You should also make separate build directories for the project.

For linux/mac release the qmake program is required.

The Windows distribution package is compiled with Inno IDE project file propside.iss . Some variables will need to be changed for your environment.