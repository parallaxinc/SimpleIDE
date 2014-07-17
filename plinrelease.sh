#!/bin/sh -x
#
# This script is intended for making production linux SimpleIDE packages
# and uses an install methodology deemed appropriate by Parallax.
#
# If you want to use the old packaging methodology, run linrelease.sh
#
# This packager does not include any dynamic support libraries as it is
# assumed that linux users will be able to find the necessary pieces.
#
# To use this release script:
#
# 1) QtSDK Desktop qmake must be in your PATH
# 2) workspace must exist in spinside folder ala:
#   hg clone https://jsdenson@code.google.com/p/propsideworkspace/ workspace
# 
#
NAME=SimpleIDE
PKG=${NAME}.zip
PROPGCC=/opt/parallax
BUILD=build
SETUPSH="./release/linux/setup.sh"
SIDERSH="./release/linux/simpleide.sh"

#
# we only provide SimpleIDE, PropellerGCC, ctags, qt libs, spin source, and workspace in this packager
#
CTAGS="./ctags-5.8"
LIBAUDIO="/usr/lib/libaudio.so.2"
LIBAUDIO2="/usr/lib/x86_64-linux-gnu/libaudio.so.2"
SPINLIB="./spin"

CLEAN=$1

if [ ! -e ./Workspace ]
then
    echo "SimpleIDE Workspace not found. Add it with this command:"
    echo "hg clone https://code.google.com/p/propsideworkspace/ Workspace"
    exit 1
fi

UARCH=`arch`
UNAME=`uname -n`
if [ `uname -s` = "msys" ]; then
    JOBS=
else
    ISARM=`echo ${UARCH} | grep -i "arm"`
    if [ x${ISARM} != "x" ]; then
        JOBS=-j2
    else
        JOBS=-j6
    fi
fi

#
# remove package
#
if [ -e ${PKG} ]; then
   rm -rf ${PKG}
fi

mkdir -p ${BUILD}

#
# build SimpleIDE for release
#
cp -r ./propside/* ${BUILD}
DIR=`pwd`
cd ${BUILD}
qmake -config ${BUILD}
if test $? != 0; then
   echo "qmake config failed."
   exit 1
fi

if [ x$CLEAN != xnoclean ]; then
    make clean
fi

make ${JOBS}

if test $? != 0; then
   echo "make failed."
   exit 1
fi
cd ${DIR}


# extract version from propside.pro
#
SEDCMD=`sed -n 's/VERSION=.*$/&/p' ${BUILD}/propside.pro | cut -d"=" -f3`
VERSION=`echo ${SEDCMD}`
VERSION=`echo ${VERSION} | sed 's/ /-/g'`
VERSION=`echo ${VERSION} | sed 's/\r//g'`
VERSION="${NAME}-${VERSION}"
echo "Creating Version ${VERSION}"

# create package direcotry
#
rm -rf ${VERSION}
mkdir -p ${VERSION}
if test $? != 0; then
   echo "mkdir ${VERSION} failed."
   exit 1
fi

cp -r ./release/template/* ${VERSION}
ls ${VERSION}

cp -r ${BUILD}/${NAME} ${VERSION}/bin
if test $? != 0; then
   echo "copy ${NAME} failed."
   exit 1
fi

cp -r ${BUILD}/translations ${VERSION}
if test $? != 0; then
   echo "copy translations failed."
   exit 1
fi

MYLDD=`ldd ${BUILD}/${NAME} | grep libQt | awk '{print $3}'`
LIBS=`echo $MYLDD`

cp -f ${LIBS} ${VERSION}/bin
if test $? != 0; then
   echo "copy ${LIBS} failed."
   exit 1
fi

MYLDD=`ldd ${BUILD}/${NAME} | grep libQt | awk '{print $3}'`
LIBS=`echo $MYLDD`

cp -f ${LIBS} ${VERSION}/bin
if test $? != 0; then
   echo "copy ${LIBS} failed."
   exit 1
fi

AULDD=`ldd ${BUILD}/${NAME} | grep libaudio | awk '{print $3}'`
LIBAUDIO=`echo $AULDD`

if [ ${LIBAUDIO}X != X ]; then
   cp -f ${LIBAUDIO} ${VERSION}/bin
   if test $? != 0; then
      if test $? != 0; then
         echo "Can't find libaudio...."
      fi
   fi
fi

#
# Quazip no longer needed
#
#MYLDD=`ldd ${BUILD}/${NAME} | grep quazip | awk '{print $3}'`
#QUAZIP=`echo $MYLDD`
#
#cp ${QUAZIP} ${VERSION}/bin
#if test $? != 0; then
#   echo "copy ${QUAZIP} failed."
#   exit 1
#fi

cd ${CTAGS}
./configure
if test $? != 0; then
   echo "configure ${CTAGS} failed."
   exit 1
fi
make
if test $? != 0; then
   echo "make ${CTAGS} failed."
   exit 1
fi
cd ..

cp -f ${CTAGS}/ctags ${VERSION}/bin
if test $? != 0; then
   echo "copy ${CTAGS}/ctags failed."
   exit 1
ls -alRF ${VERSION}/bin
fi

rm -rf parallax
cp -r ${PROPGCC} ${VERSION}
if test $? != 0; then
   echo "copy ${PROPGCC} failed."
   exit 1
fi

cp -rf ${SPINLIB} ${VERSION}/parallax
if test $? != 0; then
   echo "copy ${SPINLIB} failed."
   exit 1
fi

cp -f ${CTAGS}/ctags ${VERSION}/parallax/bin
if test $? != 0; then
   echo "copy ${CTAGS}/ctags failed."
   exit 1
ls -alRF ${VERSION}/bin
fi

cp ./SimpleIDE-User-Guide.pdf ${VERSION}/parallax/bin
if test $? != 0; then
   echo "copy User Guide failed."
   exit 1
fi

cp -r ./propside-demos/ ${VERSION}/demos
if test $? != 0; then
   echo "copy propside-demos failed."
   exit 1
fi

rm -rf ${VERSION}/parallax/Workspace
cd Workspace
hg pull
if test $? != 0; then
   echo "workspace pull failed."
   exit 1
fi
hg update
if test $? != 0; then
   echo "workspace update failed."
   exit 1
fi
cd ..
cp -r ./Workspace/ ${VERSION}/parallax
if test $? != 0; then
   echo "copy workspace failed."
   exit 1
fi
rm -rf ${VERSION}/parallax/Workspace/.hg
if test $? != 0; then
   echo "remove workspace .hg tracking failed."
   exit 1
fi

cp ${SETUPSH} ${VERSION}
cp ${SIDERSH} ${VERSION}/bin

# pack-up a bzip tarball for distribution
tar -cjf ${VERSION}.${UARCH}.${UNAME}-linux.tar.bz2 ${VERSION}

exit 0
