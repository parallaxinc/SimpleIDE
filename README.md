# propside
SimpleIDE for Propeller

This is the SimpleIDE code repository.

Included here are:
  1. SimpleIDE GUI sources, images, and translation files
  2. Ctags source with Spin language support
  3. SimpleIDE user guide
  4. Command line build scripts for Mac and Linux
  5. InnoIDE packaging scripts for Windows
  6. propside.pro for building with Qt Creator
  
Items required but not included here:
  1. propeller-gcc compiler source https://github.com/parallaxinc/propgcc
  2. Open Spin compiler source https://github.com/parallaxinc/OpenSpin
  3. proploader source https://github.com/parallaxinc/proploader
  4. Learn workspace https://github.com/parallaxinc/Simple-Libraries
  5. Qt5.4 libraries http://download.qt.io/archive/qt/
  6. QtCreator community edition https://www.qt.io/download-open-source/#section-2
  7. OpenGL xorg-x11-server-devel
  8. libz
  
Windows Build:

Mac Build:

  1. ./macrelease.sh
  2. Copy SimpleIDE.app to macpackage (result of ./macrelease.sh)
  3. Copy FTDIUSBSerialDriver.kext to macpackage (get from ftdichip.com website)
  4. cd macpackage
  5. ./macpack.sh <version> (should match version in propside/propside.pro)
  6. Test install with macpackage SimpleIDE-<version>-MacOS.pkg
  7. Parallax must sign for app store distribution.

Linux Build:

More to come ....
