#!/bin/sh
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

mkdir -p release
cp -r propside/* release
cd release
qmake -config release
if test $? != 0; then
   echo "qmake config failed."
   exit 1
fi

make clean
make
if test $? != 0; then
   echo "make failed."
   exit 1
fi

cd ..
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
# Temporarily copy files to package Learn, My\ Projects, etc....
#
cp -r ../propsideworkspace/Learn .
if test $? != 0; then
   echo "copy propsideworspace failed."
   exit 1
fi

cp -r ../propsideworkspace/My\ Projects .
if test $? != 0; then
   echo "copy propsideworspace failed."
   exit 1
fi

cp -r ../propsideworkspace/Propeller\ GCC\ Demos .
if test $? != 0; then
   echo "copy propsideworspace failed."
   exit 1
fi

cp -r ../propside/IDE_LICENSE.txt .
if test $? != 0; then
   echo "copy propside-demos failed."
   exit 1
fi

mkdir -p license
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

cd ..
zip ${PKG} -r ${NAME}
if test $? != 0; then
   echo "Zip failed."
   exit 1
fi

exit 0
