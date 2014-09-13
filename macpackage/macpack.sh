#!/bin/sh
#
# The purpose of this script is to create a Mac deployment package.
# The package will be created as ./SimpleIDE.pkg
#
# Requirements for this script are:
#
#   User must specify the package version name. I.E. 0.9.66
#
#   These files/folders must be copied to this script's folder:
#      FTDIUSBSerialDriver.kext
#      SimpleIDE.app
#   The FTDIUSBSerialDriver.kext should be taken from the FTDIChip.com Mac Package. I.E.
#      http://www.ftdichip.com/Drivers/VCP/MacOSX/FTDIUSBSerialDriver_v2_2_18.dmg
#   The SimpleIDE release must be previously built with the ./macrelease.sh script.
#

VERSION=$1
ID=com.test.Parallax
RESTART=RequireRestart
SIMPLEIDE=./SimpleIDE.app
FTDIDRIVER=./FTDIUSBSerialDriver.kext

DIST_SRC=Distribution.xml
DIST_DST=dist.xml

if [ ${2}X != X ]
then
    RESTART=$2
fi

if [ ${VERSION}X == X ]
then
  echo "Usage: $0 version"
  echo "$0 0.9.66"
  exit 1
fi

if [ -e ${SIMPLEIDE} ]
then
  echo "Found SimpleIDE app"
else
  echo "SimpleIDE app missing. Please read macpack.sh comments."
  exit 1
fi

if [ -e ${FTDIDRIVER} ]
then
  echo "Found FTDI chip driver"
else
  echo "FTDI chip driver missing. Please read macpack.sh comments."
  exit 1
fi

echo `sed "s/IDENTIFIER/${ID}/g" ${DIST_SRC}` | sed "s/RESTART/${RESTART}/g" > ./${DIST_DST}
cat ./${DIST_DST}

touch *

pkgbuild --root ./FTDIUSBSerialDriver.kext/ --identifier com.test.Parallax.FTDIUSBSerialDriver.kext --install-location /Library/Extensions/FTDIUSBSerialDriver.kext FTDIUSBSerialDriver.pkg

pkgbuild --root ./SimpleIDE.app --identifier com.test.Parallax.SimpleIDE --install-location /Applications/SimpleIDE.app SimpleIDE.pkg

productbuild --distribution ./${DIST_DST} --resources ./ --package-path . ./SimpleIDE-${VERSION}-MacOS.pkg

exit 0
