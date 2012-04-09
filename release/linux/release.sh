#!/bin/sh -x
#
# to use this release script, QtSDK Desktop qmake must be in your PATH
#
NAME=SimpleIDE
PKG=${NAME}.zip
PROPGCC=/opt/parallax
BUILD=build

CTAGS="../../ctags-5.8/ctags"
LIBS="/usr/lib/libQtGui.so.4 /usr/lib/libQtCore.so.4"
LIBAUDIO="/usr/lib/libaudio.so.2"
LIBAUDIO2="/usr/lib/x86_64-linux-gnu/libaudio.so.2"

CLEAN=$1

#
# remove package
#
if [ -e ${PKG} ]; then
   rm -rf ${PKG}
fi

#
# build SimpleIDE for release
#
mkdir -p ${BUILD}
cp -r ../../propside/* ${BUILD}
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

if [ `uname -s` = "msys" ]; then
    make
else
    make -j4
fi

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

cp -r ../template/* ${VERSION}
ls ${VERSION}

cp -r ${BUILD}/${NAME} ${VERSION}/bin
if test $? != 0; then
   echo "copy ${NAME} failed."
   exit 1
fi
cp -f ${LIBS} ${VERSION}/bin
if test $? != 0; then
   echo "copy ${LIBS} failed."
   exit 1
fi
cp -f ${LIBAUDIO} ${VERSION}/bin
if test $? != 0; then
   cp -f ${LIBAUDIO2} ${VERSION}/bin
   if test $? != 0; then
      echo "Can't find libaudio...."
   fi
fi
cp -f ${CTAGS} ${VERSION}/bin
if test $? != 0; then
   echo "copy ${CTAGS} failed."
   exit 1
ls -alRF ${VERSION}/bin
fi

rm -rf parallax
#cp -r ${PROPGCC} ${VERSION}
#if test $? != 0; then
#   echo "copy ${PROPGCC} failed."
#   exit 1
#fi

cp -r ../../propside-demos/ ${VERSION}/demos
if test $? != 0; then
   echo "copy propside-demos failed."
   exit 1
fi

# pack-up a bzip tarball for distribution
UARCH=`arch`
UNAME=`uname -n`
tar -cjf ${VERSION}.${UARCH}.${UNAME}-linux.tar.bz2 ${VERSION}

exit 0
