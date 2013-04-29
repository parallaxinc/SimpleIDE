#!/bin/sh -x
#
# to use this release script, QtSDK Desktop qmake must be in your PATH
#
NAME=SimpleIDE
APP=${NAME}.app
PKG=${NAME}.zip
PROPGCC=/opt/parallax

if [ -e ${PKG} ]; then
   rm -rf ${PKG}
fi

#if [ 1 == 0 ]; then

mkdir -p release
cp -r propside/* release
cd release
qmake -config release
if test $? != 0; then
   echo "qmake config failed."
   exit 1
fi

rm -rf ${APP}
make clean
make -j 5
if test $? != 0; then
   echo "make failed."
   exit 1
fi

cd ..

#fi

rm -rf ${NAME}
mkdir -p ${NAME}
if test $? != 0; then
   echo "mkdir ${NAME} failed."
   exit 1
fi

cd ${NAME}
rm -rf ${NAME}.app
cp -r ../release/${NAME}.app .
if test $? != 0; then
   echo "copy ${NAME}.app failed."
   exit 1
fi

macdeployqt ${NAME}.app
if test $? != 0; then
   echo "macdeployqt ${NAME}.app failed."
   exit 1
fi

cp ../release/myInfo.plist ${NAME}.app/Contents/Info.plist
if test $? != 0; then
   echo "copy myInfo.plist to ${NAME}.app failed."
   exit 1
fi

rm -rf parallax
cp -r ${PROPGCC} .
if test $? != 0; then
   echo "copy ${PROPGCC} failed."
   exit 1
fi

#cp ../propside/_SimpleIDE_UserGuide_.txt .
#if test $? != 0; then
#   echo "copy User Guide failed."
#   exit 1
#fi

#cp -r ../propside-demos/ demos
#if test $? != 0; then
#   echo "copy propside-demos failed."
#   exit 1
#fi

#
# copy workspace
#
cp -r ../Workspace .
if test $? != 0; then
   echo "copy Workspace failed."
   exit 1
fi
rm -rf Workspace/.hg

#
# remove workspace if any from parallax folder
#
rm -rf parallax/Workspace

#
# move Workspace
#
mv Workspace parallax
if test $? != 0; then
   echo "mv license failed."
   exit 1
fi

mkdir -p license

cp -r ../propside/IDE_LICENSE.txt license
if test $? != 0; then
   echo "copy propside-demos failed."
   exit 1
fi

cp -r ../../propside/icons/24x24-free-application-icons/readme.txt license/aha-soft-readme.txt
if test $? != 0; then
   echo "copy aha-soft license failed."
   exit 1
fi

cp -r ../../propside/ctags-5.8/COPYING license/ctags-COPYING.txt
if test $? != 0; then
   echo "copy ctags copying failed."
   exit 1
fi

cp -r ../../propside/ctags-5.8/README license/ctags-README.txt
if test $? != 0; then
   echo "copy ctags readme failed."
   exit 1
fi

cp -r ../../propgcc/LICENSE.txt license/propgcc-license.txt
if test $? != 0; then
   echo "copy propgcc license failed."
   exit 1
fi

#
# remove license if any from parallax folder
#
rm -rf parallax/license

#
# move license
#
mv license parallax
if test $? != 0; then
   echo "mv license failed."
   exit 1
fi

#
# no zip now. use mack PackageMaker
#

cd ..
#zip ${PKG} -r ${NAME}
#if test $? != 0; then
#   echo "Zip failed."
#   exit 1
#fi

exit 0
