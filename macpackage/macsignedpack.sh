#!/bin/sh --
#
# The purpose of this script is to create a signed SimpleIDE Mac OS X deployment package
# The package will be created as ./SimpleIDE-|VERSION|-MacOS.pkg
#
# Requirements for this script are:
#
#   User must specify the package version: (example: "-v 0.9.66")
#   All other parameters are optional: (restart requirement, developer identity, is this a deployment package)
#
#   These files/folders must be copied to this script's directory:
#      FTDIUSBSerialDriver.kext
#      SimpleIDE.app
#   The FTDIUSBSerialDriver.kext should be downloaded from: http://www.ftdichip.com/Drivers/VCP.htm
#      - select the currently supported Mac OS X VCP driver from that page (i.e. x64 (64-bit))
#      - or use: http://www.ftdichip.com/Drivers/VCP/MacOSX/FTDIUSBSerialDriver_v2_2_18.dmg
#      - install FTDI's driver package onto the development Mac OS X system
#      - copy the FTDIUSBSerialDriver.kext from /System/Library/Extensions/ to the script's directory
#   The SimpleIDE release must be:
#       - previously built with ./macrelease.sh script
#       - modified with ./macdeployqt_fix_frameworks.sh, to add Qt framework Info.plist files
#       - application-signed with ./macappcodesign.sh, to sign the app with an "identified developer" ID and checksum
#       - copied from the /propside/SimepleIDE/ directory to the script's directory
#

usage()
{
cat << EOF
usage: $0 options

This script builds a signed SimpleIDE installation package.

OPTIONS:
    -h  show usage
    -r  require restart after installation (applies only if FTDIUSBSerialDriver is included)
    -f  include FTDIUSBSerialDriver in the package
    -s  developer identity certificate key
        - example: -s "Developer Identity" (default is "Developer ID Installer")
    -v  version
        - example: -v 0.9.66 (required parameter)
    -d  use deployment identifier (default is: com.test.Parallax, deploy is: com.Parallax)

    example: ./macsignedpack.sh -r -f -s "Developer ID Installer" -v 0.9.66 -d

EOF
}

#
# Default installation locations
#
# note: the FTDI kext could be moved to "/Library/Extensions/" in accord with Apple suggestion @ some future point
FTDIDRIVER_DEST_DIR="/System/Library/Extensions/"
DEFAULT_APP_DIR="/Applications/"

#
# Default component names
#
SIMPLEIDE=SimpleIDE
SIMPLEIDE_APP=${SIMPLEIDE}.app

FTDIDRIVER=FTDIUSBSerialDriver
FTDIDRIVER_KEXT=${FTDIDRIVER}.kext

#
# Mdified temporary distro xml
#
# note: will contain copied or sed-modified version of template DistributionXXXX.xml
DIST_DST=DistributionMOD.xml

#
# initialize input options with default values
#
VERSION=
IDENTITY="Developer ID Installer"
REQUIRE_RESTART_TEXT="requireRestart"
RESTART=false
DEPLOY=false
FTDI=false

#
# get parms as flags or as requiring arguments
#
while getopts "hrfs:dv:" OPTION
do
    case $OPTION in
        h)
            usage; exit 1 ;;
        r)
            if [[ $OPTARG =~ ^[0-9]+$ ]]
            then
                RESTART=$OPTARG
            elif [[ $OPTARG =~ ^-, ]]
            then
                RESTART=true
                let OPTIND=$OPTIND-1
            else
                RESTART=true
            fi
            ;;
        f)
            if [[ $OPTARG =~ ^[0-9]+$ ]]
            then
                FTDI=$OPTARG
            elif [[ $OPTARG =~ ^-, ]]
            then
                FTDI=true
                let OPTIND=$OPTIND-1
            else
                FTDI=true
            fi
            ;;
        s)
            IDENTITY=$OPTARG
            echo "overriding Identity default with: \"${IDENTITY}\""
            ;;
        d)
            if [[ $OPTARG =~ ^[0-9]+$ ]]
            then
                DEPLOY=$OPTARG
            elif [[ $OPTARG =~ ^-, ]]
            then
                DEPLOY=true
                let OPTIND=$OPTIND-1
            else
                DEPLOY=true
            fi
            ;;
        v)
            VERSION=$OPTARG ;;
        ?)
            usage; exit  ;;
    esac
done

if [[ -z $VERSION ]]
then
     usage
     exit 1
fi

#
# Validate requirements
#
if [ ${VERSION}X == X ]
then
    echo "Usage: $0 version"
    echo "$0 0.9.66"
    exit 1
else
    echo "SimpleIDE code-signed package build..."
fi

#
# Verify that SimpleIDE app is code-signed
# A properly signed app will contain a _CodeSignature directory and CodeResource file
#
echo "Validating SimpleIDE app..."
if [[ -e ${SIMPLEIDE_APP}/Contents/_CodeSignature/CodeResources ]]
then
    echo " found signed SimpleIDE.app"
#
# How this works:
# A single "-v" == "verify app signing", gives no result on valid app signing
#
    codesign -v ${SIMPLEIDE_APP}
    if [ "$?" != "0" ]; then
        echo " [Error] app sign validation failed!" 1>&2
        exit 1
    else
        echo " verified ${SIMPLEIDE_APP} signature"
    fi
else
    echo " [Error] _CodeSignature/CodeResources missing from app. Please read macsignedpack.sh comments"
    exit 1
fi

#
# Use security utility to determine if the developer installation identity is valid
#
echo "Validating developer identity certificate..."
SECUREID=`security find-certificate -c "$IDENTITY" | grep labl`
if [[ -n ${SECUREID} ]]
then
    echo " found identity: \"${IDENTITY}\""
else
    echo " [Error] Identity: \"${IDENTITY}\" does not exist!"
    echo "         Use Keychain Access app to verify that you are using an authorized developer installation certificate..."
    echo "         i.e. search within Login Keychain 'My Certificates' Category for 'Developer ID Installer' certificate."
    echo
    exit 1
fi
echo

#
# Does the installation require a restart?
#
if [[ $RESTART == true ]]
then
    echo "OPT: Restart required after installation"
else
    echo "OPT: Restart NOT required after installation"
fi

#
# Display version
#
echo "OPT: Package version: ${VERSION}"

#
# Display identity
#
echo "OPT: Developer certificate identity: \"${IDENTITY}\""

#
# Developer PARALLAX_IDENTIFIER & FTDI_IDENTIFIER (package can be for testing or deployment)
#
if [[ $DEPLOY == true ]]
then
    PARALLAX_IDENTIFIER=com.Parallax
#   Will get modified to: "com.Parallax.SimpleIDE" below
    FTDI_IDENTIFIER=com.FTDI.driver
#   Will get modified to: "com.FTDI.driver.FTDIUSBSerialDriver" below
    echo "OPT: Package CFBundleIdentifiers will be set for deployment"
else
    PARALLAX_IDENTIFIER=com.test.Parallax
#   Will get modified to: "com.test.Parallax.SimpleIDE" below
    FTDI_IDENTIFIER=com.test.FTDI.driver
#   Will get modified to: "com.test.FTDI.driver.FTDIUSBSerialDriver" below
    echo "OPT: Package CFBundleIdentifiers will be set for testing"
fi

#
# touch the entire content of the current directory to set most-recent mod dates
#
touch *

#
# Build the FTDIUSBSerialDriver.kext component package
#
# Include FTDI in the Installer package?
if [[ ${FTDI} == true ]]
then
    echo "OPT: FTDI kext packaging requested"
#
#   is the FTDI Driver kext available?
    if [[ -e ${FTDIDRIVER_KEXT} ]]
    then
        echo "     found FTDI USB Serial Driver"
        DIST_SRC=DistributionFTDI.xml
#
#       build the FTDI Driver component package
        echo; echo "Building FTDI USB Driver package..."
        pkgbuild    --root ${FTDIDRIVER_KEXT} \
                    --identifier ${FTDI_IDENTIFIER}.${FTDIDRIVER} \
                    --timestamp \
                    --install-location ${FTDIDRIVER_DEST_DIR}${FTDIDRIVER_KEXT} \
                    --sign "$IDENTITY" \
                    --version ${VERSION} \
                    FTDIUSBSerialDriver.pkg
    else
        echo " [Error] FTDI USB Serial driver missing. Please read macsignedpack.sh comments."
        exit 1
    fi
else
    echo "OPT: FTDI kext WILL NOT be installed by this package!"
    DIST_SRC=DistributionSIDE.xml
fi

#
# Build the SimpleIDE app component package
#
echo; echo "Building SimpleIDE App package..."
pkgbuild --root ${SIMPLEIDE_APP} \
	--identifier ${PARALLAX_IDENTIFIER}.${SIMPLEIDE} \
	--timestamp \
	--install-location ${DEFAULT_APP_DIR}${SIMPLEIDE_APP} \
    --sign "$IDENTITY" \
	--version ${VERSION} \
    SimpleIDE.pkg

#
# Write a synthesized distribution xml directly (NO LONGER USED, BUT CAN PROVIDE A DISTRIBUTION XML FILE AS A TEMPLATE)
#
#productbuild --synthesize --sign "$IDENTITY" --timestamp=none --package SimpleIDE.pkg --package FTDIUSBSerialDriver.pkg ./${DIST_SRC}
#

#
# Modify the existing DistributionXXXX.xml only if requiredRestart is requested
#
if [[ ${FTDI} == true ]]
then
    if [[ ${RESTART} == true ]]
    then
	echo "modifying distribution xml to require restart..."
    sed "s/\"none\"\>FTDI/\"${REQUIRE_RESTART_TEXT}\"\>FTDI/g" ${DIST_SRC} > ./${DIST_DST}
    else
        cat ${DIST_SRC} > ./${DIST_DST}
    fi
else
    cat ${DIST_SRC} > ./${DIST_DST}
fi

#
# Build the SimpleIDE Product Installation Package
#
# note: $DIST_DST holds a copied or modified version of one of the 2 DistributionXXXX.xml files
#       The $DIST_DST contains installation options & links to resources for the product package
echo; echo "Building product package..."
productbuild    --distribution ./${DIST_DST} \
                --resources ./ \
                --timestamp \
                --version $VERSION \
                --package-path ./ \
                --sign "$IDENTITY" \
                ./SimpleIDE-${VERSION}-MacOS.pkg

if [[ -e ${DIST_DST} ]]
then
    echo
    echo "cleaning up temporary files..."   
    rm ${DIST_DST}
fi

echo; echo "done!"
exit 0
