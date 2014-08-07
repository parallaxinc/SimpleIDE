#!/bin/sh --
#
# The purpose of this script is to create a signed Mac deployment package.
# The package will be created as ./SimpleIDE.pkg
#
# Requirements for this script are:
#
#   User must specify the package version name. I.E. 0.9.66
#
#   These files/folders must be copied to this script's folder:
#      FTDIUSBSerialDriver.kext
#      SimpleIDE.app
#      propsideworkspace
#   The FTDIUSBSerialDriver.kext should be taken from the FTDIChip.com Mac Package. I.E.
#      http://www.ftdichip.com/Drivers/VCP/MacOSX/FTDIUSBSerialDriver_v2_2_18.dmg
#   The SimpleIDE release must be:
#       - previously built with ./macrelease.sh script
#       - modified with ./macdeployqt_fix_frameworks.sh, to add Qt framework Info.plist files
#       - application-signed wit ./macappcodesign.sh, to sign the app with an "identified developer" ID and checksum
#   The propsideworkspace must be copied from propsideworkspace project checkout, similar to:
#       https://code.google.com/p/propsideworkspace
#

usage()
{
cat << EOF
usage: $0 options

This script builds a signed SimpleIDE installation package.

OPTIONS:
    -h  show usage
    -r  requireRestart
    -s  identity developer certificate key   - example: -s "Developer ID Installer" (default)
    -v  version                              - example: -v 0.9.66 (required parameter)
EOF
}

VERSION=
RESTART=
IDENTITY=

while getopts "h:r:s:v:i" OPTION
do
    case $OPTION in
        h)
            usage
            exit 1
            ;;
        r)
            RESTART=$OPTARG
            ;;
        s)
            IDENTITY=$OPTARG
            ;;
        v)
            VERSION=$OPTARG
            ;;
        ?)
            usage
            exit
            ;;
    esac
done

if [[ -z $IDENTITY ]] || [[ -z $VERSION ]]
then
     usage
     exit 1
fi

SIMPLEIDE=SimpleIDE/SimpleIDE.app
FTDIDRIVER=FTDIUSBSerialDriver.kext
IDENTIFIER=com.test.Parallax

SRC_WORKSPACE=propsideworkspace

DIST_SRC=Distribution.xml
DIST_DST=dist.xml

# validate requirements
if [ ${VERSION}X == X ]
then
  echo "Usage: $0 version"
  echo "$0 0.9.66"
  exit 1
fi

#
# a properly signed app will contain a _CodeSignature directory and CodeResource file
#
echo "Validating parameters..."
if [[ -e ${SIMPLEIDE}/Contents/_CodeSignature/CodeResources ]]
then
  echo " Found signed SimpleIDE app, continuing..."
# a single "-v" == "verify app signing", gives no result on valid app signing
  codesign -v ${SIMPLEIDE}
    if [ "$?" != "0" ]; then
        echo "[Error] app sign validation failed!" 1>&2
        exit 1
    else
        echo " Validated signed ${SIMPLEIDE}, continuing..."
  fi
else
  echo "[Error] signed app missing. Please read macsignedpack.sh comments."
  exit 1
fi

#
# verify that the FTDIUSBDriver.kext is available for copying into the app package
#
if [ -e ${FTDIDRIVER} ]
then
  echo " Found FTDI chip driver, continuing..."
else
  echo "[Error] FTDI chip driver missing. Please read macsignedpack.sh comments."
  exit 1
fi

#
# use security utility to determine if the identity is valid
#
SECUREID=`security find-certificate -c "$IDENTITY" | grep labl`
if [[ -n ${SECUREID} ]]
then
    echo "  Identity: \"${IDENTITY}\" found..."
else
    echo "[Error] Identity: \"${IDENTITY}\" does not exist!"
    echo
    usage
    exit 1
fi

touch *

pkgbuild --identifier ${IDENTIFIER}.${FTDIDRIVER} --root ${FTDIDRIVER} --install-location /Library/Extensions/${FTDIDRIVER} --sign "$IDENTITY" FTDIUSBSerialDriver.pkg

pkgbuild --root ${SIMPLEIDE} --identifier ${IDENTIFIER}.SimpleIDE.app --sign "$IDENTITY" --install-location /Applications/SimpleIDE.app SimpleIDE.pkg

productbuild --synthesize --sign "$IDENTITY" --package SimpleIDE.pkg --package FTDIUSBSerialDriver.pkg ./${DIST_SRC}

# modify the created Distribution.xml if requiredRestart is requested (for .kext installation)
#echo ${RESTART}..............
#if [[ ${RESTART} == "requiredRestart"  ]]
#then
#    echo no request for "requiredRestart"
#    productbuild --distribution ${DIST_SRC} --sign "$IDENTITY" --package-path . ./SimpleIDE-${VERSION}-MacOS.pkg
#else
#    echo `sed "s/\"none\"\>FTDI/\”${RESTART}\"\>FTDI\"/g" ${DIST_SRC}` > ./${DIST_DST}
#    cat ./${DIST_DST}
#    productbuild --distribution ${DIST_DST} --sign "$IDENTITY" --package-path . ./SimpleIDE-${VERSION}-MacOS.pkg
#fi

############## ORIGINAL sed ##################################################################
#echo `sed "s/IDENTIFIER/${ID}/g" ${DIST_SRC}` | sed "s/\"none\"/${RESTART}/g" > ./${DIST_DST}
#cat ./${DIST_DST}
##############################################################################################

productbuild --distribution ./${DIST_SRC} --sign "$IDENTITY" --package-path . ./SimpleIDE-${VERSION}-MacOS.pkg

exit 0
