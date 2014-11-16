#!/bin/sh -x
#
# To use this release script,
#
# Using sudo ./macrelease.sh is strongly recommended.
# Qt qmake for the target Qt version must be in your path.
#
NAME=SimpleIDE
APP=${NAME}.app
PKG=${NAME}.zip
PROPGCC=/opt/parallax
APPGCC=${NAME}.app/Contents/propeller-gcc
APPWRK=${NAME}.app/Contents/Workspace

if [ -e ${PKG} ]; then
   rm -rf ${PKG}
fi

QMAKE=`which qmake`
QT5QMAKE=`echo ${QMAKE} | grep -i 'Qt5'`
grep -i 'Qt5' ${QMAKE}
if test $? != 0 ; then
	echo "The qmake program may not be a Qt5 vintage."
	echo "Please adjust PATH to include a Qt5 build if necessary."
	sleep 5
fi

if [ ! -e ${PROPGCC}/bin/openspin ]
then
    echo "Propeller GCC bin/openspin not found. Openspin is in github:"
    echo "git clone https://github.com/reltham/OpenSpin.git openspin"
    echo "cd openspin;make;cp openspin /opt/parallax/bin"
    exit 1
fi

if [ ! -e ./Workspace ]
then
    echo "SimpleIDE Workspace not found. Add it with this command:"
    echo "hg clone https://code.google.com/p/propsideworkspace/ Workspace"
    exit 1
fi

#
# update workspace
#
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

#
# useful for script debug
#
if [ 1 == 1 ]; then

#
# Build the IDE first
#
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

fi

rm -rf ${NAME}
mkdir -p ${NAME}
if test $? != 0; then
   echo "mkdir ${NAME} failed."
   exit 1
fi

#######################################################################
# go to ./SimpleIDE folder
#
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

rm -rf ${APPGCC}
mkdir -p ${APPGCC}
cp -r ${PROPGCC}/* ${APPGCC}
if test $? != 0; then
   echo "copy ${PROPGCC} failed."
   exit 1
fi

cp ../SimpleIDE-User-Guide.pdf ${APPGCC}/bin
if test $? != 0; then
   echo "copy User Guide failed."
   exit 1
fi

cp ../boards.txt ${APPGCC}/propeller-load
if test $? != 0; then
   echo "copy boards.txt filter failed."
   exit 1
fi

cp ../ctags-5.8/ctags ${APPGCC}/bin
if test $? != 0; then
   echo "copy ctags failed."
   exit 1
fi

#cp -r ../propside-demos/ demos
#if test $? != 0; then
#   echo "copy propside-demos failed."
#   exit 1
#fi

#
# copy workspace
#
cp -r ../Workspace ${APPWRK}
if test $? != 0; then
   echo "copy Workspace failed."
   exit 1
fi
rm -rf ${APPWRK}/.hg

#
# remove workspace if any from parallax folder
#
rm -rf ${APPGCC}/Workspace

#
# move Workspace
#
#mv Workspace parallax
#if test $? != 0; then
#   echo "mv Workspace failed."
#   exit 1
#fi

mkdir -p license

pwd

cp -r ../propside/IDE_LICENSE.txt license
if test $? != 0; then
   echo "copy propside-demos failed."
   exit 1
fi

cp -r ../icons/24x24-free-application-icons/readme.txt license/aha-soft-readme.txt
if test $? != 0; then
   echo "copy aha-soft license failed."
   exit 1
fi

cp -r ../ctags-5.8/COPYING license/ctags-COPYING.txt
if test $? != 0; then
   echo "copy ctags copying failed."
   exit 1
fi

cp -r ../ctags-5.8/README license/ctags-README.txt
if test $? != 0; then
   echo "copy ctags readme failed."
   exit 1
fi

cp -r ${PROPGCC}/propeller-elf/lib/COPYING* license
if test $? != 0; then
   echo "copy propgcc license failed."
   exit 1
fi

cp -r ${PROPGCC}/propeller-elf/lib/LIB_LICENSE.txt* license
if test $? != 0; then
   echo "copy propgcc lib license failed."
   exit 1
fi

#
# remove license if any from parallax folder
#
rm -rf ${APPGCC}/license

#
# move license
#
mv license ${APPGCC}
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
