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
    -d  DeveloperID     (required parameter)
    -v  version         (required parameter)
EOF
}

VERSION=
RESTART=
DEVELOPERID=

while getopts "h:r:d:v:i" OPTION
do
    case $OPTION in
        h)
            usage
            exit 1
            ;;
        r)
            RESTART=$OPTARG
            ;;
        d)
            DEVELOPERID=$OPTARG
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

#if [[ -z $DEVELOPERID ]] || [[ -z $VERSION ]]
#then
#     usage
#     exit 1
#fi

#VERSION=$1
#SIGNATUREID=$2

#echo restart: ${RESTART}
#echo signatureID: ${DEVELOPERID}
#echo version: ${VERSION}

SIMPLEIDE=SimpleIDE/SimpleIDE.app
FTDIDRIVER=FTDIUSBSerialDriver.kext
SRC_WORKSPACE=propsideworkspace

DIST_SRC=Distribution.xml
DIST_DST=dist.xml

if [ ${2}X != X ]
then
    RESTART=$2
fi

# validate requirements
if [ ${VERSION}X == X ]
then
  echo "Usage: $0 version"
  echo "$0 0.9.66"
  exit 1
fi

# a properly signed app will contain a _CodeSignature directory and CodeResource file
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

# verify that th FTDIUSBDriver.kext is available for copying into the app package
if [ -e ${FTDIDRIVER} ]
then
  echo " Found FTDI chip driver, continuing..."
else
  echo "[Error] FTDI chip driver missing. Please read macsignedpack.sh comments."
  exit 1
fi

# verify that th propsideworkspace is available for copying into the app package
if [ -e ${SRC_WORKSPACE} ]
then
  echo " Found workspace directory, continuing..."
else
  echo "[Error] workspace directory missing. Please read macsignedpack.sh comments."
  exit 1
fi

#exit 1

echo `sed "s/IDENTIFIER/${ID}/g" ${DIST_SRC}` | sed "s/RESTART/${RESTART}/g" > ./${DIST_DST}
cat ./${DIST_DST}

touch *

pkgbuild --root ${FTDIDRIVER} --identifier com.test.Parallax.FTDIUSBSerialDriver.kext --install-location /Library/Extensions/FTDIUSBSerialDriver.kext FTDIUSBSerialDriver.pkg

pkgbuild --root ${SIMPLEIDE} --identifier com.test.Parallax.SimpleIDE --install-location /Applications/SimpleIDE.app SimpleIDE.pkg

productbuild --distribution ./${DIST_DST} --resources ./ --package-path . ./SimpleIDE-${VERSION}-MacOS.pkg

exit 0
