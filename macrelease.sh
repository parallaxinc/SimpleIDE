#!/bin/sh
#
# to use this release script, QtSDK Desktop qmake must be in your PATH
#
DMG=SimpleIDE.dmg
PROPGCC=/opt/parallax

if [ -e ${DMG} ]; then
   rm ${DMG}
fi

cd propside
qmake -config release
if test $? != 0; then
   echo "qmake config failed."
   exit 1
fi

#make clean
make
if test $? != 0; then
   echo "make failed."
   exit 1
fi

cd ..
mkdir -p SimpleIDE
if test $? != 0; then
   echo "mkdir SimpleIDE failed."
   exit 1
fi

cd SimpleIDE
rm -rf SimpleIDE.app
cp -r ../propside/SimpleIDE.app .
if test $? != 0; then
   echo "copy SimpleIDE.app failed."
   exit 1
fi

macdeployqt SimpleIDE.app
if test $? != 0; then
   echo "macdeployqt SimpleIDE.app failed."
   exit 1
fi

rm -rf parallax
cp -r ${PROPGCC} .
if test $? != 0; then
   echo "copy ${PROPGCC} failed."
   exit 1
fi

cp -r ../propside-demos/ demos
if test $? != 0; then
   echo "copy propside-demos failed."
   exit 1
fi

echo "now run hdiutil to make a .dmg"
echo ""
echo "hdiutil create -format UDBZ -srcfolder SimpleIDE SimpleIDE"
echo ""
exit 0
